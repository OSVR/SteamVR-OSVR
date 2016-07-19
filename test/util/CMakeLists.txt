#
# Tests on utility functions
#

add_executable(test_PathTreeUtils test_PathTreeUtils.cpp)

target_link_libraries(test_PathTreeUtils
	jsoncpp_lib
	gtest
	osvr::osvrCommon)

target_include_directories(test_PathTreeUtils
	SYSTEM PRIVATE
	"${gtest_SOURCE_DIR}/include"
	"${CMAKE_SOURCE_DIR}/src"
	"${OPENVR_INCLUDE_DIR}")

add_test(NAME gtest-test_PathTreeUtils
	COMMAND test_PathTreeUtils)

set_target_properties(test_PathTreeUtils
	PROPERTIES
	FOLDER "SteamVR-OSVR Tests")


