include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADBasic/CMake/Package.cmake)

xrad_setup_package("XRADSystem")
xrad_compose_library_name(XRADSystem_Library "XRADSystem")
set(XRADSystem_Libraries
	${XRADSystem_Library}
	${XRADBasic_Libraries}
	)
xrad_set_dependencies("XRADSystem" "XRADBasic")
