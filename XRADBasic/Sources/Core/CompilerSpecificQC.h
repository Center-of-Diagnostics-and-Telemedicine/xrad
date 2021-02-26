/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file CompilerSpecificQC.h
//--------------------------------------------------------------
#ifndef CompilerSpecificQC_h__
#define CompilerSpecificQC_h__
/*!
	\addtogroup gr_CompilerSpecific
	@{

	\file
	\brief Проверка наличия обязательных зависящих от компилятора определений
	(CompilerSpecific Quality Control)

	Внутренний файл библиотеки.
*/
//--------------------------------------------------------------

namespace xrad
{

//--------------------------------------------------------------

/*!
	\file
	\par Проверка наличия макроопределения XRAD_SIZE_T_BITS
	Это макроопределение должно задавать числовую константу, равную количеству бит в типе size_t.
*/
#ifndef XRAD_SIZE_T_BITS
	#error XRAD_SIZE_T_BITS must be defined.
#endif

/*!
	\file
	\par Проверка наличия макроопределения XRAD_ENDIAN
	Это макроопределение должно задавать числовую константу, равную одному из значений
	из файла <Endian.h>.
*/
#ifndef XRAD_ENDIAN
	#error XRAD_ENDIAN must be defined.
#elif XRAD_ENDIAN != XRAD_LITTLE_ENDIAN && XRAD_ENDIAN != XRAD_BIG_ENDIAN
	#error XRAD_ENDIAN must be equal to one of the values from <Endian.h>.
#endif

//--------------------------------------------------------------

/*!
	\file
	\par Проверка наличия макроопределения XRAD_FORCE_DEBUG_BREAK_FUNCTION
	Это определение требуется для реализации ForceDebugBreak().
	Конструкция XRAD_FORCE_DEBUG_BREAK_FUNCTION() должна приводить к вызову функции выхода в отладчик
	или чему-то аналогичному. При отсутствии нужного функционала можно определить как пустой макрос.
	Анализ XRAD_DEBUG и т.п. производить не требуется, это происходит в <ForceDebugBreak.h>.
	Можно добавить еще анализ if (IsDebuggerPresent()) ... else вызов OutputDebugString().
*/
#ifndef XRAD_FORCE_DEBUG_BREAK_FUNCTION
	#error XRAD_FORCE_DEBUG_BREAK_FUNCTION must be defined.
#endif

/*!
	\file
	\par Проверка наличия макроопределения gray_pixel_black
	Этот макрос задает константу — значение черного цвета для платформо-зависимого полутонового изображения.
	Тип должен быть совместим с rgb_pixel_component_type.
	Для Windows это 0. Для MacOS Classic было равно 255.
*/
#ifndef gray_pixel_black
	#error gray_pixel_black must be defined.
#endif

/*!
	\file
	\par Проверка наличия макроопределения gray_pixel_black
	Этот макрос задает константу — значение белого цвета для платформо-зависимого полутонового изображения.
	Тип должен быть совместим с rgb_pixel_component_type.
	Для Windows это 255. Для MacOS Classic было равно 0.
*/
#ifndef gray_pixel_white
	#error gray_pixel_white must be defined.
#endif

/*!
	\file
	\par Проверка наличия макроопределения XRAD_CheckedIterator
	Этот макрос нужно размещать в public-секции итераторов.
	Сообщает компилятору, что итератор контролирует свои границы во избежание предупреждения (C4996 на MS Visual Studio).
	Если какой-то компилятор не предусматривает такой проверки, объявить пустым
*/
#ifndef XRAD_CheckedIterator
	#error XRAD_CheckedIterator must be defined
#endif

/*!
	\file
	\par Проверка наличия определения типа rgb_pixel_component_type
	Тип rgb_pixel_component_type задает тип пикселя платформо-зависимого полутонового изображения.
	Это должен быть арифметический тип или пользовательский тип, аналогичный арифметическому.
*/
namespace CompilerSpecificQC
{
typedef ::xrad::rgb_pixel_component_type rgb_pixel_component_type_defined;
} // namespace CompilerSpecificQC

//--------------------------------------------------------------

/*!
	\file
	\par Контроль размера enum
	По умолчанию предполагается равным размеру int,
	следует соответствующим образом установить настройки компилятора
	(опция "enum always int", "enum size" и т.п.).
	Если по каким-либо веским причинам при компиляции используется другой размер,
	следует задать его в макроопределении XRAD_DEFAULT_ENUM_SIZE в файле,
	подключаемом в <CompilerSpecific.h>.
*/

namespace CompilerSpecificQC
{

#ifndef XRAD_DEFAULT_ENUM_SIZE
#define XRAD_DEFAULT_ENUM_SIZE sizeof(int)
#endif

enum enum_size_checker { dummy = 0 };

static_assert(XRAD_DEFAULT_ENUM_SIZE == sizeof(enum_size_checker), "Incorrect enum size.");

} // namespace CompilerSpecificQC

//--------------------------------------------------------------

} // namespace xrad

//--------------------------------------------------------------
//! @} <!-- ^group gr_CompilerSpecific -->
#endif // CompilerSpecificQC_h__
