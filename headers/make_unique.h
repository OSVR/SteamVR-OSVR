/** @file
    @brief Header serving to backport std::make_unique to C++11 by
    conditionally including an implementation from libc++.

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
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE
#define INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE

// Internal Includes
#include "headers/osvr_compiler_tests.h"

#ifdef OSVR_HAS_STD_MAKE_UNIQUE

#include <memory>

#else // OSVR_HAS_STD_MAKE_UNIQUE

// If std::make_unique is not available, then we'll include an implementation of it.
#include <make_unique_impl.h>

#endif // OSVR_HAS_STD_MAKE_UNIQUE

#endif // INCLUDED_make_unique_h_GUID_C7526AAA_3549_41DF_AF95_5323788FBADE
