/** @file
	@brief Header providing C++-enhanced versions of the various string
   functions that work on fixed-length buffers, inspired by
   https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/

	@date 2015

	This header is maintained as a part of 'util-headers' - you can always
    find the latest version online at https://github.com/rpavlik/util-headers

    This GUID can help identify the project: d1dbc94e-e863-49cf-bc08-ab4d9f486613

   This copy of the header is from the revision that Git calls
    17f9d700834b905fd22d70ad47655cc070c2dad5

    Commit date: "2015-07-28 14:47:47 -0500"

	@author
	Sensics, Inc.
	<http://sensics.com/osvr>
*/

//               Copyright Sensics, Inc. 2015.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDED_FixedLengthStringFunctions_h_GUID_34010E53_D3F3_42BD_FB36_6D00EA79C3A9
#define INCLUDED_FixedLengthStringFunctions_h_GUID_34010E53_D3F3_42BD_FB36_6D00EA79C3A9

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <stddef.h>
#include <string.h>
#include <stdexcept>

#ifdef _MSC_VER
#define UTILHEADERS_STR_MSRUNTIME
#else
#include <limits.h>
#ifndef _WIN32
#include <sys/types.h>
#endif
#if defined(__KLIBC__) || defined(__BIONIC__) ||                               \
	(!defined(__GLIBC__) && defined(_BSD_SOURCE))
// strlcpy providers:
// klibc
// bionic
// musl if _GNU_SOURCE or _BSD_SOURCE defined (_BSD_SOURCE defined by default) -
// but not glibc!
#define UTILHEADERS_STR_STRLCPY
#elif defined(__DARWIN_C_LEVEL) && defined(__DARWIN_C_FULL) &&                 \
	(__DARWIN_C_LEVEL >= __DARWIN_C_FULL)
// Also provided in cases on Darwin
#define UTILHEADERS_STR_STRLCPY
#endif
#endif

namespace util {
/// @brief Copy a string into a char array, guaranteed to null-terminate and not
/// overrun. Does not correctly handle UTF-8 (may truncate in the middle of a
/// codepoint).
template <size_t N> inline void strcpy_safe(char(&dest)[N], const char *src) {
#if defined(UTILHEADERS_STR_STRLCPY)
    strlcpy(dest, src, N);
#elif defined(UTILHEADERS_STR_MSRUNTIME)
    strncpy_s(dest, src, _TRUNCATE);
#else
	strncpy(dest, src, N);
	dest[N - 1] = '\0';
#endif
}
}

#endif // INCLUDED_FixedLengthStringFunctions_h_GUID_34010E53_D3F3_42BD_FB36_6D00EA79C3A9
