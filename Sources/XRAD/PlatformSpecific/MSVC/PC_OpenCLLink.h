// file PC_OpenCLLink.h
//--------------------------------------------------------------
#ifndef __PC_OpenCLLink_h
#define __PC_OpenCLLink_h
//--------------------------------------------------------------

#include <XRADBasic/Sources/PlatformSpecific/MSVC/MSVC_XRADLink-Suffix.h>

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#ifndef XRAD_USE_OPENCL
	#error XRAD_USE_OPENCL must be defined
#endif

#if _MSC_VER >= 1900 // MSVC2015+
	#pragma comment(lib, "XRADOpenCL" XRAD_Library_Suffix ".lib")
	#pragma comment(lib, "OpenCL.lib")
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // __PC_OpenCLLink_h
