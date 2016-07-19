/** @file
    @brief Functions for parsing path trees

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

#ifndef INCLUDED_PathTreeUtil_h_GUID_38675790_6105_4FA0_84D8_843132FC1E9A
#define INCLUDED_PathTreeUtil_h_GUID_38675790_6105_4FA0_84D8_843132FC1E9A

// Internal Includes
// - none

// Library/third-party includes
#include <osvr/Common/RoutingConstants.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

// Standard includes
#include <string>
#include <vector>
#include <stdexcept>

// Forward declarations

/**
 * Given a path string, it returns an absolute, canonical path. For example, multiple
 * successive path separators are replaced with a single separator, . and .. are
 * resolved.
 *
 * A base path must be provided if the path is relative.
 *
 * @throws @c std::invalid_argument if the path is not absolute or we try to
 * traverse above the root.
 */
//@{
std::string getCanonicalPath(const std::string& path);
std::string getCanonicalPath(const std::string& path, const std::string& base_path);
//@}

/**
 * Appends a path to a base path and returns the result.
 */
std::string appendPath(std::string prefix, std::string suffix);

/**
 * Resolves a path given a base path and a relative or absolute path. If an
 * absolute path is provided, it will be returned. Otherwise, the relative path
 * is resolved in relation to the base path.
 */
std::string resolvePath(const std::string& path, const std::string& base_path);

// Implementations

inline std::string getCanonicalPath(const std::string& path)
{
    using boost::algorithm::split;
    using boost::is_any_of;
    using boost::token_compress_on;

    // Ensure we have an absolute path
    if (path.front() != osvr::common::getPathSeparatorCharacter()) {
        throw std::invalid_argument("Invalid path: An absolute path is required.");
    }

    // Split the path string into components and push those components onto a
    // stack.
    std::vector<std::string> components;
    split(components, path, is_any_of(osvr::common::getPathSeparator()), token_compress_on);

    std::vector<std::string> canonical_components;

    for (auto component : components) {
        if (component.empty()) {
            // This has the effect of collapsing multiple adjacent path
            // separators into a single path separator. We shouldn't see this
            // since we used token_compress_on when we split the path into
            // components.
            continue;
        } else if (component == ".") {
            // Remove this from the path, too, as it has no impact.
            continue;
        } else if (component == "..") {
            // If we've run out of parents, throw an exception
            if (canonical_components.empty()) {
                throw std::invalid_argument("Invalid path: Tried to traverse above root.");
            }

            canonical_components.pop_back();
        } else {
            canonical_components.push_back(component);
        }
    }

    std::string canonical_path;
    for (const auto& component : canonical_components) {
        canonical_path += osvr::common::getPathSeparator() + component;
    }

    // Preserve trailing slash on input path
    if (path.back() == osvr::common::getPathSeparatorCharacter()) {
        canonical_path += osvr::common::getPathSeparatorCharacter();
    }

    return canonical_path;
}

inline std::string getCanonicalPath(const std::string& path, const std::string& base_path)
{
    return getCanonicalPath(appendPath(base_path, path));
}

/**
 * Appends a path to a base path and returns the result.
 */
inline std::string appendPath(std::string prefix, std::string suffix)
{
    if (prefix.empty())
        return suffix;

    if (suffix.empty())
        return prefix;

    // Remove any trailing path sep from prefix
    if (prefix.back() == osvr::common::getPathSeparatorCharacter()) {
        prefix.pop_back();
    }

    // Remove any leading path sep from suffix
    if (suffix.front() == osvr::common::getPathSeparatorCharacter()) {
        suffix = suffix.substr(1);
    }

    return prefix + osvr::common::getPathSeparator() + suffix;
}

std::string resolvePath(const std::string& path, const std::string& base_path)
{
    if (base_path.empty())
        throw std::invalid_argument("A non-empty base path must be provided.");

    if (base_path.front() != osvr::common::getPathSeparatorCharacter())
        throw std::invalid_argument("The base path must be an absolute path.");

    if (path.empty())
        return base_path;

    if (path.front() == osvr::common::getPathSeparatorCharacter())
        return path;

    return getCanonicalPath(base_path + osvr::common::getPathSeparator() + path);
}

#endif // INCLUDED_PathTreeUtil_h_GUID_38675790_6105_4FA0_84D8_843132FC1E9A

