/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef HomomorphSamples_h__
#define HomomorphSamples_h__
/*!
	\file
	\date 2016-09-05 16:10
	\author kns
	\brief Преобразования типов отсчетов
*/

#include <XRADBasic/Core.h>
#include <type_traits>

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\defgroup gr_FloatingAnalog Типы для буферов с плавающей запятой
	@{
	При обработке целочисленных отсчетов часто происходят потери данных либо переполнения.
	Поэтому может понадобиться буфер с аналогичным типом с плавающей запятой.

	Пример: шаблон, вычисляющий среднее от массива. Возможен массив short и ComplexSample<short>.
	Тип результата, как и буфера накопления, должен быть double и ComplexSample<double>.

	Для автоматизации создания подобных аналогичных типов используются следующие конструкции:
	- \ref floating32_type, \ref floating64_type используются для создания аналогичных типов
	с плавающей запятой.
	- \ref FloatingAnalog32, \ref FloatingAnalog64 используются в качестве механизма реализации.
*/

/*!
	\brief Шаблонный класс-реализация для \ref floating32_type.
	Шаблон по умолчанию возвращает исходный тип

	Данный шаблон должен быть специализирован для всех типов, для которых
	требуется получение аналогичного типа, основанного на float.

	Класс и его специализации должны содержать публичный typedef (using),
	определяющий тип type, являющийся float-аналогом параметра шаблона T.

	\todo Подумать над правильностью подхода с наличием шаблона по умолчанию.
	Проблема с написанием специализаций в том, что специализации нужно писать
	для каждого конечного класса, а наличие специализаций может быть не всегда нужно.
	Если реализации по умолчанию не станет, возможно, стоит включить требование
	наличия специализации для типа в проверку number traits.
*/
template<class T, class Conditional = void> struct FloatingAnalog32 { typedef T type; };

/*!
	\brief Шаблонный класс-реализация для \ref floating64_type.
	Шаблон по умолчанию возвращает исходный тип

	Данный шаблон должен быть специализирован для всех типов, для которых
	требуется получение аналогичного типа, основанного на double.

	Класс и его специализации должны содержать публичный typedef (using),
	определяющий тип type, являющийся double-аналогом параметра шаблона T.

	\todo См. примечания к \ref FloatingAnalog32 (общий шаблон).
*/
template<class T, class Conditional = void> struct FloatingAnalog64 { typedef T type; };

//--------------------------------------------------------------

//! \brief Специализация для floating32_type для арифметических типов
template<class T> struct FloatingAnalog32<T, typename enable_if<is_arithmetic_but_bool<T>::value>::type> { typedef float type; };

//! \brief Специализация для floating64_type для арифметических типов
template<class T> struct FloatingAnalog64<T, typename enable_if<is_arithmetic_but_bool<T>::value>::type> { typedef double type; };

//--------------------------------------------------------------

//! \brief Аналогичный тип данных, основанный на float, или исходный тип
template <class T>
using floating32_type = typename FloatingAnalog32<typename remove_cv<T>::type>::type;

//! \brief Аналогичный тип данных, основанный на double, или исходный тип
template <class T>
using floating64_type = typename FloatingAnalog64<typename remove_cv<T>::type>::type;

//! @}

//--------------------------------------------------------------
/*!
	\defgroup gr_ReducedWidth Типы для буферов меньшей разрядности
	@{
	Совсем необязательно для показа на экране держать в памяти данные с двойной точностью или даже 32-битные целые.
	Особенно в случае больших многомерных данных это приводит к нехватке памяти. Следующий шаблон позволяет это определить.
*/

/*!
	\brief Шаблонный класс-реализация для floating32_type.
	Шаблон по умолчанию  не реализован. Используются специализации

	Данный шаблон должен быть специализирован для всех типов, для которых
	требуется используется \ref reduced_type.

	Специализации должны содержать публичный typedef (using),
	определяющий тип type, являющийся типом меньшей разрядности, аналогичным T,
	или самим типом T (если снижение разрядности для T не производится).

	\todo
	Возможно, стоит включить требование
	наличия специализации для типа в проверку number traits.
*/
template<class T, class Conditional = void>
struct ReducedWidth;

template<>
struct ReducedWidth<unsigned char> { typedef unsigned char type; };
template<>
struct ReducedWidth<int16_t> { typedef int16_t type; };
template<>
struct ReducedWidth<uint16_t> { typedef uint16_t type; };
template<>
struct ReducedWidth<int32_t> { typedef int16_t type; };
template<>
struct ReducedWidth<uint32_t> { typedef uint16_t type; };
template<>
struct ReducedWidth<float> { typedef float type; };
template<>
struct ReducedWidth<double> { typedef float type; };

//! \brief Возвращает тип меньшей разрядности или исходный тип
template <class T>
using reduced_type = typename ReducedWidth<typename remove_cv<T>::type>::type;

//! @}

//--------------------------------------------------------------

XRAD_END

#endif // HomomorphSamples_h__
