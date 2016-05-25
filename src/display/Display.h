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

#ifndef INCLUDED_Display_h_GUID_ED98FB68_A0E9_480B_9CF0_8C6D444DEB7E
#define INCLUDED_Display_h_GUID_ED98FB68_A0E9_480B_9CF0_8C6D444DEB7E

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <string>
#include <cstdint>

namespace osvr {
namespace display {

struct DisplaySize {
    int32_t width;
    int32_t height;
};

struct DisplayPosition {
    int32_t x;
    int32_t y;
};

struct DisplayAdapter {
    std::string description;
    unsigned int vendor_id;
    unsigned int device_id;
    unsigned int subsystem_id;
    unsigned int revision;
};

enum class Rotation {
    Zero,
    Ninety,
    OneEighty,
    TwoSeventy
};

struct Display {
    DisplayAdapter adapter;
    std::string name;
    DisplaySize size;
    DisplayPosition position;
    Rotation rotation;
    double verticalRefreshRate;
    bool attachedToDesktop;
    uint32_t edidVendorId;
    uint32_t edidProductId;
};

} // end namespace display
} // end namespace osvr

#endif // INCLUDED_Display_h_GUID_ED98FB68_A0E9_480B_9CF0_8C6D444DEB7E

