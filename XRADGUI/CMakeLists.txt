cmake_minimum_required (VERSION 3.8)

set(Project "XRADGUI")
set(ProjectTests "${Project}TestsLib")
project(${Project} LANGUAGES CXX)

include(../XRADBasic/CMake/CommonOptions.cmake)
include(../XRADBasic/CMake/StdLib.cmake)
include(CMake/DetectQtConfiguration.cmake)
include(CMake/DetectQwtConfiguration.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(AUTOUIC ON)
 
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/pre.h.in
	${XRAD_Project_Generated_pre_h}
	@ONLY)

set(Project_IncludeDirs ${Project_IncludeDirs}
	${XRAD_Project_GeneratedDir}
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}/QtGUIAPI/Common
	${CMAKE_CURRENT_SOURCE_DIR}/QtGUIAPI/DataDisplayWindow
	${CMAKE_CURRENT_SOURCE_DIR}/QtGUIAPI/DialogDefinitions
	${CMAKE_CURRENT_SOURCE_DIR}/QtGUIAPI/Forms
	${Qt5Widgets_INCLUDE_DIRS}
	${XRAD_Qwt_Include}
	${AUTOGEN_BUILD_DIR}/include
	)

set(Project_Sources_Qt
	QtGUIAPI/Common/AsyncTextBuffer.cpp
	QtGUIAPI/Common/AsyncTextBuffer.h
	QtGUIAPI/Common/FileSaveUtils.cpp
	QtGUIAPI/Common/FileSaveUtils.h
	QtGUIAPI/Common/FrameBitmapContainer.h
	QtGUIAPI/Common/FrameBitmapContainer.hh
	QtGUIAPI/Common/GUIController.cpp
	QtGUIAPI/Common/GUIController.h
	QtGUIAPI/Common/GUIQtUtils.h
	QtGUIAPI/Common/IGUISettingsChanged.h
	QtGUIAPI/Common/MultimodalFrameContainer.h
	QtGUIAPI/Common/MultimodalFrameContainer.hh
	QtGUIAPI/Common/OutputRedirector.cpp
	QtGUIAPI/Common/OutputRedirector.h
	QtGUIAPI/Common/OutputSubstitution.cpp
	QtGUIAPI/Common/OutputSubstitution.h
	QtGUIAPI/Common/SavedSettings.cpp
	QtGUIAPI/Common/SavedSettings.h
	QtGUIAPI/Common/StayOnTopDialog.cpp
	QtGUIAPI/Common/StayOnTopDialog.h
	QtGUIAPI/Common/stdio_console.cpp
	QtGUIAPI/Common/stdio_console.h
	QtGUIAPI/Common/ThreadGUI.cpp
	QtGUIAPI/Common/ThreadGUI.h
	QtGUIAPI/Common/ThreadSync.cpp
	QtGUIAPI/Common/ThreadSync.h
	QtGUIAPI/Common/ThreadSync.hh
	QtGUIAPI/Common/ThreadUser.cpp
	QtGUIAPI/Common/ThreadUser.h
	QtGUIAPI/Common/WorkflowControl.cpp
	QtGUIAPI/Common/WorkflowControl.h
	QtGUIAPI/Common/XRADGUIAPI.cpp
	QtGUIAPI/Common/XRADGUIAPI.h
	QtGUIAPI/Common/XRADGUIAPIDefs.h
	QtGUIAPI/Common/XRADGUIUtils.cpp
	QtGUIAPI/Common/XRADGUIUtils.h
	QtGUIAPI/DataDisplayWindow/AxisZoom.cpp
	QtGUIAPI/DataDisplayWindow/AxisZoom.h
	QtGUIAPI/DataDisplayWindow/ChartZoom.cpp
	QtGUIAPI/DataDisplayWindow/ChartZoom.h
	QtGUIAPI/DataDisplayWindow/DataDisplayWindow.cpp
	QtGUIAPI/DataDisplayWindow/DataDisplayWindow.h
	QtGUIAPI/DataDisplayWindow/DragMove.cpp
	QtGUIAPI/DataDisplayWindow/DragMove.h
	QtGUIAPI/DataDisplayWindow/DragZoom.cpp
	QtGUIAPI/DataDisplayWindow/DragZoom.h
	QtGUIAPI/DataDisplayWindow/GraphStyleSet.cpp
	QtGUIAPI/DataDisplayWindow/GraphStyleSet.h
	QtGUIAPI/DataDisplayWindow/GraphWindow.cpp
	QtGUIAPI/DataDisplayWindow/GraphWindow.h
	QtGUIAPI/DataDisplayWindow/GraphWindow.ui
	QtGUIAPI/DataDisplayWindow/ImageWindow.cpp
	QtGUIAPI/DataDisplayWindow/ImageWindow.h
	QtGUIAPI/DataDisplayWindow/ImageWindow.ui
	QtGUIAPI/DataDisplayWindow/MouseDrag.cpp
	QtGUIAPI/DataDisplayWindow/MouseDrag.h
	QtGUIAPI/DataDisplayWindow/TextWindow.cpp
	QtGUIAPI/DataDisplayWindow/TextWindow.h
	QtGUIAPI/DataDisplayWindow/TextWindow.ui
	QtGUIAPI/DataDisplayWindow/VisualizationPlot.cpp
	QtGUIAPI/DataDisplayWindow/VisualizationPlot.h
	QtGUIAPI/DataDisplayWindow/WheelZoom.cpp
	QtGUIAPI/DataDisplayWindow/WheelZoom.h
	QtGUIAPI/DialogDefinitions/DecisionDialogs.cpp
	QtGUIAPI/DialogDefinitions/DecisionDialogs.h
	QtGUIAPI/DialogDefinitions/DynamicDialogQt.cpp
	QtGUIAPI/DialogDefinitions/DynamicDialogQt.h
	QtGUIAPI/Forms/ConsoleWindow.cpp
	QtGUIAPI/Forms/ConsoleWindow.h
	QtGUIAPI/Forms/ConsoleWindow.ui
	QtGUIAPI/Forms/GetValueDialog.cpp
	QtGUIAPI/Forms/GetValueDialog.h
	QtGUIAPI/Forms/GetValueDialog.ui
	QtGUIAPI/Forms/PauseDialog.cpp
	QtGUIAPI/Forms/PauseDialog.h
	QtGUIAPI/Forms/PauseDialog.ui
	QtGUIAPI/Forms/ProgressBar.cpp
	QtGUIAPI/Forms/ProgressBar.h
	QtGUIAPI/Forms/ProgressBar.ui
	QtGUIAPI/Forms/ShowValueDialog.cpp
	QtGUIAPI/Forms/ShowValueDialog.h
	QtGUIAPI/Forms/ShowValueDialog.ui
	QtGUIAPI/Resources/XRADResources.qrc
	)

