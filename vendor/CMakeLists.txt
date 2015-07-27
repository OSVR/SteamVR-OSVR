# Interface target for Eigen 3.2.4
add_library(eigen-headers INTERFACE)
target_include_directories(eigen-headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.2.4")
target_compile_definitions(eigen-headers INTERFACE EIGEN_MPL2_ONLY)

if(NOT OSVR_HAS_STD_MAKE_UNIQUE)
    # Interface target for make_unique implementation from libc++
    add_library(make-unique-impl-header INTERFACE)
    target_include_directories(make-unique-impl-header INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/libcxx/include")
endif()
