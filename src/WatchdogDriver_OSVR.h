/** @file
    @brief OSVR watchdog driver

    @date 2017

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2017 Sensics, Inc.
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

#ifndef INCLUDED_WatchdogDriver_OSVR_h_GUID_875D03A3_439B_437C_AD2B_D7AAB2E1CA62
#define INCLUDED_WatchdogDriver_OSVR_h_GUID_875D03A3_439B_437C_AD2B_D7AAB2E1CA62

// Internal Includes
#include "osvr_compiler_detection.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <memory>
#include <thread>

/**
 * This interface must be implemented in each driver. It will be loaded in
 * vrclient.dll.
 */
class WatchdogDriver_OSVR : public vr::IVRWatchdogProvider {
public:
    WatchdogDriver_OSVR();
    virtual ~WatchdogDriver_OSVR();

    /**
     * Initializes the driver in watchdog mode.
     */
    virtual vr::EVRInitError Init(vr::IVRDriverContext* driver_context) OSVR_OVERRIDE;

    /**
     * Cleans up the driver right before it is unloaded,
     */
    virtual void Cleanup() OSVR_OVERRIDE;

private:
    std::unique_ptr<std::thread> thread_;
};

void WatchdogThreadFunction();

#endif // INCLUDED_WatchdogDriver_OSVR_h_GUID_875D03A3_439B_437C_AD2B_D7AAB2E1CA62

