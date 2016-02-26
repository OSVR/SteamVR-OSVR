/** @file
    @brief Header

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

//               Copyright Sensics, Inc. 2016.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)
#ifndef INCLUDED_ValveStrCpy_h_GUID_E8F022B2_8826_40B2_E94E_AA1F2A33ED74
#define INCLUDED_ValveStrCpy_h_GUID_E8F022B2_8826_40B2_E94E_AA1F2A33ED74

// Internal Includes
// - none

// Library/third-party includes
#include <util/FixedLengthStringFunctions.h>

// Standard includes
#include <string>

/// Tries to copy a string into the buffer of the size given, if it will fit
/// entirely. If it won't fit, don't do anything.
///
/// Designed especially to stick std::strings into the char buffers provided by
/// Valve APIs, hence the name. Similar to util::strcpy_safe in
/// util-headers/util/FixedLengthStringFunctions.h but of course, not nearly as
/// safe because it relies on the length of the buffer being honest, as opposed
/// to being correctly deduced from the type by the compiler.
///
/// @return the number of bytes copied (so 0 if we ended up not copying due to
/// insufficient space - an empty string is 1 for the null terminator)
inline std::size_t valveStrCpy(std::string const &src, char *dest,
                               std::uint32_t destSize) {
    auto sizeToCopy = src.size() + 1; // length plus null terminator.
    if (sizeToCopy > destSize) {
        // Too big for the buffer, don't copy anything.
        return 0;
    }
#if defined(UTILHEADERS_STR_STRLCPY)
    strlcpy(dest, src.c_str(), sizeToCopy);
#elif defined(UTILHEADERS_STR_MSRUNTIME)
    strcpy_s(dest, sizeToCopy, src.c_str());
#else
    // no need to manually null-terminate after unlike strcpy_safe - length
    // check verified that we'd copy the null terminator from the source string.
    strncpy(dest, src.c_str(), sizeToCopy);
#endif
    return static_cast<std::size_t>(sizeToCopy);
}

#endif // INCLUDED_ValveStrCpy_h_GUID_E8F022B2_8826_40B2_E94E_AA1F2A33ED74
