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
#include "PathTreeUtil.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>

#include <boost/filesystem.hpp>

#include <json/value.h>
#include <json/reader.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <iostream>
#include <exception>

OSVRTrackedController::OSVRTrackedController(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, const std::string& user_driver_config_dir, vr::ETrackedControllerRole controller_role) : OSVRTrackedDevice(context, driver_host, vr::TrackedDeviceClass_Controller, user_driver_config_dir, "OSVRTrackedController"), controllerRole_(controller_role), interfaces_(), controllerState_()
{
    std::string suffix = "";
    if (vr::TrackedControllerRole_LeftHand == controller_role) {
        suffix = "Left";
    } else if (vr::TrackedControllerRole_RightHand == controller_role) {
        suffix = "Right";
    } else {
        suffix = "";
    }
    name_ = "OSVRController" + suffix;

    // Initialize controller state
    controllerState_.unPacketNum = 0;
    controllerState_.ulButtonPressed = 0;
    controllerState_.ulButtonTouched = 0;
    for (size_t i = 0; i < vr::k_unControllerStateAxisCount; ++i) {
        controllerState_.rAxis[i] = { 0.0f, 0.0f };
    }

    // Initialize axis callback data array and axis type array
    for (size_t i = 0; i < vr::k_unControllerStateAxisCount; ++i) {
        axisCallbackData_[i] = { this, static_cast<uint32_t>(i), AxisCallbackData::AxisDirection::X };
        axisTypes_[i] = vr::k_eControllerAxis_None;
    }

    // Initialize button callback data array
    for (size_t i = 0; i < vr::k_EButton_Max; ++i) {
        buttonCallbackData_[i] = { this, static_cast<vr::EVRButtonId>(63) };
    }

    // These properties are required priot to Activate()
    setProperty<int32_t>(vr::Prop_DeviceClass_Int32, deviceClass_);
    setProperty<int32_t>(vr::Prop_ControllerRoleHint_Int32, controllerRole_);
    setProperty<std::string>(vr::Prop_ModelNumber_String, "OSVR Controller");
    setProperty<std::string>(vr::Prop_SerialNumber_String, name_);
}

OSVRTrackedController::~OSVRTrackedController()
{
    // do nothing
}

vr::EVRInitError OSVRTrackedController::Activate(uint32_t object_id)
{
    OSVR_LOG(trace) << name_ << ": Activate() called.";

    OSVRTrackedDevice::Activate(object_id);

    const std::time_t wait_time = 5; // wait up to 5 seconds for init

    freeInterfaces();

    // Ensure context is fully started up
    OSVR_LOG(info) << name_ << ": Waiting for the context to fully start up...";
    const auto start_time = std::time(nullptr);
    while (!context_.checkStatus()) {
        context_.update();
        if (std::time(nullptr) > start_time + wait_time) {
            OSVR_LOG(warn) << name_ << ": Context startup timed out!";
            return vr::VRInitError_Driver_Failed;
        }
    }
    OSVR_LOG(info) << name_ << ": Context connected.";

    configure();

    return vr::VRInitError_None;
}

void OSVRTrackedController::Deactivate()
{
    freeInterfaces();
}

vr::VRControllerState_t OSVRTrackedController::GetControllerState()
{
    return controllerState_;
}

bool OSVRTrackedController::TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds)
{
    return false;
}

void OSVRTrackedController::freeInterfaces()
{
    for (auto& interface : interfaces_) {
        interface.free();
    }
}

const char* OSVRTrackedController::GetId()
{
    /// @todo When available, return the actual unique ID of the HMD
    return name_.c_str();
}

void OSVRTrackedController::configure()
{
    configureController();
    configureProperties();
}

