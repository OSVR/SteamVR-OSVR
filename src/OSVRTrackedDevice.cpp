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

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "osvr_device_properties.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp

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

OSVRTrackedDevice::OSVRTrackedDevice(const std::string& display_description, osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log) : m_DisplayDescription(display_description), m_Context(context), driver_host_(driver_host), logger_(driver_log), pose_(), deviceClass_(vr::TrackedDeviceClass_HMD)
{
    // do nothing
}

OSVRTrackedDevice::~OSVRTrackedDevice()
{
    logger_ = nullptr;
    driver_host_ = nullptr;
}

vr::EVRInitError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    const std::time_t waitTime = 5; // wait up to 5 seconds for init

    // Register tracker callback
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }

    // Ensure context is fully started up
    logger_->Log("Waiting for the context to fully start up...\n");
    std::time_t startTime = std::time(nullptr);
    while (!m_Context.checkStatus()) {
        m_Context.update();
        if (std::time(nullptr) > startTime + waitTime) {
            logger_->Log("Context startup timed out!\n");
            return vr::VRInitError_Driver_Failed;
        }
    }

    m_DisplayConfig = osvr::clientkit::DisplayConfig(m_Context);

    // Ensure display is fully started up
    logger_->Log("Waiting for the display to fully start up, including receiving initial pose update...\n");
    startTime = std::time(nullptr);
    while (!m_DisplayConfig.checkStartup()) {
        m_Context.update();
        if (std::time(nullptr) > startTime + waitTime) {
            logger_->Log("Display startup timed out!\n");
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Verify valid display config
    if ((m_DisplayConfig.getNumViewers() != 1) && (m_DisplayConfig.getViewer(0).getNumEyes() != 2) && (m_DisplayConfig.getViewer(0).getEye(0).getNumSurfaces() == 1) && (m_DisplayConfig.getViewer(0).getEye(1).getNumSurfaces() != 1)) {
        logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): Unexpected display parameters!\n");

        if (m_DisplayConfig.getNumViewers() < 1) {
            logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): At least one viewer must exist.\n");
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if (m_DisplayConfig.getViewer(0).getNumEyes() < 2) {
            logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): At least two eyes must exist.\n");
            return vr::VRInitError_Driver_HmdDisplayNotFound;
        } else if ((m_DisplayConfig.getViewer(0).getEye(0).getNumSurfaces() < 1) || (m_DisplayConfig.getViewer(0).getEye(1).getNumSurfaces() < 1)) {
            logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): At least one surface must exist for each eye.\n");
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
        logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): Render Manager config is empty, using default values.\n");
        configString = "{}";
    }

    try {
        m_RenderManagerConfig.parse(configString);
    } catch(const std::exception& e) {
        const std::string msg = "OSVRTrackedDevice::OSVRTrackedDevice(): Exception parsing Render Manager config: " + std::string(e.what()) + "\n";
        logger_->Log(msg.c_str());
    }

        /// @fixme figure out ID correctly, don't hardcode to zero
    driver_host_->ProximitySensorState(0, true);
    
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
        return (vr::IVRDisplayComponent*)this;
    }

    // Override this to add a component to a driver
    return NULL;
}

void OSVRTrackedDevice::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // TODO
    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
}

void OSVRTrackedDevice::GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height)
{
    int nDisplays = m_DisplayConfig.getNumDisplayInputs();
    if (nDisplays != 1) {
        logger_->Log("OSVRTrackedDevice::OSVRTrackedDevice(): Unexpected display number of displays!\n");
    }
    osvr::clientkit::DisplayDimensions displayDims = m_DisplayConfig.getDisplayDimensions(0);
    *x = m_RenderManagerConfig.getWindowXPosition(); // todo: assumes desktop display of 1920. get this from display config when it's exposed.
    *y = m_RenderManagerConfig.getWindowYPosition();
    *width = displayDims.width;
    *height = displayDims.height;
}

bool OSVRTrackedDevice::IsDisplayOnDesktop()
{
    // TODO get this info from display description?
    return true;
}

bool OSVRTrackedDevice::IsDisplayRealDisplay()
{
    // TODO get this info from display description?
    return true;
}

void OSVRTrackedDevice::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
    /// @todo calculate overfill factor properly
    double overfillFactor = 1.0;
    int32_t x, y;
    uint32_t w, h;
    GetWindowBounds(&x, &y, &w, &h);
    *width = w * overfillFactor;
    *height = h * overfillFactor;
}

