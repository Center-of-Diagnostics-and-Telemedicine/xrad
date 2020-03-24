﻿//--------------------------------------------------------------
#ifndef XRAD__File_Keyboard_Win32_h
#define XRAD__File_Keyboard_Win32_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRAD/GUI/GUIConfig.h>

#ifdef XRAD_USE_KEYBOARD_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Анализ сочетаний регистровых клавиш

	Анализ сочетаний регистровых клавиш реализован в Qt очень плохо.
	из 5 регистровых клавиш 2 никак не учитываются
	(CapsLock, LWin/RWin).
	Хотя кроссплатформенная реализация также сделана,
	по возможности следует использовать полноценные версии, реализованные
	для конкретных платформ.
*/
int	ModifierKeysState_MS();

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_KEYBOARD_WIN32_VERSION

//--------------------------------------------------------------
#endif // XRAD__File_Keyboard_Win32_h