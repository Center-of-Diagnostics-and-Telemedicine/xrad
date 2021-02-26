cmake_minimum_required (VERSION 3.8)

set(Project "XRADSystem")
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
	Sources/CFile/shared_cfile.cpp
	Sources/IniFile/XRADIniFile.cpp
	Sources/System/FileNameOperations.cpp
	Sources/System/FileNamePatternMatch.cpp
	Sources/System/FileSystem.cpp
	Sources/System/xrad_fopen.cpp
	Sources/System/xrad_fstream.cpp
	Sources/TextFile/text_encoding.cpp
	Sources/TextFile/text_file.cpp
	Sources/Utils/BitmapFile.cpp
	)

set(Project_Sources_h
	CFile.h
	fstream.h
	Sources/CFile/shared_cfile.h
	Sources/IniFile/IniParser.hh
	Sources/IniFile/XRADIniFile.h
	Sources/System/FileNameOperations.h
	Sources/System/FileNamePatternMatch.h
	Sources/System/FileSystem.h
	Sources/System/FileSystemDefs.h
	Sources/System/SystemConfig.h
	Sources/System/xrad_fopen.h
	Sources/System/xrad_fstream.h
	Sources/TextFile/text_encoding.h
	Sources/TextFile/text_file.h
	Sources/Utils/BitmapFile.h
	System.h
	TextFile.h
	)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MSVC/XRADSystem_pre.cpp
	set(Project_Sources_PlatformSpecific_cpp
		Sources/PlatformSpecific/MSVC/Internal/FileNameOperations_Win32.cpp
		Sources/PlatformSpecific/MSVC/Internal/FileSystem_Win32.cpp
		)
	set(Project_Sources_PlatformSpecific_h
		Sources/PlatformSpecific/MSVC/Internal/FileNameOperations_Win32.h
		Sources/PlatformSpecific/MSVC/Internal/FileSystem_Win32.h
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(Project_Sources_PlatformSpecific_cpp)
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
