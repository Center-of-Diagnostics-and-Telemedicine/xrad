cmake_minimum_required (VERSION 3.8)

project ("XRADConsoleUI")

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
		${CMAKE_CURRENT_BINARY_DIR}/pre.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(XRADConsoleUI_Sources_cpp
)

set(XRADConsoleUI_Sources_h
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MSVC/XRADConsoleUI_pre.cpp
	set(XRADConsoleUI_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/MSVC/Internal/ConsoleMain_MS.cpp
	)
	set(XRADConsoleUI_Sources_PlatformSpecific_h
		Sources/PlatformSpecific/MSVC/MSVC_XRADConsoleUILink.h
	)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	# Имя "XRADConsoleUID.lib": используется текущее соглашение об именах конфигурации Debug в MSVC
	set(CMAKE_DEBUG_POSTFIX "D")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(XRADConsoleUI_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/GCC/ConsoleMain_GCC.cpp
	)

	set(XRADConsoleUI_Sources_PlatformSpecific_h)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fopenmp -fPIC")
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(XRADConsoleUI_Sources_All
	${XRADConsoleUI_Sources_cpp}
	${XRADConsoleUI_Sources_h}
	${XRADConsoleUI_Sources_PlatformSpecific_cpp}
	${XRADConsoleUI_Sources_PlatformSpecific_h}
)

add_library(XRADConsoleUI STATIC ${XRADConsoleUI_Sources_All})
