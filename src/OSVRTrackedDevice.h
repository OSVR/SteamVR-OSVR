/** @file
    @brief Header

    @date 2017

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2017 Sensics, Inc.
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

#ifndef INCLUDED_OSVRTrackedDevice_h_GUID_B9C023D1_81C6_4FC7_B994_1614E86C861C
#define INCLUDED_OSVRTrackedDevice_h_GUID_B9C023D1_81C6_4FC7_B994_1614E86C861C

// Internal Includes
#include "Settings.h"
#include "osvr_compiler_detection.h"

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/ClientKit/Context.h>

// Standard includes
#include <string>
#include <memory>

class OSVRTrackedDevice : public vr::ITrackedDeviceServerDriver {
public:
    OSVRTrackedDevice(osvr::clientkit::ClientContext& context, vr::ETrackedDeviceClass device_class, const std::string& name = "OSVR device");
    virtual ~OSVRTrackedDevice();

    /** \name Management Methods */
    //@{
    /**
     * This is called before a device is returned to the application. It will
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

    /**
     * Handles a request from the system to put this device into standby mode.
     * What that means is defined per-device.
     */
    virtual void EnterStandby() OSVR_OVERRIDE;

    /**
     * Requests a component interface of the driver for device-specific
     * functionality. The driver should return NULL if the requested interface
     * or version is not supported.
     */
    virtual void* GetComponent(const char* component_name_and_version) OSVR_OVERRIDE;

    /**
     * A VR Client has made this debug request of the driver. The set of valid
     * requests is entirely up to the driver and the client to figure out, as is
     * the format of the response. Responses that exceed the length of the
     * supplied buffer should be truncated and null terminated.
     */
    virtual void DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size) OSVR_OVERRIDE;
    //@}

    /** \name Tracking Methods */
    //@{
    virtual vr::DriverPose_t GetPose() OSVR_OVERRIDE;
    //@}

    /** \name Helper functions */
    //@{
    const char* getId();
    vr::ETrackedDeviceClass getDeviceClass() const;
    std::string getName() const;
    //@}

protected:
    void setSerialNumber(const std::string& serial_number);

    osvr::clientkit::ClientContext& context_;
    vr::ETrackedDeviceClass deviceClass_ = vr::TrackedDeviceClass_Invalid;
    std::string name_;
    vr::DriverPose_t pose_;
    uint32_t objectId_ = vr::k_unTrackedDeviceIndexInvalid;
    std::string serialNumber_;
    std::unique_ptr<Settings> settings_;
    vr::PropertyContainerHandle_t propertyContainer_ = vr::k_ulInvalidPropertyContainer;

private:

};

inline void OSVRTrackedDevice::setSerialNumber(const std::string& serial_number)
{
    serialNumber_ = serial_number;
}

#endif // INCLUDED_OSVRTrackedDevice_h_GUID_B9C023D1_81C6_4FC7_B994_1614E86C861C

