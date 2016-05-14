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
#include "OSVRTrackedDeviceController.h"

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

// TODO:
// Trackpad
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Thumbstick, "/controller/left/joystick/button"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::MotionController_Left_Shoulder, "/controller/left/bumper"),
// OSVRButton(OSVR_BUTTON_TYPE_DIGITAL, FGamepadKeyNames::SpecialLeft, "/controller/left/middle"),

OSVRTrackedDeviceController::OSVRTrackedDeviceController(int controllerIndex, osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log)
	: m_ControllerIndex(controllerIndex), m_Context(context), driver_host_(driver_host), logger_(driver_log), pose_(), deviceClass_(vr::TrackedDeviceClass_Controller)
{
	m_ControllerName = "OSVRController" + std::to_string(controllerIndex);

	m_NumAxis = 0;
	for (int iter_axis = 0; iter_axis < NUM_AXIS; iter_axis++)
	{
		m_AnalogInterface[iter_axis].parentController = this;
		m_AnalogInterface[iter_axis].axisType = vr::EVRControllerAxisType::k_eControllerAxis_None;
	}
}

OSVRTrackedDeviceController::~OSVRTrackedDeviceController()
{
	vr::IDriverLog* logger_ = nullptr;
	vr::IServerDriverHost* driver_host_ = nullptr;
}

vr::EVRInitError OSVRTrackedDeviceController::Activate(uint32_t object_id)
{
	const std::time_t waitTime = 5; // wait up to 5 seconds for init

	FreeInterfaces();
	m_NumAxis = 0;

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

	// Register callbacks
	std::string trackerPath;
	std::string buttonPath;
	std::string triggerPath;
	std::string joystickPath;
	if (m_ControllerIndex == 0)
	{
		trackerPath = "/me/hands/left";
		buttonPath = "/controller/left/";
		triggerPath = "/controller/left/trigger";
		joystickPath = "/controller/left/joystick";
	}
	else if (m_ControllerIndex == 1)
	{
		trackerPath = "/me/hands/right";
		buttonPath = "/controller/right/";
		triggerPath = "/controller/right/trigger";
		joystickPath = "/controller/right/joystick";
	}
	else
	{
		buttonPath = "/controller" + std::to_string(m_ControllerIndex) + "/";
		triggerPath = "/controller" + std::to_string(m_ControllerIndex) + "/trigger";
		joystickPath = "/controller" + std::to_string(m_ControllerIndex) + "/joystick";
	}

	if (!trackerPath.empty())
	{
		m_TrackerInterface = m_Context.getInterface(trackerPath);
		m_TrackerInterface.registerCallback(&OSVRTrackedDeviceController::ControllerTrackerCallback, this);
	}

	for (int iter_button = 0; iter_button < NUM_BUTTONS; iter_button++)
	{
		m_ButtonInterface[iter_button] = m_Context.getInterface(buttonPath + std::to_string(iter_button));
		if (m_ButtonInterface[iter_button].notEmpty())
			m_ButtonInterface[iter_button].registerCallback(&OSVRTrackedDeviceController::ControllerButtonCallback, this);
		else
			m_ButtonInterface[iter_button].free();
	}

	// TODO: ADD TOUCHPAD PART HERE
	m_NumAxis = 0;

	m_NumAxis = 1;
	for (int iter_trigger = 0; iter_trigger < NUM_TRIGGER; iter_trigger++)
	{
		if (m_NumAxis >= NUM_AXIS)
			break;

		if (iter_trigger == 0)
			m_AnalogInterface[m_NumAxis].analogInterfaceX = m_Context.getInterface(triggerPath);
		else
			m_AnalogInterface[m_NumAxis].analogInterfaceX = m_Context.getInterface(triggerPath + std::to_string(iter_trigger));

		if (m_AnalogInterface[m_NumAxis].analogInterfaceX.notEmpty())
		{
			m_AnalogInterface[m_NumAxis].axisIndex = m_NumAxis;
			m_AnalogInterface[m_NumAxis].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Trigger;
			m_AnalogInterface[m_NumAxis].analogInterfaceX.registerCallback(&OSVRTrackedDeviceController::ControllerTriggerCallback, &m_AnalogInterface[m_NumAxis]);
			m_NumAxis++;
		}
		else
			m_AnalogInterface[m_NumAxis].analogInterfaceX.free();
	}

	m_NumAxis = 2;
	for (int iter_joystick = 0; iter_joystick < NUM_JOYSTICKS; iter_joystick++)
	{
		if (m_NumAxis >= NUM_AXIS)
			break;

		if (iter_joystick == 0)
		{
			m_AnalogInterface[m_NumAxis].analogInterfaceX = m_Context.getInterface(joystickPath + "/x");
			m_AnalogInterface[m_NumAxis].analogInterfaceY = m_Context.getInterface(joystickPath + "/y");
		}
		else
		{
			m_AnalogInterface[m_NumAxis].analogInterfaceX = m_Context.getInterface(joystickPath + std::to_string(iter_joystick) + "/x");
			m_AnalogInterface[m_NumAxis].analogInterfaceY = m_Context.getInterface(joystickPath + std::to_string(iter_joystick) + "/y");
		}
		
		bool somethingFound = false;

		if (m_AnalogInterface[m_NumAxis].analogInterfaceX.notEmpty())
		{
			m_AnalogInterface[m_NumAxis].axisIndex = m_NumAxis;
			m_AnalogInterface[m_NumAxis].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Joystick;
			m_AnalogInterface[m_NumAxis].analogInterfaceX.registerCallback(&OSVRTrackedDeviceController::ControllerJoystickXCallback, &m_AnalogInterface[m_NumAxis]);
			somethingFound = true;
		}
		else
		{
			m_AnalogInterface[m_NumAxis].analogInterfaceX.free();
		}

		if (m_AnalogInterface[m_NumAxis].analogInterfaceY.notEmpty())
		{
			m_AnalogInterface[m_NumAxis].axisIndex = m_NumAxis;
			m_AnalogInterface[m_NumAxis].axisType = vr::EVRControllerAxisType::k_eControllerAxis_Joystick;
			m_AnalogInterface[m_NumAxis].analogInterfaceY.registerCallback(&OSVRTrackedDeviceController::ControllerJoystickYCallback, &m_AnalogInterface[m_NumAxis]);
			somethingFound = true;
		}
		else
		{
			m_AnalogInterface[m_NumAxis].analogInterfaceY.free();
		}

		if (somethingFound)
			m_NumAxis++;
	}

	return vr::VRInitError_None;
}

