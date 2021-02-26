/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file MathFunction2D.h
//--------------------------------------------------------------
#ifndef XRAD__File_math_function_2d_h
#define XRAD__File_math_function_2d_h
//--------------------------------------------------------------

#include "DataArray2D.h"
#include <XRADBasic/Sources/Algebra/AlgebraicStructures2D.h>
#include "FIRFilterKernel2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Двумерная функция

	Аргументом шаблона FT здесь является одномерный контейнер (MathFunction<FT::value_type,ST>
	или его наследники). Возможен альтернативный вариант: задавать контейнер только через
	value_type и ST, но в этом случае строки и столбцы двумерного контейнера будут жестко привязаны
	к MathFunction без наследников. В результате, например, строки ComplexFunction2D будут иметь тип
	MathFunction &lt;ComplexSample&gt;, а не ComplexFunction, а это нежелательно.

	Этот шаблон до 2009 года был назывался mathMatrix.
	Это было неудачное название, так как на деле речь идет не
	о матрицах, а о дискретных функциях 2х аргументов. Особенно
	неприятна такая путаница, когда приходится иметь дело одновременно
	и с функциями 2х аргументов, и с настоящими матрицами. Поэтому
	класс и соответствующие наследники переименовываются.

	01.12.2009 KNS

	В апреле 2014 года создан шаблон MathMatrix, который отвечает за
	матрицы линейной алгебры.
*/
template <class FT>
class	MathFunction2D : public Algebra2D<MathFunction2D<FT>, FT, typename FT::value_type, typename FT::scalar_type, typename FT::field_tag>
{
		PARENT(Algebra2D<MathFunction2D, FT, typename FT::value_type, typename FT::scalar_type, typename FT::field_tag>);
	public:
		typedef MathFunction2D<FT> self;
		typedef MathFunction2D<typename FT::invariable> invariable;
		typedef MathFunction2D<typename FT::variable> variable;
		typedef ReferenceOwner<self> ref;
		typedef ReferenceOwner<invariable> ref_invariable;
			// про invariable см. подробный комментарий в DataOwner.h
		typedef FT row_type;
		typedef	typename row_type::value_type value_type;
		typedef	typename row_type::scalar_type scalar_type;

		// унаследованные свойства (по требованию GCC)
		using parent::fill;
		using parent::realloc;
		using parent::at;
		using parent::vsize;
		using parent::hsize;
		using parent::row;
		using parent::col;

		//! \name Конструкторы. См. \ref pg_CopyContructorOperatorEq, \ref pg_MoveOperations
		//! @{
	public:
		// Использование родительских конструкторов и operator=
		using parent::parent;
		MathFunction2D() = default;
		MathFunction2D(const parent &p): parent(p) {}
		MathFunction2D(parent &&p): parent(std::move(p)) {}
		// Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип:
		// using parent::operator=;

		//! \brief См. \ref pg_CopyContructorOperatorEq
		//!
		//! Не форвардим родительский метод, т.к. он возвращает ссылку на родительский тип.
		template<class AT> self	&operator = (const DataArray2D<AT> &original){ parent::operator=(original); return *this; }

		template<class AT> self	&operator = (DataArray2D<AT> &&original){ parent::operator=(std::move(original)); return *this; }
		//! @}

		//! \name Roll functions
		//! @{
	public:
		void	roll_half(bool forward);
		void	roll(ptrdiff_t v, ptrdiff_t h);
		//! @}

		//! \name	Фильтрация
		//! @{

		//! \brief Линейная фильтрация
		template<class FIR_FILTER_T>
		void	Filter(const FIR_FILTER_T &);

		void	FilterGaussSeparate(double v_dispersion, double h_dispersion, double value_at_edge = 0.1, extrapolation::method ex = extrapolation::by_zero);

		//! \brief Фильтр порядковых статистик
		//!
		//! \param filter Не const, т.к. в нём есть буфер, который используется при фильтрации.
		//! Во избежание разночтений насчет состояния объекта при многопоточной обработке, не const.
		template<class B>
		void	Filter(FIRFilterKernel2DMask<B> &filter);

		//! @}

		//! \name Интерполяция
		//! @{

		//! \todo Не подходит для комплексных данных. Вынести в отдельный класс RealFunction2D, аналогично RealFunction
		floating32_type<value_type>	in(double v, double h) const;

		template<class INTERPOLATOR_T>
		floating32_type<value_type>	in(double v, double h, const INTERPOLATOR_T *interpolator) const;

		//! @}

		//! \name Дифференцирование (не к месту, для комплексных нужно иначе)
		//! \todo Вынести в отдельный класс RealFunction2D, аналогично RealFunction
		//! @{
		value_type	d_dx(double v, double h) const;
		value_type	d_dy(double v, double h) const;
		//! @}
};



//--------------------------------------------------------------
//
//	Обработка краевых эффектов
//
//--------------------------------------------------------------



enum {
	array_2D_no_side = 0,
	array_2D_left = 1,
	array_2D_right = 2,
	array_2D_top = 4,
	array_2D_bottom = 8,
	array_2D_around = array_2D_left|array_2D_right|array_2D_top|array_2D_bottom
};

//! \details
//! Все равно это набор флагов, с которыми работают как с int. Иначе compiler warning.
typedef int array_2D_sides;

//! \brief Применяет окно наподобие косинусного. См. текст функции
template<class FT>void	SmoothEdges(MathFunction2D<FT> &m,
									size_t edgeWidth,
									array_2D_sides side);



//--------------------------------------------------------------



//! \brief Обработка данных частями
template <class ARR2D, class ARR2D_PROC>
void	ProcessTiles(ARR2D &theImage, size_t tileSize, void(*theFun)(ARR2D_PROC*), char *message = NULL);

//--------------------------------------------------------------

//! \brief Обработка данных перекрывающимися частями
template <class ARR2D, class ARR2D_PROC>
void	ProcessOverlaps(ARR2D &theImage, size_t tileSize, size_t overlapSize, void(*theFun)(ARR2D_PROC*), char *message = NULL);



//--------------------------------------------------------------

XRAD_END

#include "MathFunction2D.hh"

//--------------------------------------------------------------
#endif //XRAD__File_math_function_2d_h
