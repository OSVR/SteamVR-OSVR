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

#include "OSVRTrackedDevice.h"      // for OSVRTrackedDevice
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

vr::EVRInitError ServerDriver_OSVR::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    if (vr::VRDriverLog())
        Logging::instance().setDriverLog(vr::VRDriverLog());

    context_ = std::make_unique<osvr::clientkit::ClientContext>("org.osvr.SteamVR");

    trackedDevice_ = std::make_unique<OSVRTrackedDevice>(*(context_.get()));
    vr::VRServerDriverHost()->TrackedDeviceAdded(trackedDevice_->GetId(), vr::TrackedDeviceClass_HMD, trackedDevice_.get());

    trackingReference_ = std::make_unique<OSVRTrackingReference>(*(context_.get()));
    vr::VRServerDriverHost()->TrackedDeviceAdded(trackingReference_->GetId(), vr::TrackedDeviceClass_TrackingReference, trackingReference_.get());
    return vr::VRInitError_None;
}

void ServerDriver_OSVR::Cleanup()
{
    trackedDevice_.reset();
    trackingReference_.reset();
    context_.reset();
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

