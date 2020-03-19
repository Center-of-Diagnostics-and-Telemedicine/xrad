// file PC_XRADQtLink.h
//--------------------------------------------------------------
#ifndef __PC_XRADQtLink_h
#define __PC_XRADQtLink_h
//--------------------------------------------------------------

#include "PC_QtCoreLink.h"

#ifndef _MSC_VER
	#error Этот файл может использоваться только с MS VisualStudio
#endif //_MSC_VER



#if _MSC_VER >= 1900 // MSVC2015+, Qt5
	#ifdef _DEBUG
		#pragma comment(lib, "QTMaind.lib")
		#pragma comment(lib, "QT5Widgetsd.lib")
		#pragma comment(lib, "QT5Guid.lib")
		#pragma comment(lib, "QT5Svgd.lib")
		#pragma comment(lib, "qwtd.lib")
	#else
		#pragma comment(lib, "QTMain.lib")
		#pragma comment(lib, "QT5Widgets.lib")
		#pragma comment(lib, "QT5Gui.lib")
		#pragma comment(lib, "QT5Svg.lib")
		#pragma comment(lib, "qwt.lib")
	#endif //_DEBUG
#else
	#error Unsupported Visual Studio version
#endif

//--------------------------------------------------------------
#endif // __PC_XRADQtLink_h