void OSVRTrackedController::configureController()
{
    OSVR_LOG(trace) << name_ << ": Configuring controller...";

    // Default controller configuration
    // TODO

    // Read the controller config (if it exists) and set the OSVR paths
    const auto controller_config = settings_->getSetting<std::string>("controllerConfig", "");
    if (controller_config.empty()) {
        OSVR_LOG(info) << name_ << ": A controller configuration file must be provided to use tracked controllers.";
        return;
    }

    // Read the controller config file
    namespace fs = boost::filesystem;
    //const auto controller_config_filename  = (fs::path(userDriverConfigDir_) / fs::path(controller_config)).generic_string();
    const auto controller_config_filename = userDriverConfigDir_ + "/" + controller_config;

    {
        OSVR_LOG(debug) << name_ << ": userDriverConfigDir_ = [" << std::string(userDriverConfigDir_) << "].";
        OSVR_LOG(debug) << name_ << ": controller_config = [" << std::string(controller_config) << "].";
        OSVR_LOG(debug) << name_ << ": controller_config_filename = [" << std::string(controller_config_filename) << "].";
    }

    /*
    // FIXME boost::fs causing crashes?
    // Check if file exists
    auto boost_path = fs::path(controller_config_filename);
    if (!fs::exists(boost_path)) {
        OSVR_LOG(err) << name_ << ": Error: The controller map file [" << controller_config_filename << "] doesn't exist.";
        return;
    }

    if (!fs::is_regular_file(boost_path)) {
        OSVR_LOG(err) << name_ << ": Error: The controller map file [" << controller_config_filename << "] isn't a readable file.";
        return;
    }
    */

    // Open the file into a stream
    OSVR_LOG(trace) << name_ << ": Opening the controller map file [" << controller_config_filename << "]...";
    std::ifstream config_stream { controller_config_filename };

    // Parse the file
    auto root = Json::Value();
    auto reader = Json::Reader();
    OSVR_LOG(trace) << name_ << ": Parsing the controller map file [" << controller_config << "]...";
    auto parsed_okay = reader.parse(config_stream, root, false);
    if (!parsed_okay) {
        OSVR_LOG(err) << name_ << ": Error parsing " << controller_config_filename << ": " << reader.getFormattedErrorMessages();
        return;
    }

    if (!root.isMember("controllers")) {
        OSVR_LOG(err) << name_ << ": Error parsing " << controller_config_filename << ": Missing 'controllers' entry.";
        return;
    }

    // Determine which part of the file to read
    std::string controller_hand = "left";
    if (vr::TrackedControllerRole_RightHand == controllerRole_) {
        controller_hand = "right";
    }

    const auto controller_root = root["controllers"][controller_hand];

    const auto base_path = controller_root.get("basePath", "/me/hands/" + controller_hand).asString();
    OSVR_LOG(trace) << name_ << ": Using controller base path: " << base_path;

    configureTracker(controller_root, base_path);
    configureAxes(controller_root, base_path);
    configureButtons(controller_root, base_path);
}

void OSVRTrackedController::configureTracker(const Json::Value& controller_root, const std::string& base_path)
{
    OSVR_LOG(trace) << name_ << ": Configuring controller tracker...";

    if (!controller_root.isMember("tracker")) {
        OSVR_LOG(info) << name_ << ": No tracker for this controller.";
        return;
    }

    const auto tracker_path = controller_root["tracker"].asString();
    const auto abs_tracker_path = resolvePath(tracker_path, base_path);
    auto tracker_interface = context_.getInterface(abs_tracker_path);
    interfaces_.push_back(tracker_interface);
    if (tracker_interface.notEmpty()) {
        OSVR_LOG(debug) << name_ << ": Tracker interface [" << abs_tracker_path << "] added.";
        tracker_interface.registerCallback(&OSVRTrackedController::trackerCallback, this);
    } else {
        OSVR_LOG(warn) << name_ << ": Tracker interface [" << abs_tracker_path << "] has not been initialized.";
    }
}

