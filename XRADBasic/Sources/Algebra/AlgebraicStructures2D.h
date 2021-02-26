#ifdef XRAD__File_AlgebraicStructures2D_h_inside
#error Error: Recursive inclusion of AlgebraicStructures2D.h detected.
#endif
#ifndef XRAD__File_AlgebraicStructures2D_h
#define XRAD__File_AlgebraicStructures2D_h
#define XRAD__File_AlgebraicStructures2D_h_inside
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Вспомогательные определения для уменьшения громоздкости при задании классов двумерных алгебр
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include "AlgebraicAlgorithms2D.h"

#include "AlgebraElement.h"
#include "BooleanAlgebraElement.h"

#include <XRADBasic/Sources/Containers/DataArray2D.h>

XRAD_BEGIN
/*!
	\addtogroup gr_Algebra
	@{
*/
//--------------------------------------------------------------

/*!
	\brief Задание линейных алгебр на основе DataArray2D
*/
template<class child_name, class row_name, class VT, class ST, class FIELD_TAG>
using Algebra2D = AlgebraicStructures::AlgebraElement<
		DataArray2D<row_name>,
		child_name,
		VT, ST,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray2D,
		FIELD_TAG>;

/*!
	\brief Задание булевых алгебр логических на основе DataArray2D &lt;BooleanLogical &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanLogical.
*/
template<class child_name, class row_name, class VT>
using BooleanAlgebraLogical2D = AlgebraicStructures::BooleanAlgebraElementLogical<
		DataArray2D<row_name>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray2D>;

/*!
	\brief Задание булевых алгебр побитовых на основе DataArray2D &lt;BooleanBitwise &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanBitwise.
*/
template<class child_name, class row_name, class VT>
using BooleanAlgebraBitwise2D = AlgebraicStructures::BooleanAlgebraElementBitwise<
		DataArray2D<row_name>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArray2D>;

//--------------------------------------------------------------

//! @} <!-- ^group gr_Algebra -->
XRAD_END

#undef XRAD__File_AlgebraicStructures2D_h_inside
#endif //XRAD__File_AlgebraicStructures2D_h
