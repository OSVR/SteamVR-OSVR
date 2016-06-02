#
# Display detection
#

set(OSVR_DISPLAY_SOURCES
	Display.h
	DisplayEnumerator.cpp
	DisplayEnumerator.h
	DisplayEnumerator_Linux.h
	DisplayEnumerator_MacOSX.h
	DisplayEnumerator_Windows.h
)

add_library(osvrDisplay STATIC ${OSVR_DISPLAY_SOURCES})
target_link_libraries(osvrDisplay osvr::osvrUtil)

if(UNIX AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
	set_target_properties(osvrDisplay PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

