/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_containers_basic_h
#define XRAD__File_containers_basic_h
/*!
	\file
	\brief Базовые определения для контейнеров. Внутренний файл библиотеки
*/

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\defgroup gr_ContainerChecks Рантайм-проверки контейнеров
	@{

	Макроопределения, отвечающие за
	обработку ошибок в контейнерах, наследуемых
	от DataOwner:

	\par XRAD_CHECK_NAN_FLOAT

	Если задан этот макрос, при обращении ко всем контейнерам
	будет выполняться проверка на наличие NAN в каждом элементе.
	Это выполняется только при считывании из массива/итератора.
	Дает исключение invalid_argument. Этой проверкой не следует
	злоупотреблять, так как она существенно замедляет работу
	программы. В то же время, она может облегчить поиск ошибок
	вычислительных алгоритмов, когда те приводят к появлению NAN.

	\par XRAD_CHECK_ARRAY_BOUNDARIES

	Проверяет правильность задания границ при обращении к контейнерам
	через at() или [].

	\par XRAD_CHECK_ITERATOR_BOUNDARIES

	Проверяет правильность задания границ при обращении к контейнерам
	через итераторы, заданные в Iterators.h.

	\par ForceDebugBreak()

	Чтобы можно было проанализировать ситуацию, приведшую к исключению,
	можно ставить этот макрос за строчку перед throw().

	Ситуация по умаолчанию -- когда все макро ставятся в зависимость от
	определения XRAD_DEBUG. Можно задать их по-своему. Для этого определения
	должны стоять в "pre.h" до включения XRAD.h.

	1-3 принимают значения 0 или 1.
	4 может содержать либо вызов какой-нибудь функции (Error или MessageBox),
	либо должна оставаться пустой.
*/

//--------------------------------------------------------------

#ifdef XRAD_DEBUG

#ifndef XRAD_CHECK_NAN_FLOAT
#define	XRAD_CHECK_NAN_FLOAT 0
// эта опция по умолчанию всегда выключена, так как очень замедляет вычисления
// в случае надобности включить в pre.h или в настройках проекта
#endif
//

#ifndef XRAD_CHECK_ARRAY_BOUNDARIES
#define	XRAD_CHECK_ARRAY_BOUNDARIES 1
#endif

#ifndef XRAD_CHECK_ITERATOR_BOUNDARIES
#define	XRAD_CHECK_ITERATOR_BOUNDARIES 1
#endif


#else //XRAD_DEBUG

#ifndef XRAD_CHECK_NAN_FLOAT
#define	XRAD_CHECK_NAN_FLOAT 0
#endif

#ifndef XRAD_CHECK_ARRAY_BOUNDARIES
#define	XRAD_CHECK_ARRAY_BOUNDARIES 0
#endif

#ifndef XRAD_CHECK_ITERATOR_BOUNDARIES
#define	XRAD_CHECK_ITERATOR_BOUNDARIES 0
#endif


#endif //XRAD_DEBUG

//--------------------------------------------------------------
/*!
	\brief Проверка совместимости конфигурации библиотеки и приложения

	Если функции XRAD слинкованы в виде отдельлной библиотеки,
	параметры проверки границ массивов и т.п. библиотеки могут
	отличаться от заданных в текущем проекте. Такие версии радикально
	несовместимы между собой, поскольку размеры объектов и состав
	классов оказываются различными. Эта ситуация диагностируется
	с большим трудом, так как что-то продолжает работать, и только
	в отдельных случаях возникают access violations.

	Следующие функции проверяют соответствие слинкованной версии библиотеки
	и текущих макроопределений, заданных при компиляции приложения.

	Здесь объявлены функции:

	~~~~
	void	InitNANCheck();
	void	InitArrayBoundariesCheck();
	void	InitIteratorBoundariesCheck();
	~~~~

	Они получают при линковке имя, которое зависит от условных макроопределений.
	Эти функции заданы в файле FlowControl.cpp, который должен быть включен
	в линкуемую библиотеку XRAD. Но вызывает их функция InitFlowControl, которая
	объявлена inline. Вызов этой функции включен по умолчанию в конце файла DataOwner.hh.

	Если библиотека слинкована при других значениях макросов, нужной функции
	в библиотеке не окажется, и возникнет link error. При этом имя отсутствующей функции
	будет прямо указывать, какой параметр задан неправильно.

	При возникновении такой ошибки нужно привести соответствующую опцию текущего проекта в соответствие библиотечной.
	Или (как вариант) перелинковать библиотеку, задав в ее pre.h нужные значения.
*/
namespace internal_ContainerChecksControl
{

#if XRAD_CHECK_NAN_FLOAT
#define InitNANCheck NANCheckTrue
#else
#define InitNANCheck NANCheckFalse
#endif //XRAD_CHECK_NAN_FLOAT

#if XRAD_CHECK_ARRAY_BOUNDARIES
#define InitArrayBoundariesCheck ArrayBoundariesCheckTrue
#else
#define InitArrayBoundariesCheck ArrayBoundariesCheckFalse
#endif //XRAD_CHECK_ARRAY_BOUNDARIES

#if XRAD_CHECK_ITERATOR_BOUNDARIES
#define InitIteratorBoundariesCheck IteratorBoundariesCheckTrue
#else
#define InitIteratorBoundariesCheck IteratorBoundariesCheckFalse
#endif

int	InitNANCheck();
int	InitArrayBoundariesCheck();
int	InitIteratorBoundariesCheck();

//! \brief См. комментарий к internal_ContainerChecksControl
inline int	InitFlowControl()
{
	return InitNANCheck() + InitArrayBoundariesCheck() + InitIteratorBoundariesCheck();
}

} // namespace internal_ContainerChecksControl

//--------------------------------------------------------------
//! @} <!-- group gr_ContainerChecks -->



//--------------------------------------------------------------
/*!
	\brief Методы экстраполяции при выходе за границы массива

	Вначале они были объявлены в файле FIRFilter.h и назывались
	iir_extrapolation_method. Перенесены выше, т.к., очевидно,
	такие задачи решаются не только при конечных свертках.
	В частности, этот тип введен в MathAlgorithms.hh
	для действий над разноразмерными массивами.
*/
struct extrapolation
{
	enum method
	{
		//! \brief Все значения вне области определения считаем равными нулю
		by_zero,
		//! \brief Присваиваем последнее известное значение
		by_last_value,
		//! \brief Продолжаем область определения циклически
		cyclic,
		//! \brief Экстраполяция невозможна или недопустима, должно возникать исключение
		none
	};
};

// enum extrapolation_method
// 	{
// 	extrapolate_by_zero,
// 		// все значения вне области определения считаем равными нулю
// 	extrapolate_by_last_value,
// 		// присваиваем последнее известное значение
// 	extrapolate_cyclic,
// 		// продолжаем область определения циклически
// 	extrapolate_none
// 		// экстраполяция невозможна или недопустима, должно возникать исключение
// // 	extrapolate_by_default
// // 		// TODO привести в порядок
// // 		// при копировании вызывается конструктор по умолчанию
// // 		// 1. в методах MathAlgorithms это означает экстраполяцию нулями
// // 		// 2. если это значение присвоено переменной FIRFilter::ex_method,
// // 		// обработка такая же, как и при extrapolate_by_last_value.
// // 		// 3. если это значение передается функции SetExtrapolationMethod() в алгоритмах интерполяции,
// // 		// то величина ex_method останется без изменений.
// 	};

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif //XRAD__File_containers_basic_h
