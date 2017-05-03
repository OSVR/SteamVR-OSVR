/** @file
    @brief Pretty-prints OpenVR objects.

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

#ifndef INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591
#define INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591

// Internal Includes
// - none

// Library/third-party includes
#include <openvr_driver.h>

// Standard includes
#include <ios>
#include <ostream>
#include <sstream>
#include <string>

using std::to_string;

/**
 * Simple loopback to make to_string more generic.
 */
std::string to_string(const std::string& str);

std::string to_string(void* value);

std::string to_string(const vr::ETrackedDeviceProperty& value);

std::ostream& operator<<(std::ostream& out, const vr::ETrackedDeviceProperty value);

/// Helper class to wrap a value that should be output as hex to a
/// stream.
template <typename T>
class AsHex {
public:
    explicit AsHex(T val, bool leading0x = false) : val_(val), leading0x_(leading0x)
    {
        // do nothing
    }

    T get() const
    {
        return val_;
    }

    bool leading0x() const
    {
        return leading0x_;
    }


private:
    T val_;
    bool leading0x_;
};

/// Output streaming operator for AsHex, found by ADL.
template <typename T>
std::ostream& operator<<(std::ostream& os, const AsHex<T>& val)
{
    if (val.leading0x()) {
        os << "0x";
    }
    os << std::hex << val.get() << std::dec;
    return os;
}

/// Function template to wrap a value to indicate it should be output as a
/// hex value (no leading 0x)
template <typename T>
inline AsHex<T> as_hex(T val) {
    return AsHex<T>(val);
}

/// Function template to wrap a value to indicate it should be output as a
/// hex value (with leading 0x)
template <typename T>
inline AsHex<T> as_hex_0x(T val) {
    return AsHex<T>(val, true);
}

std::ostream& operator<<(std::ostream& ostr, const vr::HmdQuaternion_t& quat);

std::ostream& operator<<(std::ostream& ostr, const vr::ETrackingResult& result);

std::ostream& operator<<(std::ostream& ostr, const double vec[3]);

/**
 * @brief Return a string representation of vr::DriverPose_t struct.
 */
std::ostream& operator<<(std::ostream& ostr, const vr::DriverPose_t& pose);


/**
 * @brief Catch-all to_string conversion function. Must be defined after all the
 * operator<< overloads to make use of them.
 */
template <typename T>
inline std::string to_string(const T& val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

#endif // INCLUDED_pretty_print_h_GUID_5CF0EE2E_1739_4CA8_BA5A_F72B8BEB3591

