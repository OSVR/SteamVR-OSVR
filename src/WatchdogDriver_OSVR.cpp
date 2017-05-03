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

// Internal Includes
#include "WatchdogDriver_OSVR.h"
#include "Logging.h"

// Library/third-party includes
// - none

// Standard includes
// - none

WatchdogDriver_OSVR::WatchdogDriver_OSVR() : thread_(nullptr)
{
    // do nothing
}

WatchdogDriver_OSVR::~WatchdogDriver_OSVR()
{
    // do nothing
}

vr::EVRInitError WatchdogDriver_OSVR::Init(vr::IVRDriverContext* driver_context)
{
    VR_INIT_WATCHDOG_DRIVER_CONTEXT(driver_context);

    Logging::instance().setDriverLog(vr::VRDriverLog());

    // Watchdog mode on Windows starts a thread that listens for the 'Y' key on the keyboard to
    // be pressed. A real driver should wait for a system button event or something else from the
    // the hardware that signals that the VR system should start up.
    g_bExiting = false;
    thread_ = new std::thread(WatchdogThreadFunction);
    if (!thread_) {
        DriverLog("Unable to create watchdog thread\n");
        return VRInitError_Driver_Failed;
    }

    return VRInitError_None;
}

void WatchdogDriver_OSVR::Cleanup()
{
    g_bExiting = true;
    if (thread_) {
        thread_->join();
        delete thread_;
        thread_ = nullptr;
    }

    CleanupDriverLog();
}

void WatchdogThreadFunction()
{
    while (!g_bExiting) {
#if defined(OSVR_WINDOWS)
        // FIXME replace with more useful event
        // on windows send the event when the Y key is pressed.
        if ((0x01 & GetAsyncKeyState('Y')) != 0) {
            // Y key was pressed.
            vr::VRWatchdogHost()->WatchdogWakeUp();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(500));
#else
        // for the other platforms, just send one every five seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        vr::VRWatchdogHost()->WatchdogWakeUp();
#endif
    }
}


