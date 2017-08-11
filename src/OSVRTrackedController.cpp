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
// Joystick support
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Thumbstick, "/controller/left/joystick/button"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Shoulder, "/controller/left/bumper"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::SpecialLeft, "/controller/left/middle"),

OSVRTrackedController::OSVRTrackedController(osvr::clientkit::ClientContext& context, int controller_index) : 
    OSVRTrackedDevice(context, vr::TrackedDeviceClass_Controller, "OSVR controller"+std::to_string(controller_index)), controllerIndex_(controller_index)
{
    OSVR_LOG(trace) << "OSVRTrackedController::constructor() called.  name = " << name_ <<"\n";
}

OSVRTrackedController::~OSVRTrackedController()
{
    // DO NOTHING
}

vr::EVRInitError OSVRTrackedController::Activate(uint32_t object_id)
{
    OSVRTrackedDevice::Activate(object_id);
    const std::time_t wait_time = 5; // wait up to 5 seconds for init

    freeInterfaces();

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
    std::string trackpadPath;
    std::string batteryPath;
    if (controllerIndex_ == vr::TrackedControllerRole_LeftHand) {
        trackerPath  = "/me/hands/left";
        buttonPath   = "/controller/left/";
        triggerPath  = "/controller/left/trigger";
        joystickPath = "/controller/left/joystick";
        trackpadPath = "/controller/left/trackpad";
	batteryPath  = "/controller/left/battery";
    } else if (controllerIndex_ == vr::TrackedControllerRole_RightHand) {
        trackerPath  = "/me/hands/right";
        buttonPath   = "/controller/right/";
        triggerPath  = "/controller/right/trigger";
        joystickPath = "/controller/right/joystick";
        trackpadPath = "/controller/right/trackpad";
	batteryPath  = "/controller/right/battery";
    } else {
        buttonPath = "/controller" + std::to_string(controllerIndex_) + "/";
        triggerPath = "/controller" + std::to_string(controllerIndex_) + "/trigger";
        joystickPath = "/controller" + std::to_string(controllerIndex_) + "/joystick";
    }

    if (!trackerPath.empty()) {
        trackerInterface_ = context_.getInterface(trackerPath);
	if(trackerInterface_.notEmpty()){
            OSVR_LOG(trace) << "OSVRTrackedController::Activate() tracker Interface is empty "<<trackerPath;
            trackerInterface_.registerCallback(&OSVRTrackedController::controllerTrackerCallback, this);
	}else{
	    trackerInterface_.free();
	}
    }else{
	OSVR_LOG(trace) << "OSVRTrackedController::Activate() tracker path is empty "<<trackerPath;
    }


    // BUTTONS
    int button_id = 0;
    registerButton(button_id++,      buttonPath+"system", vr::k_EButton_System);
    registerButton(button_id++,      buttonPath+"menu", vr::k_EButton_ApplicationMenu);
    registerButton(button_id++,      buttonPath+"grip",vr::k_EButton_Grip);
    registerButton(button_id++,      buttonPath+"trackpad/button",vr::k_EButton_SteamVR_Touchpad);
    registerButtonTouch(button_id++, buttonPath+"trackpad/touch",vr::k_EButton_SteamVR_Touchpad);
    registerButton(button_id++,      buttonPath+"trigger/button", vr::k_EButton_SteamVR_Trigger);

    // TRACKPAD
    int axis_id = 0;
    registerTrackpad(axis_id++, trackpadPath);

    // TRIGGERS
    registerTrigger(axis_id++, triggerPath);

    // BATTERY
    registerBattery(batteryPath);

    // added in for testing not sure if its correct or not.
    configure();

    return vr::VRInitError_None;
}

void OSVRTrackedController::Deactivate()
{
    /// Have to force freeing here
    freeInterfaces();
}

