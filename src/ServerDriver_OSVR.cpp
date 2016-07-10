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

// Internal Includes
#include "ServerDriver_OSVR.h"

#include "OSVRTrackedHMD.h"         // for OSVRTrackedHMD
#include "OSVRTrackingReference.h"  // for OSVRTrackingReference
#include "OSVRTrackedController.h"  // for OSVRTrackedController
#include "platform_fixes.h"         // strcasecmp
#include "make_unique.h"            // for std::make_unique
#include "Logging.h"                // for OSVR_LOG, Logging

// Library/third-party includes
#include <openvr_driver.h>          // for everything in vr namespace

#include <osvr/ClientKit/Context.h> // for osvr::clientkit::ClientContext

// Standard includes
#include <vector>                   // for std::vector
#include <cstring>                  // for std::strcmp
#include <string>                   // for std::string

vr::EVRInitError ServerDriver_OSVR::Init(vr::IDriverLog* driver_log, vr::IServerDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir)
{
    driverLog_ = driver_log;
    driverHost_ = driver_host;
    userDriverConfigDir_ = user_driver_config_dir;
    driverInstallDir_ = driver_install_dir;

    configure();

    context_ = std::make_unique<osvr::clientkit::ClientContext>("org.osvr.SteamVR");

    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedHMD>(*(context_.get()), driver_host, userDriverConfigDir_));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackingReference>(*(context_.get()), driver_host, userDriverConfigDir_));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedController>(*(context_.get()), driver_host, userDriverConfigDir_, vr::TrackedControllerRole_LeftHand));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedController>(*(context_.get()), driver_host, userDriverConfigDir_, vr::TrackedControllerRole_RightHand));

    return vr::VRInitError_None;
}

void ServerDriver_OSVR::Cleanup()
{
    trackedDevices_.clear();
    context_.reset();
}

const char* const* ServerDriver_OSVR::GetInterfaceVersions()
{
    return vr::k_InterfaceVersions;
}

uint32_t ServerDriver_OSVR::GetTrackedDeviceCount()
{
    OSVR_LOG(info) << "ServerDriver_OSVR::GetTrackedDeviceCount(): Detected " << trackedDevices_.size() << " tracked devices.\n";
    return static_cast<uint32_t>(trackedDevices_.size());
}

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::GetTrackedDeviceDriver(uint32_t index)
{
    if (index >= trackedDevices_.size()) {
        OSVR_LOG(err) << "ServerDriver_OSVR::GetTrackedDeviceDriver(): ERROR: Index " << index << " is out of range [0.." << trackedDevices_.size() << "].\n";
        return nullptr;
    }

    OSVR_LOG(info) << "ServerDriver_OSVR::GetTrackedDeviceDriver(): Returning tracked device #" << index << ".\n";
    return trackedDevices_[index].get();
}

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::FindTrackedDeviceDriver(const char* id)
{
    for (auto& tracked_device : trackedDevices_) {
        const auto device_id = getDeviceId(tracked_device.get());
        if (0 == std::strcmp(id, device_id.c_str())) {
            OSVR_LOG(info) << "ServerDriver_OSVR::FindTrackedDeviceDriver(): Returning tracked device " << id << ".\n";
            return tracked_device.get();
        }
    }

    OSVR_LOG(err) << "ServerDriver_OSVR::FindTrackedDeviceDriver(): ERROR: Failed to locate device named '" << id << "'.\n";

    return nullptr;
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
    // TODO
}

void ServerDriver_OSVR::LeaveStandby()
{
    // TODO
}

std::string ServerDriver_OSVR::getDeviceId(vr::ITrackedDeviceServerDriver* device)
{
    char device_id[vr::k_unMaxPropertyStringSize];
    vr::ETrackedPropertyError property_error;
    device->GetStringTrackedDeviceProperty(vr::Prop_SerialNumber_String, device_id, vr::k_unMaxPropertyStringSize, &property_error);
    if (vr::TrackedProp_Success == property_error) {
        return device_id;
    } else {
        return "";
    }
}

void ServerDriver_OSVR::configure()
{
    settings_ = std::make_unique<Settings>(driverHost_->GetSettings(vr::IVRSettings_Version));

    Logging::instance().setDriverLog(driverLog_);

    // Get settings from config file
    const bool verbose_logging = settings_->getSetting<bool>("verbose", false);
    if (verbose_logging) {
        OSVR_LOG(info) << "Verbose logging enabled.";
        Logging::instance().setLogLevel(trace);
    } else {
        OSVR_LOG(info) << "Verbose logging disabled.";
        Logging::instance().setLogLevel(info);
    }

}
