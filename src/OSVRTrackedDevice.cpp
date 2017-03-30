/** @file
    @brief OSVR tracked device

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "Logging.h"

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "osvr_device_properties.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"
#include "OSVRDisplay.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Display/DisplayEnumerator.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/PlatformConfig.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>
#include <osvr/RenderKit/DistortionCorrectTextureCoordinate.h>

// Standard includes
#include <algorithm>        // for std::find
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

OSVRTrackedDevice::OSVRTrackedDevice(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log) : context_(context), driverHost_(driver_host), pose_(), deviceClass_(vr::TrackedDeviceClass_HMD)
{
    OSVR_LOG(trace) << "OSVRTrackedDevice::OSVRTrackedDevice() called.";

    OSVR_LOG(debug) << "Client context: " << (&context_);
    settings_ = std::make_unique<Settings>(driver_host->GetSettings(vr::IVRSettings_Version));
    if (driver_log) {
        Logging::instance().setDriverLog(driver_log);
    }
}

OSVRTrackedDevice::~OSVRTrackedDevice()
{
    driverHost_ = nullptr;
}

vr::EVRInitError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    OSVR_LOG(trace) << "OSVRTrackedDevice::Activate() called.";

    objectId_ = object_id;

    // TODO use C++11 <chrono>
    const std::time_t waitTime = settings_->getSetting<int32_t>("serverTimeout", 5);

    // Register tracker callback
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }

    // Ensure context is fully started up
    OSVR_LOG(trace) << "OSVRTrackedDevice::Activate(): Waiting for the context to fully start up...\n";
    std::time_t startTime = std::time(nullptr);
    while (!context_.checkStatus()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Context startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    displayConfig_ = osvr::clientkit::DisplayConfig(context_);

    // Ensure display is fully started up
    OSVR_LOG(trace) << "OSVRTrackedDevice::Activate(): Waiting for the display to fully start up, including receiving initial pose update...\n";
    startTime = std::time(nullptr);
    while (!displayConfig_.checkStartup()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Display startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Verify valid display config
    if ((displayConfig_.getNumViewers() != 1) && (displayConfig_.getViewer(0).getNumEyes() != 2) && (displayConfig_.getViewer(0).getEye(0).getNumSurfaces() == 1) && (displayConfig_.getViewer(0).getEye(1).getNumSurfaces() != 1)) {
        OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Unexpected display parameters!\n";

        if (displayConfig_.getNumViewers() < 1) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least one viewer must exist.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if (displayConfig_.getViewer(0).getNumEyes() < 2) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least two eyes must exist.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if ((displayConfig_.getViewer(0).getEye(0).getNumSurfaces() < 1) || (displayConfig_.getViewer(0).getEye(1).getNumSurfaces() < 1)) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least one surface must exist for each eye.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        }
    }

    // Register tracker callback
    trackerInterface_ = context_.getInterface("/me/head");
    trackerInterface_.registerCallback(&OSVRTrackedDevice::HmdTrackerCallback, this);

    auto configString = context_.getStringParameter("/renderManagerConfig");

    // If the /renderManagerConfig parameter is missing from the configuration
    // file, use an empty dictionary instead. This allows the render manager
    // config to zero out its values.
    if (configString.empty()) {
        OSVR_LOG(info) << "OSVRTrackedDevice::Activate(): Render Manager config is empty, using default values.\n";
        configString = "{}";
    }

    try {
        renderManagerConfig_.parse(configString);
    } catch(const std::exception& e) {
        OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Exception parsing Render Manager config: " << e.what() << "\n";
    }

    configure();
    configureDistortionParameters();

    driverHost_->ProximitySensorState(objectId_, true);

    OSVR_LOG(trace) << "OSVRTrackedDevice::Activate(): Activation complete.\n";
    return vr::VRInitError_None;
}

void OSVRTrackedDevice::Deactivate()
{
    OSVR_LOG(trace) << "OSVRTrackedDevice::Deactivate() called.";

    /// Have to force freeing here
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }
}

void OSVRTrackedDevice::EnterStandby()
{
    // FIXME Implement
}

void* OSVRTrackedDevice::GetComponent(const char* component_name_and_version)
{
    if (!strcasecmp(component_name_and_version, vr::IVRDisplayComponent_Version)) {
        return static_cast<vr::IVRDisplayComponent*>(this);
    }

    // Override this to add a component to a driver
    return nullptr;
}

void OSVRTrackedDevice::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // TODO

    // Log the requests just to see what info clients are looking for
    OSVR_LOG(debug) << "Received debug request [" << request << "] with response buffer size of " << response_buffer_size << "].";

    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
    // return empty string for now
    if (response_buffer_size > 0) {
        response_buffer[0] = '\0';
    }
}

void OSVRTrackedDevice::GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height)
{
    const auto bounds = getWindowBounds(display_, scanoutOrigin_);
    *x = bounds.x;
    *y = bounds.y;
    *width = bounds.width;
    *height = bounds.height;
}

bool OSVRTrackedDevice::IsDisplayOnDesktop()
{
    // If the current display still appears in the active displays list,
    // then it's attached to the desktop.
    const auto displays = osvr::display::getDisplays();
    const auto display_on_desktop = (end(displays) != std::find(begin(displays), end(displays), display_));
    OSVR_LOG(trace) << "OSVRTrackedDevice::IsDisplayOnDesktop(): " << (display_on_desktop ? "yes" : "no");
    return display_on_desktop;
}

bool OSVRTrackedDevice::IsDisplayRealDisplay()
{
    // TODO get this info from display description?
    return true;
}

void OSVRTrackedDevice::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
    const double oversample_factor = renderManagerConfig_.getRenderOversampleFactor();
    const auto bounds = getWindowBounds(display_, scanoutOrigin_);

    *width = static_cast<uint32_t>(bounds.width * oversample_factor);
    *height = static_cast<uint32_t>(bounds.height * oversample_factor);
    OSVR_LOG(trace) << "GetRecommendedRenderTargetSize(): width = " << *width << ", height = " << *height << ".";
}

void OSVRTrackedDevice::GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    const auto display_mode = displayConfiguration_.getDisplayMode();
    const auto viewport = getEyeOutputViewport(eye, display_, scanoutOrigin_, display_mode);

    *x = static_cast<uint32_t>(viewport.x);
    *y = static_cast<uint32_t>(viewport.y);
    *width = viewport.width;
    *height = viewport.height;
}

void OSVRTrackedDevice::GetProjectionRaw(vr::EVREye eye, float* left, float* right, float* top, float* bottom)
{
    // Reference: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionRaw
    // SteamVR expects top and bottom to be swapped!
    osvr::clientkit::ProjectionClippingPlanes pl = displayConfig_.getViewer(0).getEye(eye).getSurface(0).getProjectionClippingPlanes();
    *left = static_cast<float>(pl.left);
    *right = static_cast<float>(pl.right);
    *bottom = static_cast<float>(pl.top); // SWAPPED
    *top = static_cast<float>(pl.bottom); // SWAPPED
}

vr::DistortionCoordinates_t OSVRTrackedDevice::ComputeDistortion(vr::EVREye eye, float u, float v)
{
    // Rotate the texture coordinates to match the display orientation
    const auto orientation = scanoutOrigin_ + display_.rotation;
    const auto desired_orientation = osvr::display::DesktopOrientation::Landscape;
    const auto rotation = desired_orientation - orientation;

    std::tie(u, v) = rotate(u, v, rotation);

    // Note that RenderManager expects the (0, 0) to be the lower-left corner
    // and (1, 1) to be the upper-right corner while SteamVR assumes (0, 0) is
    // upper-left and (1, 1) is lower-right.  To accommodate this, we need to
    // flip the y-coordinate before passing it to RenderManager and flip it
    // again before returning the value to SteamVR.
    using osvr::renderkit::DistortionCorrectTextureCoordinate;
    static const size_t COLOR_RED = 0;
    static const size_t COLOR_GREEN = 1;
    static const size_t COLOR_BLUE = 2;

    const auto osvr_eye = static_cast<size_t>(eye);
    const auto distortion_parameters = distortionParameters_[osvr_eye];
    const auto in_coords = osvr::renderkit::Float2 {{u, 1.0f - v}}; // flip v-coordinate

    const auto interpolators = (vr::Eye_Left == eye) ? &leftEyeInterpolators_ : &rightEyeInterpolators_;

    auto coords_red = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_RED, overfillFactor_, *interpolators);

    auto coords_green = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_GREEN, overfillFactor_, *interpolators);

    auto coords_blue = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_BLUE, overfillFactor_, *interpolators);

    vr::DistortionCoordinates_t coords;
    // flip v-coordinates again
    coords.rfRed[0] = coords_red[0];
    coords.rfRed[1] = 1.0f - coords_red[1];
    coords.rfGreen[0] = coords_green[0];
    coords.rfGreen[1] = 1.0f - coords_green[1];
    coords.rfBlue[0] = coords_blue[0];
    coords.rfBlue[1] = 1.0f - coords_blue[1];

    return coords;
}

vr::DriverPose_t OSVRTrackedDevice::GetPose()
{
    return pose_;
}

bool OSVRTrackedDevice::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const bool default_value = false;

    const auto result = checkProperty(prop, bool());
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    // Prop_ContainsProximitySensor_Bool spams our log files. Ignoring it here.
    OSVR_LOG(properties) << "OSVRTrackedDevice::GetBoolTrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // Properties that apply to all device classes
    case vr::Prop_WillDriftInYaw_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return true;
        break;
    case vr::Prop_DeviceIsWireless_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_DeviceIsCharging_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_Firmware_UpdateAvailable_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_Firmware_ManualUpdate_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_BlockServerShutdown_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    case vr::Prop_ContainsProximitySensor_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return true;
        break;
    case vr::Prop_DeviceProvidesBatteryStatus_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_DeviceCanPowerOff_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return true;
        break;
    case vr::Prop_HasCamera_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    // Properties that apply to HMDs
    case vr::Prop_ReportsTimeSinceVSync_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    case vr::Prop_IsOnDesktop_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return this->IsDisplayOnDesktop();
        break;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetBoolTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

float OSVRTrackedDevice::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const float default_value = 0.0f;

    const auto result = checkProperty(prop, float());
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    OSVR_LOG(properties) << "OSVRTrackedDevice::GetFloatTrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_DeviceBatteryPercentage_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 1.0f; // full battery
    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_SecondsFromVsyncToPhotons_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayFrequency_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return static_cast<float>(display_.verticalRefreshRate);
    case vr::Prop_UserIpdMeters_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return GetIPD();
    case vr::Prop_DisplayMCOffset_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayMCScale_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayGCBlackClamp_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayGCOffset_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayGCScale_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayGCPrescale_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_LensCenterLeftU_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_LensCenterLeftV_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_LensCenterRightU_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_LensCenterRightV_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_UserHeadToEyeDepthMeters_Float:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    // Properties that are unique to TrackedDeviceClass_TrackingReference
    case vr::Prop_FieldOfViewLeftDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewRightDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewTopDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FieldOfViewBottomDegrees_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingRangeMinimumMeters_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingRangeMaximumMeters_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetFloatTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

int32_t OSVRTrackedDevice::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const int32_t default_value = 0;

    const auto result = checkProperty(prop, int32_t());
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    OSVR_LOG(properties) << "OSVRTrackedDevice::GetInt32TrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_DeviceClass_Int32:
        if (error)
            *error = vr::TrackedProp_Success;
        return deviceClass_;
    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_DisplayMCType_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_EdidVendorID_Int32:
        if (error)
            *error = vr::TrackedProp_Success;
        return static_cast<int32_t>(display_.edidVendorId);
    case vr::Prop_EdidProductID_Int32:
        if (error)
            *error = vr::TrackedProp_Success;
        return static_cast<int32_t>(display_.edidProductId);
    case vr::Prop_DisplayGCType_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_CameraCompatibilityMode_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    // Properties that are unique to TrackedDeviceClass_Controller
    case vr::Prop_Axis0Type_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis1Type_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis2Type_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis3Type_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Axis4Type_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetInt32TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint64_t OSVRTrackedDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const uint64_t default_value = 0;

    const auto result = checkProperty(prop, uint64_t());
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    OSVR_LOG(properties) << "OSVRTrackedDevice::GetUint64TrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_HardwareRevision_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FirmwareVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_FPGAVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_VRCVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_RadioVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DongleVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_CurrentUniverseId_Uint64:
        if (error)
            *error = vr::TrackedProp_Success;
        return 1;
    case vr::Prop_PreviousUniverseId_Uint64:
        if (error)
            *error = vr::TrackedProp_Success;
        return 1;
    case vr::Prop_DisplayFirmwareVersion_Uint64:
        /// @todo This really should be read from the server
        if (error)
            *error = vr::TrackedProp_Success;
        return 192;
    case vr::Prop_CameraFirmwareVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayFPGAVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayBootloaderVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayHardwareVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_AudioFirmwareVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    // Properties that are unique to TrackedDeviceClass_Controller
    case vr::Prop_SupportedButtons_Uint64: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetUint64TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

vr::HmdMatrix34_t OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    // Default value is identity matrix
    vr::HmdMatrix34_t default_value;
    map(default_value) = Matrix34f::Identity();

    const auto result = checkProperty(prop, vr::HmdMatrix34_t());
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    OSVR_LOG(properties) << "OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_StatusDisplayTransform_Matrix34:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_CameraToHeadTransform_Matrix34:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint32_t OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* value, uint32_t buffer_size, vr::ETrackedPropertyError *error)
{
    uint32_t default_value = 0;

    const auto result = checkProperty(prop, value);
    if (vr::TrackedProp_Success != result) {
        if (error)
            *error = result;
        return default_value;
    }

    OSVR_LOG(properties) << "OSVRTrackedDevice::GetStringTrackedDeviceProperty(): Requested property: " << prop << "\n";

    std::string sValue = GetStringTrackedDeviceProperty(prop, error);
    if (*error == vr::TrackedProp_Success) {
        if (sValue.size() + 1 > buffer_size) {
            *error = vr::TrackedProp_BufferTooSmall;
        } else {
            valveStrCpy(sValue, value, buffer_size);
        }
        return static_cast<uint32_t>(sValue.size()) + 1;
    }

    return 0;
}

    // ------------------------------------
    // Private Methods
    // ------------------------------------

std::string OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error)
{
    std::string default_value = "";

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_TrackingSystemName_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ModelNumber_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return settings_->getSetting<std::string>("modelNumber", displayConfiguration_.getModel() + " " + displayConfiguration_.getVersion());
    case vr::Prop_SerialNumber_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return settings_->getSetting<std::string>("serialNumber", this->GetId());
    case vr::Prop_RenderModelName_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ManufacturerName_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return settings_->getSetting<std::string>("manufacturer", displayConfiguration_.getVendor());
    case vr::Prop_TrackingFirmwareVersion_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_HardwareRevision_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_AllWirelessDongleDescriptions_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ConnectedWirelessDongle_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Firmware_ManualUpdateURL_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_Firmware_ProgrammingTarget_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DriverVersion_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;

    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_DisplayMCImageLeft_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayMCImageRight_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_DisplayGCImage_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_CameraFirmwareDescription_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;

    // Properties that are unique to TrackedDeviceClass_Controller
    case vr::Prop_AttachedDeviceId_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;

    // Properties that are unique to TrackedDeviceClass_TrackingReference
    case vr::Prop_ModeLabel_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;

    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackedDevice::GetStringTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

void OSVRTrackedDevice::HmdTrackerCallback(void* userdata, const OSVR_TimeValue*, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedDevice*>(userdata);

    vr::DriverPose_t pose;
    pose.poseTimeOffset = 0; // close enough

    Eigen::Vector3d::Map(pose.vecWorldFromDriverTranslation) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecDriverFromHeadTranslation) = Eigen::Vector3d::Zero();

    map(pose.qWorldFromDriverRotation) = Eigen::Quaterniond::Identity();

    map(pose.qDriverFromHeadRotation) = Eigen::Quaterniond::Identity();

    // Position
    Eigen::Vector3d::Map(pose.vecPosition) = osvr::util::vecMap(report->pose.translation);

    // Position velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAcceleration) = Eigen::Vector3d::Zero();

    // Orientation
    map(pose.qRotation) = osvr::util::fromQuat(report->pose.rotation);

    // Angular velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecAngularVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAngularAcceleration) = Eigen::Vector3d::Zero();

    pose.result = vr::TrackingResult_Running_OK;
    pose.poseIsValid = true;
    pose.willDriftInYaw = true;
    pose.shouldApplyHeadModel = true;
    pose.deviceIsConnected = true;

    self->pose_ = pose;
    self->driverHost_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

float OSVRTrackedDevice::GetIPD()
{
    OSVR_Pose3 leftEye, rightEye;

    if (displayConfig_.getViewer(0).getEye(0).getPose(leftEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get left eye pose!\n";
    }

    if (displayConfig_.getViewer(0).getEye(1).getPose(rightEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get right eye pose!\n";
    }

    float ipd = static_cast<float>((osvr::util::vecMap(leftEye.translation) - osvr::util::vecMap(rightEye.translation)).norm());
    return ipd;
}

const char* OSVRTrackedDevice::GetId()
{
    if (display_.name.empty()) {
        display_.name = "OSVR HMD";
    }

    return display_.name.c_str();
}

void OSVRTrackedDevice::configure()
{
    // Get settings from config file
    const bool verbose_logging = settings_->getSetting<bool>("verbose", false);
    if (verbose_logging) {
        OSVR_LOG(info) << "Verbose logging enabled.";
        Logging::instance().setLogLevel(trace);
    } else {
        OSVR_LOG(info) << "Verbose logging disabled.";
        Logging::instance().setLogLevel(info);
    }

    // The name of the display we want to use
    const auto display_name = settings_->getSetting<std::string>("displayName", "OSVR");

    // Detect displays and find the one we're using as an HMD
    bool display_found = false;
    auto displays = osvr::display::getDisplays();
    for (const auto& display : displays) {
        if (std::string::npos == display.name.find(display_name)) {
            OSVR_LOG(trace) << "Rejecting display [" << display.name << "] since it doesn't match [" << display_name << "].";
            continue;
        }

        OSVR_LOG(trace) << "Found a match! Display [" << display.name << "] matches [" << display_name << "].";
        display_ = display;
        display_found = true;

        // The scan-out origin of the display
        const auto scan_out_origin_str = settings_->getSetting<std::string>("scanoutOrigin", "");
        if (scan_out_origin_str.empty()) {
            // Calculate the scan-out origin based on the display parameters
            //scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + display_.rotation);
            const auto rot = renderManagerConfig_.getDisplayRotation();
            scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + osvr::display::to_Rotation(static_cast<int>(rot)));
            OSVR_LOG(warn) << "Warning: scan-out origin unspecified. Defaulting to " << scanoutOrigin_ << ".";
        } else {
            scanoutOrigin_ = parseScanOutOrigin(scan_out_origin_str);
        }

        break;
    }

    if (!display_found) {
        // If the desired display wasn't detected, use the settings from the
        // display descriptor instead.
        //
        // This will most frequently occur when the HMD is in direct mode or if
        // the HMD is disconnected.
        displayDescription_ = context_.getStringParameter("/display");
        displayConfiguration_ = OSVRDisplayConfiguration(displayDescription_);
        const auto d = OSVRDisplayConfiguration(displayDescription_);
        auto active_resolution = d.activeResolution();

        const auto position_x = renderManagerConfig_.getWindowXPosition();
        const auto position_y = renderManagerConfig_.getWindowYPosition();

        // Rotation
        //   Adjust the scan-out orientation based rotation value.  Also, when
        //   the rotation is 90 or 270, we need to swap the resolution values
        //   and zero out the rotation.. Otherwise, just ignore the rotation (as
        //   we compensate for it using the scan-out origin).
        using osvr::display::Rotation;
        auto rotation = Rotation::Zero;
        const auto rot = renderManagerConfig_.getDisplayRotation();
        if (90 == rot || 270 == rot) {
            std::swap(active_resolution.width, active_resolution.height);
        }

        display_.adapter.description = "Unknown";
        display_.name = displayConfiguration_.getVendor() + " " + displayConfiguration_.getModel() + " " + displayConfiguration_.getVersion();
        display_.size.width = static_cast<uint32_t>(active_resolution.width);
        display_.size.height = static_cast<uint32_t>(active_resolution.height);
        display_.position.x = position_x;
        display_.position.y = position_y;
        display_.rotation = rotation;
        display_.verticalRefreshRate = settings_->getSetting<double>("verticalRefreshRate", getVerticalRefreshRate());
        display_.attachedToDesktop = false; // assuming direct mode
        display_.edidVendorId = settings_->getSetting<uint32_t>("edidVendorId", 0xd24e); // SVR
        display_.edidProductId = settings_->getSetting<uint32_t>("edidProductId", 0x1019);

        // The scan-out origin of the display
        const auto scan_out_origin_str = settings_->getSetting<std::string>("scanoutOrigin", "");
        if (scan_out_origin_str.empty()) {
            // Calculate the scan-out origin based on the display parameters
            scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + osvr::display::to_Rotation(static_cast<int>(rot)));
            OSVR_LOG(warn) << "Warning: scan-out origin unspecified. Defaulting to " << scanoutOrigin_ << ".";
        } else {
            scanoutOrigin_ = parseScanOutOrigin(scan_out_origin_str);
        }
    }

    // Print the display settings we're running with
    if (display_found) {
        OSVR_LOG(info) << "Detected display named [" << display_.name << "]:";
    } else {
        OSVR_LOG(info) << "Display parameters from configuration files:";
    }
    OSVR_LOG(info) << "  Adapter: " << display_.adapter.description;
    OSVR_LOG(info) << "  Monitor name: " << display_.name;
    OSVR_LOG(info) << "  Resolution: " << display_.size.width << "x" << display_.size.height;
    OSVR_LOG(info) << "  Position: (" << display_.position.x << ", " << display_.position.y << ")";
    OSVR_LOG(info) << "  Rotation: " << display_.rotation;
    OSVR_LOG(info) << "  Scan-out origin: " << scanoutOrigin_;
    OSVR_LOG(info) << "  Refresh rate: " << display_.verticalRefreshRate;
    OSVR_LOG(info) << "  " << (display_.attachedToDesktop ? "Extended mode" : "Direct mode");
    OSVR_LOG(info) << "  EDID vendor ID: " << as_hex_0x(display_.edidVendorId) << " (" << osvr::display::decodeEdidVendorId(display_.edidVendorId) << ")";
    OSVR_LOG(info) << "  EDID product ID: " << as_hex_0x(display_.edidProductId);
}

void OSVRTrackedDevice::configureDistortionParameters()
{
    // Parse the display descriptor
    displayDescription_ = context_.getStringParameter("/display");
    displayConfiguration_ = OSVRDisplayConfiguration(displayDescription_);

    // Initialize the distortion parameters
    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Number of eyes: " << displayConfiguration_.getEyes().size() << ".";
    for (size_t i = 0; i < displayConfiguration_.getEyes().size(); ++i) {
        auto distortion = osvr::renderkit::DistortionParameters { displayConfiguration_, i };
        distortion.m_desiredTriangles = 200 * 64;
        OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Adding distortion for eye " << i << ".";
        distortionParameters_.push_back(distortion);
    }
    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Number of distortion parameters: " << distortionParameters_.size() << ".";

    // Make the interpolators to be used by each eye.
    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Creating mesh interpolators for the left eye.";
    if (!makeUnstructuredMeshInterpolators(distortionParameters_[0], 0, leftEyeInterpolators_)) {
        OSVR_LOG(err) << "OSVRTrackedDevice::configureDistortionParameters(): Could not create mesh interpolators for left eye.";
    }
    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Number of left eye interpolators: " << leftEyeInterpolators_.size() << ".";

    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Creating mesh interpolators for the right eye.";
    if (!makeUnstructuredMeshInterpolators(distortionParameters_[1], 1, rightEyeInterpolators_)) {
        OSVR_LOG(err) << "OSVRTrackedDevice::configureDistortionParameters(): Could not create mesh interpolators for right eye.";
    }
    OSVR_LOG(debug) << "OSVRTrackedDevice::configureDistortionParameters(): Number of right eye interpolators: " << leftEyeInterpolators_.size() << ".";
}

template <typename T>
vr::ETrackedPropertyError OSVRTrackedDevice::checkProperty(vr::ETrackedDeviceProperty prop, const T&) const
{
    if (isWrongDataType(prop, T())) {
        return vr::TrackedProp_WrongDataType;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        return vr::TrackedProp_WrongDeviceClass;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        return vr::TrackedProp_InvalidDevice;
    }

    return vr::TrackedProp_Success;
}

osvr::display::ScanOutOrigin OSVRTrackedDevice::parseScanOutOrigin(std::string str) const
{
    // Make the string lowercase
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    if ("lower-left" == str || "ll" == str || "lowerleft" == str || "lower left" == str
        || "bottom-left" == str || "bl" == str || "bottomleft" == str || "bottom left" == str) {
        return osvr::display::ScanOutOrigin::LowerLeft;
    } else if ("lower-right" == str || "lr" == str || "lowerright" == str || "lower right" == str
        || "bottom-right" == str || "br" == str || "bottomright" == str || "bottom right" == str) {
        return osvr::display::ScanOutOrigin::LowerRight;
    } else if ("upper-left" == str || "ul" == str || "upperleft" == str || "upper left" == str
        || "top-left" == str || "tl" == str || "topleft" == str || "top left" == str) {
        return osvr::display::ScanOutOrigin::UpperLeft;
    } else if ("upper-right" == str || "ur" == str || "upperright" == str || "upper right" == str
        || "top-right" == str || "tr" == str || "topright" == str || "top right" == str) {
        return osvr::display::ScanOutOrigin::UpperRight;
    } else {
        OSVR_LOG(err) << "The string [" + str + "] could not be parsed as a scan-out origin. Use one of: lower-left, upper-left, lower-right, upper-right.";
        return osvr::display::ScanOutOrigin::UpperLeft;
    }
}

double OSVRTrackedDevice::getVerticalRefreshRate() const
{
    // The vertical refresh rate is unavailable in direct mode and isn't
    // currently provided via OSVR config file or API.
    //
    // We'll read an override value from steamvr.vrsettings if it exists.
    // Otherwise, we'll fall back on OSVR HDK defaults or use a heuristic for
    // other HMDs.
    const auto refresh_rate = settings_->getSetting<float>("refreshRate", 0.0);
    if (refresh_rate > 0.0) {
        return refresh_rate;
    }

    const auto is_hdk_1x = (std::string::npos != display_.name.find("OSVR HDK 1"));
    const auto is_hdk_20 = (std::string::npos != display_.name.find("OSVR HDK 2.0"));
    const auto is_high_res = (display_.size.width > 1920 || display_.size.height > 1920);

    if (is_hdk_1x) {
        return 60.0;
    } else if (is_hdk_20) {
        return 90.0;
    } else if (is_high_res) {
        // Assume all high-resolution displays operate at 90 Hz
        return 90.0;
    } else {
        // Assume it's a lame HMD and can only handle 60 Hz
        return 60.0;
    }
}

std::pair<float, float> OSVRTrackedDevice::rotate(float u, float v, osvr::display::Rotation rotation) const
{
    // Rotates normalized coordinates counter-clockwise
    using R = osvr::display::Rotation;
    if (R::Zero == rotation) {
        return { u, v };
    } else if (R::Ninety == rotation) {
        return { 1.0f - v, u };
    } else if (R::OneEighty == rotation) {
        return { 1.0f - u, 1.0f - v };
    } else if (R::TwoSeventy == rotation) {
        return { v, 1.0f - u };
    } else {
        OSVR_LOG(err) << "Unknown rotation [" << rotation << "] Assuming 0 degrees.";
        return { u, v };
    }
}

