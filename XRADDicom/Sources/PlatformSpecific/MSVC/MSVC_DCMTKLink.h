//--------------------------------------------------------------
#ifndef XRAD__File_MSVC_DCMTKLink_h
#define XRAD__File_MSVC_DCMTKLink_h
//--------------------------------------------------------------

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER

#if _MSC_VER >= 1900 // MSVC2015+
	#pragma comment(lib, "dcmdata.lib")
	#pragma comment(lib, "ofstd.lib")
	#pragma comment(lib, "oflog.lib")
	#pragma comment(lib, "dcmimgle.lib")
	#pragma comment(lib, "dcmjpls.lib")
	#pragma comment(lib, "dcmjpeg.lib")
	#pragma comment(lib, "dcmnet.lib")
	#pragma comment(lib, "openjp2.lib")
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // XRAD__File_MSVC_DCMTKLink_h
