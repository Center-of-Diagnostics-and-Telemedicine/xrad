include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADDicom/CMake/Package.cmake)

xrad_setup_package("XRADDicomGUI")
xrad_compose_library_name(XRADDicomGUI_Library "XRADDicomGUI")
set(XRADDicomGUI_Libraries
	${XRADDicomGUI_Library}
	${XRADDicom_Libraries}
	)
xrad_set_dependencies("XRADDicomGUI" "XRADDicom")
