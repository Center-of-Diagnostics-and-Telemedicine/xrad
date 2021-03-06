﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_Keyboard_Qt_h
#define XRAD__File_Keyboard_Qt_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADGUI/Sources/Core/GUIConfig.h>

#ifdef XRAD_USE_KEYBOARD_QT_VERSION

XRAD_BEGIN

//--------------------------------------------------------------

// анализ сочетаний регистровых клавиш реализован в Qt очень плохо.
// из 5 регистровых клавиш 2 никак не учитываются (CapsLock, LWin/RWin).
// хотя кроссплатформенная реализация также сделана, по возможности следует
// использовать полноценные версии, реализованные для конкретных платформ

int	ModifierKeysState_Qt();

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_KEYBOARD_QT_VERSION

//--------------------------------------------------------------
#endif // XRAD__File_Keyboard_Qt_h
