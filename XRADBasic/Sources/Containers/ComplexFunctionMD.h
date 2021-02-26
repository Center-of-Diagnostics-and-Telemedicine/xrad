/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_ComplexFunctionMD_h
#define XRAD__File_ComplexFunctionMD_h
/*!
	\file
	\date 2014-09-27 15:58
	\author KNS
	\brief Многомерная комплексная функция
*/
//--------------------------------------------------------------

#include "MathFunctionMD.h"
#include "ComplexFunction2D.h"
#include "ComplexContainer.h"

XRAD_BEGIN

//--------------------------------------------------------------

template<class T, class ST>
class ComplexFunctionMD : public ComplexContainer<
		MathFunctionMD<ComplexFunction2D<T,ST>>,
		MathFunctionMD<typename ComplexFunction2D<T,ST>::part_type>>
{
		PARENT(ComplexContainer<
				MathFunctionMD<ComplexFunction2D<T,ST>>,
				MathFunctionMD<typename ComplexFunction2D<T,ST>::part_type>>);

	public:
		using typename parent::value_type;
		using typename parent::value_type_invariable;
		using typename parent::value_type_variable;

		using self = ComplexFunctionMD;
		using invariable = ComplexFunctionMD<value_type_invariable, ST>;
		using variable = ComplexFunctionMD<value_type_variable, ST>;
			// про invariable см. подробный комментарий в DataOwner.h

		using slice_type = ComplexFunction2D<T, ST>;
		using slice_type_invariable = typename ComplexFunction2D<T, ST>::invariable;
		using row_type = ComplexFunction<T, ST>;
		using row_type_invariable = typename ComplexFunction<T, ST>::invariable;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		ComplexFunctionMD() = default;
		ComplexFunctionMD(const parent &p): parent(p) {}
		ComplexFunctionMD(parent &&p): parent(std::move(p)) {}
		using parent::operator=;
};

template<class T, class ST>
ComplexFunctionMD<T, ST>	zero_value(const ComplexFunctionMD<T, ST> &datum)
{
	index_vector	access_v(datum.n_dimensions(), 0);
	return ComplexFunctionMD<T, ST>(datum.sizes(), zero_value(datum.at(access_v)));
}

template<class T, class ST>
void	make_zero(ComplexFunctionMD<T, ST> &datum)
{
	index_vector	access_v(datum.n_dimensions(), 0);
	make_zero(datum.at(access_v));
	datum.fill(datum.at(access_v));
}

//	проверка корректности задания атрибутов контейнера
//	см. ContainerCheck.h
check_complex_container(ComplexFunctionMD, complexF64, double)
check_container_md(ComplexFunctionMD, complexF64, double)

//--------------------------------------------------------------

XRAD_END

#include "ComplexFunctionMD.hh"

//--------------------------------------------------------------
#endif //XRAD__File_ComplexFunctionMD_h
