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
target_link_libraries(osvrDisplay PRIVATE osvr::osvrUtil)

if(APPLE)
	# find_library must be used for OS X frameworks
	find_library(COREFOUNDATION_LIBRARY CoreFoundation)
	find_library(COREGRAPHICS_LIBRARY CoreGraphics)
	find_library(IOKIT_LIBRARY IOKit)
	target_link_libraries(osvrDisplay
		PRIVATE
		${COREFOUNDATION_LIBRARY}
		${COREGRAPHICS_LIBRARY}
		${IOKIT_LIBRARY})
endif()

if(UNIX AND CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
	set_target_properties(osvrDisplay PROPERTIES COMPILE_FLAGS "-fPIC")
endif()

