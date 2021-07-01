#include "pre.h"
#include "SecondaryScreen.h"

//------------------------------------------------------------------
//
//	created:	2021/04/06	9:29
//	filename: 	SecondaryScreen.cpp
//	file path:	Q:\XRAD\XRADGUI\QtGUIAPI\Common
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace XRAD_GUI
{

QScreen* any_secondary_screen(bool virtual_screen)
{
	QList<QScreen *> screens;

	if(virtual_screen)
	{
		// вариант, позволяющий использовать не связанные между собой экраны
		screens = QGuiApplication::screens();
	}
	else
	{
		screens = QGuiApplication::primaryScreen()->virtualSiblings();
	}

	if(screens.size() <= 1) return nullptr;
	for(auto& screen: screens)
	{
		if(screen != QGuiApplication::primaryScreen()) return screen;
	}
	return nullptr;
}


}//namespace XRAD_GUI
