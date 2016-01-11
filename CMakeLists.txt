#
# OpenVR OSVR driver
#
cmake_minimum_required(VERSION 3.1.0)
project(openvr_osvr)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Be able to find openvr as a peer.
list(APPEND CMAKE_PREFIX_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/openvr"
	"${CMAKE_CURRENT_SOURCE_DIR}/../openvr")

#
# Dependencies
#
find_package(OpenVR REQUIRED)
find_package(osvr REQUIRED)
find_package(jsoncpp REQUIRED)
if(TARGET jsoncpp_lib_static AND NOT TARGET jsoncpp_lib)
	add_library(jsoncpp_lib INTERFACE)
	target_link_libraries(jsoncpp_lib INTERFACE jsoncpp_lib_static)
endif()

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

# For our generated file
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# Detect available C++ language, library, and compiler features
#
include(WriteCompilerDetectionHeader)
write_compiler_detection_header(
	FILE "osvr_compiler_detection.h"
	PREFIX OSVR
	COMPILERS GNU Clang AppleClang MSVC
	FEATURES cxx_override cxx_noexcept
)

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
check_cxx_source_compiles("#include <memory>\nint main(int, char**) { std::unique_ptr<int> i = std::make_unique<int>(0); }" OSVR_HAS_STD_MAKE_UNIQUE)
configure_file(osvr_compiler_tests.h.in "${CMAKE_CURRENT_BINARY_DIR}/osvr_compiler_tests.h")

#
# Third-party libraries
#
add_subdirectory(vendor)


# Disable 'lib' prefix on POSIX systems
set(CMAKE_SHARED_LIBRARY_PREFIX "")

#
# OpenVR driver
#
add_library(driver_osvr
	SHARED
	ClientDriver_OSVR.h
	driver_osvr.cpp
	identity.h
	make_unique.h
	matrix_cast.h
	osvr_device_properties.h
	osvr_dll_export.h
	osvr_tracked_device.h
	ServerDriver_OSVR.h
)
target_link_libraries(driver_osvr PRIVATE osvr::osvrClientKitCpp eigen-headers util-headers jsoncpp_lib)
target_include_directories(driver_osvr PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET driver_osvr PROPERTY CXX_STANDARD 11)
target_compile_features(driver_osvr PRIVATE cxx_override)
if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
	target_link_libraries(driver_osvr PRIVATE make-unique-impl-header)
endif()

# Set the default installation directories based on the current platform
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/Library/Application Support/Steam/steamapps/common/SteamVR/")
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
		set(CMAKE_INSTALL_PREFIX "$ENV{ProgramFiles(x86)}/Steam/steamapps/common/SteamVR/")
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
		set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.steam/steam/SteamApps/common/SteamVR/")
	else()
		message(WARNING "Unknown system type (not Windows, Linux, or OS X). Please file an issue at <https://github.com/OSVR/SteamVR-OSVR/issues> and report the operating system you're using.")
	endif()
endif()

install(TARGETS driver_osvr
	DESTINATION "drivers/osvr/bin/${OPENVR_PLATFORM}")

if(WIN32)
	include(CopyImportedTarget)
	foreach(target osvr::osvrClientKit osvr::osvrClient osvr::osvrCommon osvr::osvrUtil)
		copy_imported_targets(${target}) # for use in build tree
		install_imported_target(${target} DESTINATION "drivers/osvr/bin/${OPENVR_PLATFORM}") # in installed directory
	endforeach()
endif()

#
# Test program
#
add_executable(test_hmd_driver test_hmd_driver.cpp)
target_link_libraries(test_hmd_driver PRIVATE osvr::osvrClientKitCpp eigen-headers util-headers jsoncpp_lib)
if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
	target_link_libraries(test_hmd_driver PRIVATE make-unique-impl-header)
endif()
target_include_directories(test_hmd_driver PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET test_hmd_driver PROPERTY CXX_STANDARD 11)
target_compile_features(test_hmd_driver PRIVATE cxx_override)
