/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file PC_XRADBasicLink.h
//--------------------------------------------------------------
#ifndef XRAD__File_PC_XRADBasicLink_h
#define XRAD__File_PC_XRADBasicLink_h
//--------------------------------------------------------------

#include "MSVC_XRADLink-Suffix.h"

#ifndef _MSC_VER
	#error этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#ifndef XRAD_COMPILER_CMAKE
	#pragma comment(lib, "XRADBasic" XRAD_Library_Suffix ".lib")
	#endif // XRAD_COMPILER_CMAKE
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_PC_XRADBasicLink_h
