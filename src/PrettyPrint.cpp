/** @file
    @brief Pretty-prints OpenVR objects.

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "PrettyPrint.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <ios>
#include <ostream>
#include <sstream>
#include <string>

using std::to_string;

/**
 * Simple loopback to make to_string more generic.
 */
std::string to_string(const std::string& str)
{
    return str;
}

std::string to_string(void* value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string to_string(const vr::ETrackedDeviceProperty& value)
{
    switch (value) {
        case vr::Prop_Invalid:
            return "Prop_Invalid";
        case vr::Prop_TrackingSystemName_String:
            return "Prop_TrackingSystemName_String";
        case vr::Prop_ModelNumber_String:
            return "Prop_ModelNumber_String";
        case vr::Prop_SerialNumber_String:
            return "Prop_SerialNumber_String";
        case vr::Prop_RenderModelName_String:
            return "Prop_RenderModelName_String";
        case vr::Prop_WillDriftInYaw_Bool:
            return "Prop_WillDriftInYaw_Bool";
        case vr::Prop_ManufacturerName_String:
            return "Prop_ManufacturerName_String";
        case vr::Prop_TrackingFirmwareVersion_String:
            return "Prop_TrackingFirmwareVersion_String";
        case vr::Prop_HardwareRevision_String:
            return "Prop_HardwareRevision_String";
        case vr::Prop_AllWirelessDongleDescriptions_String:
            return "Prop_AllWirelessDongleDescriptions_String";
        case vr::Prop_ConnectedWirelessDongle_String:
            return "Prop_ConnectedWirelessDongle_String";
        case vr::Prop_DeviceIsWireless_Bool:
            return "Prop_DeviceIsWireless_Bool";
        case vr::Prop_DeviceIsCharging_Bool:
            return "Prop_DeviceIsCharging_Bool";
        case vr::Prop_DeviceBatteryPercentage_Float:
            return "Prop_DeviceBatteryPercentage_Float";
        case vr::Prop_StatusDisplayTransform_Matrix34:
            return "Prop_StatusDisplayTransform_Matrix34";
        case vr::Prop_Firmware_UpdateAvailable_Bool:
            return "Prop_Firmware_UpdateAvailable_Bool";
        case vr::Prop_Firmware_ManualUpdate_Bool:
            return "Prop_Firmware_ManualUpdate_Bool";
        case vr::Prop_Firmware_ManualUpdateURL_String:
            return "Prop_Firmware_ManualUpdateURL_String";
        case vr::Prop_HardwareRevision_Uint64:
            return "Prop_HardwareRevision_Uint64";
        case vr::Prop_FirmwareVersion_Uint64:
            return "Prop_FirmwareVersion_Uint64";
        case vr::Prop_FPGAVersion_Uint64:
            return "Prop_FPGAVersion_Uint64";
        case vr::Prop_VRCVersion_Uint64:
            return "Prop_VRCVersion_Uint64";
        case vr::Prop_RadioVersion_Uint64:
            return "Prop_RadioVersion_Uint64";
        case vr::Prop_DongleVersion_Uint64:
            return "Prop_DongleVersion_Uint64";
        case vr::Prop_BlockServerShutdown_Bool:
            return "Prop_BlockServerShutdown_Bool";
        case vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool:
            return "Prop_CanUnifyCoordinateSystemWithHmd_Bool";
        case vr::Prop_ContainsProximitySensor_Bool:
            return "Prop_ContainsProximitySensor_Bool";
        case vr::Prop_DeviceProvidesBatteryStatus_Bool:
            return "Prop_DeviceProvidesBatteryStatus_Bool";
        case vr::Prop_DeviceCanPowerOff_Bool:
            return "Prop_DeviceCanPowerOff_Bool";
        case vr::Prop_Firmware_ProgrammingTarget_String:
            return "Prop_Firmware_ProgrammingTarget_String";
        case vr::Prop_DeviceClass_Int32:
            return "Prop_DeviceClass_Int32";
        case vr::Prop_HasCamera_Bool:
            return "Prop_HasCamera_Bool";
        case vr::Prop_DriverVersion_String:
            return "Prop_DriverVersion_String";
        case vr::Prop_Firmware_ForceUpdateRequired_Bool:
            return "Prop_Firmware_ForceUpdateRequired_Bool";
        case vr::Prop_ViveSystemButtonFixRequired_Bool:
            return "Prop_ViveSystemButtonFixRequired_Bool";
        case vr::Prop_ParentDriver_Uint64:
            return "Prop_ParentDriver_Uint64";
        //case vr::Prop_ResourceRoot_String:
        //    return "Prop_ResourceRoot_String";
        case vr::Prop_ReportsTimeSinceVSync_Bool:
            return "Prop_ReportsTimeSinceVSync_Bool";
        case vr::Prop_SecondsFromVsyncToPhotons_Float:
            return "Prop_SecondsFromVsyncToPhotons_Float";
        case vr::Prop_DisplayFrequency_Float:
            return "Prop_DisplayFrequency_Float";
        case vr::Prop_UserIpdMeters_Float:
            return "Prop_UserIpdMeters_Float";
        case vr::Prop_CurrentUniverseId_Uint64:
            return "Prop_CurrentUniverseId_Uint64";
        case vr::Prop_PreviousUniverseId_Uint64:
            return "Prop_PreviousUniverseId_Uint64";
        case vr::Prop_DisplayFirmwareVersion_Uint64:
            return "Prop_DisplayFirmwareVersion_Uint64";
        case vr::Prop_IsOnDesktop_Bool:
            return "Prop_IsOnDesktop_Bool";
        case vr::Prop_DisplayMCType_Int32:
            return "Prop_DisplayMCType_Int32";
        case vr::Prop_DisplayMCOffset_Float:
            return "Prop_DisplayMCOffset_Float";
        case vr::Prop_DisplayMCScale_Float:
            return "Prop_DisplayMCScale_Float";
        case vr::Prop_EdidVendorID_Int32:
            return "Prop_EdidVendorID_Int32";
        case vr::Prop_DisplayMCImageLeft_String:
            return "Prop_DisplayMCImageLeft_String";
        case vr::Prop_DisplayMCImageRight_String:
            return "Prop_DisplayMCImageRight_String";
        case vr::Prop_DisplayGCBlackClamp_Float:
            return "Prop_DisplayGCBlackClamp_Float";
        case vr::Prop_EdidProductID_Int32:
            return "Prop_EdidProductID_Int32";
        case vr::Prop_CameraToHeadTransform_Matrix34:
            return "Prop_CameraToHeadTransform_Matrix34";
        case vr::Prop_DisplayGCType_Int32:
            return "Prop_DisplayGCType_Int32";
        case vr::Prop_DisplayGCOffset_Float:
            return "Prop_DisplayGCOffset_Float";
        case vr::Prop_DisplayGCScale_Float:
            return "Prop_DisplayGCScale_Float";
        case vr::Prop_DisplayGCPrescale_Float:
            return "Prop_DisplayGCPrescale_Float";
        case vr::Prop_DisplayGCImage_String:
            return "Prop_DisplayGCImage_String";
        case vr::Prop_LensCenterLeftU_Float:
            return "Prop_LensCenterLeftU_Float";
        case vr::Prop_LensCenterLeftV_Float:
            return "Prop_LensCenterLeftV_Float";
        case vr::Prop_LensCenterRightU_Float:
            return "Prop_LensCenterRightU_Float";
        case vr::Prop_LensCenterRightV_Float:
            return "Prop_LensCenterRightV_Float";
        case vr::Prop_UserHeadToEyeDepthMeters_Float:
            return "Prop_UserHeadToEyeDepthMeters_Float";
        case vr::Prop_CameraFirmwareVersion_Uint64:
            return "Prop_CameraFirmwareVersion_Uint64";
        case vr::Prop_CameraFirmwareDescription_String:
            return "Prop_CameraFirmwareDescription_String";
        case vr::Prop_DisplayFPGAVersion_Uint64:
            return "Prop_DisplayFPGAVersion_Uint64";
        case vr::Prop_DisplayBootloaderVersion_Uint64:
            return "Prop_DisplayBootloaderVersion_Uint64";
        case vr::Prop_DisplayHardwareVersion_Uint64:
            return "Prop_DisplayHardwareVersion_Uint64";
        case vr::Prop_AudioFirmwareVersion_Uint64:
            return "Prop_AudioFirmwareVersion_Uint64";
        case vr::Prop_CameraCompatibilityMode_Int32:
            return "Prop_CameraCompatibilityMode_Int32";
        case vr::Prop_ScreenshotHorizontalFieldOfViewDegrees_Float:
            return "Prop_ScreenshotHorizontalFieldOfViewDegrees_Float";
        case vr::Prop_ScreenshotVerticalFieldOfViewDegrees_Float:
            return "Prop_ScreenshotVerticalFieldOfViewDegrees_Float";
        case vr::Prop_DisplaySuppressed_Bool:
            return "Prop_DisplaySuppressed_Bool";
        case vr::Prop_DisplayAllowNightMode_Bool:
            return "Prop_DisplayAllowNightMode_Bool";
        case vr::Prop_DisplayMCImageWidth_Int32:
            return "Prop_DisplayMCImageWidth_Int32";
        case vr::Prop_DisplayMCImageHeight_Int32:
            return "Prop_DisplayMCImageHeight_Int32";
        case vr::Prop_DisplayMCImageNumChannels_Int32:
            return "Prop_DisplayMCImageNumChannels_Int32";
        case vr::Prop_DisplayMCImageData_Binary:
            return "Prop_DisplayMCImageData_Binary";
        //case vr::Prop_SecondsFromPhotonsToVblank_Float:
        //    return "Prop_SecondsFromPhotonsToVblank_Float";
        case vr::Prop_AttachedDeviceId_String:
            return "Prop_AttachedDeviceId_String";
        case vr::Prop_SupportedButtons_Uint64:
            return "Prop_SupportedButtons_Uint64";
        case vr::Prop_Axis0Type_Int32:
            return "Prop_Axis0Type_Int32";
        case vr::Prop_Axis1Type_Int32:
            return "Prop_Axis1Type_Int32";
        case vr::Prop_Axis2Type_Int32:
            return "Prop_Axis2Type_Int32";
        case vr::Prop_Axis3Type_Int32:
            return "Prop_Axis3Type_Int32";
        case vr::Prop_Axis4Type_Int32:
            return "Prop_Axis4Type_Int32";
        case vr::Prop_ControllerRoleHint_Int32:
            return "Prop_ControllerRoleHint_Int32";
        case vr::Prop_FieldOfViewLeftDegrees_Float:
            return "Prop_FieldOfViewLeftDegrees_Float";
        case vr::Prop_FieldOfViewRightDegrees_Float:
            return "Prop_FieldOfViewRightDegrees_Float";
        case vr::Prop_FieldOfViewTopDegrees_Float:
            return "Prop_FieldOfViewTopDegrees_Float";
        case vr::Prop_FieldOfViewBottomDegrees_Float:
            return "Prop_FieldOfViewBottomDegrees_Float";
        case vr::Prop_TrackingRangeMinimumMeters_Float:
            return "Prop_TrackingRangeMinimumMeters_Float";
        case vr::Prop_TrackingRangeMaximumMeters_Float:
            return "Prop_TrackingRangeMaximumMeters_Float";
        case vr::Prop_ModeLabel_String:
            return "Prop_ModeLabel_String";
        case vr::Prop_IconPathName_String:
            return "Prop_IconPathName_String";
        case vr::Prop_NamedIconPathDeviceOff_String:
            return "Prop_NamedIconPathDeviceOff_String";
        case vr::Prop_NamedIconPathDeviceSearching_String:
            return "Prop_NamedIconPathDeviceSearching_String";
        case vr::Prop_NamedIconPathDeviceSearchingAlert_String:
            return "Prop_NamedIconPathDeviceSearchingAlert_String";
        case vr::Prop_NamedIconPathDeviceReady_String:
            return "Prop_NamedIconPathDeviceReady_String";
        case vr::Prop_NamedIconPathDeviceReadyAlert_String:
            return "Prop_NamedIconPathDeviceReadyAlert_String";
        case vr::Prop_NamedIconPathDeviceNotReady_String:
            return "Prop_NamedIconPathDeviceNotReady_String";
        case vr::Prop_NamedIconPathDeviceStandby_String:
            return "Prop_NamedIconPathDeviceStandby_String";
        case vr::Prop_NamedIconPathDeviceAlertLow_String:
            return "Prop_NamedIconPathDeviceAlertLow_String";
        case vr::Prop_DisplayHiddenArea_Binary_Start:
            return "Prop_DisplayHiddenArea_Binary_Start";
        case vr::Prop_DisplayHiddenArea_Binary_End:
            return "Prop_DisplayHiddenArea_Binary_End";
        case vr::Prop_UserConfigPath_String:
            return "Prop_UserConfigPath_String";
        case vr::Prop_InstallPath_String:
            return "Prop_InstallPath_String";
        //case vr::Prop_HasDisplayComponent_Bool:
        //    return "Prop_HasDisplayComponent_Bool";
        //case vr::Prop_HasControllerComponent_Bool:
        //    return "Prop_HasControllerComponent_Bool";
        //case vr::Prop_HasCameraComponent_Bool:
        //    return "Prop_HasCameraComponent_Bool";
        //case vr::Prop_HasDriverDirectModeComponent_Bool:
        //    return "Prop_HasDriverDirectModeComponent_Bool";
        //case vr::Prop_HasVirtualDisplayComponent_Bool:
        //    return "Prop_HasVirtualDisplayComponent_Bool";
        case vr::Prop_VendorSpecific_Reserved_Start:
            return "Prop_VendorSpecific_Reserved_Start";
        case vr::Prop_VendorSpecific_Reserved_End:
            return "Prop_VendorSpecific_Reserved_End";
	//case vr::Prop_UsesDriverDirectMode_Bool:
	//    return "Prop_UsesDriverDirectMode_Bool";
        default:
            {
                std::ostringstream oss;
                oss << value;
                std::string val = oss.str();
                return val;
            }
    }
}

std::ostream& operator<<(std::ostream& out, const vr::ETrackedDeviceProperty value)
{
    out << to_string(value);
    return out;
}

std::ostream& operator<<(std::ostream& ostr, const vr::HmdQuaternion_t& quat)
{
    ostr << "<" << quat.w << ", " << quat.x << ", " << quat.y << ", " << quat.z << ">";
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const vr::ETrackingResult& result)
{
    switch (result) {
    case vr::TrackingResult_Uninitialized:
        ostr << "Unintiialized";
        break;
    case vr::TrackingResult_Calibrating_InProgress:
        ostr << "Calibration in progress";
        break;
    case vr::TrackingResult_Calibrating_OutOfRange:
        ostr << "Calibrating - out of range";
        break;
    case vr::TrackingResult_Running_OK:
        ostr << "Running okay";
        break;
    case vr::TrackingResult_Running_OutOfRange:
        ostr << "Running - out of range";
        break;
    default:
        ostr << "Unknown tracking result";
        break;
    }
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const double vec[3])
{
    ostr << "[" << vec[0] << ", " << vec[1] << ", " << vec[2] << "]";
    return ostr;
}

/**
 * @brief Return a string representation of vr::DriverPose_t struct.
 */
std::ostream& operator<<(std::ostream& ostr, const vr::DriverPose_t& pose)
{
    ostr << "Time offset (seconds): " << pose.poseTimeOffset << "\n";
    ostr << "World-from-driver transform:\n";
    ostr << " -- translation: " << pose.vecWorldFromDriverTranslation << "\n";
    ostr << " -- rotation: " << pose.qWorldFromDriverRotation << "\n";
    ostr << "Driver-from-head transform:\n";
    ostr << " -- translation: " << pose.vecDriverFromHeadTranslation << "\n";
    ostr << " -- rotation: " << pose.qDriverFromHeadRotation << "\n";
    ostr << "Pose:\n";
    ostr << " -- position: " << pose.vecPosition << "\n";
    ostr << " -- rotation: " << pose.qRotation << "\n";
    ostr << "Velocity:\n";
    ostr << " -- linear (m/s): " << pose.vecVelocity << "\n";
    ostr << " -- angular (rad/s): " << pose.vecAngularVelocity << "\n";
    ostr << "Acceleration:\n";
    ostr << " -- linear (m/s^2): " << pose.vecAcceleration << "\n";
    ostr << " -- angular (rad/s^2): " << pose.vecAngularAcceleration << "\n";
    ostr << "Tracking result: " << pose.result << "\n";
    ostr << "Pose is valid: " << (pose.poseIsValid ? "true" : "false") << "\n";
    ostr << "Will drift in yaw: " << (pose.willDriftInYaw ? "yes" : "no") << "\n";
    ostr << "Should apply head model: " << (pose.shouldApplyHeadModel ? "yes" : "no") << "\n";
    ostr << "Device is connected: " << (pose.deviceIsConnected ? "yes" : "no");
    return ostr;
}

