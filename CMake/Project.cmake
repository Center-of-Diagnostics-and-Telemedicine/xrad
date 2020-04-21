cmake_minimum_required (VERSION 3.8)
include_guard()

option(XRAD_Use_Common_BuildDir "Use XRAD common build directory" ON)

if(XRAD_Use_Common_BuildDir)
	set(XRAD_BuildDir "${XRAD_BuildDir}" CACHE PATH "XRAD library build directory." FORCE)
	if("${XRAD_BuildDir}" STREQUAL "")
		message(FATAL_ERROR "Please, specify XRAD library build directory (XRAD_BuildDir).")
	endif()
	set(XRAD_private_BuildDir "${XRAD_BuildDir}")
else()
	if(DEFINED XRAD_BuildDir)
		# if DEFINED CACHE{x} не срабатывает с пустыми строками
		# Прячем переменную
		set(XRAD_BuildDir "${XRAD_BuildDir}" CACHE INTERNAL "XRAD library build directory." FORCE)
	endif()
	set(XRAD_private_BuildDir "${CMAKE_CURRENT_BINARY_DIR}")
endif()

# Прячем ненужные переменные
function(xrad_reset_package package_name)
	set(XRAD_Require_${package_name} FALSE CACHE INTERNAL "XRAD_Require_${package_name}" FORCE)
endfunction()
xrad_reset_package(XRADBasic)
xrad_reset_package(XRADSystem)
xrad_reset_package(XRADConsoleUI)
xrad_reset_package(XRADGUI)
xrad_reset_package(XRADDicom)
xrad_reset_package(XRADDicomGUI)
set(XRAD_CollectDependencies TRUE)

set(XRAD_Project_RootDir "${CMAKE_CURRENT_LIST_DIR}/..")

# Функция для подключения проекта XRAD
function(xrad_add_xrad_project)
	add_subdirectory("${XRAD_Project_RootDir}" "XRAD")
endfunction()
