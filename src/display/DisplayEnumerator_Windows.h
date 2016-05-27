/** @file
    @brief Windows-specific implementation of getDisplays().

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

#ifndef INCLUDED_DisplayEnumerator_Windows_h_GUID_E1608541_438C_4A60_AB12_7650DA2EE279
#define INCLUDED_DisplayEnumerator_Windows_h_GUID_E1608541_438C_4A60_AB12_7650DA2EE279

// Internal Includes
#include "DisplayEnumerator.h"
#include "Display.h"

// Library/third-party includes
#include <Windows.h>

// Standard includes
#include <vector>
#include <utility>
#include <iostream>
#include <string>
#include <codecvt>
#include <locale>

namespace osvr {
namespace display {

using PathInfoList = std::vector<DISPLAYCONFIG_PATH_INFO>;
using ModeInfoList = std::vector<DISPLAYCONFIG_MODE_INFO>;

namespace {

    // Forward declarations
    std::pair<UINT32, UINT32> getBufferSizes(const UINT32 query_flags);
    std::pair<PathInfoList, ModeInfoList> getDisplayInformation();
    Display getDisplay(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info);
    DisplayAdapter getDisplayAdapter(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info);
    std::string getAdapterName(const DISPLAYCONFIG_PATH_INFO& path_info);
    std::string to_string(const std::wstring& s);
    Rotation getRotation(const DISPLAYCONFIG_PATH_INFO& path_info);
    double getRefreshRate(const DISPLAYCONFIG_PATH_INFO& path_info);
    std::pair<uint32_t, uint32_t> getEDIDInfo(const DISPLAYCONFIG_PATH_INFO& path_info);

    std::pair<UINT32, UINT32> getBufferSizes(const UINT32 query_flags)
    {
        // Get the sizes of the path and mode buffers
        UINT32 num_path = 0;
        UINT32 num_mode_info = 0;
        const auto get_buffer_sizes_ret = GetDisplayConfigBufferSizes(query_flags, &num_path, &num_mode_info);
        if (get_buffer_sizes_ret != ERROR_SUCCESS) {
            switch (get_buffer_sizes_ret) {
            case ERROR_INVALID_PARAMETER:
                {
                    const std::string msg = "An internal error occurred. GetDisplayConfigBufferSizes() returned ERROR_INVALID_PARAMETER. Please file a bug report at <https://github.com/osvr/SteamVR-OSVR>.";
                    std::cerr << msg << std::endl;
                    throw std::runtime_error(msg);
                }
            case ERROR_NOT_SUPPORTED:
                {
                    const std::string msg = "DisplayEnumerator requires Windows 7 or above and a graphics driver that was written according to the Windows Display Driver Model (WDDM).. Please file a bug report at <https://github.com/osvr/SteamVR-OSVR>.";
                    std::cerr << msg << std::endl;
                    throw std::runtime_error(msg);
                }
            case ERROR_ACCESS_DENIED:
                {
                    const std::string msg = "Access denied to the console session. SteamVR-OSVR won't run on a remote session.";
                    std::cerr << msg << std::endl;
                    throw std::runtime_error(msg);
                }
            case ERROR_GEN_FAILURE:
                {
                    const std::string msg = "An unspecified error occurred while calling GetDisplayConfigBufferSizes(). Please file a bug report at <https://github.com/osvr/SteamVR-OSVR>.";
                    std::cerr << msg << std::endl;
                    throw std::runtime_error(msg);
                }
            }
        }

        return std::make_pair(std::move(num_path), std::move(num_mode_info));
    }


    std::pair<PathInfoList, ModeInfoList> getDisplayInformation()
    {
        const UINT32 query_flags = QDC_ONLY_ACTIVE_PATHS;
        const auto buffer_sizes = getBufferSizes(query_flags);

        auto num_path = buffer_sizes.first;
        auto num_mode_info = buffer_sizes.second;

        std::vector<DISPLAYCONFIG_PATH_INFO> path_info(num_path);
        std::vector<DISPLAYCONFIG_MODE_INFO> mode_info(num_mode_info);
        const auto query_display_config_ret = QueryDisplayConfig(query_flags, &num_path, path_info.data(), &num_mode_info, mode_info.data(), nullptr);
        switch (query_display_config_ret) {
        case ERROR_INVALID_PARAMETER:
            {
                const std::string msg = "The combination of parameters and flags that are specified is invalid.";
                std::cerr << msg << std::endl;
                throw std::runtime_error(msg);
            }
        case ERROR_NOT_SUPPORTED:
            {
                const std::string msg = "The system is not running a graphics driver that was written according to the Windows Display Driver Model (WDDM). The function is only supported on a system with a WDDM driver running.";
                std::cerr << msg << std::endl;
                throw std::runtime_error(msg);
            }
        case ERROR_ACCESS_DENIED:
            {
                const std::string msg = "The caller does not have access to the console session. This error occurs if the calling process does not have access to the current desktop or is running on a remote session.";
                std::cerr << msg << std::endl;
                throw std::runtime_error(msg);
            }
        case ERROR_GEN_FAILURE:
            {
                const std::string msg = "An unspecified error occurred.";
                std::cerr << msg << std::endl;
                throw std::runtime_error(msg);
            }
        case ERROR_INSUFFICIENT_BUFFER:
            {
                const std::string msg = "The supplied path and mode buffer are too small.";
                // TODO call GetDisplayConfigBufferSizes() again and adjust the size of the vectors
                std::cerr << msg << std::endl;
                throw std::runtime_error(msg);
            }
        }

        return {std::move(path_info), std::move(mode_info)};
    }

    Display getDisplay(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info)
    {
        Display display;
        display.adapter = getDisplayAdapter(path_info, mode_info);
        display.name = getMonitorName(path_info);
        display.size = getCurrentResolution(path_info, mode_info);
        display.position = getPosition(path_info, mode_info);
        display.rotation = getRotation(path_info);
        display.verticalRefreshRate = getRefreshRate(path_info);
        display.attachedToDesktop = true; // TODO
        const auto edid_info = getEDIDInfo(path_info);
        display.edidVendorId = edid_info.first;
        display.edidProductId = edid_info.second;

        return display;
    }

    std::pair<uint32_t, uint32_t> getEDIDInfo(const DISPLAYCONFIG_PATH_INFO& path_info)
    {
        DISPLAYCONFIG_TARGET_DEVICE_NAME target_name = {};
        target_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        target_name.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
        target_name.header.adapterId = path_info.targetInfo.adapterId;
        target_name.header.id = path_info.targetInfo.id;
        const auto ret = DisplayConfigGetDeviceInfo(&target_name.header);
        // TODO check ret value

        return {target_name.edidManufactureId, target_name.edidProductCodeId};
    }

    double getRefreshRate(const DISPLAYCONFIG_PATH_INFO& path_info)
    {
        return path_info.targetInfo..RefreshRate;
    }

    Rotation getRotation(const DISPLAYCONFIG_PATH_INFO& path_info)
    {
        const auto target_info = path_info.targetInfo;
        switch (target_info.rotation) {
        case DISPLAYCONFIG_ROTATION_IDENTITY:
            return Rotation::Zero;
        case DISPLAYCONFIG_ROTATION_ROTATE90:
            return Rotation::Ninety;
        case DISPLAYCONFIG_ROTATION_ROTATE180:
            return Rotation::OneEighty;
        case DISPLAYCONFIG_ROTATION_ROTATE270:
            return Rotation::TwoSeventy;
        default:
            return Rotation::Zero;
        }
    }

    DisplaySize getCurrentResolution(const DISPLAYCONFIG_PATH_INFO& path_info, const std::vector<DISPLAYCONFIG_MODE_INFO>& mode_info)
    {
        const auto source_info = path_info.sourceInfo;
        if (DISPLAYCONFIG_PATH_MODE_IDX_INVALID != source_info.modeInfoIdx) {
            const auto source_info = mode_info.at(source_info.modeInfoIdx);
            const auto source_mode = source_info.sourceMode;
            return {source_mode.width, source_mode.height};
        }

        return {0, 0};
    }

    DisplayPosition getPosition(const DISPLAYCONFIG_PATH_INFO& path_info, const std::vector<DISPLAYCONFIG_MODE_INFO>& mode_info)
    {
        const auto source_info = path_info.sourceInfo;
        if (DISPLAYCONFIG_PATH_MODE_IDX_INVALID != source_info.modeInfoIdx) {
            const auto source_info = mode_info.at(source_info.modeInfoIdx);
            const auto source_mode = source_info.sourceMode;
            return {source_mode.position.x, source_mode.position.y};
        }

        return {0, 0};
    }

    std::string getMonitorName(const DISPLAYCONFIG_PATH_INFO& path_info)
    {
        DISPLAYCONFIG_TARGET_DEVICE_NAME target_name = {};
        target_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        target_name.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
        target_name.header.adapterId = path_info.sourceInfo.adapterId;
        target_name.header.id = path_info.targetInfo.target_id;
        const auto ret = DisplayConfigGetDeviceInfo(&target_name.header);
        // TODO check return value

        return to_string(target_name.monitorFriendlyDeviceName);
    }

    DisplayAdapter getDisplayAdapter(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info)
    {
        // TODO
        DisplayAdapter adapter;
        adapter.description = getAdapterName(path_info);
        adapter.vendor_id = 0x00;
        adapter.device_id = 0x00;
        adapter.subsystem_id = 0x00;
        adapter.revision = 0x00;

        return adapter;
    }

    std::string getAdapterName(const DISPLAYCONFIG_PATH_INFO& path_info)
    {
        auto source_info = path_info.sourceInfo;
        auto adapter_id = source_info.adapterId;

        DISPLAYCONFIG_ADAPTER_NAME adapter_name = {};
        adapter_name.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
        adapter_name.header.adapterId = adapter_id;
        adapter_name.header.size = sizeof(DISPLAYCONFIG_ADAPTER_NAME);
        const auto ret = DisplayConfigGetDeviceInfo(&adapter_name.header);
        // TODO check ret value

        return to_string(adapter_name.adapterDevicePath);
    }

    std::string to_string(const std::wstring& s)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        return converter.to_bytes(s);
    }

} // end anonymous namespace

std::vector<Display> getDisplays()
{
    std::vector<Display> displays;

    // TODO catch exceptions and retry if we need to resize buffers
    const auto info = getDisplayInformation();

    auto path_info = info.first;
    auto mode_info = info.second;

    for (const auto& path : path_info) {
        const auto display = getDisplay(path, mode_info);
        displays.emplace_back(std::move(display));
    }

    return displays;
}

} // end namespace display
} // end namespace osvr

#endif // INCLUDED_DisplayEnumerator_Windows_h_GUID_E1608541_438C_4A60_AB12_7650DA2EE279

