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

// Internal Includes
#include "OSVRTrackedHMD.h"

#include "Logging.h"
#include "ValveStrCpy.h"
#include "make_unique.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "osvr_compiler_detection.h"
#include "platform_fixes.h" // strcasecmp

// Library/third-party includes
#include <osvr/Client/RenderManagerConfig.h>
#include <osvr/ClientKit/Display.h>
#include <osvr/Display/DisplayEnumerator.h>
#include <osvr/RenderKit/DistortionCorrectTextureCoordinate.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/PlatformConfig.h>
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <algorithm>        // for std::find
#include <cstring>
#include <ctime>
#include <exception>
#include <iostream>
#include <string>
#include <tuple>

OSVRTrackedHMD::OSVRTrackedHMD(osvr::clientkit::ClientContext& context, vr::IServerDriverHost* driver_host, const std::string& user_driver_config_dir) : OSVRTrackedDevice(context, driver_host, vr::TrackedDeviceClass_HMD, user_driver_config_dir, "OSVRTrackedHMD")
{
    OSVR_FunctionGuard("OSVRTrackedHMD::OSVRTrackedHMD()");
}

OSVRTrackedHMD::~OSVRTrackedHMD()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::~OSVRTrackedHMD()");
    // do nothing
}

vr::EVRInitError OSVRTrackedHMD::Activate(uint32_t object_id)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::Activate()");
    OSVRTrackedDevice::Activate(object_id);

    const std::time_t waitTime = 5; // wait up to 5 seconds for init

    trackerInterface_.free();

    // Ensure context is fully started up
    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate(): Waiting for the context to fully start up...\n";
    std::time_t startTime = std::time(nullptr);
    while (!context_.checkStatus()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Context startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }


    // Ensure display is fully started up
    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate(): Waiting for the display to fully start up, including receiving initial pose update...\n";
    displayConfig_ = osvr::clientkit::DisplayConfig(context_);
    startTime = std::time(nullptr);
    while (!displayConfig_.checkStartup()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Display startup timed out!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Verify valid display config
    if (displayConfig_.getNumViewers() < 1) {
        OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): At least one viewer must exist.\n";
        return vr::VRInitError_Driver_HmdDisplayNotFound;
    } else if (displayConfig_.getViewer(0).getNumEyes() < 2) {
        OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): At least two eyes must exist.\n";
        return vr::VRInitError_Driver_HmdDisplayNotFound;
    } else if ((displayConfig_.getViewer(0).getEye(0).getNumSurfaces() < 1) || (displayConfig_.getViewer(0).getEye(1).getNumSurfaces() < 1)) {
        OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): At least one surface must exist for each eye.\n";
        return vr::VRInitError_Driver_HmdDisplayNotFound;
    }

    auto configString = context_.getStringParameter("/renderManagerConfig");

    // If the /renderManagerConfig parameter is missing from the configuration
    // file, use an empty dictionary instead. This allows the render manager
    // config to zero out its values.
    if (configString.empty()) {
        OSVR_LOG(info) << "OSVRTrackedHMD::Activate(): Render Manager config is empty, using default values.\n";
        configString = "{}";
    }

    try {
        renderManagerConfig_.parse(configString);
    } catch(const std::exception& e) {
        OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Exception parsing Render Manager config: " << e.what() << "\n";
    }

    configure();

    // Register tracker callback
    trackerInterface_ = context_.getInterface("/me/head");
    trackerInterface_.registerCallback(&OSVRTrackedHMD::HmdTrackerCallback, this);

    return vr::VRInitError_None;
}

void OSVRTrackedHMD::Deactivate()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::Deactivate()");

    /// Have to force freeing here
    trackerInterface_.free();

    OSVRTrackedDevice::Deactivate();
}

