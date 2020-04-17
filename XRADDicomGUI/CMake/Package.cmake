include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADDicom/CMake/Package.cmake)

xrad_compose_library_name(XRADDicomGUI_Library "XRADDicomGUI")
set(XRADDicomGUI_Libraries
	${XRADDicomGUI_Library}
	${XRADDicom_Libraries}
	)
