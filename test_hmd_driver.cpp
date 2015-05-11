/** @file
    @brief Standalone program for testing a SteamVR driver.

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

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
#include <cstdlib>      // for EXIT_SUCCESS
#include <iostream>

int main(int argc, char* argv[])
{
    // Instantiate the HMD driver
    std::cout << "Instantiating HMD driver..." << std::endl;
    int driver_init_return = 0;
    auto hmd_driver = static_cast<CDriver_OSVR*>(HmdDriverFactory(vr::IHmdDriverProvider_Version, &driver_init_return));
    if (!hmd_driver) {
        std::cerr << "! Error creating HMD driver. ";
        switch (driver_init_return) {
        case vr::HmdError_Init_InvalidInterface:
            std::cerr << "Invalid interface.";
            break;
        case vr::HmdError_Init_InterfaceNotFound:
            std::cerr << "Interface not found.";
            break;
        default:
            std::cerr << "Unexpected error: " << driver_init_return << ".";
            break;
        }
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << " - HMD driver instantiated successfully." << std::endl;

    // Initialize the HMD driver
    std::cout << "Initializing the HMD driver..." << std::endl;
    vr::HmdError error = hmd_driver->Init("", "");
    if (vr::HmdError_None != error) {
        std::cerr << "! Error initializing HMD driver: " << error << "." << std::endl;
        hmd_driver->Cleanup();
        return EXIT_FAILURE;
    }
    std::cout << " - HMD driver initialized successfully." << std::endl;

    // Get the number of HMDs
    const auto hmd_count = hmd_driver->GetHmdCount();
    std::cout << "Detected " << hmd_count << " HMDs." << std::endl;
    if (hmd_count < 1) {
        std::cerr << "! No HMDs were detected." << std::endl;
        hmd_driver->Cleanup();
        return EXIT_FAILURE;
    }

    // Grab first HMD
    std::cout << "Acquiring first detected HMD..." << std::endl;
    vr::IHmdDriver* hmd = hmd_driver->GetHmd(0);


    return EXIT_SUCCESS;
}

