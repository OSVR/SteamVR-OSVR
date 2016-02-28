/** @file
    @brief OSVR driver provider

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

#ifndef INCLUDED_driver_osvr_h_GUID_E3B2EF81_6AAE_446C_BD6E_E3599F69B540
#define INCLUDED_driver_osvr_h_GUID_E3B2EF81_6AAE_446C_BD6E_E3599F69B540

// Internal Includes
#include "osvr_dll_export.h"    // for OSVR_DLL_EXPORT

// Library/third-party includes
// - none

// Standard includes
// - none

OSVR_DLL_EXPORT void* TrackedDeviceDriverFactory(const char* interface_name, int* return_code);

OSVR_DLL_EXPORT void* HmdDriverFactory(const char* interface_name, int* return_code);

#endif // INCLUDED_driver_osvr_h_GUID_E3B2EF81_6AAE_446C_BD6E_E3599F69B540