vr::VRControllerState_t OSVRTrackedController::GetControllerState()
{
    OSVR_LOG(trace) << "OSVRTrackedController::GetControllerState().";
    // TODO
    vr::VRControllerState_t state;
    state.unPacketNum = 0;
    return state;
}

// Currently not used
bool OSVRTrackedController::TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds)
{
    OSVR_LOG(trace) << "OSVRTrackedController::TriggerHapticPulse().";
    return false;
}

void OSVRTrackedController::freeInterfaces()
{
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }

    for (int iter_axis = 0; iter_axis < NUM_AXIS; iter_axis++) {
        if (analogInterface_[iter_axis].analogInterfaceX.notEmpty())
            analogInterface_[iter_axis].analogInterfaceX.free();
        if (analogInterface_[iter_axis].analogInterfaceY.notEmpty())
            analogInterface_[iter_axis].analogInterfaceY.free();
    }

    for (int iter_button = 0; iter_button < NUM_BUTTONS; iter_button++) {
        if (buttonInterface_[iter_button].buttonInterface.notEmpty())
            buttonInterface_[iter_button].buttonInterface.free();
    }

    if (batteryInterface.interface.notEmpty())
	batteryInterface.interface.free();
}

inline vr::HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
{
    vr::HmdQuaternion_t quat;
    quat.w = w;
    quat.x = x;
    quat.y = y;
    quat.z = z;
    OSVR_LOG(trace) << "OSVRTrackedController::HmdQuat().";
    return quat;
}

void OSVRTrackedController::controllerTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedController*>(userdata);

    vr::DriverPose_t pose = { 0 };
    pose.poseTimeOffset = 0; // close enough

    Eigen::Vector3d::Map(pose.vecWorldFromDriverTranslation) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecDriverFromHeadTranslation) = Eigen::Vector3d::Zero();

    map(pose.qWorldFromDriverRotation) = Eigen::Quaterniond::Identity();
    map(pose.qDriverFromHeadRotation) = Eigen::Quaterniond::Identity();
    //pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    //pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

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

    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(self->objectId_, self->pose_,sizeof(self->pose_));
}

void OSVRTrackedController::controllerButtonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report)
{
    if (!userdata)
        return;

    auto* button_interface = static_cast<ButtonInterface*>(userdata);
    OSVRTrackedController* self = button_interface->parentController;
    if(!self){
	return;
    }

    // error checking
    if((button_interface->button_id >=0 && button_interface->button_id <= 7) || (button_interface->button_id >= 31 && button_interface->button_id <= 36)){
	if (OSVR_BUTTON_PRESSED == report->state) {
	    vr::VRServerDriverHost()->TrackedDeviceButtonPressed(self->objectId_, button_interface->button_id, 0);
	} else {
	    vr::VRServerDriverHost()->TrackedDeviceButtonUnpressed(self->objectId_, button_interface->button_id, 0);
	}
    }
}

void OSVRTrackedController::controllerButtonTouchCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report)
{
    if (!userdata)
        return;

    auto* button_interface = static_cast<ButtonInterface*>(userdata);
    OSVRTrackedController* self = button_interface->parentController;
    if(!self){
	return;
    }

    // error checking
    if((button_interface->button_id >=0 && button_interface->button_id <= 7) || (button_interface->button_id >= 31 && button_interface->button_id <= 36)){
	if (OSVR_BUTTON_PRESSED == report->state) {
	    vr::VRServerDriverHost()->TrackedDeviceButtonTouched(self->objectId_, button_interface->button_id, 0);
	} else {
	    vr::VRServerDriverHost()->TrackedDeviceButtonUntouched(self->objectId_, button_interface->button_id, 0);
	}
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
    axis_state.y = 0;

    vr::VRServerDriverHost()->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
}

void OSVRTrackedController::controllerBatteryCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* batteryInterface                  = static_cast<BatteryInterface*>(userdata);
    OSVRTrackedController* self             = batteryInterface->parentController;
    vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(self->objectId_);
    vr::VRProperties()->SetFloatProperty(container, vr::Prop_DeviceBatteryPercentage_Float, report->state);
}

