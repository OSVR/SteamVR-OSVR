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
#include <osvr/display/Display.h>

// Standard includes
#include <cstdint>

/**
 * Gets the default scan-out origin based the detected HMD and/or OSVR
 * configuration.
 */
inline osvr::display::ScanOutOrigin getScanOutOrigin(const std::string& display_name, std::uint32_t width, std::uint32_t height)
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

#endif // INCLUDED_OSVRDisplay_h_GUID_FAE2B8A6_1225_4344_9FA0_919856E66E8E

