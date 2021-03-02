/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_complex_function_h
#define XRAD__File_complex_function_h
/*!
	\file
	\date 2014-04-29 11:03
	\author kns
*/
//--------------------------------------------------------------

#include "MathFunction.h"
#include <XRADBasic/Sources/Algebra/ComplexAlgebraElement.h>
#include "ComplexContainer.h"

XRAD_BEGIN

//--------------------------------------------------------------
/*!
	\brief Тип осцилляции комплексной функции

	\par Об осцилляции комплексной функции, задаваемой параметром complex_function_oscillation

	<b>Пример</b>

	Дана дискретная комплексная функция f(n), состоящая из 8 отсчетов.
	Пусть дискретное преобразование фурье имеет вид:

	~~~~
	ABCDEFGH
	~~~~

	Построим дискретную функцию g(n) из 16 отсчетов посредством
	интерполяции функции f(n). В зависимости от интерпретации
	исходных данных интерполяция может дать три различных результата:

	~~~~
	ABCD00000000EFGH
	ABCDEFGH00000000
	00000000ABCDEFGH
	~~~~

	В первом случае предполагается, что входные данные не имели выраженной осцилляции.
	Во втором осцилляция была с положительным приращением фазы, в третьем с отрицательным.
	Выбор конкретного варианта зависит только от априорной информации о входных данных.

	Ошибка интерпретации приводит к фатальному искажению результата. Поэтому для комплексной функции
	запрещается использование интерполяции по умолчанию, без указания вида осцилляции.
*/
enum complex_function_oscillation
{
	no_oscillation = 0,
	positive_oscillation = 1,
	negative_oscillation = -1
};



//--------------------------------------------------------------
/*!
	\brief Комплексная одномерная дискретная функция

	Основные отличия от обычной дискретной функции (действительной):
	- Особенности функций интерполяции и дифференцирования.
	- Добавлены функции-члены, размещающие ссылки на действительную или мнимую части в заранее созданный контейнер.
	- Те же действия продублированы как не-члены (возвращают контейнеры-ссылки на данные массива).
	- Добавлена функция фильтрации гауссовым фильтром со смещенной центральной частотой.
*/
template<class T, class ST>
class	ComplexFunction : public ComplexContainer<
		MathFunction<T, ST, AlgebraicStructures::FieldTagComplex>,
		MathFunction<typename T::part_type, ST, AlgebraicStructures::FieldTagScalar>>
{
	private:
		PARENT(ComplexContainer<
				MathFunction<T, ST, AlgebraicStructures::FieldTagComplex>,
				MathFunction<typename T::part_type, ST, AlgebraicStructures::FieldTagScalar>>);

	public:
		using self = ComplexFunction;
		using invariable = ComplexFunction<typename parent::value_type_invariable, ST>;
		using variable = ComplexFunction<typename parent::value_type_variable, ST>;
		using ref = ReferenceOwner<self>;
		using ref_invariable = ReferenceOwner<invariable>;
		using ref_variable = ReferenceOwner<variable>;
			// про invariable см. подробный комментарий в DataOwner.h

		using typename parent::value_type;
		using typename parent::value_part_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		ComplexFunction() = default;
		ComplexFunction(const parent &p): parent(p) {}
		ComplexFunction(parent &&p): parent(std::move(p)) {}
		using parent::operator=;

		ComplexFunction(const std::initializer_list<value_part_type> &l) : parent(l.size()) {std::copy(l.begin(), l.end(), begin());}

	public:
		using parent::begin;
		using parent::end;

		using parent::in;
		value_type	in(double x, complex_function_oscillation osc) const;
		value_type	d_dx(double x, complex_function_oscillation osc) const;

		// TODO: Реализация функции закомментирована. Исправить реализацию, см. комментарии в *.hh
		void	FilterGaussCarrier(double dispersion, double carrier, double value_at_edge = 0.3, extrapolation::method ex = extrapolation::by_zero);
};

check_complex_container(ComplexFunction, complexF64, double)

//--------------------------------------------------------------

XRAD_END

#include "ComplexFunction.hh"

//--------------------------------------------------------------
#endif //XRAD__File_complex_function_h
