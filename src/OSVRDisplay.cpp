/** @file
    @brief Implementation

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2016 Sensics, Inc.
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
#include "OSVRDisplay.h"
#include "Logging.h"

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/display/Display.h>
#include <osvr/display/DisplayIO.h>
#include <osvr/RenderKit/osvr_display_configuration.h>
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <algorithm>
#include <cstdint>
#include <ostream>
#include <string>

osvr::display::ScanOutOrigin getScanOutOrigin(const std::string& display_name, std::uint32_t width, std::uint32_t height)
{
    // TODO Use RenderManager and OSVR config files to determine scan-out
    // origin. But since some of those are currently broken, we'll base the
    // defaults on our knowledge of the HDK 1.x and 2.0.
    using SO = osvr::display::ScanOutOrigin;
    const auto is_hdk = (std::string::npos != display_name.find("OSVR HDK"));
    if (!is_hdk) {
        // Unknown HMD. Punt!
        return SO::UpperLeft;
    }

    const auto is_detected_hdk = (display_name == "OSVR HDK");
    const auto is_hdk_1x = (std::string::npos != display_name.find("OSVR HDK 1")
                            || (is_detected_hdk && 1920 == std::max(width, height)));
    const auto is_hdk_20 = (display_name == "OSVR HDK 2.0"
                            || (is_detected_hdk && 2160 == std::max(width, height)));

    if (is_hdk_1x) {
        const auto is_landscape = (height < width);
        return (is_landscape ? SO::UpperLeft : SO::UpperRight);
    } else if (is_hdk_20) {
        return SO::LowerRight;
    }

    // Some unknown HDK!
    return SO::LowerRight;
}

bool OSVRRectangle::operator==(const OSVRRectangle& other) const
{
    return (x == other.x && y == other.y && width == other.width && height == other.height);
}

std::string to_string(const OSVRRectangle& r)
{
    using std::to_string;
    std::string out = "(" + to_string(r.width) + ", " + to_string(r.height)
        + ") at (" + to_string(r.x) + ", " + to_string(r.y) + ")";
    return out;
}

std::ostream& operator<<(std::ostream& os, const OSVRRectangle& r)
{
    os << "(" << r.width << ", " << r.height << ") at (" << r.x << ", " << r.y << ")";
    return os;
}


OSVRRectangle getWindowBounds(const osvr::display::Display& display, osvr::display::ScanOutOrigin scanout_origin)
{
    OSVRRectangle bounds;
#if 0
    /* Would be used only in Linux at the moment. Linux needs OSVR-Display
     * support.
     */

    int nDisplays = displayConfig_.getNumDisplayInputs();
    if (nDisplays != 1) {
        OSVR_LOG(err) << "OSVRTrackedHMD::OSVRTrackedHMD(): Unexpected display number of displays!\n";
    }
    osvr::clientkit::DisplayDimensions displayDims = displayConfig_.getDisplayDimensions(0);
    bounds.x = renderManagerConfig_.getWindowXPosition(); // todo: assumes desktop display of 1920. get this from display config when it's exposed.
    bounds.y = renderManagerConfig_.getWindowYPosition();
    bounds.width = static_cast<uint32_t>(displayDims.width);
    bounds.height = static_cast<uint32_t>(displayDims.height);

    OSVR_LOG(trace) << "GetWindowBounds(): Config file settings: x = " << *x << ", y = " << *y << ", width = " << *width << ", height = " << *height << ".";
#endif

#if defined(OSVR_WINDOWS) || defined(OSVR_MACOSX)
    bounds.x = display.position.x;
    bounds.y = display.position.y;

    // Windows always reports the widest dimension as width regardless of the
    // orientation of the display. We need to flip these dimensions if the
    // display is in portrait orientation.
    //
    // OS X reports the resolution with respect to the orientation (e.g., in
    // portrait mode, a display's resolution might be 1080x1920).
    //
    // TODO Check to see how Linux handles this.
    const auto orientation = scanout_origin + display.rotation;
    const bool is_portrait = (osvr::display::DesktopOrientation::Portrait == orientation || osvr::display::DesktopOrientation::PortraitFlipped == orientation);
    if (is_portrait) {
        bounds.height = std::max(display.size.width, display.size.height);
        bounds.width = std::min(display.size.width, display.size.height);
    } else {
        bounds.height = std::min(display.size.width, display.size.height);
        bounds.width = std::max(display.size.width, display.size.height);
    }
    OSVR_LOG(trace) << "GetWindowBounds(): Scan-out origin: " << scanout_origin << ", rotation: " << display.rotation << ", orientation: " << orientation;