void OSVRTrackedHMD::GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::GetWindowBounds()");
    int nDisplays = displayConfig_.getNumDisplayInputs();
    if (nDisplays != 1) {
        OSVR_LOG(err) << "OSVRTrackedHMD::OSVRTrackedHMD(): Unexpected display number of displays!\n";
    }
    osvr::clientkit::DisplayDimensions displayDims = displayConfig_.getDisplayDimensions(0);
    *x = renderManagerConfig_.getWindowXPosition(); // todo: assumes desktop display of 1920. get this from display config when it's exposed.
    *y = renderManagerConfig_.getWindowYPosition();
    *width = static_cast<uint32_t>(displayDims.width);
    *height = static_cast<uint32_t>(displayDims.height);

#if defined(OSVR_WINDOWS) || defined(OSVR_MACOSX)
    // ... until we've added code for other platforms
    *x = display_.position.x;
    *y = display_.position.y;

    // Windows always reports the widest dimension as width regardless of the
    // orientation of the display. We need to flip these dimensions if the
    // display is in portrait orientation.
    //
    // OS X reports the resolution with respect to the orientation (e.g., in
    // portrait mode, a display's resolution might be 1080x1920).
    //
    // TODO Check to see how Linux handles this.
    const auto orientation = osvr::display::getDesktopOrientation(display_);
    const bool is_portrait = (osvr::display::DesktopOrientation::Portrait == orientation || osvr::display::DesktopOrientation::PortraitFlipped == orientation);
    if (is_portrait) {
        *height = std::max(display_.size.width, display_.size.height);
        *width = std::min(display_.size.width, display_.size.height);
    } else {
        *height = std::min(display_.size.width, display_.size.height);
        *width = std::max(display_.size.width, display_.size.height);
    }
#endif // OSVR_WINDOWS or OSVR_MACOSX

    OSVR_LOG(trace) << "GetWindowBounds(): x = " << *x << ", y = " << *y << ", width = " << *width << ", height = " << *height << ".";
}

bool OSVRTrackedHMD::IsDisplayOnDesktop()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::IsDisplayOnDesktop()");
    // If the current display still appears in the active displays list,
    // then it's attached to the desktop.
    const auto displays = osvr::display::getDisplays();
    const auto display_on_desktop = (end(displays) != std::find(begin(displays), end(displays), display_));
    OSVR_LOG(trace) << "OSVRTrackedHMD::IsDisplayOnDesktop(): " << (display_on_desktop ? "yes" : "no");
    return display_on_desktop;
}

bool OSVRTrackedHMD::IsDisplayRealDisplay()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::IsDisplayRealDisplay()");
    // TODO get this info from display description?
    return true;
}

void OSVRTrackedHMD::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::GetRecommendedRenderTargetSize()");
    /// @todo calculate overfill factor properly
    //const double overfill_factor = renderManagerConfig_.getRenderOverfillFactor();
    double overfill_factor = 1.0;
    int32_t x, y;
    uint32_t w, h;
    GetWindowBounds(&x, &y, &w, &h);

    *width = static_cast<uint32_t>(w * overfill_factor);
    *height = static_cast<uint32_t>(h * overfill_factor);

    OSVR_LOG(trace) << "GetRecommendedRenderTargetSize(): width = " << *width << ", height = " << *height << ".";
}

