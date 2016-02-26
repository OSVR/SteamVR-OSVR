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
#include "osvr_tracked_device.h" // for OSVRTrackedDevice

// Library/third-party includes
#include <openvr_driver.h> // for everything in vr namespace

#include <osvr/ClientKit/Context.h> // for osvr::clientkit::ClientContext

// Standard includes
#include <vector>  // for std::vector
#include <cstring> // for std::strcmp
#include <string>  // for std::string, std::to_string

class ServerDriver_OSVR : public vr::IServerTrackedDeviceProvider
{
public:
    /**
     * Initializes the driver.
     *
     * This is called when the driver is first loaded.
     *
     * @param user_config_dir the absoluate path of the directory where the
     *     driver should store any user configuration files.
     * @param driver_install_dir the absolute path of the driver's root
     *     directory.
     *
     * If Init() returns anything other than \c VRInitError_None the driver will be
     * unloaded.
     *
     * @returns VRInitError_None on success.
     */
    virtual vr::EVRInitError Init(vr::IDriverLog* driver_log, vr::IServerDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir) OSVR_OVERRIDE;

    /**
     * Performs any cleanup prior to the driver being unloaded.
     */
    virtual void Cleanup() OSVR_OVERRIDE;

    /**
     * Returns the number of tracked devices.
     */
    virtual uint32_t GetTrackedDeviceCount() OSVR_OVERRIDE;

    /**
     * Returns a single tracked device by its index.
     *
     * @param unWhich the index of the tracked device to return.
     * @param
     */
	virtual vr::ITrackedDeviceServerDriver *GetTrackedDeviceDriver( uint32_t unWhich, const char *pchInterfaceVersion ) OSVR_OVERRIDE;

    /**
     * Returns a single HMD by its name.
     *
     * @param pchId the C string name of the HMD.
     */
	virtual vr::ITrackedDeviceServerDriver* FindTrackedDeviceDriver( const char *pchId, const char *pchInterfaceVersion ) OSVR_OVERRIDE;

    /**
     * Allows the driver do to some work in the main loop of the server.
     */
    virtual void RunFrame() OSVR_OVERRIDE;

	/** Returns true if the driver wants to block Standby mode. */
	virtual bool ShouldBlockStandbyMode() OSVR_OVERRIDE;

	/** Called when the system is entering Standby mode. The driver should switch itself into whatever sort of low-power
	* state it has. */
	virtual void EnterStandby() OSVR_OVERRIDE;

	/** Called when the system is leaving Standby mode. The driver should switch itself back to
	full operation. */
	virtual void LeaveStandby() OSVR_OVERRIDE;

private:
    std::vector<std::unique_ptr<OSVRTrackedDevice>> trackedDevices_;
    std::unique_ptr<osvr::clientkit::ClientContext> context_;
    vr::IDriverLog* logger_;
};

vr::EVRInitError ServerDriver_OSVR::Init(vr::IDriverLog* driver_log, vr::IServerDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir)
{
    logger_ = driver_log;

    logger_->Log("ServerDriver_OSVR::Init() called.\n");
    context_ = std::make_unique<osvr::clientkit::ClientContext>("com.osvr.SteamVR");

    const std::string display_description = context_->getStringParameter("/display");
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedDevice>(display_description, *(context_.get()), driver_host, logger_));

    return vr::VRInitError_None;
}

void ServerDriver_OSVR::Cleanup()
{
    trackedDevices_.clear();
    context_.reset();
}

uint32_t ServerDriver_OSVR::GetTrackedDeviceCount()
{
    std::string msg = "ServerDriver_OSVR::GetTrackedDeviceCount(): Detected " + std::to_string(trackedDevices_.size()) + " tracked devices.\n";
    logger_->Log(msg.c_str());
    return trackedDevices_.size();
}

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::GetTrackedDeviceDriver(uint32_t unWhich, const char *pchInterfaceVersion)
{
    if ( 0 != strcasecmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) ) {
        std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): ERROR: Incompatible SteamVR version!\n";
        logger_->Log(msg.c_str());
        return NULL;
    }

    if (unWhich >= trackedDevices_.size()) {
        std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): ERROR: Index " + std::to_string(unWhich) + " is out of range [0.." + std::to_string(trackedDevices_.size()) + "].\n";
        logger_->Log(msg.c_str());
        return NULL;
    }

    std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): Returning tracked device " + std::to_string(unWhich) + ".\n";
    logger_->Log(msg.c_str());
    return trackedDevices_[unWhich].get();
}

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::FindTrackedDeviceDriver(const char *pchId, const char *pchInterfaceVersion)
{
    if ( 0 != strcasecmp( pchInterfaceVersion, vr::ITrackedDeviceServerDriver_Version ) ) {
        std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): ERROR: Incompatible SteamVR version!\n";
        logger_->Log(msg.c_str());
        return NULL;
    }

    for (auto& tracked_device : trackedDevices_) {
        if (0 == std::strcmp(pchId, tracked_device->GetId())) {
            std::string msg = "ServerDriver_OSVR::FindTrackedDeviceDriver(): Returning tracked device " + std::string(pchId) + ".\n";
            logger_->Log(msg.c_str());
            return tracked_device.get();
        }
    }

    std::string msg = "ServerDriver_OSVR::FindTrackedDeviceDriver(): ERROR: Failed to locate device named '" + std::string(pchId) + "'.\n";
    logger_->Log(msg.c_str());
    return NULL;
}

void ServerDriver_OSVR::RunFrame()
{
    context_->update();
}

bool ServerDriver_OSVR::ShouldBlockStandbyMode()
{
    return false;
}

void ServerDriver_OSVR::EnterStandby()
{
}

void ServerDriver_OSVR::LeaveStandby()
{
}


#endif // INCLUDED_ServerDriver_OSVR_h_GUID_136B1359_C29D_4198_9CA0_1C223CC83B84
