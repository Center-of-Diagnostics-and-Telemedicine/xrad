/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_MSVC_QtCoreLink_h
#define XRAD__File_MSVC_QtCoreLink_h
//--------------------------------------------------------------

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+, Qt5
	#ifndef XRAD_COMPILER_CMAKE
	#ifdef XRAD_DEBUG
		#pragma comment(lib, "QT5Cored.lib")
	#else
		#pragma comment(lib, "QT5Core.lib")
	#endif //XRAD_DEBUG
	#endif // XRAD_COMPILER_CMAKE
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_MSVC_QtCoreLink_h
