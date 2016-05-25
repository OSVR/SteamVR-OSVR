/** @file
    @brief Implementations of getDisplays().

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
#include "DisplayEnumerator.h"
#include "Display.h"

// Library/third-party includes
#include <osvr/Util/PlatformConfig.h>

// Standard includes
#include <vector>

#if defined(OSVR_WINDOWS)
#include "DisplayEnumerator_Windows.h"
#elif defined(OSVR_LINUX)
#include "DisplayEnumerator_Linux.h"
#elif defined(OSVR_MACOSX)
#include "DisplayEnumerator_MacOSX.h"
#else
#error "getDisplays() not yet implemented for this platform!"
namespace osvr {
namespace display {

std::vector<Display> getDisplays()
{
    return {};
}

} // namespace display
} // namespace osvr
#endif

