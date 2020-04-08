// file PC_XRADQtLink.h
//--------------------------------------------------------------
#ifndef __PC_XRADQtCoreLink_h
#define __PC_XRADQtCoreLink_h
//--------------------------------------------------------------

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER



#if _MSC_VER >= 1900 // MSVC2015+, Qt5
	#ifdef _DEBUG
		#pragma comment(lib, "QT5Cored.lib")
	#else
		#pragma comment(lib, "QT5Core.lib")
	#endif //_DEBUG
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // __PC_XRADQtCoreLink_h
