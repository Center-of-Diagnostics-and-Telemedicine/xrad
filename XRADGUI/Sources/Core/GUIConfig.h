/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_GUIConfig_h
#define XRAD__File_GUIConfig_h
/*!
	\file
	\brief Настройка конфигурации XRADGUI
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

//--------------------------------------------------------------

#if defined(XRAD_COMPILER_MSC)

// Конфигурация для MSVC

#define XRAD_USE_KEYBOARD_WIN32_VERSION
#define XRAD_USE_GUI_MS_WIN32_VERSION



#elif defined(XRAD_COMPILER_GNUC)

// Конфигурация для GCC

#define XRAD_USE_KEYBOARD_QT_VERSION



#else
#error Unknown compiler.
#endif

//--------------------------------------------------------------
#endif // XRAD__File_GUIConfig_h
