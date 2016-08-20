#
# OpenVR OSVR driver
#
cmake_minimum_required(VERSION 3.1.0)
project(openvr_osvr)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseBackportedModules)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Be able to find openvr as a peer.
list(APPEND CMAKE_PREFIX_PATH
	"${CMAKE_CURRENT_SOURCE_DIR}/openvr"
	"${CMAKE_CURRENT_SOURCE_DIR}/../openvr")

include(EnableExtraCompilerWarnings)
globally_enable_extra_compiler_warnings()

#
# Options
#
option(BUILD_TESTS "Build test programs and unit tests." OFF)

#
# Dependencies
#
find_package(OpenVR REQUIRED)
find_package(osvr REQUIRED)
find_package(osvrRenderManager REQUIRED)
find_package(JsonCpp REQUIRED)
if(TARGET jsoncpp_lib_static AND NOT TARGET jsoncpp_lib)
	add_library(jsoncpp_lib INTERFACE)
	target_link_libraries(jsoncpp_lib INTERFACE jsoncpp_lib_static)
endif()

find_package(Threads REQUIRED)
find_package(Boost REQUIRED)

# For our generated file
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# Third-party libraries
#
add_subdirectory(vendor)


#
# Default installation directories
#

include(GNUInstallDirs)
include(SteamVRPaths)

# Configuration file destination
#install(FILES
#	osvr.json
#	DESTINATION
#	${STEAMVR_CONFIG_DIR}/osvr
#)


#
# OpenVR driver
#
add_subdirectory(src)

#
# Tests
#
if(BUILD_TESTS)
	add_subdirectory(test)
endif()
