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
#include "osvr_platform.h"
#include "display/DisplayEnumerator.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <algorithm>        // for std::find

OSVRTrackedDevice::OSVRTrackedDevice(const std::string& display_description, osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log) : m_DisplayDescription(display_description), m_Context(context), driver_host_(driver_host), pose_(), deviceClass_(vr::TrackedDeviceClass_HMD)
{
    settings_ = std::make_unique<Settings>(driver_host->GetSettings(vr::IVRSettings_Version));
    if (driver_log) {
        Logging::instance().setDriverLog(driver_log);
    }
    configure();
}

OSVRTrackedDevice::~OSVRTrackedDevice()
{
    driver_host_ = nullptr;
}

vr::EVRInitError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    objectId_ = object_id;

    const std::time_t waitTime = 5; // wait up to 5 seconds for init

    // Register tracker callback
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }

    // Ensure context is fully started up
    OSVR_LOG(trace) << "Waiting for the context to fully start up...\n";
    std::time_t startTime = std::time(nullptr);
    while (!m_Context.checkStatus()) {
        m_Context.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "Context startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    m_DisplayConfig = osvr::clientkit::DisplayConfig(m_Context);

    // Ensure display is fully started up
    OSVR_LOG(trace) << "Waiting for the display to fully start up, including receiving initial pose update...\n";
    startTime = std::time(nullptr);
    while (!m_DisplayConfig.checkStartup()) {
        m_Context.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "Display startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Verify valid display config
    if ((m_DisplayConfig.getNumViewers() != 1) && (m_DisplayConfig.getViewer(0).getNumEyes() != 2) && (m_DisplayConfig.getViewer(0).getEye(0).getNumSurfaces() == 1) && (m_DisplayConfig.getViewer(0).getEye(1).getNumSurfaces() != 1)) {
        OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Unexpected display parameters!\n";

        if (m_DisplayConfig.getNumViewers() < 1) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least one viewer must exist.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if (m_DisplayConfig.getViewer(0).getNumEyes() < 2) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least two eyes must exist.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if ((m_DisplayConfig.getViewer(0).getEye(0).getNumSurfaces() < 1) || (m_DisplayConfig.getViewer(0).getEye(1).getNumSurfaces() < 1)) {
            OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): At least one surface must exist for each eye.\n";
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        }
    }

    // Register tracker callback
    m_TrackerInterface = m_Context.getInterface("/me/head");
    m_TrackerInterface.registerCallback(&OSVRTrackedDevice::HmdTrackerCallback, this);

    auto configString = m_Context.getStringParameter("/renderManagerConfig");

    // If the /renderManagerConfig parameter is missing from the configuration
    // file, use an empty dictionary instead. This allows the render manager
    // config to zero out its values.
    if (configString.empty()) {
        OSVR_LOG(info) << "OSVRTrackedDevice::Activate(): Render Manager config is empty, using default values.\n";
        configString = "{}";
    }

    try {
        m_RenderManagerConfig.parse(configString);
    } catch(const std::exception& e) {
        OSVR_LOG(err) << "OSVRTrackedDevice::Activate(): Exception parsing Render Manager config: " << e.what() << "\n";
    }

    driver_host_->ProximitySensorState(objectId_, true);

    OSVR_LOG(trace) << "OSVRTrackedDevice::Activate(): Activation complete.\n";
    return vr::VRInitError_None;
}

void OSVRTrackedDevice::Deactivate()
{
    /// Have to force freeing here
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
}

void OSVRTrackedDevice::PowerOff()
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
    int nDisplays = m_DisplayConfig.getNumDisplayInputs();
    if (nDisplays != 1) {
        OSVR_LOG(err) << "OSVRTrackedDevice::OSVRTrackedDevice(): Unexpected display number of displays!\n";
    }
    osvr::clientkit::DisplayDimensions displayDims = m_DisplayConfig.getDisplayDimensions(0);
    *x = m_RenderManagerConfig.getWindowXPosition(); // todo: assumes desktop display of 1920. get this from display config when it's exposed.
    *y = m_RenderManagerConfig.getWindowYPosition();
    *width = static_cast<uint32_t>(displayDims.width);
    *height = static_cast<uint32_t>(displayDims.height);

#if defined(OSVR_WINDOWS) || defined(OSVR_MACOSX)
    // ... until we've added code for other platforms
    *x = display_.position.x;
    *y = display_.position.y;
    *height = display_.size.height;
    *width = display_.size.width;
#endif
}

bool OSVRTrackedDevice::IsDisplayOnDesktop()
{
    // If the current display still appeara in the active displays list,
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
    /// @todo calculate overfill factor properly
    double overfill_factor = 1.0;
    int32_t x, y;
    uint32_t w, h;
    GetWindowBounds(&x, &y, &w, &h);

    *width = static_cast<uint32_t>(w * overfill_factor);
    *height = static_cast<uint32_t>(h * overfill_factor);
}

void OSVRTrackedDevice::GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    osvr::clientkit::RelativeViewport viewPort = m_DisplayConfig.getViewer(0).getEye(eye).getSurface(0).getRelativeViewport();
    *x = static_cast<uint32_t>(viewPort.left);
    *y = static_cast<uint32_t>(viewPort.bottom);
    *width = static_cast<uint32_t>(viewPort.width);
    *height = static_cast<uint32_t>(viewPort.height);
}

