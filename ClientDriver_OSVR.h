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
// - none

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
// - none

class ClientDriver_OSVR : public vr::IClientTrackedDeviceProvider
{
public:
    /**
     * Initializes the driver.
     *
     * This will be called before any other methods are called, except
     * BIsHmdPresent(). BIsHmdPresent() is called outside of the Init/Cleanup
     * pair.  If Init() returns anything other than HmdError_None the driver DLL
     * will be unloaded.
     *
     * @param user_driver_config_dir The absolute path of the directory where
     *     the driver should store user config files.
     * @param driver_install_dir The absolute path of the root directory for the
     *     driver.
     */
    virtual vr::EVRInitError Init(vr::IDriverLog* driver_log, vr::IClientDriverHost* driver_host, const char* user_driver_config_dir, const char* driver_install_dir) OSVR_OVERRIDE;

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
    virtual vr::HiddenAreaMesh_t GetHiddenAreaMesh(vr::EVREye eye) OSVR_OVERRIDE;

	/** Get the MC image for the current HMD.
	 * Returns the size in bytes of the buffer required to hold the specified resource.
     */
	virtual uint32_t GetMCImage( uint32_t *pImgWidth, uint32_t *pImgHeight, uint32_t *pChannels, void *pDataBuffer, uint32_t unBufferLen ) OSVR_OVERRIDE;

private:
    vr::IDriverLog* logger_ = nullptr;
    vr::IClientDriverHost* driverHost_ = nullptr;
    std::string userDriverConfigDir_;
    std::string driverInstallDir_;
};

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

uint32_t ClientDriver_OSVR::GetMCImage(uint32_t *pImgWidth, uint32_t *pImgHeight, uint32_t *pChannels, void *pDataBuffer, uint32_t unBufferLen )
{
    // what is this function for? SteamVR drivers return 0, doing the same here.
    return 0;
}

#endif // INCLUDED_ClientDriver_OSVR_h_GUID_7C0E8547_F8CF_4186_B637_9488CD6E3663
