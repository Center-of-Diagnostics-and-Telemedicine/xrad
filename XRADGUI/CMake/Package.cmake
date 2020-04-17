include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADSystem/CMake/Package.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/DetectQtConfiguration.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/DetectQwtConfiguration.cmake)

xrad_compose_library_name(XRADGUI_Library "XRADGUI")
set(XRADGUI_Libraries
	${XRADGUI_Library}
	${XRAD_Qwt_Libraries}
	Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Svg
	${XRADSystem_Libraries}
	)

xrad_compose_library_name(XRADGUITestsLib_Library "XRADGUI" "XRADGUITestsLib")
set(XRADGUITestsLib_Libraries
	${XRADGUITestsLib_Library}
	)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(XRADGUI_ExecutableOptions WIN32)
endif()
