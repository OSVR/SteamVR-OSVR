#
# Unit tests and test programs
#

include(CMakeParseArguments REQUIRED)

function(add_osvr_test testname)
    set(options "") # no options
    set(singleValueArgs "") # no single-value arguments
    set(multiValueArgs LIBS SOURCES INCLUDE_DIRS SYSTEM_INCLUDE_DIRS) # lists of additional libraries and source files
    cmake_parse_arguments(add_osvr_test "${options}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${testname} ${testname}.cpp ${add_osvr_test_SOURCES})
    target_link_libraries(${testname} PRIVATE ${add_osvr_test_LIBS})
    target_include_directories(${testname} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    target_include_directories(${testname} PRIVATE ${add_osvr_test_INCLUDE_DIRS})
    target_include_directories(${testname} SYSTEM PRIVATE ${add_osvr_test_SYSTEM_INCLUDE_DIRS})
    set_property(TARGET ${testname} PROPERTY CXX_STANDARD 11)
    target_compile_features(${testname} PRIVATE cxx_override)
    install(TARGETS ${testname}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
    add_test(NAME test_${testname} COMMAND ${testname})
endfunction()

add_osvr_test(test_OSVRDisplay
    SOURCES
    "${CMAKE_SOURCE_DIR}/src/OSVRDisplay.cpp"
    SYSTEM_INCLUDE_DIRS
    "${OPENVR_INCLUDE_DIRS}"
    "${CMAKE_SOURCE_DIR}/vendor/OSVR-Display"
    "${CMAKE_BINARY_DIR}/vendor/OSVR-Display"
    "${CMAKE_SOURCE_DIR}/vendor/OSVR-System-Information"
    LIBS
    make-unique-impl-header
    osvrDisplay_static
    osvrSysInfo_static
    osvrRenderManager::osvrRenderManager
)

add_osvr_test(test_HeadModel
    LIBS
    osvr::osvrClientKitCpp
)

add_osvr_test(test_PoseUtil
    SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/PoseUtil.cpp"
    LIBS
    osvr::osvrClientKitCpp
    osvr::osvrUtil
    eigen-headers
)

#add_executable(test_OSVRDisplay test_OSVRDisplay.cpp ${CMAKE_SOURCE_DIR}/src/OSVRDisplay.cpp)
#target_include_directories(test_OSVRDisplay
#    SYSTEM
#    PRIVATE
#    ${OPENVR_INCLUDE_DIRS}
#    ${CMAKE_SOURCE_DIR}/vendor/OSVR-Display
#    ${CMAKE_BINARY_DIR}/vendor/OSVR-Display)
#target_include_directories(test_OSVRDisplay
#    PRIVATE
#    ${CMAKE_CURRENT_SOURCE_DIR}/../src
#    ${CMAKE_CURRENT_BINARY_DIR}/../src)
#target_link_libraries(test_OSVRDisplay
#    PRIVATE
#    make-unique-impl-header
#    osvrDisplay_static
#    osvrRenderManager::osvrRenderManager)
#add_test(NAME test_test_OSVRDisplay COMMAND test_OSVRDisplay)

