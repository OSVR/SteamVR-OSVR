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
#include "platform_fixes.h"         // strcasecmp
#include "make_unique.h"            // for std::make_unique
#include "Logging.h"                // for OSVR_LOG, Logging

// Library/third-party includes
#include <openvr_driver.h>          // for everything in vr namespace

#include <osvr/ClientKit/Context.h> // for osvr::clientkit::ClientContext
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <vector>                   // for std::vector
#include <cstring>                  // for std::strcmp
#include <string>                   // for std::string

vr::EVRInitError ServerDriver_OSVR::Init(vr::IVRDriverContext* driver_context)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(driver_context);

    Logging::instance().setDriverLog(vr::VRDriverLog());

    context_ = std::make_unique<osvr::clientkit::ClientContext>("org.osvr.SteamVR");

    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedHMD>(*(context_.get())));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackingReference>(*(context_.get())));

    for (auto& tracked_device : trackedDevices_) {
        vr::VRServerDriverHost()->TrackedDeviceAdded(tracked_device->getId(), tracked_device->getDeviceClass(), tracked_device.get());
    }

    return vr::VRInitError_None;
}

void ServerDriver_OSVR::Cleanup()
{
    trackedDevices_.clear();
    context_.reset();
    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

const char* const* ServerDriver_OSVR::GetInterfaceVersions()
{
    return vr::k_InterfaceVersions;
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

