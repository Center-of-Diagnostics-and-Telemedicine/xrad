include_guard()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	if(XRAD_Platform STREQUAL "")
		message(FATAL_ERROR "The XRAD_Platform variable is not initialized.")
	endif()

	if("${XRAD_Platform}" STREQUAL "x64")
		set(XRAD_QwtDir $ENV{QWTDIR5x64} CACHE PATH "Qwt root directory.")
	elseif("${XRAD_Platform}" STREQUAL "x86")
		set(XRAD_QwtDir $ENV{QWTDIR5x86} CACHE PATH "Qwt root directory.")
	endif()
	if("${XRAD_QwtDir}" STREQUAL "")
		message(FATAL_ERROR "Please, specify Qwt root directory (XRAD_QwtDir).")
	endif()

	if(NOT EXISTS "${XRAD_QwtDir}/include/qwt.h")
		message(FATAL_ERROR "Qwt library is not found at ${XRAD_QwtDir} (include/qwt.h).")
	endif()

	if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		set(XRAD_Qwt_LibSuffix "d")
	else()
		set(XRAD_Qwt_LibSuffix "")
	endif()

	if(NOT EXISTS "${XRAD_QwtDir}/lib/qwt${XRAD_Qwt_LibSuffix}.lib")
		message(FATAL_ERROR "Qwt library is not found at ${XRAD_QwtDir} (lib/qwt${XRAD_Qwt_LibSuffix}.lib).")
	endif()

	set(XRAD_Qwt_Include "${XRAD_QwtDir}/include")
	set(XRAD_Qwt_Libraries "${XRAD_QwtDir}/lib/qwt${XRAD_Qwt_LibSuffix}.lib")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	include(${CMAKE_CURRENT_LIST_DIR}/../../XRADBasic/CMake/DetectThirdParty.cmake)

	if(NOT EXISTS "${XRAD_ThirdParty}/qwt-6.1.2/include/qwt.h")
		message(FATAL_ERROR "Qwt library is not found at ${XRAD_ThirdParty} (qwt-6.1.2/include/qwt.h).")
	endif()

	if(NOT EXISTS "${XRAD_ThirdParty}/qwt-6.1.2/lib/libqwt.so.6.1.2")
		message(FATAL_ERROR "Qwt library is not found at ${XRAD_ThirdParty} (qwt-6.1.2/lib/libqwt.so.6.1.2).")
	endif()

	set(XRAD_Qwt_Include "${XRAD_ThirdParty}/qwt-6.1.2/include")
	set(XRAD_Qwt_Libraries "${XRAD_ThirdParty}/qwt-6.1.2/lib/libqwt.so.6.1.2")

endif()