void OSVRTrackedHMD::GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::GetEyeOutputViewport()");
    int32_t display_x, display_y;
    uint32_t display_width, display_height;
    GetWindowBounds(&display_x, &display_y, &display_width, &display_height);

    // We have to duplicate this logic from OSVR-Core's DisplayConfig.cpp file
    // because that version doesn't handle the *detected* rotation, only the
    // rotation set in the config file.
    auto display_mode = displayConfiguration_.getDisplayMode();
    const auto orientation = scanoutOrigin_ + display_.rotation;

    // TODO Simplify this code after verifying it works properly
    if (OSVRDisplayConfiguration::DisplayMode::FULL_SCREEN == display_mode) {
        OSVR_LOG(trace) << "Display mode: full-screen.";
        *x = 0;
        *y = 0;
        *width = display_width;
        *height = display_height;
    } else if (OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE == display_mode) {
        OSVR_LOG(trace) << "Display mode: horizontal side-by-side.";
        using osvr::display::DesktopOrientation;
        if (DesktopOrientation::Portrait == orientation) {
            OSVR_LOG(trace) << "Display orientation: portrait.";
            *x = 0;
            *y = (vr::Eye_Left == eye) ? 0 : display_height / 2;
            *width = display_width;
            *height = display_height / 2;
        } else if (DesktopOrientation::PortraitFlipped == orientation) {
            OSVR_LOG(trace) << "Display orientation: portrait flipped.";
            *x = 0;
            *y = (vr::Eye_Left == eye) ? display_height / 2 : 0;
            *width = display_width;
            *height = display_height / 2;
        } else if (DesktopOrientation::Landscape == orientation) {
            OSVR_LOG(trace) << "Display orientation: landscape.";
            *x = (vr::Eye_Left == eye) ? 0 : display_width / 2;
            *y = 0;
            *width = display_width / 2;
            *height = display_height;
        } else if (DesktopOrientation::LandscapeFlipped == orientation) {
            OSVR_LOG(trace) << "Display orientation: landscape flipped.";
            *x = (vr::Eye_Left == eye) ? display_width / 2 : 0;
            *y = 0;
            *width = display_width / 2;
            *height = display_height;
        } else {
            OSVR_LOG(err) << "Unknown display orientation [" << static_cast<int>(orientation) << "]!";
        }
    } else if (OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE == display_mode) {
        OSVR_LOG(trace) << "Display mode: vertical side-by-side.";
        OSVR_LOG(err) << "This display mode hasn't been implemented yet!";
        // TODO
        *x = 0;
        *y = 0;
        *width = display_width;
        *height = display_height / 2;
    } else {
        OSVR_LOG(err) << "Unknown display mode [" << static_cast<int>(display_mode) << "]!";
    }

    const auto eye_str = (vr::Eye_Left == eye) ? "left" : "right";
    OSVR_LOG(trace) << "GetEyeOutputViewport(" << eye_str << " eye): x = " << *x << ", y = " << *y << ", width = " << *width << ", height = " << *height << ".";
}

void OSVRTrackedHMD::GetProjectionRaw(vr::EVREye eye, float* left, float* right, float* top, float* bottom)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::GetProjectionRaw()");
    // Reference: https://github.com/ValveSoftware/openvr/wiki/IVRSystem::GetProjectionRaw
    // SteamVR expects top and bottom to be swapped!
    osvr::clientkit::ProjectionClippingPlanes pl = displayConfig_.getViewer(0).getEye(eye).getSurface(0).getProjectionClippingPlanes();
    *left = static_cast<float>(pl.left);
    *right = static_cast<float>(pl.right);
    *bottom = static_cast<float>(pl.top); // SWAPPED
    *top = static_cast<float>(pl.bottom); // SWAPPED
}