void OSVRTrackedController::controllerXAxisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* analog_interface = static_cast<AnalogInterface*>(userdata);
    OSVRTrackedController* self = analog_interface->parentController;

    analog_interface->x = report->state;

    vr::VRControllerAxis_t axis_state;
    axis_state.x = static_cast<float>(analog_interface->x);
    axis_state.y = static_cast<float>(analog_interface->y);

    vr::VRServerDriverHost()->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
}

void OSVRTrackedController::controllerYAxisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
    if (!userdata)
        return;

    auto* analog_interface = static_cast<AnalogInterface*>(userdata);
    OSVRTrackedController* self = analog_interface->parentController;

    analog_interface->y = report->state;

    vr::VRControllerAxis_t axis_state;
    axis_state.x = static_cast<float>(analog_interface->x);
    axis_state.y = static_cast<float>(analog_interface->y);

    vr::VRServerDriverHost()->TrackedDeviceAxisUpdated(self->objectId_, analog_interface->axisIndex, axis_state);
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

    propertyContainer_ = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->objectId_);
    // Additional properties from nolo's osvr tracked device
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_TrackingSystemName_String, "NoloVR");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ManufacturerName_String, "LYRobotix");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_TrackingFirmwareVersion_String, "0.1.0");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_HardwareRevision_String, "0.1.0");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_AllWirelessDongleDescriptions_String, "");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ConnectedWirelessDongle_String, "");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_Firmware_ManualUpdateURL_String, "");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_Firmware_ProgrammingTarget_String, "");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_DriverVersion_String, "0.1.0");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_AttachedDeviceId_String, "3000");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ModeLabel_String, "");
    //vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_WillDriftInYaw_Bool, true);
    //vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceIsWireless_Bool, false);
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_DeviceClass_Int32,       static_cast<int32_t>(deviceClass_));
    /*
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_Axis0Type_Int32,         static_cast<int32_t>(analogInterface_[0].axisType));
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_Axis1Type_Int32,         static_cast<int32_t>(analogInterface_[1].axisType));
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_Axis2Type_Int32,         static_cast<int32_t>(analogInterface_[2].axisType));
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_Axis3Type_Int32,         static_cast<int32_t>(analogInterface_[3].axisType));
    vr::VRProperties()->SetInt32Property(propertyContainer_,  vr::Prop_Axis4Type_Int32,         static_cast<int32_t>(analogInterface_[4].axisType));
    vr::VRProperties()->SetUint64Property(propertyContainer_, vr::Prop_SupportedButtons_Uint64, static_cast<int32_t>(NUM_BUTTONS));
    */
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ModelNumber_String,      "OSVR Controller");
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_SerialNumber_String,     name_.c_str());
    // set the vive controller at the default.
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_RenderModelName_String,  "vr_controller_vive_1_5");
    //vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_RenderModelName_String, settings_->getSetting<std::string>("cameraRenderModel", "").c_str());
}

/**
 * Registers a button based on the path and id.
 * @param id the id of the button which is used for indexing an array.
 * @param path the complete path to the button.
 */
void OSVRTrackedController::registerButton(int id, const std::string& path, vr::EVRButtonId button_id){
    // check bounds
    if(id < 0 || id >= NUM_BUTTONS){
	return;
    }

    buttonInterface_[id].buttonInterface = context_.getInterface(path);
    if (buttonInterface_[id].buttonInterface.notEmpty()) {
	buttonInterface_[id].button_id        = button_id;
	buttonInterface_[id].parentController = this;
	buttonInterface_[id].buttonInterface.registerCallback( &OSVRTrackedController::controllerButtonCallback, &buttonInterface_[id]);
    }else{
	buttonInterface_[id].buttonInterface.free();
    } 
}

