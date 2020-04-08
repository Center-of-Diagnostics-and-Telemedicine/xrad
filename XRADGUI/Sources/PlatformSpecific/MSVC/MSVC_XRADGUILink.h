#ifndef XRAD__File_XRADGUILink_h
#define XRAD__File_XRADGUILink_h
//--------------------------------------------------------------

#include <XRADQt/MSVC/MSVC_QtLink.h>
#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

//--------------------------------------------------------------

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#pragma comment(lib, "XRADGUI" XRAD_Library_Suffix ".lib")
#else //other MSVS versions
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_XRADGUILink_h