vr::DistortionCoordinates_t OSVRTrackedHMD::ComputeDistortion(vr::EVREye eye, float u, float v)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::ComputeDistortion()");
#if 0
    OSVR_LOG(trace) << "OSVRTrackedHMD::ComputeDistortion(" << eye << ", " << u << ", " << v << ") called.";

    // Rotate the (u, v) coordinates as appropriate to the display orientation.
    const auto orientation = osvr::display::getDesktopOrientation(display_);
    std::tie(u, v) = rotateTextureCoordinates(orientation, u, v);

    // Note that RenderManager expects the (0, 0) to be the lower-left corner
    // and (1, 1) to be the upper-right corner while SteamVR assumes (0, 0) is
    // upper-left and (1, 1) is lower-right.  To accommodate this, we need to
    // flip the y-coordinate before passing it to RenderManager and flip it
    // again before returning the value to SteamVR.
    OSVR_LOG(trace) << "OSVRTrackedHMD::ComputeDistortion(" << eye << ", " << u << ", " << v << ") rotated.";

    using osvr::renderkit::DistortionCorrectTextureCoordinate;
    static const size_t COLOR_RED = 0;
    static const size_t COLOR_GREEN = 1;
    static const size_t COLOR_BLUE = 2;

    const auto osvr_eye = static_cast<size_t>(eye);
    const auto distortion_parameters = distortionParameters_[osvr_eye];
    const auto in_coords = osvr::renderkit::Float2 {{u, 1.0f - v}}; // flip v-coordinate

    const auto interpolators = (vr::Eye_Left == eye) ? &leftEyeInterpolators_ : &rightEyeInterpolators_;

    auto coords_red = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_RED, overfillFactor_, *interpolators);

    auto coords_green = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_GREEN, overfillFactor_, *interpolators);

    auto coords_blue = DistortionCorrectTextureCoordinate(
        osvr_eye, in_coords, distortion_parameters,
        COLOR_BLUE, overfillFactor_, *interpolators);

    vr::DistortionCoordinates_t coords;
    // flip v-coordinates again
    coords.rfRed[0] = coords_red[0];
    coords.rfRed[1] = 1.0f - coords_red[1];
    coords.rfGreen[0] = coords_green[0];
    coords.rfGreen[1] = 1.0f - coords_green[1];
    coords.rfBlue[0] = coords_blue[0];
    coords.rfBlue[1] = 1.0f - coords_blue[1];

    // Unrotate the coordinates
    const auto reverse_orientation = static_cast<osvr::display::DesktopOrientation>((4 - static_cast<int>(orientation)) % 4);
    std::tie(coords.rfRed[0], coords.rfRed[1]) = rotateTextureCoordinates(reverse_orientation, coords.rfRed[0], coords.rfRed[1]);
    std::tie(coords.rfGreen[0], coords.rfGreen[1]) = rotateTextureCoordinates(reverse_orientation, coords.rfGreen[0], coords.rfGreen[1]);
    std::tie(coords.rfBlue[0], coords.rfBlue[1]) = rotateTextureCoordinates(reverse_orientation, coords.rfBlue[0], coords.rfBlue[1]);

    return coords;
#endif

    OSVR_LOG(trace) << "OSVRTrackedHMD::ComputeDistortion(" << eye << ", " << u << ", " << v << ") called.";

    // Rotate the texture coordinates to match the display orientation
    const auto orientation = scanoutOrigin_ + display_.rotation;
    const auto desired_orientation = osvr::display::DesktopOrientation::Landscape;
    const auto rotation = desired_orientation - orientation;

    std::tie(u, v) = rotateTextureCoordinates(u, v, rotation);
    OSVR_LOG(trace) << "OSVRTrackedHMD::ComputeDistortion(" << eye << ", " << u << ", " << v << ") post-rotation.";

    // Skip distortion during testing
    // FIXME
    vr::DistortionCoordinates_t coords;
    coords.rfRed[0] = u;
    coords.rfRed[1] = v;
    coords.rfGreen[0] = u;
    coords.rfGreen[1] = v;
    coords.rfBlue[0] = u;
    coords.rfBlue[1] = v;

    return coords;
}

void OSVRTrackedHMD::HmdTrackerCallback(void* userdata, const OSVR_TimeValue*, const OSVR_PoseReport* report)
{
    OSVR_FunctionGuard("OSVRTrackedHMD::HmdTrackerCallback()");
    if (!userdata || !report)
        return;

    auto* self = static_cast<OSVRTrackedHMD*>(userdata);

    vr::DriverPose_t pose;
    pose.poseTimeOffset = 0; // close enough

    Eigen::Vector3d::Map(pose.vecWorldFromDriverTranslation) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecDriverFromHeadTranslation) = Eigen::Vector3d::Zero();

    map(pose.qWorldFromDriverRotation) = Eigen::Quaterniond::Identity();

    map(pose.qDriverFromHeadRotation) = Eigen::Quaterniond::Identity();

    // Position
    Eigen::Vector3d::Map(pose.vecPosition) = osvr::util::vecMap(report->pose.translation);

    // Position velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAcceleration) = Eigen::Vector3d::Zero();

    // Orientation
    map(pose.qRotation) = osvr::util::fromQuat(report->pose.rotation);

    // Angular velocity and acceleration are not currently consistently provided
    Eigen::Vector3d::Map(pose.vecAngularVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAngularAcceleration) = Eigen::Vector3d::Zero();

    pose.result = vr::TrackingResult_Running_OK;
    pose.poseIsValid = true;
    pose.willDriftInYaw = true;
    pose.shouldApplyHeadModel = true;
    pose.deviceIsConnected = true;

    self->pose_ = pose;
    self->driverHost_->TrackedDevicePoseUpdated(self->objectId_, self->pose_);
}

