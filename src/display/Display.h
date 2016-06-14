/** @file
    @brief Display struct and related types.

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
    uint32_t width;
    uint32_t height;

    bool operator==(const DisplaySize& other) const
    {
        if (width != other.width) return false;
        if (height != other.height) return false;

        return true;
    }

    bool operator!=(const DisplaySize& other) const
    {
        return !(*this == other);
    }
};

struct DisplayPosition {
    int32_t x;
    int32_t y;

    bool operator==(const DisplayPosition& other) const
    {
        if (x != other.x) return false;
        if (y != other.y) return false;

        return true;
    }

    bool operator!=(const DisplayPosition& other) const
    {
        return !(*this == other);
    }
};

struct DisplayAdapter {
    std::string description;

    bool operator==(const DisplayAdapter& other) const
    {
        if (description != other.description) return false;

        return true;
    }

    bool operator!=(const DisplayAdapter& other) const
    {
        return !(*this == other);
    }
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

    bool operator==(const Display& other) const
    {
        if (adapter != other.adapter) return false;
        if (name != other.name) return false;
        if (size != other.size) return false;
        if (position != other.position) return false;
        if (rotation != other.rotation) return false;
        if (verticalRefreshRate != other.verticalRefreshRate) return false;
        if (attachedToDesktop != other.attachedToDesktop) return false;
        if (edidProductId != other.edidProductId) return false;
        if (edidVendorId != other.edidVendorId) return false;

        return true;
    }

    bool operator!=(const Display& other) const
    {
        return !(*this == other);
    }
};

} // end namespace display
} // end namespace osvr

#endif // INCLUDED_Display_h_GUID_ED98FB68_A0E9_480B_9CF0_8C6D444DEB7E

