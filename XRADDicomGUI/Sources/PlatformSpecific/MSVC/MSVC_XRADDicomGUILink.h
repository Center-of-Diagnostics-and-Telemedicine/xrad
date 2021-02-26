/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_MSVC_XRADDicomGUILink_h
#define XRAD__File_MSVC_XRADDicomGUILink_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 //MSVC2015+
	#ifndef XRAD_COMPILER_CMAKE
	#pragma comment(lib, "XRADDicomGUI" XRAD_Library_Suffix ".lib")
	#endif // XRAD_COMPILER_CMAKE
#else
	#error Unsupported Visual Studio version.
#endif

//--------------------------------------------------------------
#endif // XRAD__File_MSVC_XRADDicomGUILink_h
