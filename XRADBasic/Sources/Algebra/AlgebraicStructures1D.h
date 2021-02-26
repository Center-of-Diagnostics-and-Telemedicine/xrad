/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifdef XRAD__File_AlgebraicStructures1D_h_inside
#error Error: Recursive inclusion of AlgebraicStructures1D.h detected.
#endif
#ifndef XRAD__File_AlgebraicStructures1D_h
#define XRAD__File_AlgebraicStructures1D_h
#define XRAD__File_AlgebraicStructures1D_h_inside
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Вспомогательные определения для уменьшения громоздкости при задании классов одномерных алгебр
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include <XRADBasic/Sources/Containers/DataArray.h>

#include "AlgebraicAlgorithmsDataArray.h"
#include "AlgebraElement.h"
#include "BooleanAlgebraElement.h"



XRAD_BEGIN
/*!
	\addtogroup gr_Algebra
	@{
*/
//--------------------------------------------------------------

/*!
	\brief Задание линейных алгебр на основе DataArray
*/
template<class child_name, class VT, class ST, class FIELD_TAG>
using Algebra1D = AlgebraicStructures::AlgebraElement<
		DataArray<VT>,
		child_name,
		VT, ST,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray,
		FIELD_TAG>;

/*!
	\brief Задание булевых алгебр логических на основе DataArray &lt;BooleanLogical &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanLogical.
*/
template<class child_name, class VT>
using BooleanAlgebraLogical1D = AlgebraicStructures::BooleanAlgebraElementLogical<
		DataArray<BooleanLogical<VT>>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray>;

/*!
	\brief Задание булевых алгебр побитовых на основе DataArray &lt;BooleanBitwise &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanBitwise.
*/
template<class child_name, class VT>
using BooleanAlgebraBitwise1D = AlgebraicStructures::BooleanAlgebraElementBitwise<
		DataArray<BooleanBitwise<VT>>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray>;

//--------------------------------------------------------------

//! @} <!-- ^group gr_Algebra -->
XRAD_END

#undef XRAD__File_AlgebraicStructures1D_h_inside
#endif //XRAD__File_AlgebraicStructures1D_h
