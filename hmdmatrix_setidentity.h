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
#include <steamvr.h>

// Standard includes
#include <cstdint>

inline void HmdMatrix_SetIdentity(vr::HmdMatrix44_t *pMatrix) {
  for (uint8_t i = 0; i < 4; ++i) {
    for (uint8_t j = 0; j < 4; ++j) {
      pMatrix->m[i][j] = (i == j) ? 1.f : 0.f;
    }
  }
}
