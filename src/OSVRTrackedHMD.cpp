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

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"
#include "OSVRDisplay.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Display/DisplayEnumerator.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Util/PlatformConfig.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>
#include <osvr/RenderKit/DistortionCorrectTextureCoordinate.h>
#include <osvr/ClientKit/InterfaceStateC.h>
#include <osvr/Util/EigenQuatExponentialMap.h>
#include <osvr/Util/TimeValue.h>

// Standard includes
#include <algorithm>        // for std::find
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>

OSVRTrackedHMD::OSVRTrackedHMD(osvr::clientkit::ClientContext& context) : OSVRTrackedDevice(context, vr::TrackedDeviceClass_HMD, "OSVRTrackedHMD")
{
    OSVR_LOG(trace) << "OSVRTrackedHMD::OSVRTrackedHMD() called.";
}

OSVRTrackedHMD::~OSVRTrackedHMD()
{
    // do nothing
}

vr::EVRInitError OSVRTrackedHMD::Activate(uint32_t object_id)
{
    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate() called with ID " << object_id << ".";
    OSVRTrackedDevice::Activate(object_id);

    // TODO use C++11 <chrono>
    const std::time_t waitTime = settings_->getSetting<int32_t>("serverTimeout", 5);

    // Register tracker callback
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }

    // Ensure context is fully started up
    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate(): Waiting for the context to fully start up...\n";
    std::time_t startTime = std::time(nullptr);
    while (!context_.checkStatus()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Context startup timed out after " << waitTime << " ms!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    displayConfig_ = osvr::clientkit::DisplayConfig(context_);

    // Ensure display is fully started up
    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate(): Waiting for the display to fully start up, including receiving initial pose update...\n";
    startTime = std::time(nullptr);
    while (!displayConfig_.checkStartup()) {
        context_.update();
        if (std::time(nullptr) > startTime + waitTime) {
            OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Display startup timed out after " << waitTime << " ms!\n";
            return vr::VRInitError_Driver_Failed;
        }
    }

    // Verify valid display config
    if ((displayConfig_.getNumViewers() != 1) && (displayConfig_.getViewer(0).getNumEyes() != 2) && (displayConfig_.getViewer(0).getEye(0).getNumSurfaces() == 1) && (displayConfig_.getViewer(0).getEye(1).getNumSurfaces() != 1)) {
        OSVR_LOG(err) << "OSVRTrackedHMD::Activate(): Unexpected display parameters!\n";

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
    configureDistortionParameters();
    setProperties();

    // Register tracker callback
    trackerInterface_ = context_.getInterface("/me/head");
    trackerInterface_.registerCallback(&OSVRTrackedHMD::HmdTrackerCallback, this);

    OSVR_LOG(trace) << "OSVRTrackedHMD::Activate(): Activation for object ID " << object_id << " complete.\n";
    return vr::VRInitError_None;
}

void OSVRTrackedHMD::Deactivate()
{
    OSVR_LOG(trace) << "OSVRTrackedHMD::Deactivate() called.";

    objectId_ = vr::k_unTrackedDeviceIndexInvalid;

    /// Have to force freeing here
    if (trackerInterface_.notEmpty()) {
        trackerInterface_.free();
    }
}

void OSVRTrackedHMD::EnterStandby()
{
    // FIXME Implement
}

void* OSVRTrackedHMD::GetComponent(const char* component_name_and_version)
{
    if (!strcasecmp(component_name_and_version, vr::IVRDisplayComponent_Version)) {
        return static_cast<vr::IVRDisplayComponent*>(this);
    }

    // Override this to add a component to a driver
    return nullptr;
}

void OSVRTrackedHMD::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // TODO

    // Log the requests just to see what info clients are looking for
    OSVR_LOG(debug) << "Received debug request [" << request << "] with response buffer size of " << response_buffer_size << "].";

    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
    // return empty string for now
    if (response_buffer_size > 0) {
        response_buffer[0] = '\0';
    }
}

void OSVRTrackedHMD::GetWindowBounds(int32_t* x, int32_t* y, uint32_t* width, uint32_t* height)
{
    const auto bounds = getWindowBounds(display_, scanoutOrigin_);
    *x = bounds.x;
    *y = bounds.y;
    *width = bounds.width;
    *height = bounds.height;
}

bool OSVRTrackedHMD::IsDisplayOnDesktop()
{
    // If the current display still appears in the active displays list,
    // then it's attached to the desktop.
    const auto displays = osvr::display::getDisplays();
    const auto display_on_desktop = (end(displays) != std::find(begin(displays), end(displays), display_));
    OSVR_LOG(trace) << "OSVRTrackedHMD::IsDisplayOnDesktop(): " << (display_on_desktop ? "yes" : "no");
    return display_on_desktop;
}

