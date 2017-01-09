/** @file
    @brief Header

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

#ifndef INCLUDED_OSVRDisplay_h_GUID_FAE2B8A6_1225_4344_9FA0_919856E66E8E
#define INCLUDED_OSVRDisplay_h_GUID_FAE2B8A6_1225_4344_9FA0_919856E66E8E

// Internal Includes
// - none

// Library/third-party includes
#include <openvr_driver.h>

#include <osvr/Display/Display.h>
#include <osvr/RenderKit/osvr_display_configuration.h>
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <cstdint>
#include <ostream>
#include <string>

/**
 * Gets the default scan-out origin based the detected HMD and/or OSVR
 * configuration.
 */
osvr::display::ScanOutOrigin getScanOutOrigin(const std::string& display_name, std::uint32_t width, std::uint32_t height);

/**
 * OSVRRectangle is used to define window bounds and eye viewport parameters.
 */
class OSVRRectangle {
public:
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;

    bool operator==(const OSVRRectangle& other) const;
};

std::string to_string(const OSVRRectangle& r);

std::ostream& operator<<(std::ostream& os, const OSVRRectangle& r);

OSVRRectangle getWindowBounds(const osvr::display::Display& display, osvr::display::ScanOutOrigin scanout_origin);

/**
 * Returns the eye viewport.
 */
OSVRRectangle getEyeOutputViewport(const vr::EVREye eye, const osvr::display::Display& display, const osvr::display::ScanOutOrigin scanout_origin, const OSVRDisplayConfiguration::DisplayMode display_mode);

#endif // INCLUDED_OSVRDisplay_h_GUID_FAE2B8A6_1225_4344_9FA0_919856E66E8E