void OSVRTrackedDevice::GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    osvr::clientkit::RelativeViewport viewPort = m_DisplayConfig.getViewer(0).getEye(eye).getSurface(0).getRelativeViewport();
    *x = viewPort.left;
    *y = viewPort.bottom;
    *width = viewPort.width;
    *height = viewPort.height;
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

    if (isWrongDataType(prop, bool())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetBoolTrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

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
    case vr::Prop_IsOnDesktop_Bool: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
        break;
    }

#include "ignore-warning/pop"

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

float OSVRTrackedDevice::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const float default_value = 0.0f;

    if (isWrongDataType(prop, float())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetFloatTrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

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
    case vr::Prop_DisplayFrequency_Float: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
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

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

int32_t OSVRTrackedDevice::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const int32_t default_value = 0;

    if (isWrongDataType(prop, int32_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetInt32TrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

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
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_EdidProductID_Int32:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
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

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint64_t OSVRTrackedDevice::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    const uint64_t default_value = 0;

    if (isWrongDataType(prop, uint64_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetUint64TrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

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
        return 0;
    case vr::Prop_PreviousUniverseId_Uint64:
        if (error)
            *error = vr::TrackedProp_Success;
        return 0;
    case vr::Prop_DisplayFirmwareVersion_Uint64:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
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

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

vr::HmdMatrix34_t OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
{
    // Default value is identity matrix
    vr::HmdMatrix34_t default_value;
    map(default_value) = Matrix34f::Identity();

    if (isWrongDataType(prop, vr::HmdMatrix34_t())) {
        if (error)
            *error = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (error)
            *error = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (error)
            *error = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetMatrix34TrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

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

    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint32_t OSVRTrackedDevice::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError *pError)
{
    uint32_t default_value = 0;
    if (isWrongDataType(prop, pchValue)) {
        if (pError)
            *pError = vr::TrackedProp_WrongDataType;
        return default_value;
    }

    if (isWrongDeviceClass(prop, deviceClass_)) {
        if (pError)
            *pError = vr::TrackedProp_WrongDeviceClass;
        return default_value;
    }

    if (vr::TrackedDeviceClass_Invalid == deviceClass_) {
        if (pError)
            *pError = vr::TrackedProp_InvalidDevice;
        return default_value;
    }

#ifdef VERBOSE_LOGGING
    const std::string msg = "OSVRTrackedDevice::GetFloatTrackedDeviceProperty(): Requested property: " + std::to_string(prop) + "\n";
    logger_->Log(msg.c_str());
#endif

    std::string sValue = GetStringTrackedDeviceProperty(prop, pError);
    if (*pError == vr::TrackedProp_Success) {
        if (sValue.size() + 1 > unBufferSize) {
            *pError = vr::TrackedProp_BufferTooSmall;
        } else {
            valveStrCpy(sValue, pchValue, unBufferSize);
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
    case vr::Prop_TrackingSystemName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ModelNumber_String: // TODO
        if (error)
            *error = vr::TrackedProp_Success;
        return "OSVR HMD";
    case vr::Prop_SerialNumber_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return this->GetId();
    case vr::Prop_RenderModelName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_ManufacturerName_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_TrackingFirmwareVersion_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_HardwareRevision_String: // TODO
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    case vr::Prop_AttachedDeviceId_String: // TODO
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
    // Properties that are unique to TrackedDeviceClass_HMD
    // Properties that are unique to TrackedDeviceClass_Controller
    // Properties that are unique to TrackedDeviceClass_TrackingReference
    }

#include "ignore-warning/pop"

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
    self->driver_host_->TrackedDevicePoseUpdated(0, self->pose_); /// @fixme figure out ID correctly, don't hardcode to zero
}

float OSVRTrackedDevice::GetIPD()
{
    OSVR_Pose3 leftEye, rightEye;

    if (m_DisplayConfig.getViewer(0).getEye(0).getPose(leftEye) != true) {
        logger_->Log("OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get left eye pose!\n");
    }

    if (m_DisplayConfig.getViewer(0).getEye(1).getPose(rightEye) != true) {
        logger_->Log("OSVRTrackedDevice::GetHeadFromEyePose(): Unable to get right eye pose!\n");
    }

    return (osvr::util::vecMap(leftEye.translation) - osvr::util::vecMap(rightEye.translation)).norm();
}

const char* OSVRTrackedDevice::GetId()
{
    /// @todo When available, return the actual unique ID of the HMD
    return "OSVR HMD";
}

