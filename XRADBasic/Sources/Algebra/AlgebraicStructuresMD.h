#ifdef XRAD__File_AlgebraicStructuresMD_h_inside
#error Error: Recursive inclusion of AlgebraicStructuresMD.h detected.
#endif
#ifndef XRAD__File_AlgebraicStructuresMD_h
#define XRAD__File_AlgebraicStructuresMD_h
#define XRAD__File_AlgebraicStructuresMD_h_inside
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Вспомогательные определения для уменьшения громоздкости при задании классов многомерных алгебр
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include "AlgebraicAlgorithmsMD.h"

#include "AlgebraElement.h"
#include "BooleanAlgebraElement.h"

#include <XRADBasic/Sources/Containers/DataArrayMD.h>

XRAD_BEGIN
/*!
	\addtogroup gr_Algebra
	@{
*/
//--------------------------------------------------------------

/*!
	\brief Задание линейных алгебр на основе DataArrayMD
*/
template<class child_name, class slice_name, class VT, class ST, class FIELD_TAG>
using AlgebraMD = AlgebraicStructures::AlgebraElement<
		DataArrayMD<slice_name>,
		child_name,
		VT, ST,
		AlgebraicStructures::AlgebraicAlgorithmsDataArrayMD,
		FIELD_TAG>;

/*!
	\brief Задание булевых алгебр логических на основе DataArrayMD &lt;BooleanLogical &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanLogical.
*/
template<class child_name, class slice_name, class VT>
using BooleanAlgebraLogicalMD = AlgebraicStructures::BooleanAlgebraElementLogical<
		DataArrayMD<slice_name>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArrayMD>;

/*!
	\brief Задание булевых алгебр побитовых на основе DataArrayMD &lt;BooleanBitwise &lt;T&gt;&gt;

	VT -- целочисленный тип внутреннего datum класса BooleanBitwise.
*/
template<class child_name, class slice_name, class VT>
using BooleanAlgebraBitwiseMD = AlgebraicStructures::BooleanAlgebraElementBitwise<
		DataArrayMD<slice_name>,
		child_name,
		VT,
		AlgebraicStructures::AlgebraicAlgorithmsDataArrayMD>;

//--------------------------------------------------------------

//! @} <!-- ^group gr_Algebra -->
XRAD_END

#undef XRAD__File_AlgebraicStructuresMD_h_inside
#endif //XRAD__File_AlgebraicStructuresMD_h
