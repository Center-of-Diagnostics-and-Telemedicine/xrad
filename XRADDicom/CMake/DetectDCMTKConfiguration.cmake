include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADBasic/CMake/DetectThirdParty.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if(XRAD_Platform STREQUAL "")
		message(FATAL_ERROR "The XRAD_Platform variable is not initialized.")
	endif()
	set(XRAD_DCMTKPlatformTarget ${XRAD_Platform})
	set(XRAD_DCMTKConfiguration ${CMAKE_BUILD_TYPE})

	set(XRAD_DCMTK_InstallPath ${XRAD_ThirdParty}/dcmtk/install/${XRAD_DCMTKPlatformTarget}/${XRAD_DCMTKConfiguration})
	if(NOT IS_DIRECTORY "${XRAD_DCMTK_InstallPath}/include/dcmtk")
		message(FATAL_ERROR "DCMTK library is not found at ${XRAD_DCMTK_InstallPath}.")
	endif()

	set(XRAD_DCMTK_ExternalsPath ${XRAD_ThirdParty}/dcmtk/externals/${XRAD_DCMTKPlatformTarget})
	if(NOT IS_DIRECTORY "${XRAD_DCMTK_ExternalsPath}/zlib-1.2.11/include")
		message(FATAL_ERROR "DCMTK library extenals is not found at ${XRAD_DCMTK_ExternalsPath}.")
	endif()

	set(XRAD_OpenJPEG_InstallPath ${XRAD_ThirdParty}/openjpeg/install/${XRAD_DCMTKPlatformTarget}/${XRAD_DCMTKConfiguration})
	if(NOT IS_DIRECTORY "${XRAD_OpenJPEG_InstallPath}/include/openjpeg-2.2")
		message(FATAL_ERROR "OpenJPEG library is not found at ${XRAD_OpenJPEG_InstallPath}.")
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(NOT IS_DIRECTORY "${XRAD_ThirdParty}/include/dcmtk")
		message(FATAL_ERROR "DCMTK library is not found at ${XRAD_ThirdParty}.")
	endif()

	if(NOT IS_DIRECTORY "${XRAD_ThirdParty}/include/openjpeg-2.2")
		message(FATAL_ERROR "OpenJPEG library is not found at ${XRAD_ThirdParty}.")
	endif()

	if(NOT EXISTS "${XRAD_ThirdParty}/include/zlib.h")
		message(FATAL_ERROR "ZLib library is not found at ${XRAD_ThirdParty}.")
	endif()

endif()
