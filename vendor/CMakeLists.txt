# Interface target for Eigen 3.2.4
add_library(eigen-headers INTERFACE)
target_include_directories(eigen-headers SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.2.4")
target_compile_definitions(eigen-headers INTERFACE EIGEN_MPL2_ONLY)

# Interface target for util-headers
add_library(util-headers INTERFACE)
target_include_directories(util-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/util-headers")

if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
    # Interface target for make_unique implementation from libc++
    add_library(make-unique-impl-header INTERFACE)
    target_include_directories(make-unique-impl-header INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libcxx/include")
endif()

# Google test
if(BUILD_TESTS)
	if(WIN32)
		set(gtest_force_shared_crt ON)
	endif()
	set(BUILD_GTEST ON)
	add_subdirectory(googletest)
endif()

# OSVR-Display
set(_build_tests ${BUILD_TESTS})
set(BUILD_TESTS OFF)
set(BUILD_APPS OFF)
add_subdirectory(OSVR-Display)
unset(BUILD_APPS)
set(BUILD_TESTS ${_build_tests})
unset(_build_tests)

