# Attempts to locate the SteamVR paths for installing drivers and configuration files.
#
# The following variables are set:
#
#   STEAMVR_RUNTIME_DIR
#   STEAMVR_CONFIG_DIR
#   STEAMVR_LOG_DIR
#   STEAMVR_DRIVER_DIR
#   STEAMVR_PLATFORM
#
# Author:
#   Kevin M. Godby <kevin@godby.org>
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#


# The folder containing the SteamVR executables and libraries is
# platform-specific.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(STEAMVR_PLATFORM "osx32")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(STEAMVR_PLATFORM "win32")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(STEAMVR_PLATFORM "linux32")
else()
	message(WARNING "Unknown system type (not Windows, Linux, or OS X). Please file an issue at <https://github.com/OSVR/SteamVR-OSVR/issues> and report the operating system you're using.")
endif()

function(_read_openvrpaths_file)
	# We couldn't find the vrpathreg program. We'll need to parse the
	# openvrpaths.vrpath file manually.

	# Locate the openvrpaths.vrpath file.
	find_file(_openvrpaths_file
		NAME
		openvrpaths.vrpath
		PATHS
		$ENV{HOME}/.openvr
		$ENV{LOCALAPPDATA}/openvr
	)

	if(_openvrpaths_file)
		file(READ "${_openvrpaths_file}" _openvrpaths)

		# Turn string into CMake list (one line per item)
		string(REGEX REPLACE "\n" ";" _openvrpaths_list "${_openvrpaths}")

		# Parse list and set variables
		foreach(_line ${_openvrpaths_list})
			string(STRIP "${_line}" _line)
			string(FIND "${_line}" ":" _pos)
			if(_pos EQUAL -1)
				continue()
			endif()

			# Get the name
			string(SUBSTRING "${_line}" 0 ${_pos} _name)
			string(TOUPPER "${_name}" _name)
			string(REPLACE "\"" "" _name "${_name}")
			string(STRIP "${_name}" _name)

			# Get the path
			math(EXPR _pos "${_pos} + 1")
			string(SUBSTRING "${_line}" ${_pos} -1 _path)
			string(FIND "${_path}" "\"" _first_quote)
			string(FIND "${_path}" "\"" _last_quote REVERSE)
			math(EXPR _first_quote "${_first_quote} + 1")
			math(EXPR _len "${_last_quote} - ${_first_quote}")
			string(SUBSTRING "${_path}" ${_first_quote} ${_len} _path)

			set(STEAMVR_${_name}_DIR "${_path}" PARENT_SCOPE)
		endforeach()
	endif()
endfunction()

# Try to find the vrpathreg program. This will save us the hassle of parsing
# the config file ourselves.
set(ProgramFilesx86 "ProgramFiles(x86)")
find_program(VRPATHREG
	NAME
	vrpathreg
	PATHS
	"$ENV{${ProgramFilesx86}}/Steam/steamapps/common/SteamVR/bin/${STEAMVR_PLATFORM}/"
	"$ENV{XDG_DATA_HOME}/Steam/SteamApps/common/SteamVR/bin/${STEAMVR_PLATFORM}/"
	"$ENV{HOME}/.local/share/Steam/SteamApps/common/SteamVR/bin/${STEAMVR_PLATFORM}/"
	"$ENV{HOME}/Library/Application Support/Steam/steamapps/common/SteamVR/bin/${STEAMVR_PLATFORM}/"
)

if(VRPATHREG)
	# We found the vrpathreg program. Let's run it and parse the results.
	message(STATUS "Found vrpathreg: ${VRPATHREG}")
	execute_process(COMMAND "${VRPATHREG}" show
		RESULT_VARIABLE _vrpathreg_result
		OUTPUT_VARIABLE _vrpathreg_output
		ERROR_QUIET
	)

	if(_vrpathreg_result EQUAL 0)
		# Turn output into CMake list (one line per item)
		string(REGEX REPLACE "\n" ";" _vrpathreg_output_list "${_vrpathreg_output}")

		# Parse list and set variables
		foreach(_line ${_vrpathreg_output_list})
			string(REPLACE "path = " "" _line "${_line}")
			string(FIND "${_line}" " " _pos)
			if (_pos EQUAL -1)
				continue()
			endif()
			string(SUBSTRING "${_line}" 0 ${_pos} _name)
			string(TOUPPER "${_name}" _name)
			math(EXPR _pos "${_pos} + 1")
			string(SUBSTRING "${_line}" ${_pos} -1 _path)
			set(STEAMVR_${_name}_DIR "${_path}")
		endforeach()
	else()
		_read_openvrpaths_file()
	endif()
else()
	_read_openvrpaths_file()
endif()


if(STEAMVR_RUNTIME_DIR)
	file(TO_CMAKE_PATH "${STEAMVR_RUNTIME_DIR}/drivers" STEAMVR_DRIVER_DIR)
endif()

