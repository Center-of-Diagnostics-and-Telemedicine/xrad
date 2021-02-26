/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file PC_ConsoleLibLink.h
//--------------------------------------------------------------
#ifndef XRAD__File_PC_ConsoleLibLink_h
#define XRAD__File_PC_ConsoleLibLink_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

#ifndef _MSC_VER
	#error этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#ifndef XRAD_COMPILER_CMAKE
	#pragma comment(lib, "XRADConsoleUI" XRAD_Library_Suffix ".lib")
	#endif // XRAD_COMPILER_CMAKE
	// Задаем использование wmain для линкера:
	#pragma comment(linker, "/SUBSYSTEM:CONSOLE /ENTRY:wmainCRTStartup")
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_PC_ConsoleLibLink_h