bool OSVRTrackedHMD::IsDisplayRealDisplay()
{
    // TODO get this info from display description?
    return true;
}

void OSVRTrackedHMD::GetRecommendedRenderTargetSize(uint32_t* width, uint32_t* height)
{
    //const double overfill_factor = renderManagerConfig_.getRenderOverfillFactor();
    const double overfill_factor = 1.0;
    const auto bounds = getWindowBounds(display_, scanoutOrigin_);

    *width = static_cast<uint32_t>(bounds.width * overfill_factor)/2;
    *height = static_cast<uint32_t>(bounds.height * overfill_factor);
    OSVR_LOG(trace) << "GetRecommendedRenderTargetSize(): width = " << *width << ", height = " << *height << ".";
}

void OSVRTrackedHMD::GetEyeOutputViewport(vr::EVREye eye, uint32_t* x, uint32_t* y, uint32_t* width, uint32_t* height)
{
    const auto display_mode = displayConfiguration_.getDisplayMode();
    const auto viewport = getEyeOutputViewport(eye, display_, scanoutOrigin_, display_mode);

    *x = static_cast<uint32_t>(viewport.x);
    *y = static_cast<uint32_t>(viewport.y);
    *width = viewport.width;
    *height = viewport.height;
}

void OSVRTrackedHMD::GetProjectionRaw(vr::EVREye eye, float* left, float* right, float* top, float* bottom)
{
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
    // Rotate the texture coordinates to match the display orientation
    const auto orientation = scanoutOrigin_ + display_.rotation;
    const auto desired_orientation = osvr::display::DesktopOrientation::Landscape;
    const auto rotation = desired_orientation - orientation;

    std::tie(u, v) = rotate(u, v, rotation);

    // Note that RenderManager expects the (0, 0) to be the lower-left corner
    // and (1, 1) to be the upper-right corner while SteamVR assumes (0, 0) is
    // upper-left and (1, 1) is lower-right.  To accommodate this, we need to
    // flip the y-coordinate before passing it to RenderManager and flip it
    // again before returning the value to SteamVR.
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

    return coords;
}

vr::DriverPose_t OSVRTrackedHMD::GetPose()
{
    return pose_;
}


// ------------------------------------
// Private Methods
// ------------------------------------


void OSVRTrackedHMD::HmdTrackerCallback(void* userdata, const OSVR_TimeValue* timeval, const OSVR_PoseReport* report)
{
    if (!userdata)
        return;

    auto* self = static_cast<OSVRTrackedHMD*>(userdata);

    vr::DriverPose_t pose;

    map(pose.qWorldFromDriverRotation) = Eigen::Quaterniond::Identity();
    Eigen::Vector3d::Map(pose.vecWorldFromDriverTranslation) = Eigen::Vector3d::Zero();

    map(pose.qDriverFromHeadRotation) = Eigen::Quaterniond::Identity();
    Eigen::Vector3d::Map(pose.vecDriverFromHeadTranslation) = Eigen::Vector3d::Zero();

    // Position
    Eigen::Vector3d::Map(pose.vecPosition) = osvr::util::vecMap(report->pose.translation);

    // Velocity (m/s) and angular velocity (rad/s)
    Eigen::Vector3d::Map(pose.vecVelocity) = Eigen::Vector3d::Zero();
    Eigen::Vector3d::Map(pose.vecAngularVelocity) = Eigen::Vector3d::Zero();

    if (!self->ignoreVelocityReports_) {
        OSVR_TimeValue tv;
        OSVR_VelocityState velocity_state;
        const auto has_velocity_state = osvrGetVelocityState(self->trackerInterface_.get(), &tv, &velocity_state);
        if (OSVR_RETURN_SUCCESS == has_velocity_state) {
            if (velocity_state.linearVelocityValid) {
                std::copy(std::begin(velocity_state.linearVelocity.data), std::end(velocity_state.linearVelocity.data), std::begin(pose.vecVelocity));
            }

            if (velocity_state.angularVelocityValid) {
                // Change the reference frame
                const auto pose_rotation = osvr::util::fromQuat(report->pose.rotation);
                const auto inc_rotation = pose_rotation.inverse() * osvr::util::fromQuat(velocity_state.angularVelocity.incrementalRotation) * pose_rotation;

                // Convert incremental rotation to angular velocity
                const auto dt = velocity_state.angularVelocity.dt;
                const auto angular_velocity = osvr::util::quat_ln(inc_rotation) * 2.0 / dt;

                Eigen::Vector3d::Map(pose.vecAngularVelocity) = angular_velocity;
            }
        }
    }

    // Acceleration of the pose in meters/second
    Eigen::Vector3d::Map(pose.vecAcceleration) = Eigen::Vector3d::Zero();

    // Orientation
    map(pose.qRotation) = osvr::util::fromQuat(report->pose.rotation);

    // Angular acceleration is not currently provided
    Eigen::Vector3d::Map(pose.vecAngularAcceleration) = Eigen::Vector3d::Zero();

    pose.result = vr::TrackingResult_Running_OK;
    pose.poseIsValid = true;
    pose.willDriftInYaw = true;
    pose.shouldApplyHeadModel = true;
    pose.deviceIsConnected = true;

    // Time offset of this pose, in seconds from the actual time of the pose,
    // relative to the time of the PoseUpdated() call made by the driver.
    const auto now = osvr::util::time::getNow();
    const auto elapsed = osvr::util::time::duration(now, *timeval);
    pose.poseTimeOffset = elapsed;

    //OSVR_LOG(trace) << "OSVRTrackedHMD::HmdTrackerCallback(): Got a new head pose: " << pose.vecPosition << " at angle " << pose.qRotation << ".";
    self->pose_ = pose;
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(self->objectId_, self->pose_, sizeof(vr::DriverPose_t));
}

