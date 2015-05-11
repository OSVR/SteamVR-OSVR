#
# SteamVR OSVR driver
#
cmake_minimum_required(VERSION 3.1.0)
project(steamvr_osvr)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Dependencies
#
find_package(SteamVR REQUIRED)
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
	FEATURES cxx_override
)

include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
check_cxx_source_compiles("#include <memory>\nint main(int, char**) { std::unique_ptr<int> i = std::make_unique<int>(0); }" HAVE_STD_MAKE_UNIQUE)
configure_file(osvr_compiler_tests.h.in "${CMAKE_CURRENT_BINARY_DIR}/osvr_compiler_tests.h")

#
# Third-party libraries
#
add_subdirectory(vendor)

#
# SteamVR driver
#
add_library(driver_osvr
	MODULE
	ClientMainloop.h
	ClientMainloopThread.h
	make_unique.h
	matrix_cast.h
	osvr_display_configuration.h
	osvr_dll_export.h
	osvr_hmd.h
	projection_matrix.h
	driver_osvr.cpp
)
target_link_libraries(driver_osvr osvr::osvrClientKit eigen-headers jsoncpp_lib)
target_include_directories(driver_osvr PRIVATE osvr::osvrClientKit ${STEAMVR_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set_property(TARGET driver_osvr PROPERTY CXX_STANDARD 11)
target_compile_features(driver_osvr PRIVATE cxx_override)

include(GenerateExportHeader)
generate_export_header(driver_osvr)

install(TARGETS driver_osvr
	DESTINATION drivers/${STEAMVR_PLATFORM})

#
# Test program
#
add_executable(test_hmd_driver
	test_hmd_driver.cpp
	)
target_link_libraries(test_hmd_driver osvr::osvrClientKit jsoncpp_lib ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} eigen-headers)
target_include_directories(test_hmd_driver PRIVATE osvr::osvrClientKit ${STEAMVR_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
set_property(TARGET test_hmd_driver PROPERTY CXX_STANDARD 11)
target_compile_features(test_hmd_driver PRIVATE cxx_override)
