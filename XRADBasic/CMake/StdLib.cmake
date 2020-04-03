include_guard()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	# Имя "*D.lib": используется текущее соглашение об именах конфигурации Debug в MSVC
	set(CMAKE_DEBUG_POSTFIX "D")
endif()