float OSVRTrackedHMD::GetIPD()
{
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

const char* OSVRTrackedHMD::getId()
{
    if (display_.name.empty()) {
        display_.name = "OSVR HMD";
    }

    return display_.name.c_str();
}

vr::ETrackedDeviceClass OSVRTrackedHMD::getDeviceClass() const
{
    return deviceClass_;
}

void OSVRTrackedHMD::configure()
{
    // Get settings from config file
    ignoreVelocityReports_ = settings_->getSetting<bool>("ignoreVelocityReports", false);
    OSVR_LOG(info) << (ignoreVelocityReports_ ? "Ignoring velocity reports." : "Utilizing velocity reports.");

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

        // The scan-out origin of the display
        const auto scan_out_origin_str = settings_->getSetting<std::string>("scanoutOrigin", "");
        if (scan_out_origin_str.empty()) {
            // Calculate the scan-out origin based on the display parameters
            //scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + display_.rotation);
            const auto rot = renderManagerConfig_.getDisplayRotation();
            scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + osvr::display::to_Rotation(static_cast<int>(rot)));
            OSVR_LOG(warn) << "Warning: scan-out origin unspecified. Defaulting to " << scanoutOrigin_ << ".";
        } else {
            scanoutOrigin_ = parseScanOutOrigin(scan_out_origin_str);
        }

        break;
    }

    if (!display_found) {
        // If the desired display wasn't detected, use the settings from the
        // display descriptor instead.
        //
        // This will most frequently occur when the HMD is in direct mode or if
        // the HMD is disconnected.
        displayDescription_ = context_.getStringParameter("/display");
        displayConfiguration_ = OSVRDisplayConfiguration(displayDescription_);
        const auto d = OSVRDisplayConfiguration(displayDescription_);
        auto active_resolution = d.activeResolution();

        const auto position_x = renderManagerConfig_.getWindowXPosition();
        const auto position_y = renderManagerConfig_.getWindowYPosition();

        // Rotation
        //   Adjust the scan-out orientation based rotation value.  Also, when
        //   the rotation is 90 or 270, we need to swap the resolution values
        //   and zero out the rotation.. Otherwise, just ignore the rotation (as
        //   we compensate for it using the scan-out origin).
        using osvr::display::Rotation;
        auto rotation = Rotation::Zero;
        const auto rot = renderManagerConfig_.getDisplayRotation();
        if (90 == rot || 270 == rot) {
            std::swap(active_resolution.width, active_resolution.height);
        }

        display_.adapter.description = "Unknown";
        display_.name = displayConfiguration_.getVendor() + " " + displayConfiguration_.getModel() + " " + displayConfiguration_.getVersion();
        display_.size.width = static_cast<uint32_t>(active_resolution.width);
        display_.size.height = static_cast<uint32_t>(active_resolution.height);
        display_.position.x = position_x;
        display_.position.y = position_y;
        display_.rotation = rotation;
        display_.verticalRefreshRate = getVerticalRefreshRate();
        display_.attachedToDesktop = false; // assuming direct mode
        display_.edidVendorId = settings_->getSetting<uint32_t>("edidVendorId", 0xd24e); // SVR
        display_.edidProductId = settings_->getSetting<uint32_t>("edidProductId", 0x1019);

        // The scan-out origin of the display
        const auto scan_out_origin_str = settings_->getSetting<std::string>("scanoutOrigin", "");
        if (scan_out_origin_str.empty()) {
            // Calculate the scan-out origin based on the display parameters
            scanoutOrigin_ = osvr::display::to_ScanOutOrigin(osvr::display::ScanOutOrigin::UpperLeft + osvr::display::to_Rotation(static_cast<int>(rot)));
            OSVR_LOG(warn) << "Warning: scan-out origin unspecified. Defaulting to " << scanoutOrigin_ << ".";
        } else {
            scanoutOrigin_ = parseScanOutOrigin(scan_out_origin_str);
        }
    }

    // Print the display settings we're running with
    if (display_found) {
        OSVR_LOG(info) << "Detected display named [" << display_.name << "]:";
    } else {
        OSVR_LOG(info) << "Display parameters from configuration files:";
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
    // Parse the display descriptor
    displayDescription_ = context_.getStringParameter("/display");
    displayConfiguration_ = OSVRDisplayConfiguration(displayDescription_);

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

double OSVRTrackedHMD::getVerticalRefreshRate() const
{
    // The vertical refresh rate is unavailable in direct mode and isn't
    // currently provided via OSVR config file or API.
    //
    // We'll read an override value from steamvr.vrsettings if it exists.
    // Otherwise, we'll fall back on OSVR HDK defaults or use a heuristic for
    // other HMDs.
    const auto refresh_rate = settings_->getSetting<float>("verticalRefreshRate", 0.0);
    if (refresh_rate > 0.0) {
        return refresh_rate;
    }

    const auto is_hdk_1x = (std::string::npos != display_.name.find("OSVR HDK 1"));
    const auto is_hdk_20 = (std::string::npos != display_.name.find("OSVR HDK 2.0"));
    const auto is_high_res = (display_.size.width > 1920 || display_.size.height > 1920);

    if (is_hdk_1x) {
        return 60.0;
    } else if (is_hdk_20) {
        return 90.0;
    } else if (is_high_res) {
        // Assume all high-resolution displays operate at 90 Hz
        return 90.0;
    } else {
        // Assume it's a lame HMD and can only handle 60 Hz
        return 60.0;
    }
}

std::pair<float, float> OSVRTrackedHMD::rotate(float u, float v, osvr::display::Rotation rotation) const
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

void OSVRTrackedHMD::setProperties()
{
    propertyContainer_ = vr::VRProperties()->TrackedDeviceToPropertyContainer(objectId_);

    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_WillDriftInYaw_Bool, true);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceIsWireless_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceIsCharging_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_Firmware_UpdateAvailable_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_Firmware_ManualUpdate_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_BlockServerShutdown_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_ContainsProximitySensor_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceProvidesBatteryStatus_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_DeviceCanPowerOff_Bool, true);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_HasCamera_Bool, false);
    vr::VRProperties()->SetBoolProperty(propertyContainer_, vr::Prop_IsOnDesktop_Bool, IsDisplayOnDesktop());
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_DeviceBatteryPercentage_Float, 1.0f);
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_DisplayFrequency_Float, static_cast<float>(display_.verticalRefreshRate));
    vr::VRProperties()->SetFloatProperty(propertyContainer_, vr::Prop_UserIpdMeters_Float, GetIPD());
    vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_EdidVendorID_Int32, static_cast<int32_t>(display_.edidVendorId));
    vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_EdidProductID_Int32, static_cast<int32_t>(display_.edidProductId));

    // return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
    vr::VRProperties()->SetUint64Property(propertyContainer_, vr::Prop_CurrentUniverseId_Uint64, 1);
    vr::VRProperties()->SetUint64Property(propertyContainer_, vr::Prop_PreviousUniverseId_Uint64, 1);

    /// @todo This really should be read from the server
    vr::VRProperties()->SetUint64Property(propertyContainer_, vr::Prop_DisplayFirmwareVersion_Uint64, 192);

    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ModelNumber_String, getModelNumber().c_str());
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_SerialNumber_String, settings_->getSetting<std::string>("serialNumber", getId()).c_str());
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_ManufacturerName_String, getManufacturerName().c_str());
}

std::string OSVRTrackedHMD::getModelNumber() const
{
    const auto model_number_default = displayConfiguration_.getModel() + " " + displayConfiguration_.getVersion();
    const auto model_number_override = settings_->getSetting<std::string>("modelNumber", model_number_default);
    if (model_number_override.empty()) {
        return model_number_default;
    }

    return model_number_override;
}

std::string OSVRTrackedHMD::getManufacturerName() const
{
    const auto manufacturer_default = displayConfiguration_.getVendor();
    const auto manufacturer_override = settings_->getSetting<std::string>("manufacturer", manufacturer_default);
    if (manufacturer_override.empty()) {
        return manufacturer_default;
    }

    return manufacturer_override;
}


