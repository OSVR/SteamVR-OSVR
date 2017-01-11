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
find_package(Threads REQUIRED)

if(WIN32)
	# RenderManager requires the d3dcompiler_47.dll file. This ships with
	# Windows 8 and above by default, but we'll need to ship it for Windows 7
	# users. To simplify our lives, we'll just ship it for everyone.
	find_package(WindowsSDK REQUIRED COMPONENTS tools)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_DIR x64)
	else()
		set(ARCH_DIR x86)
	endif()
	find_file(DIRECT3D_COMPILER_REDISTRIBUTABLE d3dcompiler_47.dll
		PATH_SUFFIXES Redist/D3D/${ARCH_DIR}
		PATHS ${WINDOWSSDK_DIRS}
		NO_DEFAULT_PATH)
endif()


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
install(DIRECTORY
	resources
	DESTINATION
	"${CMAKE_INSTALL_FULL_LIBDIR}/openvr/osvr/"
)

file(TO_CMAKE_PATH
	"${CMAKE_INSTALL_FULL_LIBDIR}/openvr/osvr/bin/${STEAMVR_PLATFORM}"
	DRIVER_INSTALL_DIR)

if(WIN32)
	# Install d3dcompiler_47.dll
	install(FILES ${DIRECT3D_COMPILER_REDISTRIBUTABLE}
		DESTINATION ${DRIVER_INSTALL_DIR})
endif()


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
