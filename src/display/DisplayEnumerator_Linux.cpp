/** @file
    @brief Linux-specific implementation of DisplayEnumerator.

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

// Library/third-party includes
// - none

// Standard includes
#include <vector>

namespace osvr {
namespace display {

DisplayEnumerator_Linux::DisplayEnumerator_Linux()
{
    // do nothing
}

DisplayEnumerator_Linux::~DisplayEnumerator_Linux()
{
    // do nothing
}

std::vector<Display> DisplayEnumerator_Linux::getDisplays()
{
    std::vector<Display> displays;
#if 0
    // TODO

    char* display_name = nullptr;
    Display* dpy = XOpenDisplay(display_name);
    if (!dpy) {
        // Unable to open display (XDisplayName(displayname))
        return;
    }

    print_display_info(dpy);
    for (i = 0; i < ScreenCount(dpy); i++) {
        print_screen_info(dpy, i);
    }

    print_marked_extensions(dpy);

    XCloseDisplay(dpy);
#endif
    return displays;
}

} // end namespace display
} // end namespace osvr

