/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_linear_vector_h
#define XRAD__File_linear_vector_h
/*!
	\file
	\brief Конечномерный вектор в линейном пространстве
*/
//--------------------------------------------------------------

#include "DataArray.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures1D.h>
#include <XRADBasic/Sources/Algebra/AlgebraicAlgorithmsDataArray.h>

XRAD_BEGIN

//--------------------------------------------------------------

#define XRAD__LinearVector_template class T, class ST, class FIELD_TAG
#define XRAD__LinearVector_template_args T, ST, FIELD_TAG
#define XRAD__LinearVector_template2 class T2, class ST2, class FIELD_TAG2
#define XRAD__LinearVector_template_args2 T2, ST2, FIELD_TAG2

//--------------------------------------------------------------

#define	XRAD__Field1D(child_name, VT, ST, FIELD_TAG) AlgebraicStructures::FieldElement<DataArray<VT>, child_name<VT, ST, FIELD_TAG>, VT, ST, AlgebraicStructures::AlgebraicAlgorithmsDataArray, FIELD_TAG>
/*!
	\brief Конечномерный вектор в линейном пространстве

	Заданы все виды сложения, умножение на скаляр ST,
	скалярное произведение, отношения равенства.
*/
template<class T, class ST, class FIELD_TAG>
class	LinearVector: public XRAD__Field1D(LinearVector,T,ST, FIELD_TAG)
{
	public:
		PARENT(XRAD__Field1D(LinearVector,T,ST, FIELD_TAG));
#undef	XRAD__Field1D

		typedef LinearVector<T, ST, FIELD_TAG> self;
		typedef LinearVector<const T, ST, FIELD_TAG> invariable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h

		typedef T value_type;
		typedef ST scalar_type;

	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		LinearVector() = default;
		LinearVector(const parent &p): parent(p) {}
		LinearVector(parent &&p): parent(std::move(p)) {}
		// Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип:
		// using parent::operator=;

		// TODO Перенести к MathMatrix
		// TODO LinearVector::matrix_multiply. продокументировать этот материал получше
		// матричное умножение
		// (умножения матрицы на вектор, когда результат укладывается в столбец)
		// во избежание путаницы не объявляем через operator*,
		// но делаем функцию с таким названием.
		// объявлена эта функция здесь, но описание в файле MathMatrix.hh
		// пока лучше не придумал, как сделать
/*		template<class T1, class ST1, class T2, class ST2, class FIELD_TAG2>
			self	&matrix_multiply(const MathMatrix<T1, ST1> &m1, const LinearVector<T2, ST2, FIELD_TAG2> &v);
		template<class T1, class ST1, class FIELD_TAG1, class T2, class ST2>
			self	&matrix_multiply(const LinearVector<T1, ST1, FIELD_TAG1> &m1, const MathMatrix<T2, ST2> &v);*/

};

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_linear_vector_h
