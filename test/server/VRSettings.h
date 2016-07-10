/** @file
    @brief Header

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

#ifndef INCLUDED_VRSettings_h_GUID_BF76DFB4_72A8_4316_9E2D_87B69B0DA6C2
#define INCLUDED_VRSettings_h_GUID_BF76DFB4_72A8_4316_9E2D_87B69B0DA6C2

// Internal Includes
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none

class VRSettings : public vr::IVRSettings {
public:
    VRSettings();
    virtual ~VRSettings();

    virtual const char* GetSettingsErrorNameFromEnum(vr::EVRSettingsError error) OSVR_OVERRIDE;

    virtual bool Sync(bool force = false, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;

    virtual bool GetBool(const char* section, const char* settings_key, bool default_value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void SetBool(const char* section, const char* settings_key, bool value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual int32_t GetInt32(const char* section, const char* settings_key, int32_t default_value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void SetInt32(const char* section, const char* settings_key, int32_t value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual float GetFloat(const char* section, const char* settings_key, float default_value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void SetFloat(const char* section, const char* settings_key, float value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void GetString(const char* section, const char* settings_key, VR_OUT_STRING() char* value, uint32_t value_len, const char* default_value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void SetString(const char* section, const char* settings_key, const char* value, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;

    virtual void RemoveSection(const char* section, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
    virtual void RemoveKeyInSection(const char* section, const char* settings_key, vr::EVRSettingsError* error = nullptr) OSVR_OVERRIDE;
};

#endif // INCLUDED_VRSettings_h_GUID_BF76DFB4_72A8_4316_9E2D_87B69B0DA6C2

