/** @file
    @brief Header wrapping a CMake-generated export header to add extern "C"

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

#include <driver_oculus_export.h>

#define HMD_DLL_EXPORT extern "C" DRIVER_OCULUS_EXPORT
