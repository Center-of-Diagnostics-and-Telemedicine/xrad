include_guard()
include(${CMAKE_CURRENT_LIST_DIR}/../../XRADBasic/CMake/PackageSetup.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/GetDependencyLibraries.cmake)

xrad_compose_library_name(XRADDicom_Library "XRADDicom")
set(XRADDicom_Libraries
	${XRADDicom_Library}
	${XRADDicom_Dep_Libraries}
	)
