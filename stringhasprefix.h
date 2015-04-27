/** @file
    @brief Header replacing functionality of internal header in steamvr.

    @date 2015

    @author
    Ryan Pavlik
    <ryan@sensics.com>

*/

//           Copyright Sensics, Inc. 2015.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
#include <string>

// This is not an ideal implementation, but it's a rarely called function
// and certainly no reason to require internal headers. I don't particularly
// feel like using C-style string manipulation methods right now in such
// non-critical code.
inline bool StringHasPrefix(const char *str, const char *prefix) {
  std::string s(str);
  std::string pre(prefix);
  if (pre.length() > s.length()) {
    return false;
  }
  return s.substr(0, pre.length()) == pre;
}
