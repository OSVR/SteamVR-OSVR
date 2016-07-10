/** @file
    @brief VRSettings

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
#include "VRSettings.h"

// Library/third-party includes
// - none

// Standard includes
// - none

VRSettings::VRSettings()
{
    // do nothing
}

VRSettings::~VRSettings()
{
    // do nothing
}

const char* VRSettings::GetSettingsErrorNameFromEnum(vr::EVRSettingsError error)
{
    // TODO
    return "";
}

bool VRSettings::Sync(bool force, vr::EVRSettingsError* error)
{
    // TODO
    return false;
}

bool VRSettings::GetBool(const char* section, const char* settings_key, bool default_value, vr::EVRSettingsError* error)
{
    // TODO
    return default_value;
}

void VRSettings::SetBool(const char* section, const char* settings_key, bool value, vr::EVRSettingsError* error)
{
    // TODO
}

int32_t VRSettings::GetInt32(const char* section, const char* settings_key, int32_t default_value, vr::EVRSettingsError* error)
{
    // TODO
    return default_value;
}

void VRSettings::SetInt32(const char* section, const char* settings_key, int32_t value, vr::EVRSettingsError* error)
{
    // TODO
}

float VRSettings::GetFloat(const char* section, const char* settings_key, float default_value, vr::EVRSettingsError* error)
{
    // TODO
    return default_value;
}

void VRSettings::SetFloat(const char* section, const char* settings_key, float value, vr::EVRSettingsError* error)
{
    // TODO
}

void VRSettings::GetString(const char* section, const char* settings_key, VR_OUT_STRING() char* value, uint32_t value_len, const char* default_value, vr::EVRSettingsError* error)
{
    // TODO
    value = "";
}

void VRSettings::SetString(const char* section, const char* settings_key, const char* value, vr::EVRSettingsError* error)
{
    // TODO
}

void VRSettings::RemoveSection(const char* section, vr::EVRSettingsError* error)
{
    // TODO
}

void VRSettings::RemoveKeyInSection(const char* section, const char* settings_key, vr::EVRSettingsError* error)
{
    // TODO
}

