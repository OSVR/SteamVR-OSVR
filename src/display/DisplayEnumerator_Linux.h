/** @file
    @brief Linux-specific implementation of getDisplays().

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

#ifndef INCLUDED_DisplayEnumerator_Linux_h_GUID_CA8EA9D4_36A1_4492_8383_30419BD91FD3
#define INCLUDED_DisplayEnumerator_Linux_h_GUID_CA8EA9D4_36A1_4492_8383_30419BD91FD3

// Internal Includes
#include "DisplayEnumerator.h"
#include "Display.h"

// Library/third-party includes
// - none

// Standard includes
#include <vector>

namespace osvr {
namespace display {

std::vector<Display> getDisplays()
{
	// TODO
    return {};
}

DesktopOrientation getDesktopOrientation(const Display& display)
{
	// TODO
	return DesktopOrientation::Landscape;
}

} // end namespace display
} // end namespace osvr

#endif // INCLUDED_DisplayEnumerator_Linux_h_GUID_CA8EA9D4_36A1_4492_8383_30419BD91FD3