void OSVRTrackedController::configureAxes(const Json::Value& controller_root, const std::string& base_path)
{
    OSVR_LOG(trace) << name_ << ": Configuring axes (joysticks and triggers)...";

    if (!controller_root.isMember("axes")) {
        OSVR_LOG(info) << name_ << ": There are no axes for this controller.";
        return;
    }

    const auto axes = controller_root["axes"];
    uint32_t index = 0;
    for (const auto& axis : axes) {
        if (index > vr::k_unControllerStateAxisCount - 1) {
            OSVR_LOG(err) << name_ << ": SteamVR only supports up to " << vr::k_unControllerStateAxisCount << " axes at this time. The extraneous axes will be ignored.";
            break;
        }

        const auto type_str = axis.get("type", "joystick").asString();
        auto type = vr::k_eControllerAxis_None;
        if ("joystick" == type_str) {
            type = vr::k_eControllerAxis_Joystick;
        } else if ("trigger" == type_str) {
            type = vr::k_eControllerAxis_Trigger;
        } else if ("trackpad" == type_str) {
            type = vr::k_eControllerAxis_TrackPad;
        } else {
            type = vr::k_eControllerAxis_None;
        }
        axisTypes_[index] = type;

        const auto x_path = resolvePath(axis.get("type", "x").asString(), base_path);
        const auto y_path = resolvePath(axis.get("type", "y").asString(), base_path);
        const auto button_path = resolvePath(axis.get("button", "joystick/button").asString(), base_path);

        {
            axisCallbackData_[index] = { this, index, AxisCallbackData::AxisDirection::X };
            auto interface_x = context_.getInterface(x_path);
            if (interface_x.notEmpty()) {
                OSVR_LOG(debug) << name_ << ": Analog interface [" << x_path << "] added.";
                interface_x.registerCallback(&OSVRTrackedController::axisCallback, &axisCallbackData_[index]);
            } else {
                OSVR_LOG(warn) << name_ << ": Analog interface [" << x_path << "] has not been initialized.";
            }
        }

        {
            axisCallbackData_[index] = { this, index, AxisCallbackData::AxisDirection::Y };
            auto interface_y = context_.getInterface(y_path);
            if (interface_y.notEmpty()) {
                OSVR_LOG(debug) << name_ << ": Analog interface [" << y_path << "] added.";
                interface_y.registerCallback(&OSVRTrackedController::axisCallback, &axisCallbackData_[index]);
            } else {
                OSVR_LOG(warn) << name_ << ": Analog interface [" << y_path << "] has not been initialized.";
            }
        }

        {
            const auto button_id = static_cast<vr::EVRButtonId>(vr::k_EButton_Axis0 + index);
            const auto button_callback_data = ButtonCallbackData { this, button_id };
            const auto full_button_path = resolvePath(button_path, base_path);
            auto interface_button = context_.getInterface(full_button_path);
            if (interface_button.notEmpty()) {
                OSVR_LOG(debug) << name_ << ": Button interface [" << full_button_path << "] added.";
                interface_button.registerCallback(&OSVRTrackedController::buttonCallback, &buttonCallbackData_[numButtons_ - 1]);
                numButtons_++;
            } else {
                OSVR_LOG(warn) << name_ << ": Button interface [" << full_button_path << "] has not been initialized.";
            }
        }

        index++;
    }
}

void OSVRTrackedController::configureButtons(const Json::Value& controller_root, const std::string& base_path)
{
    OSVR_LOG(trace) << name_ << ": Configuring buttons for this controller.";

    if (!controller_root.isMember("buttons")) {
        OSVR_LOG(info) << name_ << ": There are no buttons configured for this controller.";
        return;
    }

    for (const auto& button_name : buttonNames_) {
        if (!controller_root["buttons"].isMember(button_name))
            continue;

        const auto button_id = getButtonId(button_name);
        const auto button_callback_data = ButtonCallbackData { this, button_id };
        const auto button_path = controller_root["buttons"][button_name].asString();
        const auto full_button_path = resolvePath(button_path, base_path);
        auto interface_button = context_.getInterface(full_button_path);
        if (interface_button.notEmpty()) {
            OSVR_LOG(debug) << name_ << ": Button interface [" << full_button_path << "] added.";
            interface_button.registerCallback(&OSVRTrackedController::buttonCallback, &buttonCallbackData_[numButtons_ - 1]);
            numButtons_++;
        } else {
            OSVR_LOG(warn) << name_ << ": Button interface [" << full_button_path << "] has not been initialized.";
        }
    }
}

