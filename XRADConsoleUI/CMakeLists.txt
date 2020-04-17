cmake_minimum_required (VERSION 3.8)

set(Project "XRADConsoleUI")
project(${Project} LANGUAGES CXX)

include(../XRADBasic/CMake/CommonOptions.cmake)
include(../XRADBasic/CMake/StdLib.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
	${XRAD_Project_Generated_pre_h}
	@ONLY)

set(Project_IncludeDirs ${Project_IncludeDirs}
	${XRAD_Project_GeneratedDir}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	)

set(Project_Sources_cpp
	)

set(Project_Sources_h
	)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(Project_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/MSVC/Internal/ConsoleMain_MS.cpp
		)
	set(Project_Sources_PlatformSpecific_h
		Sources/PlatformSpecific/MSVC/MSVC_XRADConsoleUILink.h
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(Project_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/GCC/ConsoleMain_GCC.cpp
		)
	set(Project_Sources_PlatformSpecific_h)
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(Project_Sources_All
	${XRAD_Project_Generated_pre_h}
	${Project_Sources_cpp}
	${Project_Sources_h}
	${Project_Sources_PlatformSpecific_cpp}
	${Project_Sources_PlatformSpecific_h}
	)

add_library(${Project} STATIC ${Project_Sources_All})
target_include_directories(${Project} PRIVATE ${Project_IncludeDirs})
