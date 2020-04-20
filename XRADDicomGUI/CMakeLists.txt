cmake_minimum_required (VERSION 3.8)

set(Project "XRADDicomGUI")
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
	Sources/dialogs.cpp
	Sources/DisplayFunctions/DisplayAcquisitionXRay.cpp
	Sources/DisplayFunctions/DisplayProcessAcquisition.cpp
	Sources/DisplayFunctions/DisplayTomogram_CTAcquisition.cpp
	Sources/DisplayFunctions/DisplayTomogram_GenericAcquisition.cpp
	Sources/DisplayFunctions/DisplayTomogram_MRAcquisition.cpp
	Sources/DisplayFunctions/DisplayTomogram_MRAcquisitionSiemens.cpp
	Sources/SelectSeriesInteractive.cpp
	)

set(Project_Sources_h
	Sources/dialogs.h
	Sources/DisplayFunctions/DisplayAcquisitionXRay.h
	Sources/DisplayFunctions/DisplayProcessAcquisition.h
	Sources/DisplayFunctions/DisplayTomogram_CTAcquisition.h
	Sources/DisplayFunctions/DisplayTomogram_GenericAcquisition.h
	Sources/DisplayFunctions/DisplayTomogram_MRAcquisition.h
	Sources/DisplayFunctions/DisplayTomogram_MRAcquisitionSiemens.h
	Sources/SelectSeriesInteractive.h
	XRADDicomGUI.h
	)

set(Project_Sources_All
	${XRAD_Project_Generated_pre_h}
	${Project_Sources_cpp}
	${Project_Sources_h}
	)

add_library(${Project} STATIC ${Project_Sources_All})
target_include_directories(${Project} PRIVATE ${Project_IncludeDirs})
