cmake_minimum_required (VERSION 3.8)
include_guard()

option(Use_XRAD_BuildDir "Use XRAD common build directory" ON)

if(Use_XRAD_BuildDir)
	set(XRAD_BuildDir "" CACHE PATH "XRAD library build directory.")
	if("${XRAD_BuildDir}" STREQUAL "")
		message(FATAL_ERROR "Please, specify XRAD library build directory (XRAD_BuildDir).")
	endif()
	set(XRAD_private_BuildDir "${XRAD_BuildDir}")
else()
	set(XRAD_private_BuildDir "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/.." "XRAD")
