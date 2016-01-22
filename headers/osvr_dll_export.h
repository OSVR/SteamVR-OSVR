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
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0
#define INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0

#if defined(_WIN32)
#define OSVR_DLL_EXPORT extern "C" __declspec(dllexport)
#define OSVR_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(GNUC) || defined(COMPILER_GCC) || defined(__APPLE__)
#define OSVR_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define OSVR_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

#endif // INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0
