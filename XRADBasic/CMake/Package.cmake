include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/PackageSetup.cmake)

xrad_setup_package("XRADBasic")
xrad_compose_library_name(XRADBasic_Library "XRADBasic")
set(XRADBasic_Libraries
	${XRADBasic_Library}
	${XRAD_Vld_Libraries}
	)
