cmake_minimum_required (VERSION 3.8)

set(Project "XRADDicom")
project(${Project} LANGUAGES CXX)

include(../XRADBasic/CMake/CommonOptions.cmake)
include(../XRADBasic/CMake/StdLib.cmake)
include(../XRADBasic/CMake/DetectThirdParty.cmake)
include(CMake/DetectDCMTKConfiguration.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
	${XRAD_Project_Generated_pre_h}
	@ONLY)

set(Project_IncludeDirs ${Project_IncludeDirs}
	${XRAD_Project_GeneratedDir}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	)

set(Project_Sources_cpp
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
	Sources/DicomClasses/Indexing/DicomFileIndex.cpp
	Sources/DicomClasses/Indexing/DicomFileIndexJson.cpp
	Sources/DicomClasses/Indexing/SingleDirectoryIndex.cpp
	Sources/DicomClasses/Indexing/SingleDirectoryIndexJson.cpp
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
	Sources/DicomClasses/ProcessContainers/GenericImageAcquisition.cpp	
	Sources/DicomClasses/ProcessContainers/CTAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/MRAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/MRAcquisitionSiemens.cpp
	Sources/DicomClasses/ProcessContainers/ProcessAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/TomogramAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/XRayAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/XRayInstance.cpp
	Sources/DicomClasses/XRADDicomGeneral.cpp
	Sources/Utils/file_info.cpp
	Sources/Utils/logger.cpp
	Sources/Utils/Utils.cpp
	Sources/Utils/XRADDicomTools.cpp
	)

set(Project_Sources_h
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
	Sources/DicomClasses/Indexing/DicomFileIndex.h
	Sources/DicomClasses/Indexing/DicomFileIndexJson.h
	Sources/DicomClasses/Indexing/SingleDirectoryIndex.h
	Sources/DicomClasses/Indexing/SingleDirectoryIndexJson.h
	Sources/DicomClasses/Instances/CreateInstance.h
	Sources/DicomClasses/Instances/ct_slice.h
	Sources/DicomClasses/Instances/image.h
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
	Sources/DicomClasses/ProcessContainers/GenericImageAcquisition.cpp
	Sources/DicomClasses/ProcessContainers/CTAcquisition.h
	Sources/DicomClasses/ProcessContainers/MRAcquisition.h
	Sources/DicomClasses/ProcessContainers/MRAcquisitionSiemens.h
	Sources/DicomClasses/ProcessContainers/ProcessAcquisition.h
	Sources/DicomClasses/ProcessContainers/TomogramAcquisition.h
	Sources/DicomClasses/ProcessContainers/XRayAcquisition.h
	Sources/DicomClasses/ProcessContainers/XRayInstance.h
	Sources/DicomClasses/tags_enum.h
	Sources/DicomClasses/XRADDicomGeneral.h
	Sources/Utils/file_info.h
	Sources/Utils/logger.h
	Sources/Utils/Utils.h
	Sources/Utils/XRADDicomTools.h
	XRADDicom.h
	)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(Project_Sources_PlatformSpecific_cpp)
	set(Project_Sources_PlatformSpecific_h)

	set(Project_IncludeDirs ${Project_IncludeDirs}
		${XRAD_DCMTK_InstallPath}/include
		${XRAD_DCMTK_ExternalsPath}/zlib-1.2.11/include
		${XRAD_OpenJPEG_InstallPath}/include/openjpeg-2.2
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(Project_Sources_PlatformSpecific_cpp)
	set(Project_Sources_PlatformSpecific_h)

	set(Project_IncludeDirs ${Project_IncludeDirs}
		${XRAD_ThirdParty}/include
		${XRAD_ThirdParty}/include/openjpeg-2.2
		)
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
