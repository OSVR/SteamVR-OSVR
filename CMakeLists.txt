#
# OpenVR OSVR driver
#
cmake_minimum_required(VERSION 3.1.0)
project(openvr_osvr)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Dependencies
#
find_package(OpenVR REQUIRED)
find_package(osvr REQUIRED)
find_package(jsoncpp REQUIRED)
if(TARGET jsoncpp_lib_static AND NOT TARGET jsoncpp_lib)
	add_library(jsoncpp_lib ALIAS jsoncpp_lib_static)
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
set(CMAKE_SHARED_MODULE_PREFIX "")

#
# OpenVR driver
#
add_library(driver_osvr
	MODULE
	ClientDriver_OSVR.h
	driver_osvr.cpp
	identity.h
	make_unique.h
	matrix_cast.h
	osvr_device_properties.h
	osvr_display_configuration.h
	osvr_dll_export.h
	osvr_tracked_device.h
	projection_matrix.h
	ServerDriver_OSVR.h
	string_algo.h
)
target_link_libraries(driver_osvr PRIVATE osvr::osvrClientKitCpp eigen-headers jsoncpp_lib)
target_include_directories(driver_osvr PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET driver_osvr PROPERTY CXX_STANDARD 11)
target_compile_features(driver_osvr PRIVATE cxx_override)

include(GenerateExportHeader)
generate_export_header(driver_osvr)

install(TARGETS driver_osvr
	DESTINATION drivers/${OPENVR_PLATFORM})

#
# Test program
#
add_executable(test_hmd_driver test_hmd_driver.cpp)
target_link_libraries(test_hmd_driver PRIVATE osvr::osvrClientKitCpp eigen-headers jsoncpp_lib)
target_include_directories(test_hmd_driver PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET test_hmd_driver PROPERTY CXX_STANDARD 11)
target_compile_features(test_hmd_driver PRIVATE cxx_override)