void OSVRTrackedDeviceController::Deactivate()
{
	/// Have to force freeing here
	FreeInterfaces();
}

void OSVRTrackedDeviceController::PowerOff()
{
	// FIXME Implement
}

void* OSVRTrackedDeviceController::GetComponent(const char* component_name_and_version)
{
	if (!strcasecmp(component_name_and_version, vr::IVRControllerComponent_Version)) {
		return (vr::IVRControllerComponent*)this;
	}

	// Override this to add a component to a driver
	return NULL;
}

void OSVRTrackedDeviceController::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
	// TODO
	// make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
}

/** Gets the current state of a controller. */
vr::VRControllerState_t OSVRTrackedDeviceController::GetControllerState()
{
	//logger_->Log("ControllerState requested");
	vr::VRControllerState_t state;
	state.unPacketNum = 0;
	return state;
}

/** Returns a uint64 property. If the property is not available this function will return 0. */
bool OSVRTrackedDeviceController::TriggerHapticPulse(uint32_t unAxisId, uint16_t usPulseDurationMicroseconds)
{
	return 0;
}

vr::DriverPose_t OSVRTrackedDeviceController::GetPose()
{
	return pose_;
}

bool OSVRTrackedDeviceController::GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

	switch (prop) {
	case vr::Prop_WillDriftInYaw_Bool: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
		break;
	case vr::Prop_ContainsProximitySensor_Bool: // TODO
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

float OSVRTrackedDeviceController::GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

	switch (prop) {
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

int32_t OSVRTrackedDeviceController::GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

	switch (prop) {
	case vr::Prop_DeviceClass_Int32:
		if (error)
			*error = vr::TrackedProp_Success;
		return deviceClass_;
	case vr::Prop_Axis0Type_Int32:
		//if (m_NumAxis > 0)
		//{
			if (error)
				*error = vr::TrackedProp_Success;
			return m_AnalogInterface[0].axisType;
		//}
		//else
		//{
		//	if (error)
		//		*error = vr::TrackedProp_ValueNotProvidedByDevice;
		//	return default_value;
		//}
	case vr::Prop_Axis1Type_Int32:
		//if (m_NumAxis > 1)
		//{
			if (error)
				*error = vr::TrackedProp_Success;
			return m_AnalogInterface[1].axisType;
		//}
		//else
		//{
		//	if (error)
		//		*error = vr::TrackedProp_ValueNotProvidedByDevice;
		//	return default_value;
		//}
	case vr::Prop_Axis2Type_Int32:
		//if (m_NumAxis > 2)
		//{
			if (error)
				*error = vr::TrackedProp_Success;
			return m_AnalogInterface[2].axisType;
		//}
		//else
		//{
		//	if (error)
		//		*error = vr::TrackedProp_ValueNotProvidedByDevice;
		//	return default_value;
		//}
	case vr::Prop_Axis3Type_Int32:
		//if (m_NumAxis > 3)
		//{
			if (error)
				*error = vr::TrackedProp_Success;
			return m_AnalogInterface[3].axisType;
		//}
		//else
		//{
		//	if (error)
		//		*error = vr::TrackedProp_ValueNotProvidedByDevice;
		//	return default_value;
		//}
	case vr::Prop_Axis4Type_Int32:
		//if (m_NumAxis > 4)
		//{
			if (error)
				*error = vr::TrackedProp_Success;
			return m_AnalogInterface[4].axisType;
		//}
		//else
		//{
		//	if (error)
		//		*error = vr::TrackedProp_ValueNotProvidedByDevice;
		//	return default_value;
		//}
	}

#include "ignore-warning/pop"

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint64_t OSVRTrackedDeviceController::GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

	switch (prop) {
	case vr::Prop_SupportedButtons_Uint64: // TODO
		if (error)
			*error = vr::TrackedProp_Success;
		return NUM_BUTTONS;
	}

#include "ignore-warning/pop"

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

vr::HmdMatrix34_t OSVRTrackedDeviceController::GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error)
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

	switch (prop) {
	case vr::Prop_StatusDisplayTransform_Matrix34: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	}

#include "ignore-warning/pop"

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

uint32_t OSVRTrackedDeviceController::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char *pchValue, uint32_t unBufferSize, vr::ETrackedPropertyError *pError)
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

	std::string sValue = GetStringTrackedDeviceProperty(prop, pError);
	if (*pError == vr::TrackedProp_Success) {
		if (sValue.size() + 1 > unBufferSize) {
			*pError = vr::TrackedProp_BufferTooSmall;
		}
		else {
			valveStrCpy(sValue, pchValue, unBufferSize);
		}
		return static_cast<uint32_t>(sValue.size()) + 1;
	}

	return 0;
}

