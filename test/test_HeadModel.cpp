/** @file
    @brief Test head model code.

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
// - none

// Library/third-party includes
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include <osvr/ClientKit/InterfaceStateC.h>

// Standard includes
#include <chrono>
#include <iostream>
#include <thread>


int main(int argc, char* argv[])
{
    osvr::clientkit::ClientContext context("org.osvr.steamvr.test.TrackerState");

    osvr::clientkit::Interface head_tracker = context.getInterface("/me/head");

    // Pretend that this is your application's mainloop.
    for (int i = 0; i < 1000000; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        context.update();

        // Note that there is not currently a tidy C++ wrapper for
        // state access, so we're using the C API call directly here.
        OSVR_PositionState state;
        OSVR_TimeValue timestamp;
        OSVR_ReturnCode ret = osvrGetPositionState(head_tracker.get(), &timestamp, &state);
        if (OSVR_RETURN_SUCCESS != ret) {
            std::cout << "No position state. Apply SteamVR head model instead." << std::endl;
        } else {
            std::cout << "Has position state. Use OSVR's head model." << std::endl;
        }
    }

    return 0;
}

