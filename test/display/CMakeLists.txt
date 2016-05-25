#
# Display detection tests
#

# For our generated file
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DISPLAY_LIBRARIES "")
if(WIN32)
elseif(LINUX)
elseif(MACOSX)
endif()

add_executable(osvr_print_displays osvr_print_displays.cpp)
target_link_libraries(osvr_print_displays PRIVATE ${DISPLAY_LIBRARIES})
target_include_directories(osvr_print_displays SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/src")
set_property(TARGET osvr_print_displays PROPERTY CXX_STANDARD 11)
target_compile_features(osvr_print_displays PRIVATE cxx_override)