// ------------------------------------
// Private Methods
// ------------------------------------

void OSVRTrackedDeviceController::FreeInterfaces()
{
	if (m_TrackerInterface.notEmpty()) {
		m_TrackerInterface.free();
	}

	for (int iter_axis = 0; iter_axis < NUM_AXIS; iter_axis++)
	{
		if (m_AnalogInterface[iter_axis].analogInterfaceX.notEmpty())
			m_AnalogInterface[iter_axis].analogInterfaceX.free();
		if (m_AnalogInterface[iter_axis].analogInterfaceY.notEmpty())
			m_AnalogInterface[iter_axis].analogInterfaceY.free();
	}

	for (int iter_button = 0; iter_button < NUM_BUTTONS; iter_button++)
	{
		if (m_ButtonInterface[iter_button].notEmpty())
			m_ButtonInterface[iter_button].free();
	}
}

std::string OSVRTrackedDeviceController::GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error)
{
	std::string default_value = "";

#include "ignore-warning/push"
#include "ignore-warning/switch-enum"

	switch (prop) {
	case vr::Prop_TrackingSystemName_String: // TODO
		if (error)
			*error = vr::TrackedProp_ValueNotProvidedByDevice;
		return default_value;
	case vr::Prop_ModelNumber_String: // TODO
		if (error)
			*error = vr::TrackedProp_Success;
		return "OSVR Controller";
	case vr::Prop_SerialNumber_String:
		if (error)
			*error = vr::TrackedProp_Success;
		return m_ControllerName.c_str();
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
	}

#include "ignore-warning/pop"

	if (error)
		*error = vr::TrackedProp_UnknownProperty;
	return default_value;
}

