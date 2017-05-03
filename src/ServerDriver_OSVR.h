/** @file
    @brief OSVR server driver for OpenVR

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

#ifndef INCLUDED_ServerDriver_OSVR_h_GUID_136B1359_C29D_4198_9CA0_1C223CC83B84
#define INCLUDED_ServerDriver_OSVR_h_GUID_136B1359_C29D_4198_9CA0_1C223CC83B84

// Internal Includes
#include "OSVRTrackedDevice.h"          // for OSVRTrackedDevice
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "Settings.h"                   // for Settings

// Library/third-party includes
#include <openvr_driver.h>              // for everything in vr namespace

#include <osvr/ClientKit/Context.h>     // for osvr::clientkit::ClientContext

// Standard includes
#include <vector>                       // for std::vector
#include <cstring>                      // for std::strcmp
#include <string>                       // for std::string, std::to_string
#include <memory>                       // for std::unique_ptr

/**
 * This interface is loaded in vrserver.exe.
 */
class ServerDriver_OSVR : public vr::IServerTrackedDeviceProvider {
public:
    ServerDriver_OSVR() = default;
    virtual ~ServerDriver_OSVR() = default;

    /**
     * Initializes the driver.
     *
     * This is called when the driver is first loaded.
     *
     * @param driver_context This interface is provided by vrserver to allow the
     *     driver to notify the system when something changes about a device.
     *     These changes must not change the serial number or class of the
     *     device because those values are permanently associated with the
     *     device's index.
     *
     * If Init() returns anything other than \c VRInitError_None the driver will be
     * unloaded.
     *
     * @returns VRInitError_None on success.
     */
    virtual vr::EVRInitError Init(vr::IVRDriverContext* driver_context) OSVR_OVERRIDE;

    /**
     * Performs any cleanup prior to the driver being unloaded.
     */
    virtual void Cleanup() OSVR_OVERRIDE;

    /**
     * Returns the version of the @c ITrackedDeviceServerDriver interface used by
     * this driver.
     */
    virtual const char* const* GetInterfaceVersions() OSVR_OVERRIDE;

    /**
     * Allows the driver do to some work in the main loop of the server.
     */
    virtual void RunFrame() OSVR_OVERRIDE;

    /** \name Power state functions */
    //@{
    /**
     * Returns @c true if the driver wants to block Standby mode.
     */
    virtual bool ShouldBlockStandbyMode() OSVR_OVERRIDE;

    /**
     * Called when the system is entering Standby mode.
     *
     * The driver should switch itself into whatever sort of low-power state it
     * has.
     */
    virtual void EnterStandby() OSVR_OVERRIDE;

    /**
     * Called when the system is leaving Standby mode.
     *
     * The driver should switch itself back to full operation.
     */
    virtual void LeaveStandby() OSVR_OVERRIDE;
    //@}

private:
    //std::vector<std::unique_ptr<vr::ITrackedDeviceServerDriver>> trackedDevices_;
    std::vector<std::unique_ptr<OSVRTrackedDevice>> trackedDevices_;
    std::unique_ptr<osvr::clientkit::ClientContext> context_;
    std::unique_ptr<Settings> settings_;
    int standbyWaitPeriod_ = 100; // ms
    int activeWaitPeriod_ = 1; // ms
};

#endif // INCLUDED_ServerDriver_OSVR_h_GUID_136B1359_C29D_4198_9CA0_1C223CC83B84

