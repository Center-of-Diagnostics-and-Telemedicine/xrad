/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file CoreUtils_MS.h
//--------------------------------------------------------------
#ifndef XRAD__CoreUtils_MS_h
#define XRAD__CoreUtils_MS_h
//--------------------------------------------------------------

// Этот файл не должен использовать весь <Core.h> во избежание циклических зависимостей.
// Включаем только минимально необходимый набор заголовочных файлов.
#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

//--------------------------------------------------------------

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

//! \brief Счетчик производительности (QueryPerformaceCounter).
//! Результат в миллисекундах
double	GetMillisecondCounter_MS();

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_MS_VERSION

//--------------------------------------------------------------
#endif // XRAD__CoreUtils_MS_h
