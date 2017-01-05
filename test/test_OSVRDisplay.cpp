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

TEST_CASE("scan-out origin", "[scanoutOrigin]")
{
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

class DisplayTestFixture {
public:
    osvr::display::Display getDisplay() const
    {
        return display_;
    }

    osvr::display::ScanOutOrigin getScanOutOrigin() const
    {
        return scanoutOrigin_;
    }

    OSVRDisplayConfiguration::DisplayMode getDisplayMode() const
    {
        return displayMode_;
    }

protected:
    osvr::display::Display display_ = { };
    osvr::display::ScanOutOrigin scanoutOrigin_ = osvr::display::ScanOutOrigin::UpperLeft;
    OSVRDisplayConfiguration::DisplayMode displayMode_;
};

class HDK13TestFixture : public DisplayTestFixture {
public:
    HDK13TestFixture()
    {
        display_.adapter.description = "Unknown";
        display_.name = "OSVR HDK 1.3";
        display_.size.width = 1920;
        display_.size.height = 1080;
        display_.position.x = 1920;
        display_.position.y = 0;
        display_.rotation = osvr::display::Rotation::Zero;
        display_.verticalRefreshRate = 60.0;
        display_.attachedToDesktop = false; // assuming direct mode
        display_.edidVendorId = 0xd24e; // SVR
        display_.edidProductId = 0x1019;

        scanoutOrigin_ = osvr::display::ScanOutOrigin::UpperRight;

        displayMode_ = OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE;
    }
};

class HDK20TestFixture : public DisplayTestFixture {
public:
    HDK20TestFixture()
    {
        display_.adapter.description = "Unknown";
        display_.name = "OSVR HDK 2.0";
        display_.size.width = 2160;
        display_.size.height = 1200;
        display_.position.x = 1920;
        display_.position.y = 0;
        display_.rotation = osvr::display::Rotation::Zero;
        display_.verticalRefreshRate = 90.0;
        display_.attachedToDesktop = false; // assuming direct mode
        display_.edidVendorId = 0xd24e; // SVR
        display_.edidProductId = 0x1019;

        scanoutOrigin_ = osvr::display::ScanOutOrigin::LowerRight;

        displayMode_ = OSVRDisplayConfiguration::DisplayMode::HORIZONTAL_SIDE_BY_SIDE;
    }
};


TEST_CASE_METHOD(HDK13TestFixture, "getWindowBounds HDK13", "[getWindowBounds]")
{
    const auto bounds_hdk13 = Rectangle { 1920, 0, 1080, 1920 };
    CHECK(getWindowBounds(getDisplay(), getScanOutOrigin()) == bounds_hdk13);
}

TEST_CASE_METHOD(HDK20TestFixture, "getWindowBounds_HDK20", "[getWindowBounds]")
{
    const auto bounds_hdk20 = Rectangle { 1920, 0, 2160, 1200 };
    CHECK(getWindowBounds(getDisplay(), getScanOutOrigin()) == bounds_hdk20);
}

TEST_CASE_METHOD(HDK13TestFixture, "getEyeOutputViewPort HDK13", "[getEyeOutputViewport")
{
    const auto left_eye = Rectangle { 0, 0, 1080, 1920 / 2 };
    const auto right_eye = Rectangle { 0, 1920 / 2, 1080, 1920 / 2 };

    CHECK(getEyeOutputViewport(vr::Eye_Left, getDisplay(), getScanOutOrigin(), getDisplayMode()) == left_eye);
    CHECK(getEyeOutputViewport(vr::Eye_Right, getDisplay(), getScanOutOrigin(), getDisplayMode()) == right_eye);
}

TEST_CASE_METHOD(HDK20TestFixture, "getEyeOutputViewPort HDK20", "[getEyeOutputViewport")
{
    const auto left_eye = Rectangle { 2160 / 2, 0, 2160 / 2, 1200 };
    const auto right_eye = Rectangle { 0, 0, 2160 / 2, 1200 };

    CHECK(getEyeOutputViewport(vr::Eye_Left, getDisplay(), getScanOutOrigin(), getDisplayMode()) == left_eye);
    CHECK(getEyeOutputViewport(vr::Eye_Right, getDisplay(), getScanOutOrigin(), getDisplayMode()) == right_eye);
}