set(Project_Sources_CXX
	Sources/Core/GUIConfig.h
	Sources/Core/GUICore.h
	Sources/GUI/DataDisplayer.cpp
	Sources/GUI/DataDisplayer.h
	Sources/GUI/DisplaySampleType.h
	Sources/GUI/DynamicDialog.cpp
	Sources/GUI/DynamicDialog.h
	Sources/GUI/DynamicDialog.hh
	Sources/GUI/GraphScale.cpp
	Sources/GUI/GraphScale.h
	Sources/GUI/GraphSet.cpp
	Sources/GUI/GraphSet.h
	Sources/GUI/GUIValue.h
	Sources/GUI/I18nSupport.cpp
	Sources/GUI/I18nSupport.h
	Sources/GUI/Keyboard.cpp
	Sources/GUI/Keyboard.h
	Sources/GUI/MathFunctionGUI.cpp
	Sources/GUI/MathFunctionGUI.h
	Sources/GUI/MathFunctionGUI2D.h
	Sources/GUI/MathFunctionGUI2D.hh
	Sources/GUI/MathFunctionGUIMD.h
	Sources/GUI/MathFunctionGUIMD.hh
	Sources/GUI/MatrixVectorGUI.h
	Sources/GUI/ModifierKeys.h
	Sources/GUI/PixelNormalizers.h
	Sources/GUI/PixelNormalizersGUI.h
	Sources/GUI/RasterImageSet.cpp
	Sources/GUI/RasterImageSet.h
	Sources/GUI/SaveRasterImage.cpp
	Sources/GUI/SaveRasterImage.h
	Sources/GUI/SaveRasterImage.hh
	Sources/GUI/ShowComplex.h
	Sources/GUI/TextDisplayer.cpp
	Sources/GUI/TextDisplayer.h
	Sources/GUI/XRADGUI.cpp
	Sources/GUI/XRADGUI.h
	Sources/Internal/std.h
	Sources/PlatformSpecific/MSVC/Internal/Keyboard_Win32.cpp
	Sources/PlatformSpecific/MSVC/Internal/Keyboard_Win32.h
	Sources/PlatformSpecific/MSVC/MSVC_XRADGUILink.h
	Sources/PlatformSpecific/MSVC/MSVC_XRADGUITestsLibLink.h
	Sources/PlatformSpecific/MSVC/MSVC_XRADLink.h
	Sources/PlatformSpecific/Qt/Internal/Keyboard_Qt.cpp
	Sources/PlatformSpecific/Qt/Internal/Keyboard_Qt.h
	Sources/Startup/XRADGUIMain.cpp
	Sources/Utils/AutoProgressIndicatorScheduler.cpp
	Sources/Utils/AutoProgressIndicatorScheduler.h
	XRAD.h
	../XRADQt/QtStringConverters.cpp
	../XRADQt/QtStringConverters.h
	)

#set(ProjectTests_Sources_CXX
#	../XRADQt/Tests/QtStringConverters_Test.cpp
#	../XRADQt/Tests/QtStringConverters_Test.h
#	)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(Project_Sources_PlatformSpecific)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(Project_Sources_PlatformSpecific)
else()
	message(FATAL_ERROR "Unsupported CMAKE_CXX_COMPILER_ID: \"${CMAKE_CXX_COMPILER_ID}\".")
endif()

set(Project_Sources_All
	${XRAD_Project_Generated_pre_h}
	${Project_Sources_Qt}
	${Project_Sources_CXX}
	${Project_Sources_PlatformSpecific}
	)

set(ProjectTests_Sources_All
	${XRAD_Project_Generated_pre_h}
	${ProjectTests_Sources_CXX}
	)

add_library(${Project} STATIC ${Project_Sources_All})
target_include_directories(${Project} PRIVATE ${Project_IncludeDirs})

add_library(${ProjectTests} STATIC ${ProjectTests_Sources_All})
target_include_directories(${ProjectTests} PRIVATE ${Project_IncludeDirs})
