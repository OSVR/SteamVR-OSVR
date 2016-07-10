/** @file
    @brief OSVR tracked controller

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
#include "OSVRTrackedController.h"

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "Logging.h"

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

// TODO:
// Trackpad
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Thumbstick, "/controller/left/joystick/button"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Shoulder, "/controller/left/bumper"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::SpecialLeft, "/controller/left/middle"),

OSVRTrackedController::OSVRTrackedController(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, const std::string& user_driver_config_dir, vr::ETrackedControllerRole controller_role) : OSVRTrackedDevice(context, driver_host, vr::TrackedDeviceClass_Controller, user_driver_config_dir, "OSVRTrackedController"), controllerRole_(controller_role)
{
    name_ = "OSVRController" + std::to_string(controller_role);

    numAxis_ = 0;
    for (int iter_axis = 0; iter_axis < NUM_AXIS; ++iter_axis) {
        analogInterface_[iter_axis].parentController = this;
        analogInterface_[iter_axis].axisType = vr::EVRControllerAxisType::k_eControllerAxis_None;
    }
}

OSVRTrackedController::~OSVRTrackedController()
{
    // do nothing
}

vr::EVRInitError OSVRTrackedController::Activate(uint32_t object_id)
{
    OSVRTrackedDevice::Activate(object_id);

    const std::time_t wait_time = 5; // wait up to 5 seconds for init

    freeInterfaces();
    numAxis_ = 0;

    // Ensure context is fully started up
    OSVR_LOG(info) << "Waiting for the context to fully start up...";
    const auto start_time = std::time(nullptr);
    while (!context_.checkStatus()) {
        context_.update();
        if (std::time(nullptr) > start_time + wait_time) {
            OSVR_LOG(warn) << "Context startup timed out!";
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Register callbacks
    std::string trackerPath;
    std::string buttonPath;
    std::string triggerPath;
    std::string joystickPath;
    if (vr::TrackedControllerRole_LeftHand == controllerRole_) {
        trackerPath = "/me/hands/left";
        buttonPath = "/controller/left/";
        triggerPath = "/controller/left/trigger";
        joystickPath = "/controller/left/joystick";
    } else if (vr::TrackedControllerRole_LeftHand == controllerRole_) {
        trackerPath = "/me/hands/right";
        buttonPath = "/controller/right/";
        triggerPath = "/controller/right/trigger";
        joystickPath = "/controller/right/joystick";
    } else {
        buttonPath = "/controller" + std::to_string(controllerRole_) + "/";
        triggerPath = "/controller" + std::to_string(controllerRole_) + "/trigger";
        joystickPath = "/controller" + std::to_string(controllerRole_) + "/joystick";
    }

    if (!trackerPath.empty()) {
        trackerInterface_ = context_.getInterface(trackerPath);
        trackerInterface_.registerCallback(&OSVRTrackedController::controllerTrackerCallback, this);
    }

    for (int iter_button = 0; iter_button < NUM_BUTTONS; ++iter_button) {
        buttonInterface_[iter_button] = context_.getInterface(buttonPath + std::to_string(iter_button));
        if (buttonInterface_[iter_button].notEmpty()) {
            buttonInterface_[iter_button].registerCallback(&OSVRTrackedController::controllerButtonCallback, this);
        } else {
            buttonInterface_[iter_button].free();
        }
    }

    // TODO: ADD TOUCHPAD PART HERE
    numAxis_ = 0;

    numAxis_ = 1;
    for (int iter_trigger = 0; iter_trigger < NUM_TRIGGER; ++iter_trigger) {
        if (numAxis_ >= NUM_AXIS)
            break;

        if (iter_trigger == 0) {
            analogInterface_[numAxis_].analogInterfaceX = context_.getInterface(triggerPath);
        } else {
            analogInterface_[numAxis_].analogInterfaceX = context_.getInterface(triggerPath + std::to_string(iter_trigger));
        }

        if (analogInterface_[numAxis_].analogInterfaceX.notEmpty()) {
            analogInterface_[numAxis_].axisIndex = numAxis_;
            analogInterface_[numAxis_].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Trigger;
            analogInterface_[numAxis_].analogInterfaceX.registerCallback(&OSVRTrackedController::controllerTriggerCallback, &analogInterface_[numAxis_]);
            numAxis_++;
        } else {
            analogInterface_[numAxis_].analogInterfaceX.free();
        }
    }

    numAxis_ = 2;
    for (int iter_joystick = 0; iter_joystick < NUM_JOYSTICKS; ++iter_joystick) {
        if (numAxis_ >= NUM_AXIS)
            break;

        if (iter_joystick == 0) {
            analogInterface_[numAxis_].analogInterfaceX = context_.getInterface(joystickPath + "/x");
            analogInterface_[numAxis_].analogInterfaceY = context_.getInterface(joystickPath + "/y");
        } else {
            analogInterface_[numAxis_].analogInterfaceX = context_.getInterface(joystickPath + std::to_string(iter_joystick) + "/x");
            analogInterface_[numAxis_].analogInterfaceY = context_.getInterface(joystickPath + std::to_string(iter_joystick) + "/y");
        }

        bool somethingFound = false;

        if (analogInterface_[numAxis_].analogInterfaceX.notEmpty()) {
            analogInterface_[numAxis_].axisIndex = numAxis_;
            analogInterface_[numAxis_].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Joystick;
            analogInterface_[numAxis_].analogInterfaceX.registerCallback(&OSVRTrackedController::controllerJoystickXCallback, &analogInterface_[numAxis_]);
            somethingFound = true;
        } else {
            analogInterface_[numAxis_].analogInterfaceX.free();
        }

        if (analogInterface_[numAxis_].analogInterfaceY.notEmpty()) {
            analogInterface_[numAxis_].axisIndex = numAxis_;
            analogInterface_[numAxis_].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Joystick;
            analogInterface_[numAxis_].analogInterfaceY.registerCallback(&OSVRTrackedController::controllerJoystickYCallback, &analogInterface_[numAxis_]);
            somethingFound = true;
        } else {
            analogInterface_[numAxis_].analogInterfaceY.free();
        }

        if (somethingFound)
            numAxis_++;
    }

    return vr::VRInitError_None;
}

void OSVRTrackedController::Deactivate()
{
    /// Have to force freeing here
    freeInterfaces();
}

vr::VRControllerState_t OSVRTrackedController::GetControllerState()
{
    // TODO
    vr::VRControllerState_t state;
    state.unPacketNum = 0;
    return state;
}

bool OSVRTrackedController::TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds)
{
    return false;
}

void OSVRTrackedController::freeInterfaces()
{
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }

    for (int iter_axis = 0; iter_axis < NUM_AXIS; ++iter_axis) {
        if (analogInterface_[iter_axis].analogInterfaceX.notEmpty())
            analogInterface_[iter_axis].analogInterfaceX.free();
        if (analogInterface_[iter_axis].analogInterfaceY.notEmpty())
            analogInterface_[iter_axis].analogInterfaceY.free();
    }

    for (int iter_button = 0; iter_button < NUM_BUTTONS; ++iter_button) {
        if (buttonInterface_[iter_button].notEmpty())
            buttonInterface_[iter_button].free();
    }
}

void OSVRTrackedController::controllerTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedController*>(userdata);

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
    //pose.willDriftInYaw = true;
    //pose.shouldApplyHeadModel = true;
    pose.deviceIsConnected = true;

    self->pose_ = pose;

    self->driverHost_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

void OSVRTrackedController::controllerButtonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedController*>(userdata);

    vr::EVRButtonId button_id;
    if ((report->sensor >= 0 && report->sensor <= 7) || (report->sensor >= 32 && report->sensor <= 36)) {
        button_id = static_cast<vr::EVRButtonId>(report->sensor);
    } else if (report->sensor >= 8 && report->sensor <= 12) {
        button_id = static_cast<vr::EVRButtonId>(report->sensor + 24);
    } else {
        return;
    }

    if (OSVR_BUTTON_PRESSED == report->state) {
        self->driverHost_->TrackedDeviceButtonPressed(self->objectId_, button_id, 0);
    } else {
        self->driverHost_->TrackedDeviceButtonUnpressed(self->objectId_, button_id, 0);
    }
}

void OSVRTrackedController::controllerTriggerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* analog_interface = static_cast<AnalogInterface*>(userdata);
    OSVRTrackedController* self = analog_interface->parentController;

    analog_interface->x = report->state;

    vr::VRControllerAxis_t axis_state;
    axis_state.x = static_cast<float>(analog_interface->x);

    self->driverHost_->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
}

void OSVRTrackedController::controllerJoystickXCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* analog_interface = static_cast<AnalogInterface*>(userdata);
    OSVRTrackedController* self = analog_interface->parentController;

    analog_interface->x = report->state;

    vr::VRControllerAxis_t axis_state;
    axis_state.x = static_cast<float>(analog_interface->x);
    axis_state.y = static_cast<float>(analog_interface->y);

    self->driverHost_->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
}

void OSVRTrackedController::controllerJoystickYCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* analog_interface = static_cast<AnalogInterface*>(userdata);
    OSVRTrackedController* self = analog_interface->parentController;

    analog_interface->y = report->state;

    vr::VRControllerAxis_t axis_state;
    axis_state.x = static_cast<float>(analog_interface->x);
    axis_state.y = static_cast<float>(analog_interface->y);

    self->driverHost_->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
}

const char* OSVRTrackedController::GetId()
{
    /// @todo When available, return the actual unique ID of the HMD
    return name_.c_str();
}

void OSVRTrackedController::configure()
{
    configureProperties();
}

void OSVRTrackedController::configureProperties()
{
    // Properties that are unique to TrackedDeviceClass_Controller
    setProperty<int32_t>(vr::Prop_DeviceClass_Int32, deviceClass_);
    setProperty<int32_t>(vr::Prop_Axis0Type_Int32, analogInterface_[0].axisType);
    setProperty<int32_t>(vr::Prop_Axis1Type_Int32, analogInterface_[1].axisType);
    setProperty<int32_t>(vr::Prop_Axis2Type_Int32, analogInterface_[2].axisType);
    setProperty<int32_t>(vr::Prop_Axis3Type_Int32, analogInterface_[3].axisType);
    setProperty<int32_t>(vr::Prop_Axis4Type_Int32, analogInterface_[4].axisType);

    setProperty<int32_t>(vr::Prop_SupportedButtons_Uint64, NUM_BUTTONS);

    setProperty<std::string>(vr::Prop_ModelNumber_String, "OSVR Controller");
    setProperty<std::string>(vr::Prop_SerialNumber_String, name_);
    setProperty<std::string>(vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    //Prop_AttachedDeviceId_String				= 3000,
}

