/** @file
    @brief Standalone program for testing a SteamVR driver.

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
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
#include "driver_osvr.cpp"
#include "make_unique.h"

// Library/third-party includes
// - none

// Standard includes
#include <cstdlib> // for EXIT_SUCCESS
#include <iostream>

/**
 * Log messages to the console by default.
 */
class Logger : public vr::IDriverLog {
public:
    void Log(const char* message) OSVR_OVERRIDE
    {
        std::cout << message << std::endl;
    }

    virtual ~Logger()
    {
        // do nothing
    }
};

int main(int argc, char* argv[])
{
    // Instantiate the tracker driver
    std::cout << "Instantiating tracker driver..." << std::endl;
    int driver_init_return = 0;
    auto tracker_driver = static_cast<ServerDriver_OSVR*>(TrackedDeviceDriverFactory(vr::IServerTrackedDeviceProvider_Version, &driver_init_return));
    if (!tracker_driver) {
        std::cerr << "! Error creating tracker driver. ";
        switch (driver_init_return) {
        case vr::VRInitError_Init_InvalidInterface:
            std::cerr << "Invalid interface.";
            break;
        case vr::VRInitError_Init_InterfaceNotFound:
            std::cerr << "Interface not found.";
            break;
        default:
            std::cerr << "Unexpected error: " << driver_init_return << ".";
            break;
        }
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << " - Tracker driver instantiated successfully." << std::endl;

	Logger logger;

    // Initialize the tracker driver
    std::cout << "Initializing the tracker driver..." << std::endl;
    vr::EVRInitError error = tracker_driver->Init(&logger, nullptr, "", "");
    if (vr::VRInitError_None != error) {
        std::cerr << "! Error initializing tracker driver: " << error << "." << std::endl;
        tracker_driver->Cleanup();
        return EXIT_FAILURE;
    }
    std::cout << " - Tracker driver initialized successfully." << std::endl;

    // Get the number of trackers
    const auto tracker_count = tracker_driver->GetTrackedDeviceCount();
    std::cout << "Detected " << tracker_count << " trackers." << std::endl;
    if (tracker_count < 1) {
        std::cerr << "! No trackers were detected." << std::endl;
        tracker_driver->Cleanup();
        return EXIT_FAILURE;
    }

    // Grab first tracker
    std::cout << "Acquiring first detected tracker..." << std::endl;
    vr::ITrackedDeviceServerDriver* tracker = tracker_driver->GetTrackedDeviceDriver(0);

    tracker_driver->Cleanup();

    return EXIT_SUCCESS;
}
