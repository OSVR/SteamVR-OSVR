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

#ifndef INCLUDED_Settings_h_GUID_3C3922D1_0C13_4E57_9EE4_85E6F23FFC67
#define INCLUDED_Settings_h_GUID_3C3922D1_0C13_4E57_9EE4_85E6F23FFC67

// Internal Includes
#include "identity.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <stdexcept>
#include <string>

class Settings {
public:
    /**
     * Constructor.  Requires non-null IVRSettings.
     */
    Settings(vr::IVRSettings* settings, const std::string& section = "driver_osvr");

    /**
     * @brief Returns true if file sync occurred (force or settings dirty).
     */
    bool sync(bool force = false, vr::EVRSettingsError* error = nullptr);

    /**
     * Returns the setting value or returns @c value if the setting
     * doesn't exist or an error occurred (e.g., read error, failure to parse
     * JSON settings file).
     */
    //@{
    template <typename T> T getSetting(const std::string& setting);
    template <typename T> T getSetting(const std::string& setting, vr::EVRSettingsError* error = nullptr);
    template <typename T> T getSetting(const std::string& setting, const T& value);
    //@}

    /**
     * @brief Removes the entire  section of the configuration settings.
     */
    void removeSection(vr::EVRSettingsError* error = nullptr);

    /**
     * @brief Removes a setting from the current section.
     */
    void removeSetting(const std::string& setting, vr::EVRSettingsError* error = nullptr);

    /**
     * @brief Returns true if the settings key exists, false otherwise.
     */
    bool hasSetting(const std::string& settings_key);

private:
    /** \name Accessors for settings values. */
    //template <typename T> T getSetting(identity<T>, const std::string& setting, vr::EVRSettingsError* error = nullptr);
    bool getSetting(identity<bool>, const std::string& setting, vr::EVRSettingsError* error = nullptr);
    int32_t getSetting(identity<int32_t>, const std::string& setting, vr::EVRSettingsError* error = nullptr);
    float getSetting(identity<float>, const std::string& setting, vr::EVRSettingsError* error = nullptr);
    std::string getSetting(identity<std::string>, const std::string& setting, vr::EVRSettingsError* error = nullptr);
    //@}

    vr::IVRSettings* settings_ = nullptr;
    std::string section_;
};

inline Settings::Settings(vr::IVRSettings* settings, const std::string& section) : settings_(settings), section_(section)
{
    if (!settings) {
        throw std::invalid_argument("Must use non-null IVRSettings.");
    }
}

inline bool Settings::sync(bool force, vr::EVRSettingsError* error)
{
    return settings_->Sync(force, error);
}

template<typename T> inline T Settings::getSetting(const std::string& setting)
{
    // Redirect to the private method
    return getSetting(identity<T>(), setting);
}

template <typename T> inline T getSetting(const std::string& setting, vr::EVRSettingsError* error)
{
    return getSetting(identity<T>(), setting, &error);
}

template<typename T> inline T Settings::getSetting(const std::string& setting, const T& value)
{
    // Redirect to the private method
    vr::EVRSettingsError error = vr::VRSettingsError_None;
    auto result = getSetting(identity<T>(), setting, &error);
    if (vr::VRSettingsError_None != error) {
        result = value;
    }
    return result;
}

inline void Settings::removeSection(vr::EVRSettingsError* error)
{
    settings_->RemoveSection(section_.c_str(), error);
}

inline void Settings::removeSetting(const std::string& setting, vr::EVRSettingsError *error)
{
    settings_->RemoveKeyInSection(section_.c_str(), setting.c_str(), error);
}

inline bool Settings::hasSetting(const std::string& setting)
{
    vr::EVRSettingsError error = vr::VRSettingsError_None;
    settings_->GetBool(section_.c_str(), setting.c_str(), &error);
    return (vr::VRSettingsError_UnsetSettingHasNoDefault != error);
}

/*
template<typename T> inline T Settings::getSetting(identity<T>, const std::string& setting, vr::EVRSettingsError* error)
{
    static_assert(false, "Can only retrieve settings of type bool, float, int32_t, and std::string.");
    return 0;
}
*/

inline bool Settings::getSetting(identity<bool>, const std::string& setting, vr::EVRSettingsError* error)
{
    return settings_->GetBool(section_.c_str(), setting.c_str(), error);
}

inline float Settings::getSetting(identity<float>, const std::string& setting, vr::EVRSettingsError* error)
{
    return settings_->GetFloat(section_.c_str(), setting.c_str(), error);
}

inline int32_t Settings::getSetting(identity<int32_t>, const std::string& setting, vr::EVRSettingsError* error)
{
    return settings_->GetInt32(section_.c_str(), setting.c_str(), error);
}

inline std::string Settings::getSetting(identity<std::string>, const std::string& setting, vr::EVRSettingsError* error)
{
    char buf[1024] = { 0 };
    settings_->GetString(section_.c_str(), setting.c_str(), buf, sizeof(buf), error);
    std::string ret = buf;
    return ret;
}

#endif // INCLUDED_Settings_h_GUID_3C3922D1_0C13_4E57_9EE4_85E6F23FFC67

