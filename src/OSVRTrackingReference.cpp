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
#include <osvr/Util/EigenInterop.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>

OSVRTrackingReference::OSVRTrackingReference(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host) : OSVRTrackedDevice(context, driver_host, vr::TrackedDeviceClass_TrackingReference)
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
    objectId_ = object_id;
    configure();

    // Clean up tracker callback if exists
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }

    // Register tracker callback
    trackerInterface_ = context_.getInterface(trackerPath_);
    trackerInterface_.registerCallback(&OSVRTrackingReference::TrackerCallback, this);

    return vr::VRInitError_None;
}

void OSVRTrackingReference::Deactivate()
{
    OSVR_LOG(trace) << "OSVRTrackingReference::Deactivate() called.";

    // Clean up tracker callback if exists
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }
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
    self->driverHost_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

void OSVRTrackingReference::configure()
{
    // Read tracking reference values from config file
    trackerPath_ = settings_->getSetting<std::string>("cameraPath", trackerPath_);

    configureProperties();
}

void OSVRTrackingReference::configureProperties()
{
    // Properties that apply to all device classes
    properties_[vr::Prop_WillDriftInYaw_Bool] = false;
    properties_[vr::Prop_DeviceIsWireless_Bool] = false;
    properties_[vr::Prop_DeviceIsCharging_Bool] = false;
    properties_[vr::Prop_Firmware_UpdateAvailable_Bool] = false;
    properties_[vr::Prop_Firmware_ManualUpdate_Bool] = false;
    properties_[vr::Prop_BlockServerShutdown_Bool] = false;
    //properties_[vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool] = true;
    properties_[vr::Prop_ContainsProximitySensor_Bool] = false;
    properties_[vr::Prop_DeviceProvidesBatteryStatus_Bool] = false;
    properties_[vr::Prop_DeviceCanPowerOff_Bool] = false;
    properties_[vr::Prop_HasCamera_Bool] = false;
    properties_[vr::Prop_DeviceBatteryPercentage_Float] = 1.0f;
    properties_[vr::Prop_DeviceClass_Int32] = deviceClass_;
    //properties_[vr::Prop_HardwareRevision_Uint64] = 0ul;
    //properties_[vr::Prop_FirmwareVersion_Uint64] = 0ul;
    //properties_[vr::Prop_FPGAVersion_Uint64] = 0ul;
    //properties_[vr::Prop_VRCVersion_Uint64] = 0ul;
    //properties_[vr::Prop_RadioVersion_Uint64] = 0ul;
    //properties_[vr::Prop_DongleVersion_Uint64] = 0ul;
    //properties_[vr::Prop_StatusDisplayTransform_Matrix34] = /* ... */;
    //properties_[vr::Prop_TrackingSystemName_String] = "";
    properties_[vr::Prop_ModelNumber_String] = "OSVR Tracking Reference";
    properties_[vr::Prop_SerialNumber_String] = GetId(); // FIXME read value from server
    properties_[vr::Prop_RenderModelName_String] = "dk2_camera"; // FIXME replace with HDK IR camera model
    properties_[vr::Prop_ManufacturerName_String] = "OSVR"; // FIXME read value from server
    //properties_[vr::Prop_TrackingFirmwareVersion_String] = "";
    //properties_[vr::Prop_HardwareRevision_String] = "";
    //properties_[vr::Prop_AllWirelessDongleDescriptions_String] = "";
    //properties_[vr::Prop_ConnectedWirelessDongle_String] = "";
    //properties_[vr::Prop_Firmware_ManualUpdateURL_String] = "";
    //properties_[vr::Prop_Firmware_ProgrammingTarget_String] = "";
    //properties_[vr::Prop_DriverVersion_String] = "";

    // Properties that are unique to TrackedDeviceClass_TrackingReference

    // Default tracking volume values are for the OSVR HDK IR camera
    properties_[vr::Prop_FieldOfViewLeftDegrees_Float] = settings_->getSetting<float>("cameraFOVLeftDegrees", 35.235f);
    properties_[vr::Prop_FieldOfViewRightDegrees_Float] = settings_->getSetting<float>("cameraFOVRightDegrees", 35.235f);
    properties_[vr::Prop_FieldOfViewTopDegrees_Float] = settings_->getSetting<float>("cameraFOVTopDegrees", 27.95f);
    properties_[vr::Prop_FieldOfViewBottomDegrees_Float] = settings_->getSetting<float>("cameraFOVBottomDegrees", 27.95f);
    properties_[vr::Prop_TrackingRangeMinimumMeters_Float] = settings_->getSetting<float>("minTrackingRangeMeters", 0.15f);
    properties_[vr::Prop_TrackingRangeMaximumMeters_Float] = settings_->getSetting<float>("maxTrackingRangeMeters", 1.5f);
    //properties_[vr::Prop_ModeLabel_String] = "";
}


