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

// Internal Includes
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "OSVRDisplay.h"

// Library/third-party includes
// - none

// Standard includes
// - none

TEST_CASE("scan-out origin", "[scanoutOrigin]") {
    using SO = osvr::display::ScanOutOrigin;

    // All OSVR HDK 1.x and 2.0 have an EDID name of "OSVR HDK"
    CHECK(getScanOutOrigin("OSVR HDK", 1920, 1080) == SO::UpperLeft);
    CHECK(getScanOutOrigin("OSVR HDK", 1080, 1920) == SO::UpperRight);
    CHECK(getScanOutOrigin("OSVR HDK", 2160, 1200) == SO::LowerRight);

    // Display descriptors
    CHECK(getScanOutOrigin("OSVR HDK 1.2", 1920, 1080) == SO::UpperLeft); // 1.1, 1.2
    CHECK(getScanOutOrigin("OSVR HDK 1.2", 1080, 1920) == SO::UpperRight); // 1.1, 1.2
    CHECK(getScanOutOrigin("OSVR HDK 1.3", 1920, 1080) == SO::UpperLeft); // 1.3, 1.4
    CHECK(getScanOutOrigin("OSVR HDK 1.3", 1080, 1920) == SO::UpperRight); // 1.3, 1.4
    CHECK(getScanOutOrigin("OSVR HDK 2.0", 2160, 1200) == SO::LowerRight); // 2.0

    // Default
    CHECK(getScanOutOrigin("Oculus Rift DK2", 1920, 1080) == SO::UpperLeft);
}

