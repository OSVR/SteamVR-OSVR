/** @file
    @brief OS X-specific implementation of getDisplays().

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

#ifndef INCLUDED_DisplayEnumerator_MacOSX_h_GUID_C1F03499_42F5_4CD5_AECF_F9CAFD07E99F
#define INCLUDED_DisplayEnumerator_MacOSX_h_GUID_C1F03499_42F5_4CD5_AECF_F9CAFD07E99F

// Internal Includes
#include "DisplayEnumerator.h"
#include "Display.h"

// Library/third-party includes
#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/graphics/IOGraphicsLib.h>

// Standard includes
#include <vector>
#include <iostream>

namespace osvr {
namespace display {

namespace {
//
// Forward declarations
//

io_service_t getIOServicePort(const CGDirectDisplayID& display_id);
std::string to_string(CFStringRef str);
std::string to_string(CFDataRef ref);
uint32_t getNumDisplays();
std::string getDisplayAdapterDescription(const CGDirectDisplayID& display_id);
DisplayAdapter getDisplayAdapter(const CGDirectDisplayID& display_id);
std::string getDisplayName(const CGDirectDisplayID& display_id);
DisplaySize getDisplaySize(const CGDirectDisplayID& display_id);
DisplayPosition getDisplayPosition(const CGDirectDisplayID& display_id);
Rotation getDisplayRotation(const CGDirectDisplayID& display_id);
double getDisplayRefreshRate(const CGDirectDisplayID& display_id);
bool getDisplayAttachedToDesktop(const CGDirectDisplayID& display_id);
uint32_t getDisplayEDIDVendorID(const CGDirectDisplayID& display_id);
uint32_t getDisplayEDIDProductID(const CGDirectDisplayID& display_id);
Display getDisplay(const CGDirectDisplayID& display_id);

//
// Utility functions
//

namespace {

void printDict(CFDictionaryRef dict)
{
    const auto size = CFDictionaryGetCount(dict);
    CFTypeRef* keys_ref = new CFTypeRef[size];
    CFTypeRef* values_ref = new CFTypeRef[size];
    CFDictionaryGetKeysAndValues(dict, (const void**)keys_ref, (const void**)values_ref);

    std::cout << "Dictionary contents:" << std::endl;
    for (size_t i = 0; i < size; ++i) {
        std::cout << "  " << std::flush;
        CFShow(keys_ref[i]);
        std::cout << "    = " << std::flush;
        CFShow(values_ref[i]);
        //const auto key = keys_ref[i];
        //const auto value = values_ref[i];
        //std::cout << "  " << key << ": [" << value << "]." << std::endl;
    }
    std::cout << "\n" << std::endl;
}

std::string getString(const CFDictionaryRef& dict, const char* key, const std::string& default_value = "")
{
    const auto key_str = CFStringCreateWithCString(nullptr, key, kCFStringEncodingMacRoman);
    CFStringRef str_ref;
    if (CFDictionaryGetValueIfPresent(dict, key_str, (const void**)&str_ref)) {
        CFRelease(key_str);
        return to_string(str_ref);
    }

    CFRelease(key_str);
    return default_value;
}

} // anonymous namespace


//
// Implementations
//

io_service_t getIOServicePort(const CGDirectDisplayID& display_id)
{
    io_service_t service_port = 0;

    CFMutableDictionaryRef matching_dict = IOServiceMatching("IODisplayConnect");
    io_iterator_t iter;
    auto ret = IOServiceGetMatchingServices(kIOMasterPortDefault, matching_dict, &iter);
    if (ret) {
        std::cerr << "getIOServicePort(): Error calling IOServiceGetMatchingServices(): " << ret << std::endl;
        return service_port;
    }

    io_service_t serv = 0;
    while ((serv = IOIteratorNext(iter)) != 0) {
        auto display_info = IODisplayCreateInfoDictionary(serv, kIODisplayOnlyPreferredName);

        CFNumberRef vendor_id_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(display_info, CFSTR(kDisplayVendorID)));
        CFIndex vendor_id;
        auto vendor_okay = CFNumberGetValue(vendor_id_ref, kCFNumberCFIndexType, &vendor_id);

        CFNumberRef product_id_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(display_info, CFSTR(kDisplayProductID)));
        CFIndex product_id;
        auto product_okay = CFNumberGetValue(product_id_ref, kCFNumberCFIndexType, &product_id);


        CFNumberRef serial_number_ref = static_cast<CFNumberRef>(CFDictionaryGetValue(display_info, CFSTR(kDisplaySerialNumber)));
        CFIndex serial_number;
        Boolean serial_okay = false;
        if (serial_number_ref) {
            serial_okay = CFNumberGetValue(serial_number_ref, kCFNumberCFIndexType, &serial_number);
        } else {
            serial_okay = true;
            serial_number = 0x00;
        }

        CFRelease(display_info);

        const auto success = vendor_okay && product_okay && serial_okay;
        if (!success) {
            std::cerr << "getIOServicePort(): Failed to retrieve vendor, product, or serial IDs." << std::endl;
            continue;
        }

        const auto vendor_matches = CGDisplayVendorNumber(display_id) == vendor_id;
        const auto model_matches = CGDisplayModelNumber(display_id) == product_id;
        const auto serial_matches = CGDisplaySerialNumber(display_id) == serial_number;
        const auto is_same_display = vendor_matches && model_matches && serial_matches;
        if (!is_same_display) {
            continue;
        }

        service_port = serv;
        break;
    }

    IOObjectRelease(iter);
    return service_port;
}

std::string to_string(CFStringRef str)
{
    // Convert from CFStringRef to C-style string
    const auto size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8);
    auto value = new char[size + 1];
    CFStringGetCString(str, value, size, kCFStringEncodingUTF8);

    // Convert to std::string and clean up
    std::string s { value };
    delete [] value;

    return s;
}

std::string to_string(CFDataRef ref)
{
    // Copy the bytes into a null-terminated buffer
    const auto size = CFDataGetLength(ref);
    const auto range = CFRangeMake(0, size);
    auto buffer = new unsigned char[size + 1];
    CFDataGetBytes(ref, range, buffer);
    buffer[size] = '\0';

    std::string s { reinterpret_cast<const char*>(buffer) };
    delete [] buffer;

    return s;
}

uint32_t getNumDisplays()
{
    uint32_t num_displays = 0;
    const auto ret = CGGetOnlineDisplayList(0, nullptr, &num_displays);
    if (kCGErrorSuccess != ret) {
        std::cerr << "Error detecting number of displays: " << ret << std::endl;
    }

    return num_displays;
}

std::string getDisplayAdapterDescription(const CGDirectDisplayID& display_id)
{
    std::string adapter_name = "FIXME: getDisplayAdapterDescription()";

    io_registry_entry_t io_service_port = getIOServicePort(display_id);
    CFDataRef model = static_cast<CFDataRef>(IORegistryEntrySearchCFProperty(io_service_port, kIOServicePlane, CFSTR("model"), kCFAllocatorDefault, kIORegistryIterateRecursively | kIORegistryIterateParents));

    if (model) {
        adapter_name = to_string(model);
        CFRelease(model);
    }

    return adapter_name;
}

DisplayAdapter getDisplayAdapter(const CGDirectDisplayID& display_id)
{
    DisplayAdapter adapter;
    adapter.description = getDisplayAdapterDescription(display_id);

    return adapter;
}

std::string getDisplayName(const CGDirectDisplayID& display_id)
{
    std::string display_name = "[Unknown]";

    CFDictionaryRef display_info = IODisplayCreateInfoDictionary(getIOServicePort(display_id), kIODisplayOnlyPreferredName);

    //printDict(display_info);

    CFDictionaryRef display_names;
    if (!CFDictionaryGetValueIfPresent(display_info, CFSTR(kDisplayProductName), (const void**)&display_names)) {
        // No display names were available
        CFRelease(display_info);
        return display_name;
    }

    display_name = getString(display_names, "en_US", display_name);

    CFRelease(display_info);
    return display_name;
}

DisplaySize getDisplaySize(const CGDirectDisplayID& display_id)
{
    DisplaySize display_size;

    display_size.height = CGDisplayPixelsHigh(display_id);
    display_size.width = CGDisplayPixelsWide(display_id);

    return display_size;
}

DisplayPosition getDisplayPosition(const CGDirectDisplayID& display_id)
{
    DisplayPosition display_position;

    const auto display_bounds = CGDisplayBounds(display_id);

    display_position.x = static_cast<uint32_t>(display_bounds.origin.x);
    display_position.y = static_cast<uint32_t>(display_bounds.origin.y);

    return display_position;
}

Rotation getDisplayRotation(const CGDirectDisplayID& display_id)
{
    const auto degrees = CGDisplayRotation(display_id);

    // TODO make this a bit a nicer: round the values to the nearest 90 degrees
    if (0.0 == degrees) {
        return Rotation::Zero;
    } else if (90.0 == degrees) {
        return Rotation::Ninety;
    } else if (180.0 == degrees) {
        return Rotation::OneEighty;
    } else if (270.0 == degrees) {
        return Rotation::TwoSeventy;
    } else {
        return Rotation::Zero;
    }
}

double getDisplayRefreshRate(const CGDirectDisplayID& display_id)
{
    auto display_mode_ref = CGDisplayCopyDisplayMode(display_id);
    if (!display_mode_ref) {
        std::cerr << "getDisplayRefreshRate(): Got NULL display mode reference." << std::endl;
    }
    const auto refresh_rate = CGDisplayModeGetRefreshRate(display_mode_ref);
    CGDisplayModeRelease(display_mode_ref);

    return refresh_rate;
}

bool getDisplayAttachedToDesktop(const CGDirectDisplayID& display_id)
{
    return true;
}

uint32_t getDisplayEDIDVendorID(const CGDirectDisplayID& display_id)
{
    return CGDisplayVendorNumber(display_id);
}

uint32_t getDisplayEDIDProductID(const CGDirectDisplayID& display_id)
{
    return CGDisplayModelNumber(display_id);
}

Display getDisplay(const CGDirectDisplayID& display_id)
{
    Display display;
    display.adapter = getDisplayAdapter(display_id);
    display.name = getDisplayName(display_id);
    display.size = getDisplaySize(display_id);
    display.position = getDisplayPosition(display_id);
    display.rotation = getDisplayRotation(display_id);
    display.verticalRefreshRate = getDisplayRefreshRate(display_id);
    display.attachedToDesktop = getDisplayAttachedToDesktop(display_id);
    display.edidVendorId = getDisplayEDIDVendorID(display_id);
    display.edidProductId = getDisplayEDIDProductID(display_id);

    return display;
}

} // end anonymous namespace

std::vector<Display> getDisplays()
{
    std::vector<Display> displays;

    auto max_displays = getNumDisplays();

    std::vector<CGDirectDisplayID> display_ids(max_displays);
    uint32_t num_displays = 0;
    auto ret = CGGetOnlineDisplayList(max_displays, display_ids.data(), &num_displays);
    if (kCGErrorSuccess != ret) {
        std::cerr << "Error getting list of online display IDs: " << ret << std::endl;
    }

    for (const auto& display_id : display_ids) {
        try {
            const auto display = getDisplay(display_id);
            displays.emplace_back(std::move(display));
        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << ".";
            std::cerr << "Ignoring this display." << std::endl;
        }
    }

    return displays;
}

DesktopOrientation getDesktopOrientation(const Display& display)
{
    // OS X reports the resolution values in terms of orientation
    // (e.g., 1080x1920 in portrait orientation, but 1920x1080 in landscape
    // orientation).
    const auto rotation = display.rotation;
    const auto is_landscape = display.size.height < display.size.width;

    if (osvr::display::Rotation::Zero == rotation) {
        return (is_landscape ? DesktopOrientation::Landscape : DesktopOrientation::Portrait);
    } else if (osvr::display::Rotation::Ninety == rotation) {
        return (is_landscape ? DesktopOrientation::Landscape : DesktopOrientation::PortraitFlipped);
    } else if (osvr::display::Rotation::OneEighty == rotation) {
        return (is_landscape ? DesktopOrientation::LandscapeFlipped : DesktopOrientation::PortraitFlipped);
    } else if (osvr::display::Rotation::TwoSeventy == rotation) {
        return (is_landscape ? DesktopOrientation::LandscapeFlipped : DesktopOrientation::Portrait);
    } else {
        std::cerr << "Invalid rotation value: " << static_cast<int>(rotation) << ".";
        return DesktopOrientation::Landscape;
    }
}

} // end namespace display
} // end namespace osvr

#endif // INCLUDED_DisplayEnumerator_MacOSX_h_GUID_C1F03499_42F5_4CD5_AECF_F9CAFD07E99F

