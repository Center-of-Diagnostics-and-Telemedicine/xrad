/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file SystemConfig.h
//--------------------------------------------------------------
#ifndef XRAD__File_SystemConfig_h
#define XRAD__File_SystemConfig_h
/*!
	\file
	\brief Настройка конфигурации XRADSystem
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

//--------------------------------------------------------------

#if defined(XRAD_COMPILER_MSC)

// Конфигурация для MSVC

#define XRAD_USE_FILENAMES_WIN32_VERSION
#if _MSC_VER < 1920 // < MSVC 2019
#define XRAD_USE_FILESYSTEM_WIN32_VERSION
#else
#define XRAD_USE_FILESYSTEM_STD_VERSION // Требует поддержки C++17
#endif
#define XRAD_USE_CFILE_WIN32_VERSION



#elif defined(XRAD_COMPILER_GNUC)

// Конфигурация для GCC

#define XRAD_USE_FILENAMES_UNIX_VERSION
#define XRAD_USE_FILESYSTEM_STD_VERSION
#define XRAD_USE_CFILE_UNIX_VERSION



#else
#error Unknown compiler.
#endif

//--------------------------------------------------------------
#endif // XRAD__File_SystemConfig_h
