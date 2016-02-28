#
# OpenVR OSVR driver
#

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


# Disable 'lib' prefix on POSIX systems
set(CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(driver_osvr
	SHARED
	ClientDriver_OSVR.cpp
	ClientDriver_OSVR.h
	driver_osvr.cpp
	identity.h
	make_unique.h
	matrix_cast.h
	osvr_device_properties.h
	osvr_dll_export.h
	OSVRTrackedDevice.cpp
	OSVRTrackedDevice.h
	platform_fixes.h
	ServerDriver_OSVR.cpp
	ServerDriver_OSVR.h
	ValveStrCpy.h
)
target_link_libraries(driver_osvr PRIVATE osvr::osvrClientKitCpp eigen-headers util-headers jsoncpp_lib)
target_include_directories(driver_osvr PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET driver_osvr PROPERTY CXX_STANDARD 11)
target_compile_features(driver_osvr PRIVATE cxx_override)
if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
	target_link_libraries(driver_osvr PRIVATE make-unique-impl-header)
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
target_link_libraries(test_hmd_driver PRIVATE driver_osvr osvr::osvrClientKitCpp eigen-headers util-headers jsoncpp_lib)
if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
	target_link_libraries(test_hmd_driver PRIVATE make-unique-impl-header)
endif()
target_include_directories(test_hmd_driver PRIVATE ${OPENVR_INCLUDE_DIRS})
set_property(TARGET test_hmd_driver PROPERTY CXX_STANDARD 11)
target_compile_features(test_hmd_driver PRIVATE cxx_override)