void OSVRTrackedController::trackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
    if (!userdata || !report)
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

void OSVRTrackedController::axisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto axis_callback_data = static_cast<AxisCallbackData*>(userdata);
    auto self = static_cast<OSVRTrackedController*>(axis_callback_data->controller);
    auto index = axis_callback_data->index;
    auto& axis = self->controllerState_.rAxis[index];

    if (AxisCallbackData::AxisDirection::X == axis_callback_data->direction) {
        axis.x = static_cast<float>(report->state);
    } else if (AxisCallbackData::AxisDirection::Y == axis_callback_data->direction) {
        axis.y = static_cast<float>(report->state);
    } else {
        // Uh oh!
    }

    self->controllerState_.unPacketNum++;
    self->driverHost_->TrackedDeviceAxisUpdated(self->objectId_, index, axis);
}

void OSVRTrackedController::buttonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report)
{
    if (!userdata)
        return;

    auto button_callback_data = static_cast<ButtonCallbackData*>(userdata);
    auto self = static_cast<OSVRTrackedController*>(button_callback_data->controller);

    self->controllerState_.unPacketNum++;
    if (OSVR_BUTTON_PRESSED == report->state) {
        self->driverHost_->TrackedDeviceButtonPressed(self->objectId_, button_callback_data->button_id, 0.0);
    } else {
        self->driverHost_->TrackedDeviceButtonUnpressed(self->objectId_, button_callback_data->button_id, 0.0);
    }
}

void OSVRTrackedController::configureProperties()
{
    // Properties that are unique to TrackedDeviceClass_Controller
    setProperty<int32_t>(vr::Prop_DeviceClass_Int32, deviceClass_);
    setProperty<int32_t>(vr::Prop_Axis0Type_Int32, axisTypes_[0]);
    setProperty<int32_t>(vr::Prop_Axis1Type_Int32, axisTypes_[1]);
    setProperty<int32_t>(vr::Prop_Axis2Type_Int32, axisTypes_[2]);
    setProperty<int32_t>(vr::Prop_Axis3Type_Int32, axisTypes_[3]);
    setProperty<int32_t>(vr::Prop_Axis4Type_Int32, axisTypes_[4]);

    setProperty<uint64_t>(vr::Prop_SupportedButtons_Uint64, numButtons_);

    setProperty<std::string>(vr::Prop_ModelNumber_String, "OSVR Controller");
    setProperty<std::string>(vr::Prop_SerialNumber_String, name_);
    setProperty<std::string>(vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");

    //Prop_AttachedDeviceId_String				= 3000,
}

vr::EVRButtonId OSVRTrackedController::getButtonId(const std::string& key) const
{
    if (key == "system") {
        return vr::k_EButton_System;
    } else if (key == "menu") {
        return vr::k_EButton_ApplicationMenu;
    } else if (key == "grip") {
        return vr::k_EButton_Grip;
    } else if (key == "left") {
        return vr::k_EButton_DPad_Left;
    } else if (key == "up") {
        return vr::k_EButton_DPad_Up;
    } else if (key == "right") {
        return vr::k_EButton_DPad_Right;
    } else if (key == "down") {
        return vr::k_EButton_DPad_Down;
    } else if (key == "a") {
        return vr::k_EButton_A;
    } else if (key == "axis0") {
        return vr::k_EButton_Axis0;
    } else if (key == "axis1") {
        return vr::k_EButton_Axis1;
    } else if (key == "axis2") {
        return vr::k_EButton_Axis2;
    } else if (key == "axis3") {
        return vr::k_EButton_Axis3;
    } else if (key == "axis4") {
        return vr::k_EButton_Axis4;
    } else {
        OSVR_LOG(err) << name_ << ": Invalid button name [" << key << "].";
        return static_cast<vr::EVRButtonId>(63);
    }
}