/**
 * Registers a touch button based on the path and id.
 * @param id the id of the button which is used for indexing an array.
 * @param path the complete path to the button.
 */
void OSVRTrackedController::registerButtonTouch(int id, const std::string& path, vr::EVRButtonId button_id){
    // check bounds
    if(id < 0 || id >= NUM_BUTTONS){
	return;
    }

    buttonInterface_[id].buttonInterface = context_.getInterface(path);
    if (buttonInterface_[id].buttonInterface.notEmpty()) {
	buttonInterface_[id].button_id        = button_id;
	buttonInterface_[id].parentController = this;
	buttonInterface_[id].buttonInterface.registerCallback( &OSVRTrackedController::controllerButtonTouchCallback, &buttonInterface_[id]);
    }else{
	buttonInterface_[id].buttonInterface.free();
    } 
}

void OSVRTrackedController::registerBattery(const std::string& path){
    batteryInterface.interface = context_.getInterface(path);
    if (batteryInterface.interface.notEmpty()) {
	batteryInterface.parentController = this;
        batteryInterface.interface.registerCallback(&OSVRTrackedController::controllerBatteryCallback, &batteryInterface);
        propertyContainer_                    = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->objectId_);
        vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceProvidesBatteryStatus_Bool,true);
    } else {
        batteryInterface.interface.free();
    }
}

void OSVRTrackedController::registerTrigger(int id, const std::string& path){
    // check bounds
    if(id < 0 || id >= NUM_AXIS){
	return;
    }
    analogInterface_[id].analogInterfaceX = context_.getInterface(path);
    if (analogInterface_[id].analogInterfaceX.notEmpty()) {
        analogInterface_[id].axisIndex        = id;
        analogInterface_[id].axisType         = vr::EVRControllerAxisType::k_eControllerAxis_Trigger;
	analogInterface_[id].parentController = this;
        analogInterface_[id].analogInterfaceX.registerCallback(&OSVRTrackedController::controllerTriggerCallback, &analogInterface_[id]);
        propertyContainer_                    = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->objectId_);
        vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_Axis1Type_Int32, static_cast<int32_t>(analogInterface_[id].axisType));
    } else {
        analogInterface_[id].analogInterfaceX.free();
    }
}

void OSVRTrackedController::registerTrackpad(int id, const std::string& path){
    // check bounds
    if(id < 0 || id >= NUM_AXIS){
	return;
    }
    analogInterface_[id].analogInterfaceX = context_.getInterface(path + "/x");
    analogInterface_[id].analogInterfaceY = context_.getInterface(path + "/y");

    if (analogInterface_[id].analogInterfaceX.notEmpty()) {
        analogInterface_[id].axisIndex        = id;
        analogInterface_[id].axisType         = vr::EVRControllerAxisType::k_eControllerAxis_TrackPad;
	analogInterface_[id].parentController = this;
	analogInterface_[id].analogInterfaceX.registerCallback(&OSVRTrackedController::controllerXAxisCallback, &analogInterface_[id]);
        propertyContainer_                    = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->objectId_);
        vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_Axis0Type_Int32, static_cast<int32_t>(analogInterface_[id].axisType));
    } else {
	analogInterface_[id].analogInterfaceX.free();
    }

    if (analogInterface_[id].analogInterfaceY.notEmpty()) {
        analogInterface_[id].axisIndex        = id;
        analogInterface_[id].axisType         = vr::EVRControllerAxisType::k_eControllerAxis_TrackPad;
	analogInterface_[id].parentController = this;
        analogInterface_[id].analogInterfaceY.registerCallback(&OSVRTrackedController::controllerYAxisCallback, &analogInterface_[id]);
        propertyContainer_                    = vr::VRProperties()->TrackedDeviceToPropertyContainer(this->objectId_);
        vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_Axis0Type_Int32, static_cast<int32_t>(analogInterface_[id].axisType));
    } else {
        analogInterface_[id].analogInterfaceY.free();
    }
}