#endif // OSVR_WINDOWS or OSVR_MACOSX

    OSVR_LOG(trace) << "GetWindowBounds(): Calculated settings: x = " << bounds.x << ", y = " << bounds.y << ", width = " << bounds.width << ", height = " << bounds.height << ".";
    return bounds;
}


/**
 * Returns the eye viewport.
 */
OSVRRectangle getEyeOutputViewport(const vr::EVREye eye, const osvr::display::Display& display, const osvr::display::ScanOutOrigin scanout_origin, const OSVRDisplayConfiguration::DisplayMode display_mode)
{
    OSVRRectangle viewport;

    const auto bounds = getWindowBounds(display, scanout_origin);

    // We have to duplicate this logic from OSVR-Core's DisplayConfig.cpp file
    // because that version doesn't handle the *detected* rotation, only the
    // rotation set in the config file.
    const auto orientation = scanout_origin + display.rotation;

    // TODO Simplify this code after verifying it works properly
    if (OSVRDisplayConfiguration::DisplayMode::FULL_SCREEN == display_mode) {
        OSVR_LOG(trace) << "Display mode: full-screen.";
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = bounds.width;
        viewport.height = bounds.height;
    } else if (OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE == display_mode) {
        OSVR_LOG(trace) << "Display mode: horizontal side-by-side.";
        using Orientation = osvr::display::DesktopOrientation;
        if (Orientation::Portrait == orientation) {
            OSVR_LOG(trace) << "Display orientation: portrait.";
            viewport.x = 0;
            viewport.y = (vr::Eye_Left == eye) ? 0 : bounds.height / 2;
            viewport.width = bounds.width;
            viewport.height = bounds.height / 2;
        } else if (Orientation::PortraitFlipped == orientation) {
            OSVR_LOG(trace) << "Display orientation: portrait flipped.";
            viewport.x = 0;
            viewport.y = (vr::Eye_Left == eye) ? bounds.height / 2 : 0;
            viewport.width = bounds.width;
            viewport.height = bounds.height / 2;
        } else if (Orientation::Landscape == orientation) {
            OSVR_LOG(trace) << "Display orientation: landscape.";
            viewport.x = (vr::Eye_Left == eye) ? 0 : bounds.width / 2;
            viewport.y = 0;
            viewport.width = bounds.width / 2;
            viewport.height = bounds.height;
        } else if (Orientation::LandscapeFlipped == orientation) {
            OSVR_LOG(trace) << "Display orientation: landscape flipped.";
            viewport.x = (vr::Eye_Left == eye) ? bounds.width / 2 : 0;
            viewport.y = 0;
            viewport.width = bounds.width / 2;
            viewport.height = bounds.height;
        } else {
            OSVR_LOG(err) << "Unknown display orientation [" << static_cast<int>(orientation) << "]!";
        }
    } else if (OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE == display_mode) {
        OSVR_LOG(trace) << "Display mode: vertical side-by-side.";
        OSVR_LOG(err) << "This display mode hasn't been implemented yet!";
        // TODO
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = bounds.width;
        viewport.height = bounds.height / 2;
    } else {
        OSVR_LOG(err) << "Unknown display mode [" << static_cast<int>(display_mode) << "]!";
    }

    const auto eye_str = (vr::Eye_Left == eye) ? "left" : "right";
    OSVR_LOG(trace) << "GetEyeOutputViewport(" << eye_str << " eye): Calculated settings: x = " << viewport.x << ", y = " << viewport.y << ", width = " << viewport.width << ", height = " << viewport.height << ".";

    return viewport;
}

