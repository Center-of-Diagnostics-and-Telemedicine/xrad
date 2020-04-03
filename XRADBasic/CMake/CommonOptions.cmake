include_guard()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(FindOpenMP)
if(!OPENMP_FOUND)
	message(FATAL_ERROR "No OpenMP support.")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# При сборке из MSVC 2019 переменные CMAKE_VS_PLATFORM_NAME, CMAKE_GENERATOR_PLATFORM
	# не заданы.
	# Определяем платформу по флагам линкера.
	string(TOLOWER ${CMAKE_EXE_LINKER_FLAGS} XRAD_LFLAGS_LC)
	if(XRAD_LFLAGS_LC MATCHES "/machine:x64")
		set(XRAD_Platform "x64")
	elseif(XRAD_LFLAGS_LC MATCHES "/machine:x86")
		set(XRAD_Platform "x86")
	else()
		message(FATAL_ERROR "Unknown platform in CMAKE_EXE_LINKER_FLAGS: \"${CMAKE_EXE_LINKER_FLAGS}\".")
	endif()
	set(XRAD_Configuration ${CMAKE_BUILD_TYPE})
endif()
