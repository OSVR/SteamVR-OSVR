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

#ifndef INCLUDED_OSVRTrackedController_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645
#define INCLUDED_OSVRTrackedController_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Client/RenderManagerConfig.h>

#include <Json/Value.h>

// Standard includes
#include <string>

class OSVRTrackedController;

struct AxisCallbackData {
    OSVRTrackedController* controller;
    uint32_t index;
    enum class AxisDirection {
        X, Y
    } direction;
};

struct ButtonCallbackData {
    OSVRTrackedController* controller;
    vr::EVRButtonId button_id;
};

class OSVRTrackedController : public OSVRTrackedDevice, public vr::IVRControllerComponent {
    friend class ServerDriver_OSVR;

public:
    OSVRTrackedController(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, const std::string& user_driver_config_dir, vr::ETrackedControllerRole controller_role);

    virtual ~OSVRTrackedController();

    // ------------------------------------
    // Management Methods
    // ------------------------------------

    /**
     * This is called before an HMD is returned to the application. It will
     * always be called before any display or tracking methods. Memory and
     * processor use by the ITrackedDeviceServerDriver object should be kept to
     * a minimum until it is activated.  The pose listener is guaranteed to be
     * valid until Deactivate is called, but should not be used after that
     * point.
     */
    virtual vr::EVRInitError Activate(uint32_t object_id) OSVR_OVERRIDE;

    /**
     * This is called when The VR system is switching from this Hmd being the
     * active display to another Hmd being the active display. The driver should
     * clean whatever memory and thread use it can when it is deactivated.
     */
    virtual void Deactivate() OSVR_OVERRIDE;

    // ------------------------------------
    // Controller Methods
    // ------------------------------------

    /**
     * Gets the current state of a controller.
     */
    virtual vr::VRControllerState_t GetControllerState() OSVR_OVERRIDE;

    /**
     * Returns a uint64 property. If the property is not available this function will return 0.
     */
    virtual bool TriggerHapticPulse(uint32_t axis_id, uint16_t pulse_duration_microseconds) OSVR_OVERRIDE;

protected:
    const char* GetId();

private:
    void configure();
    void configureController();
    void configureTracker(const Json::Value& controller_root, const std::string& base_path);
    void configureAxes(const Json::Value& controller_root, const std::string& base_path);
    void configureButtons(const Json::Value& controller_root, const std::string& base_path);
    void configureProperties();

    void freeInterfaces();

    /** \name OSVR callback functions */
    //@{
    static void trackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);
    static void axisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);
    static void buttonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report);
    //@}

    /**
     * Takes a JSON key and returns the corresponding @c vr::EVRButton button.
     */
    vr::EVRButtonId getButtonId(const std::string& key) const;

    vr::ETrackedControllerRole controllerRole_ = vr::TrackedControllerRole_Invalid;
    std::vector<osvr::clientkit::Interface> interfaces_;
    AxisCallbackData axisCallbackData_[vr::k_unControllerStateAxisCount] = { };
    vr::EVRControllerAxisType axisTypes_[vr::k_unControllerStateAxisCount] = { vr::k_eControllerAxis_None };
    ButtonCallbackData buttonCallbackData_[vr::k_EButton_Max] = { };
    uint64_t numButtons_ = 0;

    vr::VRControllerState_t controllerState_;
    std::vector<std::string> buttonNames_ {
        "system",
        "menu",
        "grip",
        "left",
        "up",
        "right",
        "down",
        "a",
        "axis0",
        "axis1",
        "axis2",
        "axis3",
        "axis4",
    };
};

#endif // INCLUDED_OSVRTrackedDevice_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

