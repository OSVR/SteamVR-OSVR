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

#ifndef INCLUDED_OSVRTrackedDevice_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645
#define INCLUDED_OSVRTrackedDevice_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

// Internal Includes
#include "osvr_compiler_detection.h"    // for OSVR_OVERRIDE
#include "Settings.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <osvr/Display/Display.h>
#include <osvr/RenderKit/DistortionParameters.h>
#include <osvr/RenderKit/UnstructuredMeshInterpolator.h>
#include <osvr/RenderKit/osvr_display_configuration.h>

// Standard includes
#include <string>
#include <memory>
#include <vector>
#include <map>

class OSVRTrackedDevice : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent {
friend class ServerDriver_OSVR;
public:
    OSVRTrackedDevice(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, vr::IDriverLog* driver_log = nullptr);

    virtual ~OSVRTrackedDevice();
    // ------------------------------------
    // Management Methods
    // ------------------------------------
    /**
     * This is called before an HMD is returned to the application. It will
     * always be called before any display or tracking methods. Memory and
     * processor use by the ITrackedDeviceServerDriver object should be kept to
     * a minimum until it is activated.  The pose listener is guaranteed to be
     * valid until Deactivate is called, but should not be used after that
     * point.
     */
    virtual vr::EVRInitError Activate(uint32_t object_id) OSVR_OVERRIDE;

    /**
     * This is called when The VR system is switching from this Hmd being the
     * active display to another Hmd being the active display. The driver should
     * clean whatever memory and thread use it can when it is deactivated.
     */
    virtual void Deactivate() OSVR_OVERRIDE;

    /**
     * Handles a request from the system to put this device into standby mode.
     * What that means is defined per-device.
     */
    virtual void EnterStandby() OSVR_OVERRIDE;

    /**
     * Requests a component interface of the driver for device-specific
     * functionality. The driver should return NULL if the requested interface
     * or version is not supported.
     */
    virtual void* GetComponent(const char* component_name_and_version) OSVR_OVERRIDE;

    /**
     * A VR Client has made this debug request of the driver. The set of valid
     * requests is entirely up to the driver and the client to figure out, as is
     * the format of the response. Responses that exceed the length of the
     * supplied buffer should be truncated and null terminated.
     */
    virtual void DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size) OSVR_OVERRIDE;

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

    // ------------------------------------
    // Tracking Methods
    // ------------------------------------
    virtual vr::DriverPose_t GetPose() OSVR_OVERRIDE;

    // ------------------------------------
    // Property Methods
    // ------------------------------------

    /**
     * Returns a bool property. If the property is not available this function
     * will return false.
     */
    virtual bool GetBoolTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a float property. If the property is not available this function
     * will return 0.
     */
    virtual float GetFloatTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns an int property. If the property is not available this function
     * will return 0.
     */
    virtual int32_t GetInt32TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a uint64 property. If the property is not available this function
     * will return 0.
     */
    virtual uint64_t GetUint64TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a matrix property. If the device index is not valid or the
     * property is not a matrix type, this function will return identity.
     */
    virtual vr::HmdMatrix34_t GetMatrix34TrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

    /**
     * Returns a string property. If the property is not available this function
     * will return 0 and @p error will be set to an error. Otherwise it returns
     * the length of the number of bytes necessary to hold this string including
     * the trailing null. If the buffer is too small the error will be
     * @c TrackedProp_BufferTooSmall. Drivers may not return strings longer
     * than @c k_unMaxPropertyStringSize.
     */
    virtual uint32_t GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, char* value, uint32_t buffer_size, vr::ETrackedPropertyError* error) OSVR_OVERRIDE;

protected:
    const char* GetId();

private:
    std::string GetStringTrackedDeviceProperty(vr::ETrackedDeviceProperty prop, vr::ETrackedPropertyError *error);

    /**
     * Callback function which is called whenever new data has been received
     * from the tracker.
     */
    static void HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timestamp, const OSVR_PoseReport* report);

    float GetIPD();

    /**
     * Read configuration settings from configuration file.
     */
    void configure();

    /**
     * Configure RenderManager and distortion parameters.
     */
    void configureDistortionParameters();

    /**
     * Cecks to see if the requested property is valid for the device class and
     * type requested.
     *
     * @tparam T type of value requested
     * @param prop property requested
     *
     * @return vr::TrackedProp_Success if the checks pass, other
     * vr::ETrackedPropertyError values on failure
     */
    template <typename T>
    vr::ETrackedPropertyError checkProperty(vr::ETrackedDeviceProperty prop, const T&) const;

    /**
     * Parses a string into a scan-out origin option.
     */
    osvr::display::ScanOutOrigin parseScanOutOrigin(std::string str) const;

    /**
     * Gets the vertical refresh rate of the display.
     */
    double getVerticalRefreshRate() const;

    /**
     * Rotates a normalized (u, v) texture coordinate by a rotation
     * (counter-clockwise).
     */
    std::pair<float, float> rotate(float u, float v, osvr::display::Rotation rotation) const;

    /**
     * Returns the EDID vendor ID of the HMD.
     *
     * If the EDID vendor ID value has been set in the driver_osvr section of
     * the SetamVR settings, that value will be returned.
     *
     * If the HMD is an OSVR HDK, it will attempt to communicate with the HDK
     * and determine the EDID vendor ID based on the firmware version.
     */
    std::uint32_t getEdidVendorId() const;

    osvr::clientkit::ClientContext& context_;
    std::string displayDescription_;
    osvr::clientkit::DisplayConfig displayConfig_;
    osvr::client::RenderManagerConfig renderManagerConfig_;
    vr::IServerDriverHost* driverHost_ = nullptr;
    osvr::clientkit::Interface trackerInterface_;
    vr::DriverPose_t pose_;
    vr::ETrackedDeviceClass deviceClass_;
    std::unique_ptr<Settings> settings_;
    uint32_t objectId_ = 0;
    std::vector<osvr::renderkit::DistortionParameters> distortionParameters_;
    OSVRDisplayConfiguration displayConfiguration_;

    // per-eye mesh interpolators
    using MeshInterpolators = std::vector<std::unique_ptr<osvr::renderkit::UnstructuredMeshInterpolator>>;
    MeshInterpolators leftEyeInterpolators_;
    MeshInterpolators rightEyeInterpolators_;

    float overfillFactor_ = 1.0; // TODO get from RenderManager

    // Settings
    bool verboseLogging_ = false;
    osvr::display::Display display_ = {};
    osvr::display::ScanOutOrigin scanoutOrigin_ = osvr::display::ScanOutOrigin::UpperLeft;
};

#endif // INCLUDED_OSVRTrackedDevice_h_GUID_128E3B29_F5FC_4221_9B38_14E3F402E645

