// file PC_XRADBasicLink.h
//--------------------------------------------------------------
#ifndef __PC_XRADBasicLink_h
#define __PC_XRADBasicLink_h
//--------------------------------------------------------------

#include "MSVC_XRADLink-Suffix.h"

#ifndef _MSC_VER
	#error этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#pragma comment(lib, "XRADBasic" XRAD_Library_Suffix ".lib")
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // __PC_XRADBasicLink_h
