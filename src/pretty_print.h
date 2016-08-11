/** @file
    @brief Pretty-prints OpenVR enums.

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

#ifndef INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591
#define INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591

// Internal Includes
// - none

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <ostream>
#include <sstream>

using std::to_string;

/**
 * Simple loopback to make to_string more generic.
 */
inline std::string to_string(const std::string& str)
{
    return str;
}

inline std::string to_string(void* value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

inline std::string to_string(const vr::ETrackedDeviceProperty& value)
{
    switch (value) {
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
        case vr::Prop_VendorSpecific_Reserved_Start:
            return "Prop_VendorSpecific_Reserved_Start";
        case vr::Prop_VendorSpecific_Reserved_End:
            return "Prop_VendorSpecific_Reserved_End";
        default:
            {
                std::ostringstream oss;
                oss << value;
                std::string val = oss.str();
                return val;
            }
    }
}

inline std::ostream& operator<<(std::ostream& out, const vr::ETrackedDeviceProperty value)
{
    out << to_string(value);
    return out;
}

/// Helper class to wrap a value that should be output as hex to a
/// stream.
template <typename T>
class AsHex {
public:
    explicit AsHex(T val, bool leading0x = false) : val_(val), leading0x_(leading0x)
    {
        // do nothing
    }

    T get() const
    {
        return val_;
    }

    bool leading0x() const
    {
        return leading0x_;
    }

private:
    T val_;
    bool leading0x_;
};

/// Output streaming operator for AsHex, found by ADL.
template <typename T>
std::ostream& operator<<(std::ostream& os, const AsHex<T>& val)
{
    if (val.leading0x()) {
        os << "0x";
    }
    os << std::hex << val.get() << std::dec;
    return os;
}

/// Function template to wrap a value to indicate it should be output as a
/// hex value (no leading 0x)
template <typename T>
inline AsHex<T> as_hex(T val) {
    return AsHex<T>(val);
}

/// Function template to wrap a value to indicate it should be output as a
/// hex value (with leading 0x)
template <typename T>
inline AsHex<T> as_hex_0x(T val) {
    return AsHex<T>(val, true);
}

template <typename T>
std::string to_string(const T& val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

#endif // INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591

