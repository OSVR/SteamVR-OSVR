/** @file
    @brief Header

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

#ifndef INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E
#define INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E

// Internal Includes
// - none

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, const bool&)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_WillDriftInYaw_Bool:
    case vr::Prop_ReportsTimeSinceVSync_Bool:
    case vr::Prop_IsOnDesktop_Bool:
    case vr::Prop_DeviceIsWireless_Bool:
    case vr::Prop_DeviceIsCharging_Bool:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, const float&)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_SecondsFromVsyncToPhotons_Float:
    case vr::Prop_DisplayFrequency_Float:
    case vr::Prop_UserIpdMeters_Float:
    case vr::Prop_FieldOfViewLeftDegrees_Float:
    case vr::Prop_FieldOfViewRightDegrees_Float:
    case vr::Prop_FieldOfViewTopDegrees_Float:
    case vr::Prop_FieldOfViewBottomDegrees_Float:
    case vr::Prop_TrackingRangeMinimumMeters_Float:
    case vr::Prop_TrackingRangeMaximumMeters_Float:
    case vr::Prop_DeviceBatteryPercentage_Float:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, const int32_t&)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_Axis0Type_Int32:
    case vr::Prop_Axis1Type_Int32:
    case vr::Prop_Axis2Type_Int32:
    case vr::Prop_Axis3Type_Int32:
    case vr::Prop_Axis4Type_Int32:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, const uint64_t&)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_CurrentUniverseId_Uint64:
    case vr::Prop_PreviousUniverseId_Uint64:
    case vr::Prop_SupportedButtons_Uint64:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, const char*)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_TrackingSystemName_String:
    case vr::Prop_ModelNumber_String:
    case vr::Prop_SerialNumber_String:
    case vr::Prop_RenderModelName_String:
    case vr::Prop_ManufacturerName_String:
    case vr::Prop_TrackingFirmwareVersion_String:
    case vr::Prop_HardwareRevision_String:
    case vr::Prop_DisplayFirmwareVersion_String:
    case vr::Prop_AttachedDeviceId_String:
    case vr::Prop_AllWirelessDongleDescriptions_String:
    case vr::Prop_ConnectedWirelessDongle_String:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDataType(vr::ETrackedDeviceProperty prop, vr::HmdMatrix34_t)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    case vr::Prop_StatusDisplayTransform_Matrix34:
        return false;
    }
#include "ignore-warning/pop"

    return true;
}

inline bool isWrongDeviceClass(vr::ETrackedDeviceProperty prop, vr::ETrackedDeviceClass device_class)
{
#include "ignore-warning/push"
#include "ignore-warning/switch-enum"
    switch (prop) {
    // General properties that apply to all device classes
    case vr::Prop_TrackingSystemName_String:
    case vr::Prop_ModelNumber_String:
    case vr::Prop_SerialNumber_String:
    case vr::Prop_RenderModelName_String:
    case vr::Prop_WillDriftInYaw_Bool:
    case vr::Prop_ManufacturerName_String:
    case vr::Prop_TrackingFirmwareVersion_String:
    case vr::Prop_HardwareRevision_String:
    case vr::Prop_AllWirelessDongleDescriptions_String:
    case vr::Prop_ConnectedWirelessDongle_String:
    case vr::Prop_DeviceIsWireless_Bool:
    case vr::Prop_DeviceIsCharging_Bool:
    case vr::Prop_DeviceBatteryPercentage_Float:
    case vr::Prop_StatusDisplayTransform_Matrix34:
        return false;

    // Properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_ReportsTimeSinceVSync_Bool:
    case vr::Prop_SecondsFromVsyncToPhotons_Float:
    case vr::Prop_DisplayFrequency_Float:
    case vr::Prop_UserIpdMeters_Float:
    case vr::Prop_CurrentUniverseId_Uint64:
    case vr::Prop_PreviousUniverseId_Uint64:
    case vr::Prop_DisplayFirmwareVersion_String:
    case vr::Prop_IsOnDesktop_Bool:
        return (vr::TrackedDeviceClass_HMD == device_class);

    // Properties that are unique to TrackedDeviceClass_Controller
    case vr::Prop_AttachedDeviceId_String:
    case vr::Prop_SupportedButtons_Uint64:
    case vr::Prop_Axis0Type_Int32:
    case vr::Prop_Axis1Type_Int32:
    case vr::Prop_Axis2Type_Int32:
    case vr::Prop_Axis3Type_Int32:
    case vr::Prop_Axis4Type_Int32:
        return (vr::TrackedDeviceClass_Controller == device_class);

    // Properties that are unique to TrackedDeviceClass_TrackingReference
    case vr::Prop_FieldOfViewLeftDegrees_Float:
    case vr::Prop_FieldOfViewRightDegrees_Float:
    case vr::Prop_FieldOfViewTopDegrees_Float:
    case vr::Prop_FieldOfViewBottomDegrees_Float:
    case vr::Prop_TrackingRangeMinimumMeters_Float:
    case vr::Prop_TrackingRangeMaximumMeters_Float:
        return (vr::TrackedDeviceClass_TrackingReference == device_class);

    default:
        return true;
    }
#include "ignore-warning/pop"
}

#endif // INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E
