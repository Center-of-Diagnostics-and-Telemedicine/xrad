// file PC_ConsoleLibLink.h
//--------------------------------------------------------------
#ifndef __PC_ConsoleLibLink_h
#define __PC_ConsoleLibLink_h
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
#endif // __PC_ConsoleLibLink_h
