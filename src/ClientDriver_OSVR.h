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

#ifndef INCLUDED_ClientDriver_OSVR_h_GUID_7C0E8547_F8CF_4186_B637_9488CD6E3663
#define INCLUDED_ClientDriver_OSVR_h_GUID_7C0E8547_F8CF_4186_B637_9488CD6E3663

// Internal Includes
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "Settings.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <string>
#include <memory>

class ClientDriver_OSVR : public vr::IClientTrackedDeviceProvider {
public:
    ClientDriver_OSVR() = default;
    virtual ~ClientDriver_OSVR() = default;

    /**
     * Initializes the driver.
     *
     * This will be called before any other methods are called, except
     * BIsHmdPresent(). BIsHmdPresent() is called outside of the Init() /
     * Cleanup() pair.
     *
     * If Init() returns anything other than vr::EVRInitError::VRInitError_None
     * the driver DLL will be unloaded.
     *
     * @param driver_mode When this is passed as @c ClientDriverMode_Watchdog the
     * driver should enter a low-power state where hardware is being monitored.
     * If the driver does not support watchdog mode it should return
     * @c VRInitError_Init_LowPowerWatchdogNotSupported.
     *
     * @param driver_log a (potentially NULL) pointer to a vr::IDriverLog.
     *
     * @param driver_host will never be NULL, and will always be a pointer to a
     * vr::IServerDriverHost interface
     *
     * @param user_driver_config_dir The absolute path of the directory where
     * the driver should store user config files.
     *
     * @param driver_install_dir The absolute path of the root directory for the
     * driver.
     */
    virtual vr::EVRInitError Init(vr::EClientDriverMode driver_mode, vr::IDriverLog* driver_log, vr::IClientDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir) OSVR_OVERRIDE;

    /**
     * Cleans up the driver right before it is unloaded.
     */
    virtual void Cleanup() OSVR_OVERRIDE;

    /**
     * Called when the client needs to inform an application if an HMD is
     * attached that uses this driver.
     *
     * This method should be as lightweight as possible and should have no side
     * effects such as hooking process functions or leaving resources loaded.
     * Init() will not be called before this method and Cleanup() will not be called
     * after it.
     *
     * @param user_config_dir The absolute path of the directory where
     * the driver should store user config files.
     */
    virtual bool BIsHmdPresent(const char* user_config_dir) OSVR_OVERRIDE;

    /**
     * Called when the client inits an HMD to let the client driver know which
     * one is in use.
     */
    virtual vr::EVRInitError SetDisplayId(const char* display_id) OSVR_OVERRIDE;

    /**
     * Returns the stencil mesh information for the current HMD. If this HMD
     * does not have a stencil mesh the vertex data and count will be NULL and 0
     * respectively. This mesh is meant to be rendered into the stencil buffer
     * (or into the depth buffer setting nearz) before rendering each eye's
     * view. The pixels covered by this mesh will never be seen by the user
     * after the lens distortion is applied and based on visibility to the
     * panels.  This will improve perf by letting the GPU early-reject pixels
     * the user will never see before running the pixel shader.  NOTE: Render
     * this mesh with backface culling disabled since the winding order of the
     * vertices can be different per-HMD or per-eye.
     */
    virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh(vr::EVREye eye, vr::EHiddenAreaMeshType type) OSVR_OVERRIDE;

    /**
     * Get the MC image for the current HMD.
     *
     * @return Returns the size in bytes of the buffer required to hold the specified resource.
     */
    virtual uint32_t GetMCImage(uint32_t* img_width, uint32_t* img_height, uint32_t* channels, void* data_buffer, uint32_t buffer_len) OSVR_OVERRIDE;

private:
    vr::IClientDriverHost* driverHost_ = nullptr;
    std::string userDriverConfigDir_;
    std::string driverInstallDir_;
    std::unique_ptr<Settings> settings_;
};

#endif // INCLUDED_ClientDriver_OSVR_h_GUID_7C0E8547_F8CF_4186_B637_9488CD6E3663

