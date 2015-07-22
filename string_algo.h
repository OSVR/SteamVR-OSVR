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

#ifndef INCLUDED_string_algo_h_GUID_A677EA52_82B0_4F30_B9D5_5E8F05050918
#define INCLUDED_string_algo_h_GUID_A677EA52_82B0_4F30_B9D5_5E8F05050918

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <cctype> // for std::tolower
#include <string> // for std::string

/**
 * Tests whether a string begins with a prefix (case-sensitive).
 *
 * @param str String to be tested.
 * @param prefix Prefix.
 *
 * @return true if @p str begins with @p prefix, false otherwise.
 */
bool starts_with(const std::string& str, const std::string& prefix);

/**
 * Tests whether a string begins with a prefix (case-insensitive).
 *
 * @param str String to be tested.
 * @param prefix Prefix.
 *
 * @return true if @p str begins with @p prefix, false otherwise.
 */
bool istarts_with(std::string str, std::string prefix);

/**
 * Converts a string to UPPERCASE.
 */
std::string to_upper(std::string str);

/**
 * Converts a string to lowercase.
 */
std::string to_lower(std::string str);

//
// Implementations
//

inline bool starts_with(const std::string& str, const std::string& prefix)
{
    return (0 == str.compare(0, prefix.size(), prefix));
}

inline bool istarts_with(std::string str, std::string prefix)
{
    // Convert to lowercase, then compare
    str = to_lower(str);
    prefix = to_lower(prefix);

    return starts_with(str, prefix);
}

inline std::string to_upper(std::string str)
{
    for (auto& ch : str)
        std::toupper(ch);

    return str;
}

inline std::string to_lower(std::string str)
{
    for (auto& ch : str)
        std::tolower(ch);

    return str;
}

#endif // INCLUDED_string_algo_h_GUID_A677EA52_82B0_4F30_B9D5_5E8F05050918
