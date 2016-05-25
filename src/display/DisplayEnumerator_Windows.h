/** @file
    @brief Windows-specific implementation of DisplayEnumerator.

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

// Library/third-party includes
// - none

// Standard includes
#include <vector>
#include <utility>

// Forward declarations
class Display;

class DisplayEnumerator_Windows : public DisplayEnumerator {
    using PathInfoList = std::vector<DISPLAYCONFIG_PATH_INFO>;
    using ModeInfoList = std::vector<DISPLAYCONFIG_MODE_INFO>;

public:
    DisplayEnumerator_Windows();
    virtual ~DisplayEnumerator_Windows();

    std::vector<Display> getDisplays() OSVR_OVERRIDE;

private:
    std::pair<UINT32, UINT32> getBufferSizes();
    std::pair<PathInfoList, ModeInfoList> getDisplayInformation();
    Display getDisplay(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info);
    DisplayAdapter getDisplayAdapter(const DISPLAYCONFIG_PATH_INFO& path_info, const ModeInfoList& mode_info);

    IDXGIFactory* factory_;
};

#endif // INCLUDED_DisplayEnumerator_Windows_h_GUID_E1608541_438C_4A60_AB12_7650DA2EE279

