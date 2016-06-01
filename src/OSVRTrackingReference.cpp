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
#include "osvr_platform.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/Util/EigenInterop.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>
//#include <fstream>
//#include <algorithm>        // for std::find

OSVRTrackingReference::OSVRTrackingReference(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log) : m_Context(context), driver_host_(driver_host), deviceClass_(vr::TrackedDeviceClass_TrackingReference)
{
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
    // Clean up tracker callback if exists
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }

    // Register tracker callback
    m_TrackerInterface = m_Context.getInterface("/me/head");
    m_TrackerInterface.registerCallback(&OSVRTrackingReference::RefTrackerCallback, this);

    return vr::VRInitError_None;
}

void OSVRTrackingReference::Deactivate()
{
    /// Have to force freeing here
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
}

void OSVRTrackingReference::PowerOff()
{
}

void* OSVRTrackingReference::GetComponent(const char* component_name_and_version)
{
    return NULL;
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetBoolTrackedDeviceProperty(): Requested property: " << prop << "\n";
    if(error)
        *error = vr::TrackedProp_ValueNotProvidedByDevice;
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
    case vr::Prop_FieldOfViewLeftDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 45.0;
    case vr::Prop_FieldOfViewRightDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 45.0;
    case vr::Prop_FieldOfViewTopDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 45.0;
    case vr::Prop_FieldOfViewBottomDegrees_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 45.0;
    case vr::Prop_TrackingRangeMinimumMeters_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 0.0;
    case vr::Prop_TrackingRangeMaximumMeters_Float:
        if (error)
            *error = vr::TrackedProp_Success;
        return 10.0;
    default:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }
#include "ignore-warning/pop"
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
    default:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }
#include "ignore-warning/pop"
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetUint64TrackedDeviceProperty(): Requested property: " << prop << "\n";
    if(error)
        *error = vr::TrackedProp_ValueNotProvidedByDevice;
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

    OSVR_LOG(trace) << "OSVRTrackingReference::GetMatrix34TrackedDeviceProperty(): Requested property: " << prop << "\n";
    if(error)
        *error = vr::TrackedProp_ValueNotProvidedByDevice;
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
        return "OSVR_HMD_Camera_1";
    case vr::Prop_RenderModelName_String:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return "dk2_camera";
    default:
        if (error)
            *error = vr::TrackedProp_ValueNotProvidedByDevice;
        return default_value;
    }
#include "ignore-warning/pop"
}

void OSVRTrackingReference::RefTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
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
    Eigen::Vector3d::Map(pose.vecPosition) = Eigen::Vector3d::Zero();

    // Position velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAcceleration) = Eigen::Vector3d::Zero();

    // Orientation
    map(pose.qRotation) = Eigen::Quaterniond::Identity();

    // Angular velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecAngularVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAngularAcceleration) = Eigen::Vector3d::Zero();

    pose.result = vr::TrackingResult_Running_OK;
    pose.poseIsValid = true;
    pose.willDriftInYaw = false;
    pose.shouldApplyHeadModel = false;
    self->pose_ = pose;
    self->driver_host_->TrackedDevicePoseUpdated(1, self->pose_); /// @fixme figure out ID correctly, don't hardcode to zero
}


void OSVRTrackingReference::configure()
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
}
