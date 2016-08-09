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
#include <iostream>
#include <ctime>
#include <chrono>

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
 * @brief The ConsoleLogger prints messages to the console.
 */
class ConsoleLogger : public vr::IDriverLog {
public:
    virtual void Log(const char* log_message) override
    {
        std::cout << "osvr: " << log_message;
    }

    virtual ~ConsoleLogger()
    {
        // do nothing
    }
};

class BufferedLogger : public vr::IDriverLog {
public:
    BufferedLogger(vr::IDriverLog* logger) : logger_(logger)
    {
        // do nothing
    }

    virtual void Log(const char* log_message) override
    {
        const auto elapsed_time = std::time(nullptr) - previousTime_;
        const bool is_same_msg = (previousMessage_ == log_message);
        const bool is_within_time_window = (elapsed_time < maxBufferTime_);
        if (is_same_msg && is_within_time_window) {
            // Buffer the message
            count_++;
            return;
        }

        // Flush the buffer
        if (count_ > 1) {
            const std::string msg = "Last message repeated " + std::to_string(count_) + " times.\n";
            logger_->Log(msg.c_str());
        }

        // Log the new message
        logger_->Log(log_message);

        // Reset the buffer
        previousMessage_ = log_message;
        previousTime_ = std::time(nullptr);
        count_ = 0;
    }

    virtual ~BufferedLogger()
    {
        // Flush and buffered messages
        if (count_ > 1) {
            const std::string msg = "Last message repeated " + std::to_string(count_) + " times.\n";
            logger_->Log(msg.c_str());
        }
        logger_ = nullptr;
    }

protected:
    vr::IDriverLog* logger_ = nullptr;
    std::time_t maxBufferTime_ = 1; // seconds
    std::time_t previousTime_ = std::time(nullptr);
    std::string previousMessage_ = "";
    unsigned int count_ = 0;
};

/**
 * @brief Log message severity levels.
 */
enum LogLevel {
    trace,     ///< function entry and exit, control flow.
    debug,     ///< developer-facing messages.
    info,      ///< user-facing messages.
    notice,    ///< normal but significant condition.
    warn,      ///< warning conditions.
    err,       ///< error messages.
    critical,  ///< critical conditions.
    alert,     ///< action must be taken immediately.
    emerg      ///< system is unusable.
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
        if (driverLog_) {
            driverLog_ = std::make_unique<BufferedLogger>(driver_log);
        } else {
            driverLog_ = std::make_unique<NullLogger>();
        }
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
        return LineLogger{ should_log, driverLog_.get() };
    }

protected:
    Logging()
    {
        // Point the driver log to a null logger until a real logger is set.
        driverLog_ = std::make_unique<NullLogger>();
    }

    ~Logging()
    {
        // do nothing
    }

    std::unique_ptr<vr::IDriverLog> driverLog_;
    LogLevel severity_ = LogLevel::info;
};

#define OSVR_LOG(x) Logging::instance().log(x)

/**
 * Prints a message to the log for function entry and exit and the execution time.
 */
class FunctionGuard {
public:
    inline FunctionGuard(const std::string& function_name, const char* filename, int line) : functionName_(function_name)
    {
        OSVR_LOG(trace) << functionName_ << " called [" << filename << ":" << line << "].";
        //startTime_ = std::chrono::system_clock::now();
    }

    inline ~FunctionGuard()
    {
        const auto end_time = std::chrono::steady_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - startTime_);
        OSVR_LOG(trace) << functionName_ << " exiting. Execution time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << ".";
    }

private:
    std::string functionName_; ///< the name of the method/function that we're recording the scope of
    std::chrono::steady_clock::time_point startTime_ = std::chrono::steady_clock::now();
};

/** \name Concatenation macros. */
//@{
#define OSVR_CONCAT_3_(a, b) a##b
#define OSVR_CONCAT_2_(a, b) OSVR_CONCAT_3_(a, b)
#define OSVR_CONCAT(a, b)    OSVR_CONCAT_2_(a, b)
//@}

#define OSVR_FunctionGuard(function_name) OSVR_CONCAT(FunctionGuard function_guard,__LINE__)(function_name, __FILE__, __LINE__)

#endif // INCLUDED_Logging_h_GUID_E2F9C0D8_05AD_4D95_922B_3305E93990D3

