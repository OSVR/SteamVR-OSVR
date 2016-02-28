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
# Third-party libraries
#
add_subdirectory(vendor)


#
# Default installation directories
#

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


#
# OpenVR driver
#
add_subdirectory(src)

