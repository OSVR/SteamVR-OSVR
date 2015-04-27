# - try to find the SteamVR SDK - currently designed for the version on GitHub.
#
# Cache Variables: (probably not for direct use in your scripts)
#  STEAMVR_INCLUDE_DIR
#  STEAMVR_SOURCE_DIR
#  STEAMVR_VRTEST_API_LIBRARY
#
# Non-cache variables you might use in your CMakeLists.txt:
#  STEAMVR_FOUND
#  STEAMVR_INCLUDE_DIRS
#  STEAMVR_PLATFORM - something like Win32, Win64, etc.
#
# Requires these CMake modules:
#  FindPackageHandleStandardArgs (known included with CMake >=2.6.2)
#
# Original Author:
# 2015 Ryan A. Pavlik <ryan@sensics.com>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)

set(STEAMVR_ROOT_DIR
	"${STEAMVR_ROOT_DIR}"
	CACHE
	PATH
	"Directory to search for SteamVR SDK")

set(_root_dirs)
if(STEAMVR_ROOT_DIR)
	set(_root_dirs "${STEAMVR_ROOT_DIR}" "${STEAMVR_ROOT_DIR}/public")
endif()

# todo fails for universal builds
set(_dll_suffix)
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(_bitness 64)
	if(WIN32)
		set(_dll_suffix _x64)
	endif()
else()
	set(_bitness 32)
endif()

# Test platform

set(_platform)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	message(FATAL_ERROR "Don't have an example of a Mac OS X build to work from!")
else()
	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		set(_platform_base linux)
		add_definitions(-DGNUC -DPOSIX -DCOMPILER_GCC -D_LINUX -DLINUX -DPOSIX -D_POSIX)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(_platform_base win)
	endif()
	set(STEAMVR_PLATFORM ${_platform_base}${_bitness})
	set(_libpath lib/${STEAMVR_PLATFORM})
endif()

find_library(STEAMVR_VRTEST_API_LIBRARY
	NAMES
	vrtest_api
	PATHS
	${_root_dirs}
	PATH_SUFFIXES
	${_libpath}
	public/${_libpath})

if(STEAMVR_VRTEST_API_LIBRARY)
	get_filename_component(_libdir "${STEAMVR_VRTEST_API_LIBRARY}" PATH)
endif()

find_path(STEAMVR_INCLUDE_DIR
	NAMES
	steamvr.h
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	"${_libdir}/../.."
	PATHS
	${_root_dirs}
	PATH_SUFFIXES
	headers
	public/headers
	steam
	public/steam)

find_path(STEAMVR_SOURCE_DIR
	NAMES
	common/ihmdsystem.h
	HINTS
	"${_libdir}"
	"${_libdir}/.."
	"${_libdir}/../.."
	"${_libdir}/../../.."
	PATHS
	${_root_dirs}
	PATH_SUFFIXES
	src)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SteamVR
	DEFAULT_MSG
	STEAMVR_INCLUDE_DIR)

if(STEAMVR_FOUND)
	set(STEAMVR_INCLUDE_DIRS ${STEAMVR_INCLUDE_DIR})
	mark_as_advanced(STEAMVR_ROOT_DIR)
endif()

mark_as_advanced(STEAMVR_INCLUDE_DIR
	STEAMVR_SOURCE_DIR
	STEAMVR_VRTEST_API_LIBRARY)
