/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file BasicMacros.h
//	Created by KNS on 20.06.03
//--------------------------------------------------------------
#ifndef XRAD__basic_macros_h
#define XRAD__basic_macros_h
/*!
	\addtogroup gr_XRAD_Basic
	@{

	\file
	\brief Базовые макроопределения

	Внутренний файл библиотеки.
*/

#include "ForceDebugBreak.h"
#include <cstddef>
#include <climits>

//--------------------------------------------------------------
//
//	namespace definitions
//
//--------------------------------------------------------------

//! \brief Открытие namespace xrad
#define XRAD_BEGIN namespace xrad {

/*!
	\brief Закрытие namespace xrad

	\note До 2013_09_12 в XRAD_END по умолчанию было "using namespace xrad;".
	Это приводит к путанице, удалено.
*/
#define XRAD_END }

//! \brief Использование namespace xrad;
#define XRAD_USING using namespace ::xrad;

//--------------------------------------------------------------
/*!
	\brief Определение типов для вызова родительских методов

	2014_10_01 определение PARENT изменено.
	В текущем виде позволяет подставлять любые шаблоны в аргумент.
	Требует C++11.

	Прошлое определение:

			#define PARENT(T) typedef T parent; typedef T inherited
*/
//--------------------------------------------------------------

#define PARENT(...) typedef __VA_ARGS__ parent; typedef __VA_ARGS__ inherited

//--------------------------------------------------------------
//	отладочные макроопределения

/*!
	\brief Обеспечивает однократное выполнение заданного действия при первом обращении

	Первоначальный вариант не работал в конструкциях if(something) do_once{}:

			#define do_once static int first_time__##__LINE__ = 0; if(!first_time__##__LINE__++)

	Внимание! Не подходит для многопоточного выполнения.
	Использование atomic не поможет?
*/
#define do_once	for(static size_t first_time__##__LINE__ = 0; first_time__##__LINE__ < 1; ++first_time__##__LINE__)

//! \brief Размер в битах
template<class T>
constexpr size_t bitsizeof() {return sizeof(T)*CHAR_BIT;}

//--------------------------------------------------------------

//! @} <!-- ^group gr_XRAD_Basic -->

#endif // XRAD__basic_macros_h
