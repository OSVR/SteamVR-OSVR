/** @file
    @brief OSVR tracked device

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

#ifndef INCLUDED_OSVRTrackedHMD_h_GUID_233AC6EA_4833_4EE2_B4ED_1F60A2208C9D
#define INCLUDED_OSVRTrackedHMD_h_GUID_233AC6EA_4833_4EE2_B4ED_1F60A2208C9D

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "display/Display.h"

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/ClientKit/Display.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <osvr/RenderKit/DistortionParameters.h>
#include <osvr/RenderKit/UnstructuredMeshInterpolator.h>
#include <osvr/RenderKit/osvr_display_configuration.h>

// Standard includes
#include <string>
#include <memory>
#include <vector>

class OSVRTrackedHMD : public OSVRTrackedDevice, public vr::IVRDisplayComponent {
friend class ServerDriver_OSVR;
public:
    OSVRTrackedHMD(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, const std::string& user_driver_config_dir);

    virtual ~OSVRTrackedHMD();

    // ------------------------------------
    // Management Methods
    // ------------------------------------

    virtual vr::EVRInitError Activate(uint32_t object_id) OSVR_OVERRIDE;
    virtual void Deactivate() OSVR_OVERRIDE;

    // ------------------------------------
    // Display Methods
    // ------------------------------------

    /**
     * Size and position that the window needs to be on the VR display.
     */
    virtual void GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * Returns true if the display is extending the desktop.
     */
    virtual bool IsDisplayOnDesktop() OSVR_OVERRIDE;

    /**
     * Returns true if the display is real and not a fictional display.
     */
    virtual bool IsDisplayRealDisplay() OSVR_OVERRIDE;

    /**
     * Suggested size for the intermediate render target that the distortion
     * pulls from.
     */
    virtual void GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * Gets the viewport in the frame buffer to draw the output of the distortion
     * into
     */
    virtual void GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height) OSVR_OVERRIDE;

    /**
     * The components necessary to build your own projection matrix in case your
     * application is doing something fancy like infinite Z
     */
    virtual void GetProjectionRaw(vr::EVREye eye, float* left, float* right, float* top, float* bottom) OSVR_OVERRIDE;

    /**
     * Returns the result of the distortion function for the specified eye and
     * input UVs. UVs go from 0,0 in the upper left of that eye's viewport and
     * 1,1 in the lower right of that eye's viewport.
     */
    virtual vr::DistortionCoordinates_t ComputeDistortion(vr::EVREye eye, float u, float v) OSVR_OVERRIDE;

private:
    /**
     * Callback function which is called whenever new data has been received
     * from the tracker.
     */
    static void HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);

    float GetIPD();

    /** \name Configure components and properties. */
    //@{
    void configure();
    void configureDisplay();
    void configureDistortionParameters();
    void configureProperties();
    //@}

    std::string displayDescription_;
    osvr::clientkit::DisplayConfig displayConfig_;
    osvr::client::RenderManagerConfig renderManagerConfig_;
    osvr::clientkit::Interface trackerInterface_;
    std::vector<osvr::renderkit::DistortionParameters> distortionParameters_;
    OSVRDisplayConfiguration displayConfiguration_;

    // per-eye mesh interpolators
    using MeshInterpolators = std::vector<std::unique_ptr<osvr::renderkit::UnstructuredMeshInterpolator>>;
    MeshInterpolators leftEyeInterpolators_;
    MeshInterpolators rightEyeInterpolators_;

    float overfillFactor_ = 1.0; // TODO get from RenderManager

    // Settings
    osvr::display::Display display_ = {};
};

#endif // INCLUDED_OSVRTrackedHMD_h_GUID_233AC6EA_4833_4EE2_B4ED_1F60A2208C9D

