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
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/InterfaceStateC.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/PlatformConfig.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>

OSVRTrackingReference::OSVRTrackingReference(osvr::clientkit::ClientContext& context) : OSVRTrackedDevice(context, vr::TrackedDeviceClass_TrackingReference, "OSVRTrackingReference")
{
    OSVR_LOG(trace) << "OSVRTrackingReference::OSVRTrackingReference() called.";
}

OSVRTrackingReference::~OSVRTrackingReference()
{
    // do nothing
}

vr::EVRInitError OSVRTrackingReference::Activate(uint32_t object_id)
{
    OSVR_LOG(trace) << "OSVRTrackingReference::Activate() called.";

    OSVRTrackedDevice::Activate(object_id);

    configure();

    // Clean up tracker callback if exists
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }

    // Register tracker callback
    m_TrackerInterface = context_.getInterface(trackerPath_);
    m_TrackerInterface.registerCallback(&OSVRTrackingReference::TrackerCallback, this);

    setProperties();

    return vr::VRInitError_None;
}

void OSVRTrackingReference::Deactivate()
{
    OSVR_LOG(trace) << "OSVRTrackingReference::Deactivate() called.";

    objectId_ = vr::k_unTrackedDeviceIndexInvalid;

    // Clean up tracker callback if exists
    if (m_TrackerInterface.notEmpty()) {
        m_TrackerInterface.free();
    }
}

void OSVRTrackingReference::EnterStandby()
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

void OSVRTrackingReference::setProperties()
{
    propertyContainer_ = vr::VRProperties()->TrackedDeviceToPropertyContainer(objectId_);

    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_WillDriftInYaw_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceIsWireless_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceIsCharging_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_Firmware_UpdateAvailable_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_Firmware_ManualUpdate_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_BlockServerShutdown_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_ContainsProximitySensor_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceProvidesBatteryStatus_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceCanPowerOff_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_HasCamera_Bool, false);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_DeviceBatteryPercentage_Float, 1.0f); // full battery
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_FieldOfViewLeftDegrees_Float, fovLeft_);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_FieldOfViewRightDegrees_Float, fovRight_);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_FieldOfViewTopDegrees_Float, fovTop_);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_FieldOfViewBottomDegrees_Float, fovBottom_);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_TrackingRangeMinimumMeters_Float, minTrackingRange_);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_TrackingRangeMaximumMeters_Float, maxTrackingRange_);
    vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_DeviceClass_Int32, deviceClass_);
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ModelNumber_String, "OSVR camera");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_SerialNumber_String, this->getId());
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_RenderModelName_String, settings_->getSetting<std::string>("cameraRenderModel", "").c_str());
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ManufacturerName_String, "OSVR"); // FIXME read value from server
}

const char* OSVRTrackingReference::getId()
{
    return "OSVR IR camera";
}

vr::ETrackedDeviceClass OSVRTrackingReference::getDeviceClass() const
{
    return deviceClass_;
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
    pose.deviceIsConnected = true;

    //OSVR_LOG(trace) << "OSVRTrackingReference::TrackerCallback(): Got a new camera pose: " << pose.vecPosition << " at angle " << pose.qRotation << ".";
    self->pose_ = pose;
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(self->objectId_, self->pose_, sizeof(vr::DriverPose_t));
}

void OSVRTrackingReference::configure()
{
    // Get settings from config file

    // Read tracking reference values from config file
    trackerPath_ = getTrackerPath();
    fovLeft_ = settings_->getSetting<float>("cameraFOVLeftDegrees", fovLeft_);
    fovRight_ = settings_->getSetting<float>("cameraFOVRightDegrees", fovRight_);
    fovTop_ = settings_->getSetting<float>("cameraFOVTopDegrees", fovTop_);
    fovBottom_ = settings_->getSetting<float>("cameraFOVBottomDegrees", fovBottom_);
    minTrackingRange_ = settings_->getSetting<float>("minTrackingRangeMeters", minTrackingRange_);
    maxTrackingRange_ = settings_->getSetting<float>("maxTrackingRangeMeters", maxTrackingRange_);

    //configureProperties();
    setProperties();
}

std::string OSVRTrackingReference::getTrackerPath() const
{
    // If the camera path is set explicitly in the configuration file, then use
    // that path regardless of whether or not it works (in the hopes that the
    // camera will eventually turn up).
    const auto settings_camera_path = settings_->getSetting<std::string>("cameraPath", "");
    if (!settings_camera_path.empty()) {
        OSVR_LOG(info) << "Using configured camera path [" << settings_camera_path << "].";
        return settings_camera_path;
    }

    // The camera path wasn't explicitly specified, so we'll test a couple
    // common values.
    //
    // Different OSVR tracking plugins use different paths for the tracking
    // camera pose.  The original video-based IMU fusion plugin used
    // `/org_osvr_filter_videoimufusion/HeadFusion/semantic/camera` while the
    // newer unified video inertial tracker plugin usees `/trackingCamera`.
    const auto new_camera_path = std::string{"/trackingCamera"};
#if 0
    // TODO spin this off into a separate thread so it can loop until it finds a
    // valid camera path

    const auto old_camera_path = std::string{"/org_osvr_filter_videoimufusion/HeadFusion/semantic/camera"};
    const auto new_camera_path = std::string{"/trackingCamera"};

    const auto camera_paths = std::vector<std::string> { new_camera_path, old_camera_path };

    for (const auto& camera_path : camera_paths) {
        auto iface = context_.getInterface(camera_path.c_str());

        OSVR_PoseState state;
        OSVR_TimeValue timestamp;

        const auto ret = osvrGetPoseState(iface.get(), &timestamp, &state);
        if (OSVR_RETURN_SUCCESS != ret) {
            OSVR_LOG(debug) << "Failed to get pose state from path " << camera_path << ".";
            continue;
        }

        OSVR_LOG(info) << "Using detected camera path [" << camera_path << "].";
        return camera_path;
    }
#endif

    // None of the default camera paths appear to be valid, so we'll use the
    // default camera path and hope it eventually shows up.
    OSVR_LOG(info) << "Using fallback camera path [" << new_camera_path << "].";
    return new_camera_path;
}

