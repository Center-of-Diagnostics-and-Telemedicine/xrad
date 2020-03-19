cmake_minimum_required (VERSION 3.8)

project ("XRADSystem")

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
		${CMAKE_CURRENT_BINARY_DIR}/pre.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(XRADSystem_Sources_cpp
	Sources/CFile/shared_cfile.cpp
	Sources/IniFile/XRADIniFile.cpp
	Sources/System/FileNameOperations.cpp
	Sources/System/FileNamePatternMatch.cpp
	Sources/System/FileSystem.cpp
	Sources/TextFile/text_encoding.cpp
	Sources/TextFile/text_file.cpp
	Sources/Utils/BitmapFile.cpp
)

set(XRADSystem_Sources_h
	CFile.h
	Sources/CFile/shared_cfile.h
	Sources/IniFile/IniParser.hh
	Sources/IniFile/XRADIniFile.h
	Sources/System/FileNameOperations.h
	Sources/System/FileNamePatternMatch.h
	Sources/System/FileSystem.h
	Sources/System/FileSystemDefs.h
	Sources/System/SystemConfig.h
	Sources/TextFile/text_encoding.h
	Sources/TextFile/text_file.h
	Sources/Utils/BitmapFile.h
	System.h
	TextFile.h
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MSVC/XRADSystem_pre.cpp
	set(XRADSystem_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/MSVC/Internal/FileNameOperations_Win32.cpp
		Sources/PlatformSpecific/MSVC/Internal/FileSystem_Win32.cpp
	)
	set(XRADSystem_Sources_PlatformSpecific_h
		Sources/PlatformSpecific/MSVC/Internal/FileNameOperations_Win32.h
		Sources/PlatformSpecific/MSVC/Internal/FileSystem_Win32.h
		Sources/PlatformSpecific/MSVC/MSVC_XRADSystemLink.h
	)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	# Имя "XRADSystemD.lib": используется текущее соглашение об именах конфигурации Debug в MSVC
	set(CMAKE_DEBUG_POSTFIX "D")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(XRADSystem_Sources_PlatformSpecific_cpp)
	set(XRADSystem_Sources_PlatformSpecific_h)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fopenmp")
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(XRADSystem_Sources_All
	${XRADSystem_Sources_cpp}
	${XRADSystem_Sources_h}
	${XRADSystem_Sources_PlatformSpecific_cpp}
	${XRADSystem_Sources_PlatformSpecific_h}
)

add_library(XRADSystem STATIC ${XRADSystem_Sources_All})
