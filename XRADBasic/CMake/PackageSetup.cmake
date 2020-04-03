include_guard()

if("${XRAD_Root}" STREQUAL "")
	message(FATAL_ERROR "The XRAD_Root variable is not provided.")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(XRAD_IncludeDir "${XRAD_Root}")
	set(XRAD_BuildDir "${XRAD_Root}")

	set(XRAD_MSVCRelativeBuildDir "out/build/${XRAD_Platform}-${XRAD_Configuration}/")

	# Записать в переменную var_name имя файла библиотеки library_name с путем
	function(xrad_compose_library_name var_name library_name)
		if(XRAD_Configuration STREQUAL "Debug")
			set(LibSuffix "D")
		else()
			set(LibSuffix "")
		endif()
		set(${var_name} "${XRAD_BuildDir}/${library_name}/${XRAD_MSVCRelativeBuildDir}${library_name}${LibSuffix}.lib" PARENT_SCOPE)
	endfunction()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(XRAD_IncludeDir "${XRAD_Root}")
	set(XRAD_BuildDir "" CACHE PATH "XRAD library build directory.")
	if("${XRAD_BuildDir}" STREQUAL "")
		message(FATAL_ERROR "Please, specify XRAD library build directory (XRAD_BuildDir).")
	endif()

	# Записать в переменную var_name имя файла библиотеки library_name с путем
	function(xrad_compose_library_name var_name library_name)
		set(${var_name} "${XRAD_BuildDir}/${library_name}/lib${library_name}.a" PARENT_SCOPE)
	endfunction()
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()
