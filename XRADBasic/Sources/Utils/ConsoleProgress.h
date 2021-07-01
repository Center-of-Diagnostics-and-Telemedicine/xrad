/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ConsoleProgress.h
//--------------------------------------------------------------
#ifndef XRAD__File_ConsoleProgress_h
#define XRAD__File_ConsoleProgress_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Создать консольный индикатор прогресса
ProgressProxy ConsoleProgressProxy();

/*!
	\brief Разрешить вывод процентов исполнения в консоль

	По умолчанию вывод процентов исполнения разрешен.

	Вызов не поддерживается в многопоточном режиме
	(во время работы консольных прогрессов).

	\return Возвращает текущее значение параметра.
*/
bool ConsoleProgressProxyEnablePercent(bool enable);

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_ConsoleProgress_h