float OSVRTrackedHMD::GetIPD()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::GetIPD()");

    if (!displayConfig_.valid()) {
        OSVR_LOG(trace) << "OSVRTrackedHMD::GetIPD(): DisplayConfig is invalid.";
        return 0.0;
    }

    OSVR_Pose3 leftEye, rightEye;

    if (displayConfig_.getViewer(0).getEye(0).getPose(leftEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedHMD::GetHeadFromEyePose(): Unable to get left eye pose!\n";
    }

    if (displayConfig_.getViewer(0).getEye(1).getPose(rightEye) != true) {
        OSVR_LOG(err) << "OSVRTrackedHMD::GetHeadFromEyePose(): Unable to get right eye pose!\n";
    }

    float ipd = static_cast<float>((osvr::util::vecMap(leftEye.translation) - osvr::util::vecMap(rightEye.translation)).norm());
    return ipd;
}

void OSVRTrackedHMD::configure()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::configure()");

    configureDisplay();
    configureDistortionParameters();
    configureProperties();
}

void OSVRTrackedHMD::configureDisplay()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::configureDisplay()");

    // The name of the display we want to use
    const auto display_name = settings_->getSetting<std::string>("displayName", "OSVR");

    // Detect displays and find the one we're using as an HMD
    bool display_found = false;
    auto displays = osvr::display::getDisplays();
    for (const auto& display : displays) {
        if (std::string::npos == display.name.find(display_name)) {
            OSVR_LOG(trace) << "Rejecting display [" << display.name << "] since it doesn't match [" << display_name << "].";
            continue;
        }

        OSVR_LOG(trace) << "Found a match! Display [" << display.name << "] matches [" << display_name << "].";
        display_ = display;
        display_found = true;
        break;
    }

    if (!display_found) {
        // If the desired display wasn't detected, use the settings from the
        // display descriptor instead.
        //
        // This will most frequently occur when the HMD is in direct mode or if
        // the HMD is disconnected.
        const auto display_description = context_.getStringParameter("/display");
        displayConfiguration_ = OSVRDisplayConfiguration(display_description);
        const auto d = OSVRDisplayConfiguration(display_description);
        const auto active_resolution = d.activeResolution();

        const auto  vertical_refresh = renderManagerConfig_.getVerticalSync();

        const auto position_x = renderManagerConfig_.getWindowXPosition();
        const auto position_y = renderManagerConfig_.getWindowYPosition();

        // Rotation
        using osvr::display::Rotation;
        auto rotation = Rotation::Zero;
        const auto rot = renderManagerConfig_.getDisplayRotation();
        if (0 == rot) {
            rotation = Rotation::Zero;
        } else if (90 == rot) {
            rotation = Rotation::Ninety;
        } else if (180 == rot) {
            rotation = Rotation::OneEighty;
        } else if (270 == rot) {
            rotation = Rotation::TwoSeventy;
        } else {
            OSVR_LOG(err) << "Invalid rotation from RenderManager configuration: " << rot << ".";
        }

        display_.name = displayConfiguration_.getVendor() + " " + displayConfiguration_.getModel();
        display_.size.width = active_resolution.width;
        display_.size.height = active_resolution.height;
        display_.position.x = position_x;
        display_.position.y = position_y;
        display_.rotation = rotation;
        display_.verticalRefreshRate = vertical_refresh;
        display_.attachedToDesktop = false; // assuming direct mode
        display_.edidVendorId = 0xd24e; // SVR // TODO not provided by config files
        display_.edidProductId = 0x1019; // TODO not provided by config files
    }

    // The scan-out origin of the display
    const auto scan_out_origin_str = settings_->getSetting<std::string>("scanoutOrigin", "");
    if (scan_out_origin_str.empty()) {
        // Calculate the scan-out origin based on the display parameters
        scanoutOrigin_ = getScanOutOrigin();
        OSVR_LOG(warn) << "Warning: scan-out origin unspecified. Defaulting to " << scanoutOrigin_ << ".";
    } else {
        scanoutOrigin_ = parseScanOutOrigin(scan_out_origin_str);
    }

    // Print the display settings we're running with
    if (display_found) {
        OSVR_LOG(info) << "Detected display named [" << display_.name << "]:";
    } else {
        OSVR_LOG(info) << "Default display:";
    }
    OSVR_LOG(info) << "  Adapter: " << display_.adapter.description;
    OSVR_LOG(info) << "  Monitor name: " << display_.name;
    OSVR_LOG(info) << "  Resolution: " << display_.size.width << "x" << display_.size.height;
    OSVR_LOG(info) << "  Position: (" << display_.position.x << ", " << display_.position.y << ")";
    OSVR_LOG(info) << "  Rotation: " << display_.rotation;
    OSVR_LOG(info) << "  Scan-out origin: " << scanoutOrigin_;
    OSVR_LOG(info) << "  Refresh rate: " << display_.verticalRefreshRate;
    OSVR_LOG(info) << "  " << (display_.attachedToDesktop ? "Extended mode" : "Direct mode");
    OSVR_LOG(info) << "  EDID vendor ID: " << as_hex_0x(display_.edidVendorId) << " (" << osvr::display::decodeEdidVendorId(display_.edidVendorId) << ")";
    OSVR_LOG(info) << "  EDID product ID: " << as_hex_0x(display_.edidProductId);
}

