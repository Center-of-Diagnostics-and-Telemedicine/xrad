/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_MSVC_QtLink_h
#define XRAD__File_MSVC_QtLink_h
//--------------------------------------------------------------

#include "MSVC_QtCoreLink.h"

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+, Qt5
	#ifndef XRAD_COMPILER_CMAKE
	#ifdef XRAD_DEBUG
		#pragma comment(lib, "QTMaind.lib")
		#pragma comment(lib, "QT5Widgetsd.lib")
		#pragma comment(lib, "QT5Guid.lib")
		#pragma comment(lib, "QT5Svgd.lib")
		#pragma comment(lib, "qwtd.lib")
	#else
		#pragma comment(lib, "QTMain.lib")
		#pragma comment(lib, "QT5Widgets.lib")
		#pragma comment(lib, "QT5Gui.lib")
		#pragma comment(lib, "QT5Svg.lib")
		#pragma comment(lib, "qwt.lib")
	#endif //XRAD_DEBUG
	#endif // XRAD_COMPILER_CMAKE
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_MSVC_QtLink_h