inline vr::HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
{
	vr::HmdQuaternion_t quat;
	quat.w = w;
	quat.x = x;
	quat.y = y;
	quat.z = z;
	return quat;
}

void OSVRTrackedDeviceController::ControllerTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report)
{
	if (!userdata)
		return;

	auto* self = static_cast<OSVRTrackedDeviceController*>(userdata);

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
	self->driver_host_->TrackedDevicePoseUpdated(self->m_ControllerIndex + 1, self->pose_); /// @fixme figure out ID correctly, don't hardcode like this
}

void OSVRTrackedDeviceController::ControllerButtonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report)
{
	if (!userdata)
		return;

	auto* self = static_cast<OSVRTrackedDeviceController*>(userdata);

	/*if (report->state != OSVR_BUTTON_NOT_PRESSED)
		self->logger_->Log(("Some button report channel:" + std::to_string(report->sensor) + " state:" + std::to_string(report->state) + "\n").c_str());*/

	vr::EVRButtonId buttonID;
	if ((report->sensor >= 0 && report->sensor <= 7) || (report->sensor >= 32 && report->sensor <= 36))
		buttonID = static_cast<vr::EVRButtonId>(report->sensor);
	else if (report->sensor >= 8 && report->sensor <= 12)
		buttonID = static_cast<vr::EVRButtonId>(report->sensor + 24);
	else
		return;

	if (report->state == OSVR_BUTTON_PRESSED)
		self->driver_host_->TrackedDeviceButtonPressed(self->m_ControllerIndex + 1, buttonID, 0);
	else
		self->driver_host_->TrackedDeviceButtonUnpressed(self->m_ControllerIndex + 1, buttonID, 0);
}

void OSVRTrackedDeviceController::ControllerTriggerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
	if (!userdata)
		return;

	auto* analogInterface = static_cast<AnalogInterface*>(userdata);
	OSVRTrackedDeviceController* self = analogInterface->parentController;

	analogInterface->x = report->state;
	
	vr::VRControllerAxis_t axisState;
	axisState.x = (float)analogInterface->x;
	self->driver_host_->TrackedDeviceAxisUpdated(self->m_ControllerIndex + 1, analogInterface->axisIndex, axisState);
	//self->logger_->Log(("Some Trigger report channel:" + std::to_string(analogInterface->axisIndex) + " state:" + std::to_string(analogInterface->x) + "\n").c_str());
}

void OSVRTrackedDeviceController::ControllerJoystickXCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
	if (!userdata)
		return;

	auto* analogInterface = static_cast<AnalogInterface*>(userdata);
	OSVRTrackedDeviceController* self = analogInterface->parentController;

	analogInterface->x = report->state;

	vr::VRControllerAxis_t axisState;
	axisState.x = (float)analogInterface->x;
	axisState.y = (float)analogInterface->y;
	self->driver_host_->TrackedDeviceAxisUpdated(self->m_ControllerIndex + 1, analogInterface->axisIndex, axisState);

	//self->logger_->Log(("Some JoystickX report channel:" + std::to_string(analogInterface->axisIndex) + " state:" + std::to_string(analogInterface->x) + "\n").c_str());
}

void OSVRTrackedDeviceController::ControllerJoystickYCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report)
{
	if (!userdata)
		return;

	auto* analogInterface = static_cast<AnalogInterface*>(userdata);
	OSVRTrackedDeviceController* self = analogInterface->parentController;

	analogInterface->y = report->state;

	vr::VRControllerAxis_t axisState;
	axisState.x = (float)analogInterface->x;
	axisState.y = (float)analogInterface->y;
	self->driver_host_->TrackedDeviceAxisUpdated(self->m_ControllerIndex + 1, analogInterface->axisIndex, axisState);

	//self->logger_->Log(("Some JoystickY report channel:" + std::to_string(analogInterface->axisIndex) + " state:" + std::to_string(analogInterface->y) + "\n").c_str());
}

const char* OSVRTrackedDeviceController::GetId()
{
	/// @todo When available, return the actual unique ID of the HMD
	return m_ControllerName.c_str();
}