void OSVRTrackedHMD::configureDistortionParameters()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::configureDistortionParameters()");

    // Parse the display descriptor
    const auto display_description = context_.getStringParameter("/display");
    displayConfiguration_ = OSVRDisplayConfiguration(display_description);

    // Initialize the distortion parameters
    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Number of eyes: " << displayConfiguration_.getEyes().size() << ".";
    for (size_t i = 0; i < displayConfiguration_.getEyes().size(); ++i) {
        auto distortion = osvr::renderkit::DistortionParameters { displayConfiguration_, i };
        distortion.m_desiredTriangles = 200 * 64;
        OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Adding distortion for eye " << i << ".";
        distortionParameters_.push_back(distortion);
    }
    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Number of distortion parameters: " << distortionParameters_.size() << ".";

    // Make the interpolators to be used by each eye.
    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Creating mesh interpolators for the left eye.";
    if (!makeUnstructuredMeshInterpolators(distortionParameters_[0], 0, leftEyeInterpolators_)) {
        OSVR_LOG(err) << "OSVRTrackedHMD::configureDistortionParameters(): Could not create mesh interpolators for left eye.";
    }
    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Number of left eye interpolators: " << leftEyeInterpolators_.size() << ".";

    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Creating mesh interpolators for the right eye.";
    if (!makeUnstructuredMeshInterpolators(distortionParameters_[1], 1, rightEyeInterpolators_)) {
        OSVR_LOG(err) << "OSVRTrackedHMD::configureDistortionParameters(): Could not create mesh interpolators for right eye.";
    }
    OSVR_LOG(debug) << "OSVRTrackedHMD::configureDistortionParameters(): Number of right eye interpolators: " << leftEyeInterpolators_.size() << ".";
}

