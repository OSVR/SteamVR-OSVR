/** @file
    @brief OSVR driver provider

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
#include "ServerDriver_OSVR.h" // for ServerDriver_OSVR
#include "ClientDriver_OSVR.h" // for ClientDriver_OSVR

#include "osvr_dll_export.h" // for OSVR_DLL_EXPORT

// Library/third-party includes
#include <openvr_driver.h> // for everything in vr namespace

// Standard includes
#include <cstring> // for std::strcmp

static ServerDriver_OSVR g_ServerDriverOSVR;
static ClientDriver_OSVR g_ClientDriverOSVR;

OSVR_DLL_EXPORT void* TrackedDeviceDriverFactory(const char* interface_name, int* return_code)
{
    if (0 == std::strcmp(vr::IServerTrackedDeviceProvider_Version, interface_name)) {
        return &g_ServerDriverOSVR;
    }

    if (0 == std::strcmp(vr::IClientTrackedDeviceProvider_Version, interface_name)) {
        return &g_ClientDriverOSVR;
    }

    if (return_code) {
        *return_code = vr::VRInitError_Init_InterfaceNotFound;
    }

    return NULL;
}

OSVR_DLL_EXPORT void* HmdDriverFactory(const char* interface_name, int* return_code)
{
    return TrackedDeviceDriverFactory(interface_name, return_code);
}
