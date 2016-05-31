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