void OSVRTrackedHMD::configureProperties()
{
    OSVR_FunctionGuard("OSVRTrackedHMD::configureProperties()");

    // General properties that apply to all device classes

    setProperty(vr::Prop_WillDriftInYaw_Bool, true);
    setProperty(vr::Prop_DeviceIsWireless_Bool, false);
    setProperty(vr::Prop_DeviceIsCharging_Bool, false);
    setProperty(vr::Prop_Firmware_UpdateAvailable_Bool, false);
    setProperty(vr::Prop_Firmware_ManualUpdate_Bool, false);
    setProperty(vr::Prop_BlockServerShutdown_Bool, false);
    //setProperty(vr::Prop_CanUnifyCoordinateSystemWithHmd_Bool, true);
    setProperty(vr::Prop_ContainsProximitySensor_Bool, false);
    setProperty(vr::Prop_DeviceProvidesBatteryStatus_Bool, false);
    setProperty(vr::Prop_DeviceCanPowerOff_Bool, true);
    setProperty(vr::Prop_HasCamera_Bool, false);

    setProperty(vr::Prop_DeviceBatteryPercentage_Float, 1.0f); // full battery

    //setProperty<uint64_t>(vr::Prop_HardwareRevision_Uint64, 0ul);
    //setProperty<uint64_t>(vr::Prop_FirmwareVersion_Uint64, 0ul);
    //setProperty<uint64_t>(vr::Prop_FPGAVersion_Uint64, 0ul);
    //setProperty<uint64_t>(vr::Prop_VRCVersion_Uint64, 0ul);
    //setProperty<uint64_t>(vr::Prop_RadioVersion_Uint64, 0ul);
    //setProperty<uint64_t>(vr::Prop_DongleVersion_Uint64, 0ul);

    //setProperty(vr::Prop_StatusDisplayTransform_Matrix34, /* TODO */

    //setProperty<std::string>(vr::Prop_TrackingSystemName_String, "");
    setProperty<std::string>(vr::Prop_ModelNumber_String, "OSVR HMD");
    //setProperty<std::string>(vr::Prop_RenderModelName_String, "");
    //setProperty<std::string>(vr::Prop_ManufacturerName_String, "");
    //setProperty<std::string>(vr::Prop_TrackingFirmwareVersion_String, "");
    //setProperty<std::string>(vr::Prop_HardwareRevision_String, "");
    //setProperty<std::string>(vr::Prop_AllWirelessDongleDescriptions_String, "");
    //setProperty<std::string>(vr::Prop_ConnectedWirelessDongle_String, "");
    //setProperty<std::string>(vr::Prop_Firmware_ManualUpdateURL_String, "");
    //setProperty<std::string>(vr::Prop_Firmware_ProgrammingTarget_String, "");
    //setProperty<std::string>(vr::Prop_DriverVersion_String, "");


    // Properties that apply to HMDs

    //setProperty(vr::Prop_ReportsTimeSinceVSync_Bool, false);
    setProperty(vr::Prop_IsOnDesktop_Bool, IsDisplayOnDesktop());

    setProperty<float>(vr::Prop_SecondsFromVsyncToPhotons_Float, 0.0f); // TODO
    setProperty<float>(vr::Prop_DisplayFrequency_Float, static_cast<float>(display_.verticalRefreshRate));
    setProperty<float>(vr::Prop_UserIpdMeters_Float, GetIPD());
    //setProperty<float>(vr::Prop_DisplayMCOffset_Float, 0.0f);
    //setProperty<float>(vr::Prop_DisplayMCScale_Float, 0.0f);
    //setProperty<float>(vr::Prop_DisplayGCBlackClamp_Float, 0.0f);
    //setProperty<float>(vr::Prop_DisplayGCOffset_Float, 0.0f);
    //setProperty<float>(vr::Prop_DisplayGCScale_Float, 0.0f);
    //setProperty<float>(vr::Prop_DisplayGCPrescale_Float, 0.0f);
    //setProperty<float>(vr::Prop_LensCenterLeftU_Float, 0.5f); // TODO
    //setProperty<float>(vr::Prop_LensCenterLeftV_Float, 0.5f); // TODO
    //setProperty<float>(vr::Prop_LensCenterRightU_Float, 0.5f); // TODO
    //setProperty<float>(vr::Prop_LensCenterRightV_Float, 0.5f); // TODO
    setProperty<float>(vr::Prop_UserHeadToEyeDepthMeters_Float, 0.0f); // TODO

    //setProperty<int32_t>(vr::Prop_DisplayMCType_Int32, 0);
    setProperty<int32_t>(vr::Prop_EdidVendorID_Int32, static_cast<int32_t>(display_.edidVendorId));
    setProperty<int32_t>(vr::Prop_EdidProductID_Int32, static_cast<int32_t>(display_.edidProductId));
    //setProperty<int32_t>(vr::Prop_DisplayGCType_Int32, 0);
    //setProperty<int32_t>(vr::Prop_CameraCompatibilityMode_Int32, 0);

    setProperty<uint64_t>(vr::Prop_CurrentUniverseId_Uint64, 1);
    setProperty<uint64_t>(vr::Prop_PreviousUniverseId_Uint64, 1);
    setProperty<uint64_t>(vr::Prop_DisplayFirmwareVersion_Uint64, 192); // FIXME read from OSVR server
    //setProperty<uint64_t>(vr::Prop_CameraFirmwareVersion_Uint64, 0);
    //setProperty<uint64_t>(vr::Prop_DisplayFPGAVersion_Uint64, 0);
    //setProperty<uint64_t>(vr::Prop_DisplayBootloaderVersion_Uint64, 0);
    //setProperty<uint64_t>(vr::Prop_DisplayHardwareVersion_Uint64, 0);
    //setProperty<uint64_t>(vr::Prop_AudioFirmwareVersion_Uint64, 0);

    //setProperty(vr::Prop_CameraToHeadTransform_Matrix34, /* TODO */);

    //setProperty<std::string>(vr::Prop_DisplayMCImageLeft_String, "");
    //setProperty<std::string>(vr::Prop_DisplayMCImageRight_String, "");
    //setProperty<std::string>(vr::Prop_DisplayGCImage_String, "");
    //setProperty<std::string>(vr::Prop_CameraFirmwareDescription_String, "");
}