void OSVRTrackedDevice::GetProjectionRaw(vr::EVREye eye, float* left, float* right, float* top, float* bottom)
{
    // Reference: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionRaw
    // SteamVR expects top and bottom to be swapped!
    osvr::clientkit::ProjectionClippingPlanes pl = m_DisplayConfig.getViewer(0).getEye(eye).getSurface(0).getProjectionClippingPlanes();
    *left = static_cast<float>(pl.left);
    *right = static_cast<float>(pl.right);
    *bottom = static_cast<float>(pl.top); // SWAPPED
    *top = static_cast<float>(pl.bottom); // SWAPPED
}

vr::DistortionCoordinates_t OSVRTrackedDevice::ComputeDistortion(vr::EVREye eye, float u, float v)
{
    /// @todo FIXME Compute distortion using display configuration data
    vr::DistortionCoordinates_t coords;
    coords.rfRed[0] = u;
    coords.rfRed[1] = v;
    coords.rfBlue[0] = u;
    coords.rfBlue[1] = v;
    coords.rfGreen[0] = u;
    coords.rfGreen[1] = v;
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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetBoolTrackedDeviceProperty(): Requested property: " << prop << "\n";

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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetFloatTrackedDeviceProperty(): Requested property: " << prop << "\n";

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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetInt32TrackedDeviceProperty(): Requested property: " << prop << "\n";

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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetUint64TrackedDeviceProperty(): Requested property: " << prop << "\n";

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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(): Requested property: " << prop << "\n";

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

    OSVR_LOG(trace) << "OSVRTrackedDevice::GetStringTrackedDeviceProperty(): Requested property: " << prop << "\n";

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
        return "OSVR HMD";
    case vr::Prop_SerialNumber_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return this->GetId();
    case vr::Prop_RenderModelName_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ManufacturerName_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
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

void OSVRTrackedDevice::HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
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

    self->pose_ = pose;
    self->driver_host_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

float OSVRTrackedDevice::GetIPD()
{
    OSVR_Pose3 leftEye, rightEye;

    if (m_DisplayConfig.getViewer(0).getEye(0).getPose(leftEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get left eye pose!\n";
    }

    if (m_DisplayConfig.getViewer(0).getEye(1).getPose(rightEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get right eye pose!\n";
    }

    float ipd = static_cast<float>((osvr::util::vecMap(leftEye.translation) - osvr::util::vecMap(rightEye.translation)).norm());
    return ipd;
}

const char* OSVRTrackedDevice::GetId()
{
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
    const std::string display_name = settings_->getSetting<std::string>("displayName", "OSVR");


    // Detect displays and find the one we're using as an HMD
    bool display_found = false;
    auto displays = osvr::display::getDisplays();
    for (const auto& display : displays) {
        if (std::string::npos == display.name.find(display_name))
            continue;

        display_ = display;
        display_found = true;
        break;
    }

    if (!display_found) {
        // Default to OSVR HDK display settings
        display_.adapter.description = "Unknown";
        display_.name = "OSVR HDK";
        display_.size.width = 1920;
        display_.size.height = 1080;
        display_.position.x = 1920;
        display_.position.y = 0;
        display_.rotation = osvr::display::Rotation::Zero;
        display_.verticalRefreshRate = 60.0;
        display_.attachedToDesktop = true;
        display_.edidVendorId = 0xd24e;// 53838
        display_.edidProductId = 0x1019; // 4121
    }

    if (display_found) {
        OSVR_LOG(info) << "Detected display named [" << display_.name << "]:";
    } else {
        OSVR_LOG(info) << "Default display:";
    }
    OSVR_LOG(info) << "  Adapter: " << display_.adapter.description;
    OSVR_LOG(info) << "  Monitor name: " << display_.name;
    OSVR_LOG(info) << "  Resolution: " << display_.size.width << "x" << display_.size.height;
    OSVR_LOG(info) << "  Position: (" << display_.position.x << ", " << display_.position.y << ")";
    switch (display_.rotation) {
    case osvr::display::Rotation::Zero:
        OSVR_LOG(info) << "  Rotation: Landscape";
        break;
    case osvr::display::Rotation::Ninety:
        OSVR_LOG(info) << "  Rotation: Portrait";
        break;
    case osvr::display::Rotation::OneEighty:
        OSVR_LOG(info) << "  Rotation: Landscape (flipped)";
        break;
    case osvr::display::Rotation::TwoSeventy:
        OSVR_LOG(info) << "  Rotation: Portrait (flipped)";
        break;
    default:
        OSVR_LOG(info) << "  Rotation: Landscape";
        break;
    }
    OSVR_LOG(info) << "  Refresh rate: " << display_.verticalRefreshRate;
    OSVR_LOG(info) << "  " << (display_.attachedToDesktop ? "Extended mode" : "Direct mode");
    OSVR_LOG(info) << "  EDID vendor ID: " << display_.edidVendorId;
    OSVR_LOG(info) << "  EDID product ID: " << display_.edidProductId;
}

template <typename T>
vr::ETrackedPropertyError OSVRTrackedDevice::checkProperty(vr::ETrackedDeviceProperty prop, const T&)
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

