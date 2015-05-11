/** @file
    @brief Header

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

#ifndef INCLUDED_ClientMainloopThread_h_GUID_552DB227_9D15_4B9D_E3CD_42A57C734029
#define INCLUDED_ClientMainloopThread_h_GUID_552DB227_9D15_4B9D_E3CD_42A57C734029

// Internal Includes
#include "ClientMainloop.h"

// Library/third-party includes
// - none

// Standard includes
#include <stdexcept>
#include <thread>
#include <chrono>

static const auto SLEEP_TIME = std::chrono::milliseconds(1);

class ClientMainloopThread {
  public:
    typedef ClientMainloop::mutex_type mutex_type;
    typedef ClientMainloop::lock_type lock_type;
    typedef std::thread thread_type;
    ClientMainloopThread(osvr::clientkit::ClientContext &ctx,
                         bool startNow = false)
        : m_run(false), m_started(false), m_mainloop(ctx) {
        if (startNow) {
            start();
        }
    }

    /// Can't copy-construct
    ClientMainloopThread(ClientMainloopThread const&) = delete;
    /// Can't assign
    ClientMainloopThread& operator=(ClientMainloopThread const&) = delete;

    void start() {
        if (m_run || m_started) {
            throw std::logic_error(
                "Can't start if it's already started or if this is a re-start");
        }
        m_started = true;
        m_run = true;
        m_thread = thread_type([&] {
            while (m_run) {
                oneLoop();
            }
        });
    }

    void oneLoop() {
        m_mainloop.mainloop();
        std::this_thread::sleep_for(SLEEP_TIME);
    }

    template <typename T>
    void loopForDuration(T duration = std::chrono::seconds(2)) {
        typedef std::chrono::steady_clock clock;
        auto start = clock::now();
        do {
            oneLoop();
        } while (clock::now() - start < duration);
    }

    ~ClientMainloopThread() {
        m_run = false;
        m_thread.join();
    }
    mutex_type &getMutex() { return m_mainloop.getMutex(); }

  private:
    volatile bool m_run;
    bool m_started;
    ClientMainloop m_mainloop;
    thread_type m_thread;
};

#endif // INCLUDED_ClientMainloopThread_h_GUID_552DB227_9D15_4B9D_E3CD_42A57C734029
