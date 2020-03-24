﻿// file PC_XRADGUITestsLibLink.h
//--------------------------------------------------------------
#ifndef __PC_XRADGUITestsLibLink_h
#define __PC_XRADGUITestsLibLink_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#pragma comment(lib, "XRADGUITestsLib" XRAD_Library_Suffix ".lib")
#else //other MSVS versions
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // __PC_XRADGUITestsLibLink_h