cmake_minimum_required (VERSION 3.8)

project ("XRADDicom")

configure_file(	${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
		${CMAKE_CURRENT_BINARY_DIR}/pre.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(XRADDicom_Sources_cpp
	Sources/DCMTKAccess/dcmtkCodec.cpp
	Sources/DCMTKAccess/dcmtkElementsTools.cpp
	Sources/DCMTKAccess/dcmtkUtils.cpp
	Sources/DCMTKAccess/pacsTools.cpp
	Sources/DCMTKAccess/pacsUtils.cpp
	Sources/DicomClasses/DataContainers/Container.cpp
	Sources/DicomClasses/DataContainers/ContainerDCMTK.cpp
	Sources/DicomClasses/DataContainers/datasource.cpp
	Sources/DicomClasses/DicomFilenameFilter.cpp
	Sources/DicomClasses/DicomPredicates.cpp
	Sources/DicomClasses/DicomStorageAnalyze.cpp
	Sources/DicomClasses/Hierarchy.cpp
	Sources/DicomClasses/Indexing/DicomCatalogIndex.cpp
	Sources/DicomClasses/Indexing/DicomClustering.cpp
	Sources/DicomClasses/Indexing/DicomDirectoryIndex.cpp
	Sources/DicomClasses/Indexing/DicomDirectoryIndexJson.cpp
	Sources/DicomClasses/Indexing/DicomFileIndex.cpp
	Sources/DicomClasses/Indexing/DicomFileIndexJson.cpp
	Sources/DicomClasses/Instances/CreateInstance.cpp
	Sources/DicomClasses/Instances/ct_slice.cpp
	Sources/DicomClasses/Instances/image.cpp
	Sources/DicomClasses/Instances/instance.cpp
	Sources/DicomClasses/Instances/instance_cache.cpp
	Sources/DicomClasses/Instances/LoadGenericClasses.cpp
	Sources/DicomClasses/Instances/Modality.cpp
	Sources/DicomClasses/Instances/mr_slice.cpp
	Sources/DicomClasses/Instances/mr_slice_siemens.cpp
	Sources/DicomClasses/Instances/source.cpp
	Sources/DicomClasses/Instances/tomogram_slice.cpp
	Sources/DicomClasses/Instances/xray_image.cpp
	Sources/DicomClasses/ProcessContainers/CreateProcessAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/CTAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/MRAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/MRAcquisitionSiemens.cpp
	Sources/DicomClasses/ProcessContainers/ProcessAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/TomogramAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/XRAYAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/XRAYInstance.cpp
	Sources/DicomClasses/XRADDicomGeneral.cpp
	Sources/Utils/file_info.cpp
	Sources/Utils/logger.cpp
	Sources/Utils/Utils.cpp
	Sources/Utils/XRADDicomTools.cpp
)

set(XRADDicom_Sources_h
	Sources/DCMTKAccess/dcmtkCodec.h
	Sources/DCMTKAccess/dcmtkElementsTools.h
	Sources/DCMTKAccess/dcmtkUtils.h
	Sources/DCMTKAccess/pacsTools.h
	Sources/DCMTKAccess/pacsUtils.h
	Sources/DicomClasses/DataContainers/Container.h
	Sources/DicomClasses/DataContainers/ContainerDCMTK.h
	Sources/DicomClasses/DataContainers/datasource.h
	Sources/DicomClasses/dataelement.h
	Sources/DicomClasses/DicomFilenameFilter.h
	Sources/DicomClasses/DicomFilterCore.h
	Sources/DicomClasses/DicomFilters.h
	Sources/DicomClasses/DicomParameterCounter.h
	Sources/DicomClasses/DicomPredicates.h
	Sources/DicomClasses/DicomProcessors.h
	Sources/DicomClasses/DicomStorageAnalyze.h
	Sources/DicomClasses/Hierarchy.h
	Sources/DicomClasses/Indexing/DicomCatalogIndex.h
	Sources/DicomClasses/Indexing/DicomClustering.h
	Sources/DicomClasses/Indexing/DicomDirectoryIndex.h
	Sources/DicomClasses/Indexing/DicomDirectoryIndexJson.h
	Sources/DicomClasses/Indexing/DicomFileIndex.h
	Sources/DicomClasses/Indexing/DicomFileIndexJson.h
	Sources/DicomClasses/Instances/CreateInstance.h
	Sources/DicomClasses/Instances/ct_slice.h
	Sources/DicomClasses/Instances/Image.h
	Sources/DicomClasses/Instances/instance.h
	Sources/DicomClasses/Instances/instance_cache.h
	Sources/DicomClasses/Instances/LoadGenericClasses.h
	Sources/DicomClasses/Instances/Modality.h
	Sources/DicomClasses/Instances/mr_slice.h
	Sources/DicomClasses/Instances/mr_slice_siemens.h
	Sources/DicomClasses/Instances/source.h
	Sources/DicomClasses/Instances/tomogram_slice.h
	Sources/DicomClasses/Instances/xray_image.h
	Sources/DicomClasses/ProcessContainers/CreateProcessAcquisition.h
	Sources/DicomClasses/ProcessContainers/CTAcquisition.h
	Sources/DicomClasses/ProcessContainers/MRAcquisition.h
	Sources/DicomClasses/ProcessContainers/MRAcquisitionSiemens.h
	Sources/DicomClasses/ProcessContainers/ProcessAcquisition.h
	Sources/DicomClasses/ProcessContainers/TomogramAcquisition.h
	Sources/DicomClasses/ProcessContainers/XRAYAcquisition.h
	Sources/DicomClasses/ProcessContainers/XRAYInstance.h
	Sources/DicomClasses/tags_enum.h
	Sources/DicomClasses/XRADDicomGeneral.h
	Sources/Utils/file_info.h
	Sources/Utils/logger.h
	Sources/Utils/Utils.h
	Sources/Utils/XRADDicomTools.h
	XRADDicom.h
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# MSVC/XRADDicom_pre.cpp
	set(XRADDicom_Sources_PlatformSpecific_cpp
	)
	set(XRADDicom_Sources_PlatformSpecific_h
		Sources/PlatformSpecific/MSVC/MSVC_DCMTKLink.h
		Sources/PlatformSpecific/MSVC/MSVC_XRADDicomLink.h
	)

	set(XRADThirdParty $ENV{XRADThirdParty} CACHE PATH "XRAD third party root directory.")
	if("${XRADThirdParty}" STREQUAL "")
		message(FATAL_ERROR "The XRAD third party root directory must be set.")
	endif()

	set(PlatformTarget x64 CACHE STRING "DCMTK platform (x86|x64)")
	set(XRAD_DCMTKConfiguration Debug CACHE STRING "DCMTK configuration (Debug|Release)")

    set(XRAD_DCMTKInstallPath ${XRADThirdParty}/dcmtk/install/${PlatformTarget}/${XRAD_DCMTKConfiguration})
    set(XRAD_DCMTKExternalsPath ${XRADThirdParty}/dcmtk/externals/${PlatformTarget})
    set(XRAD_OpenJPEGInstallPath ${XRADThirdParty}/openjpeg/install/${PlatformTarget}/${XRAD_DCMTKConfiguration})

	include_directories(${XRAD_DCMTKInstallPath}/include)
	include_directories(${XRAD_DCMTKExternalsPath}/zlib-1.2.11/include)
	include_directories(${XRAD_OpenJPEGInstallPath}/include/openjpeg-2.2)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	# Имя "XRADDicomD.lib": используется текущее соглашение об именах конфигурации Debug в MSVC
	set(CMAKE_DEBUG_POSTFIX "D")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(XRADDicom_Sources_PlatformSpecific_cpp)
	set(XRADDicom_Sources_PlatformSpecific_h)

	set(XRADThirdPartyInstall $ENV{XRADThirdPartyInstall} CACHE PATH "XRAD third party install directory.")
	if("${XRADThirdPartyInstall}" STREQUAL "")
		message(FATAL_ERROR "The XRAD third party install directory must be set.")
	endif()

	include_directories(${XRADThirdPartyInstall}/include)
	include_directories(${XRADThirdPartyInstall}/include/openjpeg-2.2)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fopenmp -fPIC")
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(XRADDicom_Sources_All
	${XRADDicom_Sources_cpp}
	${XRADDicom_Sources_h}
	${XRADDicom_Sources_PlatformSpecific_cpp}
	${XRADDicom_Sources_PlatformSpecific_h}
)

add_library(XRADDicom STATIC ${XRADDicom_Sources_All})
