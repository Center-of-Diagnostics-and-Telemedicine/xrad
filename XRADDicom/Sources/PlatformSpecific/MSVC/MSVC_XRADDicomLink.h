// file PC_XRADDicomLink.h
//--------------------------------------------------------------
#ifndef PC_XRADDicomLink_h__
#define PC_XRADDicomLink_h__
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 //MSVC2015+
	#pragma comment(lib, "XRADDicom" XRAD_Library_Suffix ".lib")
#else
	#error Unsupported Visual Studio version.
#endif

//--------------------------------------------------------------
#endif // PC_XRADDicomLink_h__
