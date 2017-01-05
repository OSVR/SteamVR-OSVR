/** @file
    @brief Header

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

#ifndef INCLUDED_Logging_h_GUID_E2F9C0D8_05AD_4D95_922B_3305E93990D3
#define INCLUDED_Logging_h_GUID_E2F9C0D8_05AD_4D95_922B_3305E93990D3

// Internal Includes
#include "make_unique.h"
#include "pretty_print.h"

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <memory>
#include <ostream>
#include <string>
#include <utility>

/**
 * @brief The NullLogger just swallows any log messages it's sent.
 */
class NullLogger : public vr::IDriverLog {
public:
    virtual void Log(const char* /*log_message*/) override
    {
        // do nothing
    }

    virtual ~NullLogger()
    {
        // do nothing
    }
};

/**
 * @brief Log message severity levels.
 */
enum LogLevel {
    properties, ///< property requests and changes.
    trace,      ///< function entry and exit, control flow.
    debug,      ///< developer-facing messages.
    info,       ///< user-facing messages.
    notice,     ///< normal but significant condition.
    warn,       ///< warning conditions.
    err,        ///< error messages.
    critical,   ///< critical conditions.
    alert,      ///< action must be taken immediately.
    emerg       ///< system is unusable.
};

/**
 * @brief A helper class for logging using the stream operator.
 */
class LineLogger {
public:
    LineLogger(bool should_log, vr::IDriverLog* driver_log) : shouldLog_(should_log), driverLog_(driver_log), message_()
    {
        // do nothing
    }

    ~LineLogger()
    {
        // Log the queued message
        if (message_.empty())
            return;

        if (message_.at(message_.size() - 1) != '\n')
            message_ += "\n";

        driverLog_->Log(message_.c_str());
    }

    LineLogger& operator<<(const char msg[])
    {
        if (shouldLog_)
            message_ += msg;

        return *this;
    }

    template <typename T>
    LineLogger& operator<<(T&& msg)
    {
        if (shouldLog_)
            message_ += to_string(std::forward<T>(msg));

        return *this;
    }

protected:
    const bool shouldLog_;
    vr::IDriverLog* driverLog_;
    std::string message_;
};

/**
 * @brief The Logging class is a singleton that's used for logging messages to
 * SteamVR's logging system.
 */
class Logging {
public:
    static Logging& instance()
    {
        static Logging instance_;
        return instance_;
    }

    // We're a singleton
    Logging(Logging const&) = delete;             // Copy construct
    Logging(Logging&&) = delete;                  // Move construct
    Logging& operator=(Logging const&) = delete;  // Copy assign
    Logging& operator=(Logging &&) = delete;      // Move assign

    void setDriverLog(vr::IDriverLog* driver_log)
    {
        driverLog_ = driver_log;
    }

    void setLogLevel(LogLevel severity)
    {
        severity_ = severity;
    }

    LogLevel getLogLevel() const
    {
        return severity_;
    }

    LineLogger log(LogLevel severity)
    {
        const bool should_log = (severity >= severity_);
        return LineLogger{ should_log, driverLog_ };
    }

protected:
    Logging()
    {
        // Point the driver log to a null logger until a real logger is set.
        nullLogger_ = std::make_unique<NullLogger>();
        driverLog_ = nullLogger_.get();
    }

    ~Logging()
    {
        driverLog_ = nullptr;
    }

    std::unique_ptr<NullLogger> nullLogger_;
    vr::IDriverLog* driverLog_;
    LogLevel severity_ = LogLevel::info;
};

#define OSVR_LOG(x) Logging::instance().log(x)

#endif // INCLUDED_Logging_h_GUID_E2F9C0D8_05AD_4D95_922B_3305E93990D3

