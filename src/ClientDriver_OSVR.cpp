/** @file
    @brief OSVR client driver for OpenVR

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

#include <ClientDriver_OSVR.h>

vr::EVRInitError ClientDriver_OSVR::Init(vr::IDriverLog* driver_log, vr::IClientDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir)
{
    logger_ = driver_log;
    driverHost_ = driver_host;
    userDriverConfigDir_ = user_driver_config_dir;
    driverInstallDir_ = driver_install_dir;

    // TODO ?

    return vr::VRInitError_None;
}

void ClientDriver_OSVR::Cleanup()
{
    logger_ = nullptr;
    driverHost_ = nullptr;
    userDriverConfigDir_.clear();
    driverInstallDir_.clear();
}

bool ClientDriver_OSVR::BIsHmdPresent(const char* user_config_dir)
{
    // TODO
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

uint32_t ClientDriver_OSVR::GetMCImage(uint32_t *pImgWidth, uint32_t *pImgHeight, uint32_t *pChannels, void *pDataBuffer, uint32_t unBufferLen)
{
    // what is this function for? SteamVR drivers return 0, doing the same here.
    return 0;
}
