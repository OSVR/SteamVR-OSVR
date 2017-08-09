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
#include "OSVRTrackedController.h"  // for OSVRTrackingReference
#include "platform_fixes.h"         // strcasecmp
#include "make_unique.h"            // for std::make_unique
#include "Logging.h"                // for OSVR_LOG, Logging
#include "Version.h"                // for STEAMVR_OSVR_VERSION

// Library/third-party includes
#include <openvr_driver.h>          // for everything in vr namespace

#include <osvr/ClientKit/Context.h> // for osvr::clientkit::ClientContext
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <vector>                   // for std::vector
#include <cstring>                  // for std::strcmp
#include <string>                   // for std::string
#include <chrono>
#include <thread>
#include <atomic>

namespace {

static std::thread client_update_thread;
static std::atomic<bool> client_update_thread_quit;
static std::atomic<int> client_update_thread_ms_wait;
static void client_update_thread_work(osvr::clientkit::ClientContext& ctx)
{
    while (!client_update_thread_quit.load()) {
        ctx.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(client_update_thread_ms_wait.load()));
    }
    client_update_thread_quit = false;
}

} // anonymous namespace

vr::EVRInitError ServerDriver_OSVR::Init(vr::IVRDriverContext* driver_context)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(driver_context);

    Logging::instance().setDriverLog(vr::VRDriverLog());
    OSVR_LOG(notice) << "SteamVR-OSVR version " << STEAMVR_OSVR_VERSION;

    settings_ = std::make_unique<Settings>();

    // Verbose logging
    const auto verbose = settings_->getSetting<bool>("verbose", false);
    Logging::instance().setLogLevel(verbose ? trace : info);
    OSVR_LOG(info) << "Verbose logging " << (verbose ? "enabled" : "disabled") << ".";

    // Client loop update rate
    standbyWaitPeriod_ = settings_->getSetting<int>("standbyWaitPeriod", 100);
    activeWaitPeriod_ = settings_->getSetting<int>("activeWaitPeriod", 1);
    OSVR_LOG(debug) << "Standby wait period is " << standbyWaitPeriod_ << " ms.";
    OSVR_LOG(debug) << "Active wait period is " << activeWaitPeriod_ << " ms.";

    context_ = std::make_unique<osvr::clientkit::ClientContext>("org.osvr.SteamVR");

    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedHMD>(*(context_.get())));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackingReference>(*(context_.get())));
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedController>(*(context_.get()), 0)); // left hand
    trackedDevices_.emplace_back(std::make_unique<OSVRTrackedController>(*(context_.get()), 1)); // right hand

    for (auto& tracked_device : trackedDevices_) {
        OSVR_LOG(debug) << "ServerDriver_OSVR - for loop - before TrackedDeviceAdded\n";
        vr::VRServerDriverHost()->TrackedDeviceAdded(tracked_device->getId(), tracked_device->getDeviceClass(), tracked_device.get());
        OSVR_LOG(debug) << "ServerDriver_OSVR - for loop - after TrackedDeviceAdded\n";
    }

    client_update_thread_quit.store(false);
    client_update_thread_ms_wait.store(activeWaitPeriod_);
    client_update_thread = std::thread(client_update_thread_work, std::ref(*context_));

    return vr::VRInitError_None;
}

void ServerDriver_OSVR::Cleanup()
{
    client_update_thread_quit.store(true);
    if (client_update_thread.joinable()) {
        client_update_thread.join();
    }

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
    // do nothing
}

bool ServerDriver_OSVR::ShouldBlockStandbyMode()
{
    return false;
}

void ServerDriver_OSVR::EnterStandby()
{
    OSVR_LOG(debug) << "Entering standby mode...";
    client_update_thread_ms_wait.store(standbyWaitPeriod_);
}

void ServerDriver_OSVR::LeaveStandby()
{
    OSVR_LOG(debug) << "Leaving standby mode...";
    client_update_thread_ms_wait.store(activeWaitPeriod_);
}

