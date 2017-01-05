#
# Unit tests and test programs
#

add_executable(test_OSVRDisplay test_OSVRDisplay.cpp)
target_include_directories(test_OSVRDisplay
    SYSTEM
    PRIVATE
    ${OPENVR_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/vendor/OSVR-Display
    ${CMAKE_BINARY_DIR}/vendor/OSVR-Display)
target_include_directories(test_OSVRDisplay
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src)
target_link_libraries(test_OSVRDisplay
    PRIVATE
    make-unique-impl-header
    osvrDisplay_static
    osvrRenderManager::osvrRenderManager)
add_test(NAME test_test_OSVRDisplay COMMAND test_OSVRDisplay)

