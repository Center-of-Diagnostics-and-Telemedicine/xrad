include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADSystem/CMake/Package.cmake)

xrad_compose_library_name(XRADConsoleUI_Library "XRADConsoleUI")
set(XRADConsoleUI_Libraries
	${XRADConsoleUI_Library}
	${XRADSystem_Libraries}
	)
