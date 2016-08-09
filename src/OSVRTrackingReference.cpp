/** @file
    @brief OSVR tracking reference (e.g., camera or base station)

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
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
#include "OSVRTrackingReference.h"
#include "Logging.h"

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "osvr_device_properties.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/PlatformConfig.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>

OSVRTrackingReference::OSVRTrackingReference(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log) : m_Context(context), driver_host_(driver_host), pose_(), deviceClass_(vr::TrackedDeviceClass_TrackingReference)
{
    OSVR_LOG(trace) << "OSVRTrackingReference::OSVRTrackingReference() called.";
    settings_ = std::make_unique<Settings>(driver_host->GetSettings(vr::IVRSettings_Version));
    if (driver_log) {
        Logging::instance().setDriverLog(driver_log);
    }
    configure();
}

OSVRTrackingReference::~OSVRTrackingReference()
{
    driver_host_ = nullptr;
}

vr::EVRInitError OSVRTrackingReference::Activate(uint32_t object_id)
{
    OSVR_LOG(trace) << "OSVRTrackingReference::Activate() called.";
    objectId_ = object_id;

    // Clean up tracker callback if exists
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }

    // Register tracker callback
    m_TrackerInterface = m_Context.getInterface(trackerPath_);
    m_TrackerInterface.registerCallback(&OSVRTrackingReference::TrackerCallback, this);

    return vr::VRInitError_None;
}

void OSVRTrackingReference::Deactivate()
{
    OSVR_LOG(trace) << "OSVRTrackingReference::Deactivate() called.";

    // Clean up tracker callback if exists
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
}

void OSVRTrackingReference::PowerOff()
{
    // do nothing
}

void* OSVRTrackingReference::GetComponent(const char* component_name_and_version)
{
    if (!strcasecmp(component_name_and_version, vr::ITrackedDeviceServerDriver_Version)) {
        return static_cast<vr::ITrackedDeviceServerDriver*>(this);
    }

    return nullptr;
}

void OSVRTrackingReference::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // TODO
    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
    // return empty string for now
    if (response_buffer_size > 0) {
        response_buffer[0] = '\0';
    }
}

vr::DriverPose_t OSVRTrackingReference::GetPose()
{
    return pose_;
}

bool OSVRTrackingReference::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

    //OSVR_LOG(trace) << "OSVRTrackingReference::GetBoolTrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // Properties that apply to all device classes
    case vr::Prop_WillDriftInYaw_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
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
        return false;
        break;
    case vr::Prop_DeviceProvidesBatteryStatus_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_DeviceCanPowerOff_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    case vr::Prop_HasCamera_Bool:
        if (error)
            *error = vr::TrackedProp_Success;
        return false;
        break;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetBoolTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

float OSVRTrackingReference::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetFloatTrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_DeviceBatteryPercentage_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 1.0f; // full battery
    // Properties that are unique to TrackedDeviceClass_TrackingReference
    case vr::Prop_FieldOfViewLeftDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return fovLeft_;
    case vr::Prop_FieldOfViewRightDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return fovRight_;
    case vr::Prop_FieldOfViewTopDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return fovTop_;
    case vr::Prop_FieldOfViewBottomDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return fovBottom_;
    case vr::Prop_TrackingRangeMinimumMeters_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return minTrackingRange_;
    case vr::Prop_TrackingRangeMaximumMeters_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return maxTrackingRange_;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetFloatTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

int32_t OSVRTrackingReference::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetInt32TrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_DeviceClass_Int32:
        if (error)
            *error = vr::TrackedProp_Success;
        return deviceClass_;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetInt32TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint64_t OSVRTrackingReference::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetUint64TrackedDeviceProperty(): Requested property: " << prop << "\n";

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
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetUint64TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

vr::HmdMatrix34_t OSVRTrackingReference::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetMatrix34TrackedDeviceProperty(): Requested property: " << prop << "\n";

    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_StatusDisplayTransform_Matrix34:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetMatrix34TrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

uint32_t OSVRTrackingReference::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError *pError)
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetStringTrackedDeviceProperty(): Requested property: " << prop << "\n";

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

std::string OSVRTrackingReference::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error)
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
            *error = vr::TrackedProp_Success;
        return "dk2_camera"; // FIXME replace with HDK IR camera model
    case vr::Prop_ManufacturerName_String:
        if (error)
            *error = vr::TrackedProp_Success;
        return "OSVR"; // FIXME read value from server
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
    // Properties that are unique to TrackedDeviceClass_TrackingReference
    case vr::Prop_ModeLabel_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }

#include "ignore-warning/pop"

    OSVR_LOG(warn) << "OSVRTrackingReference::GetStringTrackedDeviceProperty(): Unknown property " << prop << " requested.\n";
    if (error)
        *error = vr::TrackedProp_UnknownProperty;
    return default_value;
}

const char* OSVRTrackingReference::GetId()
{
    return "OSVR IR camera";
}

void OSVRTrackingReference::TrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackingReference*>(userdata);

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
    pose.willDriftInYaw = false;
    pose.shouldApplyHeadModel = false;
    self->pose_ = pose;
    self->driver_host_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

void OSVRTrackingReference::configure()
{
    // Get settings from config file
    const bool verbose_logging = settings_->getSetting<bool>("verbose", verboseLogging_);
    if (verbose_logging) {
        OSVR_LOG(info) << "Verbose logging enabled.";
        Logging::instance().setLogLevel(trace);
    } else {
        OSVR_LOG(info) << "Verbose logging disabled.";
        Logging::instance().setLogLevel(info);
    }

    // Read tracking reference values from config file
    trackerPath_ = settings_->getSetting<std::string>("cameraPath", trackerPath_);
    fovLeft_ = settings_->getSetting<float>("cameraFOVLeftDegrees", fovLeft_);
    fovRight_ = settings_->getSetting<float>("cameraFOVRightDegrees", fovRight_);
    fovTop_ = settings_->getSetting<float>("cameraFOVTopDegrees", fovTop_);
    fovBottom_ = settings_->getSetting<float>("cameraFOVBottomDegrees", fovBottom_);
    minTrackingRange_ = settings_->getSetting<float>("minTrackingRangeMeters", minTrackingRange_);
    maxTrackingRange_ = settings_->getSetting<float>("maxTrackingRangeMeters", maxTrackingRange_);
}

