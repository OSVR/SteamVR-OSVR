/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2015 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E
#define INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E

// Internal Includes
// - none

// Library/third-party includes
#include <vrtypes.h>

// Standard includes
// - none

inline bool isWrongDataType(vr::TrackedDeviceProperty prop, const bool&)
{
    switch (prop) {
    case vr::Prop_WillDriftInYaw_Bool:
    case vr::Prop_ReportsTimeSinceVSync_Bool:
        return false;
    }

    return true;
}

inline bool isWrongDataType(vr::TrackedDeviceProperty prop, const float&)
{
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
        return false;
    }

    return true;
}

inline bool isWrongDataType(vr::TrackedDeviceProperty prop, const int32_t&)
{
    switch (prop) {
    case vr::Prop_Axis0Type_Int32:
    case vr::Prop_Axis1Type_Int32:
    case vr::Prop_Axis2Type_Int32:
    case vr::Prop_Axis3Type_Int32:
    case vr::Prop_Axis4Type_Int32:
        return false;
    }

    return true;
}

inline bool isWrongDataType(vr::TrackedDeviceProperty prop, const uint64_t&)
{
    switch (prop) {
    case vr::Prop_CurrentUniverseId_Uint64:
    case vr::Prop_PreviousUniverseId_Uint64:
    case vr::Prop_SupportedButtons_Uint64:
        return false;
    }

    return true;
}

inline bool isWrongDataType(vr::TrackedDeviceProperty prop, const char*)
{
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
        return false;
    }

    return true;
}

inline bool isWrongDeviceClass(vr::TrackedDeviceProperty prop, vr::TrackedDeviceClass device_class)
{
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
        return false;

    // properties that are unique to TrackedDeviceClass_HMD
    case vr::Prop_ReportsTimeSinceVSync_Bool:
    case vr::Prop_SecondsFromVsyncToPhotons_Float:
    case vr::Prop_DisplayFrequency_Float:
    case vr::Prop_UserIpdMeters_Float:
    case vr::Prop_CurrentUniverseId_Uint64:
    case vr::Prop_PreviousUniverseId_Uint64:
    case vr::Prop_DisplayFirmwareVersion_String:
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
}

#endif // INCLUDED_osvr_device_properties_h_GUID_5212DE9D_B211_4139_A140_45A578EFA47E

