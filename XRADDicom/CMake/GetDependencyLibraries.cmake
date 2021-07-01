include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADBasic/CMake/DetectThirdParty.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/DetectDCMTKConfiguration.cmake)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(XRAD_DCMTK_LibrariesDir "${XRAD_DCMTK_InstallPath}/lib")
	set(XRAD_DCMTK_Libraries
		"${XRAD_DCMTK_LibrariesDir}/dcmdata.lib"
		"${XRAD_DCMTK_LibrariesDir}/ofstd.lib"
		"${XRAD_DCMTK_LibrariesDir}/oflog.lib"
		"${XRAD_DCMTK_LibrariesDir}/dcmimgle.lib"
		"${XRAD_DCMTK_LibrariesDir}/dcmjpls.lib"
		"${XRAD_DCMTK_LibrariesDir}/dcmjpeg.lib"
		"${XRAD_DCMTK_LibrariesDir}/dcmnet.lib"
		)

	set(XRAD_OpenJPEG_LibrariesDir "${XRAD_OpenJPEG_InstallPath}/lib")
	set(XRAD_OpenJPEG_Libraries
		"${XRAD_OpenJPEG_LibrariesDir}/openjp2.lib"
		)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(XRAD_DCMTK_LibrariesDir "${XRAD_ThirdParty}/lib")
	set(XRAD_DCMTK_Libraries
		"${XRAD_DCMTK_LibrariesDir}/libdcmnet.a"
		"${XRAD_DCMTK_LibrariesDir}/libdcmimgle.a"
        "${XRAD_DCMTK_LibrariesDir}/libdcmimage.a"
		"${XRAD_DCMTK_LibrariesDir}/libdcmdata.a"
		"${XRAD_DCMTK_LibrariesDir}/libdcmjpeg.a"
		"${XRAD_DCMTK_LibrariesDir}/libdcmjpls.a"
		"${XRAD_DCMTK_LibrariesDir}/libcharls.a"
		"${XRAD_DCMTK_LibrariesDir}/liboflog.a"
		"${XRAD_DCMTK_LibrariesDir}/libofstd.a"
		"${XRAD_DCMTK_LibrariesDir}/libijg16.a"
		"${XRAD_DCMTK_LibrariesDir}/libijg12.a"
		"${XRAD_DCMTK_LibrariesDir}/libijg8.a"
		)

	set(XRAD_OpenJPEG_LibrariesDir "${XRAD_ThirdParty}/lib")
	set(XRAD_OpenJPEG_Libraries
		"${XRAD_OpenJPEG_LibrariesDir}/libopenjp2.a"
		)

	set(XRADZLib_LibrariesDir "${XRAD_ThirdParty}/lib")
	set(XRADZLib_Libraries
		"${XRADZLib_LibrariesDir}/libz.a"
		)
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(XRADDicom_Dep_Libraries
	${XRAD_DCMTK_Libraries}
	${XRAD_OpenJPEG_Libraries}
	${XRADZLib_Libraries}
	)
