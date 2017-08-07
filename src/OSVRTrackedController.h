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


#define NUM_BUTTONS 64
#define NUM_TOUCHPAD 1 // only SteamVR Axis 0 for the moment (not implemented yet)
#define NUM_TRIGGER 1 // only SteamVR Axis 1 for the moment
#define NUM_JOYSTICKS 3 // only SteamVR Axis 2,3,4 for the moment (there is always x and y in one joystick)
#define NUM_AXIS 5

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Client/RenderManagerConfig.h>

// Standard includes
#include <string>

class OSVRTrackedController;

struct ButtonInterface {
    osvr::clientkit::Interface buttonInterface;
    OSVRTrackedController* parentController;
    vr::EVRButtonId button_id;
};

struct AnalogInterface {
    osvr::clientkit::Interface analogInterfaceX;
    osvr::clientkit::Interface analogInterfaceY;

    OSVRTrackedController* parentController;

    vr::EVRControllerAxisType axisType;
    double x;
    double y;
    uint32_t axisIndex;
};

class OSVRTrackedController : public OSVRTrackedDevice, public vr::IVRControllerComponent {
    friend class ServerDriver_OSVR;

public:
    OSVRTrackedController(osvr::clientkit::ClientContext& context, int controller_index);
    //OSVRTrackedController(osvr::clientkit::ClientContext& context);

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
    void configureProperties();

    void freeInterfaces();

    /**
     * Callback function which is called whenever new data has been received
     * from the tracker.
     */
    static void controllerTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);
    static void controllerButtonCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_ButtonReport* report);
    static void controllerTriggerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);
    static void controllerJoystickXCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);
    static void controllerJoystickYCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);
    static void controllerXAxisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);
    static void controllerYAxisCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_AnalogReport* report);

    int controllerIndex_;
    osvr::clientkit::Interface trackerInterface_;
    //osvr::clientkit::Interface buttonInterface_[NUM_BUTTONS];
    uint32_t numAxis_;
    AnalogInterface analogInterface_[NUM_AXIS];
    ButtonInterface buttonInterface_[NUM_BUTTONS];
    void registerButton(int id, std::string path, vr::EVRButtonId button_id);
    void registerTrigger(int id, std::string path);
    void registerTrackpad(int id, std::string path);
};

#endif // INCLUDED_OSVRTrackedDevice_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

