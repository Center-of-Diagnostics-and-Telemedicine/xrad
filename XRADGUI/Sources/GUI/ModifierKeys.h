/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_ModifiersKeys_h
#define XRAD__File_ModifiersKeys_h
/*!
	\file
	\date 2014-03-17 21:51
	\author KNS
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

enum
{
	modkeyCommand = 0x01,	// Mac: Command
	modkeyShift = 0x02,		// Shift
	modkeyLock = 0x04,		// Caps Lock
	modkeyOption = 0x08,	// Mac: Option
	modkeyControl = 0x10,

	modkeyWin = modkeyCommand,	// PC: LWin|RWin
	modkeyAlt = modkeyOption,	// PC: Alt

	modkeyAltShift = modkeyAlt|modkeyShift,
	modkeyControlShift = modkeyControl|modkeyShift,
	modkeyControlAlt = modkeyControl|modkeyAlt,
	modkeyControlAltShift = modkeyControl|modkeyAlt|modkeyShift,

	modkeyOptionShift = modkeyOption|modkeyShift,
	modkeyControlOption = modkeyControl|modkeyOption,
	modkeyControlOptionShift = modkeyControl|modkeyOption|modkeyShift,

	modkeyCommandShift = modkeyCommand|modkeyShift,
	modkeyCommandOption = modkeyCommand|modkeyOption,
	modkeyCommandOptionShift = modkeyCommand|modkeyOption|modkeyShift,

	modkeyMask = 0x1F
};

XRAD_END

//--------------------------------------------------------------

#endif // XRAD__File_ModifiersKeys_h
