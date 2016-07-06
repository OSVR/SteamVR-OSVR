/** @file
    @brief OSVR client driver for OpenVR

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
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "ClientDriver_OSVR.h"
#include "make_unique.h"
#include "Logging.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none

vr::EVRInitError ClientDriver_OSVR::Init(vr::EClientDriverMode driver_mode, vr::IDriverLog* driver_log, vr::IClientDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir)
{
    if (driver_log)
        Logging::instance().setDriverLog(driver_log);

    driverHost_ = driver_host;
    userDriverConfigDir_ = user_driver_config_dir;
    driverInstallDir_ = driver_install_dir;
    settings_ = std::make_unique<Settings>(driver_host->GetSettings(vr::IVRSettings_Version));

    // We don't support watchdog mode
    if (vr::ClientDriverMode_Watchdog == driver_mode) {
        return vr::VRInitError_Init_LowPowerWatchdogNotSupported;
    }

    return vr::VRInitError_None;
}

void ClientDriver_OSVR::Cleanup()
{
    driverHost_ = nullptr;
    userDriverConfigDir_.clear();
    driverInstallDir_.clear();
    settings_.reset();
}

bool ClientDriver_OSVR::BIsHmdPresent(const char* user_config_dir)
{
    // Optimistically return true. We'll have a chance to say whether there's
    // actually an HMD later.
    return true;
}

vr::EVRInitError ClientDriver_OSVR::SetDisplayId(const char* display_id)
{
    // TODO
    return vr::VRInitError_None;
}

vr::HiddenAreaMesh_t ClientDriver_OSVR::GetHiddenAreaMesh(vr::EVREye eye)
{
    vr::HiddenAreaMesh_t hidden_area_mesh;
    hidden_area_mesh.pVertexData = nullptr;
    hidden_area_mesh.unTriangleCount = 0;

    return hidden_area_mesh;
}

uint32_t ClientDriver_OSVR::GetMCImage(uint32_t* img_width, uint32_t* img_height, uint32_t* channels, void* data_buffer, uint32_t buffer_len)
{
    return 0;
}

