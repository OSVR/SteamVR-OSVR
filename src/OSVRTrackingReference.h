/** @file
    @brief OSVR tracking reference (e.g., camera or base station)

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
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

#ifndef INCLUDED_OSVRTrackingReference_h_GUID_250D4551_4054_9D37_A8F6_F2FCFDB1C188
#define INCLUDED_OSVRTrackingReference_h_GUID_250D4551_4054_9D37_A8F6_F2FCFDB1C188

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "Settings.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/ClientKit.h>

// Standard includes
#include <string>
#include <memory>

class OSVRTrackingReference : public OSVRTrackedDevice {
friend class ServerDriver_OSVR;
public:
    OSVRTrackingReference(osvr::clientkit::ClientContext& context);

    virtual ~OSVRTrackingReference();

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

    // ------------------------------------
    // Tracking Methods
    // ------------------------------------
    virtual vr::DriverPose_t GetPose() OSVR_OVERRIDE;

    const char* getId();
    vr::ETrackedDeviceClass getDeviceClass() const;

private:
    std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error);
    static void TrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);

    /**
     * Read configuration settings from configuration file.
     */
    void configure();
    void setProperties();

    osvr::clientkit::Interface m_TrackerInterface;

    // Settings
    bool verboseLogging_ = false;
    std::string trackerPath_ = "/org_osvr_filter_videoimufusion/HeadFusion/semantic/camera";

    // Default values are those for the OSVR HDK IR camera
    float fovLeft_ = 35.235f; // degrees
    float fovRight_ = 35.235f; // degrees
    float fovTop_ = 27.95f; // degrees
    float fovBottom_ = 27.95f; // degrees

    float minTrackingRange_ = 0.15f; // meters
    float maxTrackingRange_ = 1.5f; // meters
};

#endif // INCLUDED_OSVRTrackingReference_h_GUID_4D3F2E76_D0A2_4876_A7E9_CF0E772B02EF

