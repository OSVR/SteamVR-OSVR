/** @file
    @brief Server driver host

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

#ifndef INCLUDED_ServerDriverHost_h_GUID_05FFC5CC_18FD_4583_B460_627577D0765E
#define INCLUDED_ServerDriverHost_h_GUID_05FFC5CC_18FD_4583_B460_627577D0765E

// Internal Includes
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "VRSettings.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none

class ServerDriverHost : public vr::IServerDriverHost {
public:
    ServerDriverHost();

    virtual ~ServerDriverHost();

    /**
     * Notifies the server that a tracked device has been added. If this
     * function returns true the server will call Activate on the device. If it
     * returns false some kind of error has occurred and the device will not be
     * activated.
     */
    virtual bool TrackedDeviceAdded(const char* device_serial_number) OSVR_OVERRIDE;

    /**
     * Notifies the server that a tracked device's pose has been updated.
     */
    virtual void TrackedDevicePoseUpdated(uint32_t which_device, const vr::DriverPose_t& new_pose) OSVR_OVERRIDE;

    /**
     * Notifies the server that the property cache for the specified device
     * should be invalidated.
     */
    virtual void TrackedDevicePropertiesChanged(uint32_t which_device) OSVR_OVERRIDE;

    /**
     * Notifies the server that vsync has occurred on the the display attached
     * to the device. This is only permitted on devices of the HMD class.
     */
    virtual void VsyncEvent(double vsync_time_offset_seconds) OSVR_OVERRIDE;

    /**
     * Notifies the server that the button was pressed.
     */
    virtual void TrackedDeviceButtonPressed(uint32_t which_device, vr::EVRButtonId button_id, double event_time_offset) OSVR_OVERRIDE;

    /**
     * Notifies the server that the button was unpressed.
     */
    virtual void TrackedDeviceButtonUnpressed(uint32_t which_device, vr::EVRButtonId button_id, double event_time_offset) OSVR_OVERRIDE;

    /**
     * Notifies the server that the button was pressed.
     */
    virtual void TrackedDeviceButtonTouched(uint32_t which_device, vr::EVRButtonId button_id, double event_time_offset) OSVR_OVERRIDE;

    /**
     * Notifies the server that the button was unpressed.
     */
    virtual void TrackedDeviceButtonUntouched(uint32_t which_device, vr::EVRButtonId button_id, double event_time_offset) OSVR_OVERRIDE;

    /**
     * Notifies the server than a controller axis changed.
     */
    virtual void TrackedDeviceAxisUpdated(uint32_t which_device, uint32_t which_axis, const vr::VRControllerAxis_t& axis_state) OSVR_OVERRIDE;

    /**
     * Notifies the server that the MC image has been updated for the display
     * attached to the device. This is only permitted on devices of the HMD
     * class.
     */
    virtual void MCImageUpdated() OSVR_OVERRIDE;

    /**
     * Always returns a pointer to a valid interface pointer of vr::IVRSettings.
     */
    virtual vr::IVRSettings* GetSettings(const char* interface_version) OSVR_OVERRIDE;

    /**
     * Notifies the server that the physical IPD adjustment has been moved on
     * the HMD.
     */
    virtual void PhysicalIpdSet(uint32_t which_device, float physical_ipd_meters) OSVR_OVERRIDE;

    /**
     * Notifies the server that the proximity sensor on the specified device.
     */
    virtual void ProximitySensorState(uint32_t which_device, bool proximity_sensor_triggered) OSVR_OVERRIDE;

    /**
     * Sends a vendor specific event
     * (VREvent_VendorSpecific_Reserved_Start..VREvent_VendorSpecific_Reserved_End.
     */
    virtual void VendorSpecificEvent(uint32_t which_device, vr::EVREventType event_type, const vr::VREvent_Data_t& event_data, double event_time_offset) OSVR_OVERRIDE;

    /**
     * Returns true if SteamVR is exiting.
     */
    virtual bool IsExiting() OSVR_OVERRIDE;

protected:
    VRSettings vrSettings_;
};

#endif // INCLUDED_ServerDriverHost_h_GUID_05FFC5CC_18FD_4583_B460_627577D0765E