osvr::display::ScanOutOrigin OSVRTrackedHMD::parseScanOutOrigin(std::string str) const
{
    // Make the string lowercase
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    if ("lower-left" == str || "ll" == str || "lowerleft" == str || "lower left" == str
        || "bottom-left" == str || "bl" == str || "bottomleft" == str || "bottom left" == str) {
        return osvr::display::ScanOutOrigin::LowerLeft;
    } else if ("lower-right" == str || "lr" == str || "lowerright" == str || "lower right" == str
        || "bottom-right" == str || "br" == str || "bottomright" == str || "bottom right" == str) {
        return osvr::display::ScanOutOrigin::LowerRight;
    } else if ("upper-left" == str || "ul" == str || "upperleft" == str || "upper left" == str
        || "top-left" == str || "tl" == str || "topleft" == str || "top left" == str) {
        return osvr::display::ScanOutOrigin::UpperLeft;
    } else if ("upper-right" == str || "ur" == str || "upperright" == str || "upper right" == str
        || "top-right" == str || "tr" == str || "topright" == str || "top right" == str) {
        return osvr::display::ScanOutOrigin::UpperRight;
    } else {
        OSVR_LOG(err) << "The string [" + str + "] could not be parsed as a scan-out origin. Use one of: lower-left, upper-left, lower-right, upper-right.";
        return osvr::display::ScanOutOrigin::UpperLeft;
    }
}

osvr::display::ScanOutOrigin OSVRTrackedHMD::getScanOutOrigin() const
{
    // TODO Use RenderManager and OSVR config files to determine scan-out
    // origin. But since some of those are currently broken, we'll base the
    // defaults on our knowledge of the HDK 1.x and 2.0.
    using SO = osvr::display::ScanOutOrigin;
    if ("OSVR HDK2" == display_.name) {
        return SO::LowerRight;
    } else if ("OSVR HDK" == display_.name) {
        const auto is_landscape = (display_.size.height < display_.size.width);
        return (is_landscape ? SO::UpperLeft : SO::UpperRight);
    } else {
        // Unknown HMD. Punt!
        return SO::UpperLeft;
    }
}

std::pair<float, float> OSVRTrackedHMD::rotateTextureCoordinates(float u, float v, osvr::display::Rotation rotation) const
{
    // Rotates normalized coordinates counter-clockwise
    using R = osvr::display::Rotation;
    if (R::Zero == rotation) {
        return { u, v };
    } else if (R::Ninety == rotation) {
        return { 1.0f - v, u };
    } else if (R::OneEighty == rotation) {
        return { 1.0f - u, 1.0f - v };
    } else if (R::TwoSeventy == rotation) {
        return { v, 1.0f - u };
    } else {
        OSVR_LOG(err) << "Unknown rotation [" << rotation << "] Assuming 0 degrees.";
        return { u, v };
    }
}

