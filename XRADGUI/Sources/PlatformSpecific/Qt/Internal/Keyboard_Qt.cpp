﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#include "pre.h"
#include "Keyboard_Qt.h"

#ifdef XRAD_USE_KEYBOARD_QT_VERSION

#include <XRADGUI/Sources/GUI/ModifierKeys.h>
#include <QtWidgets/QApplication>

XRAD_BEGIN

//--------------------------------------------------------------

//	modifier key state
int	ModifierKeysState_Qt()
{
	Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
	int	result = 0;

	if(modifiers&Qt::MetaModifier) result |= modkeyControl;
	if(modifiers&Qt::ShiftModifier) result |= modkeyShift;
	if(modifiers&Qt::ControlModifier) result |= modkeyCommand;
	if(modifiers&Qt::AltModifier) result |= modkeyAlt;

	// проверка CapsLock средствами Qt недоступна
	// https://stackoverflow.com/questions/9830905/qt-4-7-4-is-there-a-way-to-find-out-the-status-of-caps-lock

	return result;
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_KEYBOARD_QT_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_Keyboard_Qt() {}
XRAD_END

#endif // XRAD_USE_KEYBOARD_QT_VERSION

//--------------------------------------------------------------
