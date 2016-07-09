#
# A simple VR Server application to load and test the OSVR driver
#

# For our generated file
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#
# Test program
#
add_executable(test_hmd_driver
	test_hmd_driver.cpp
	ServerDriverHost.h
	ServerDriverHost.cpp
	VRSettings.h
	VRSettings.cpp
	)

target_link_libraries(test_hmd_driver
	PRIVATE
	driver_osvr
	osvr::osvrClientKitCpp
	eigen-headers
	util-headers
	jsoncpp_lib
	osvrRenderManager::osvrRenderManager)

target_include_directories(test_hmd_driver
	PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR}/src)
target_include_directories(test_hmd_driver
	SYSTEM PRIVATE
	${OPENVR_INCLUDE_DIRS})
if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
	target_link_libraries(test_hmd_driver PRIVATE make-unique-impl-header)
endif()
set_property(TARGET test_hmd_driver PROPERTY CXX_STANDARD 11)
target_compile_features(test_hmd_driver PRIVATE cxx_override)

