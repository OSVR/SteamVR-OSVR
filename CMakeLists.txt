#
# SteamVR OSVR driver
#
cmake_minimum_required(VERSION 3.1.0)
project(steamvr_osvr)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
# Dependencies
#
find_package(SteamVR REQUIRED)
find_package(osvr REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS thread system REQUIRED)

# For our generated file
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# Detect available C++ language, library, and compiler features
#
include(WriteCompilerDetectionHeader)
write_compiler_detection_header(
	FILE "${CMAKE_CURRENT_BINARY_DIR}/osvr_compiler_detection.h"
	PREFIX OSVR
	COMPILERS GNU Clang AppleClang MSVC
	FEATURES cxx_override
)

include(CheckCXXSymbolExists)
check_cxx_symbol_exists(std::make_unique memory OSVR_HAS_MAKE_UNIQUE)
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
	driver_osvr.cpp
	export.h
	hmdmatrix_setidentity.h
	stringhasprefix.h
	cxx_feature_override.h
	)
target_link_libraries(driver_osvr osvr::osvrClientKit eigen-headers)
target_include_directories(driver_osvr PRIVATE osvr::osvrClientKit ${STEAMVR_INCLUDE_DIRS})
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


