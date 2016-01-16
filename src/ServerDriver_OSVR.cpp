/** @file
    @brief OSVR server driver for OpenVR

    @date 2016

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

#include <ServerDriver_OSVR.h>

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

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::GetTrackedDeviceDriver(uint32_t index)
{
    if (index >= trackedDevices_.size()) {
        std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): ERROR: Index " + std::to_string(index) + " is out of range [0.." + std::to_string(trackedDevices_.size()) + "].\n";
        logger_->Log(msg.c_str());
        return NULL;
    }

    std::string msg = "ServerDriver_OSVR::GetTrackedDeviceDriver(): Returning tracked device " + std::to_string(index) + ".\n";
    logger_->Log(msg.c_str());
    return trackedDevices_[index].get();
}

vr::ITrackedDeviceServerDriver* ServerDriver_OSVR::FindTrackedDeviceDriver(const char* id)
{
    for (auto& tracked_device : trackedDevices_) {
        if (0 == std::strcmp(id, tracked_device->GetId())) {
            std::string msg = "ServerDriver_OSVR::FindTrackedDeviceDriver(): Returning tracked device " + std::string(id) + ".\n";
            logger_->Log(msg.c_str());
            return tracked_device.get();
        }
    }

    std::string msg = "ServerDriver_OSVR::FindTrackedDeviceDriver(): ERROR: Failed to locate device named '" + std::string(id) + "'.\n";
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
