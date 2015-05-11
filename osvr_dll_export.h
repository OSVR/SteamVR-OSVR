/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com>

*/

// Copyright 2015 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0
#define INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0

// Internal Includes
// - none

// Library/third-party includes
// - none

// Standard includes
// - none

#if defined(_WIN32)
#define OSVR_DLL_EXPORT extern "C" __declspec(dllexport)
#define OSVR_DLL_IMPORT extern "C" __declspec(dllimport)
#elif defined(GNUC) || defined(COMPILER_GCC)
#define OSVR_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define OSVR_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

#endif // INCLUDED_osvr_dll_export_h_GUID_86142D80_1F9B_45D5_9C6C_04FA424822B0
