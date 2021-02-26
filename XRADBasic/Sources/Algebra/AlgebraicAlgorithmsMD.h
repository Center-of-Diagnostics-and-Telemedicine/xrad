/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file AlgebraicAlgorithmsMD.h
//--------------------------------------------------------------
#ifndef XRAD__File_AlgebraicAlgorithmsMD_h
#define XRAD__File_AlgebraicAlgorithmsMD_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Операции над многомерными массивами типа DataArrayMD для FieldElement

	Данный файл не должен зависеть от DataArrayMD. Он зависит только от соглашения
	об _интерфейсе_ класса.
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include <XRADBasic/Sources/Containers/BasicArrayInteractionsMD.h>

XRAD_BEGIN

namespace	AlgebraicStructures
{

//--------------------------------------------------------------
/*!
	\brief Алгоритмы для FieldElement для произвольного многомерного контейнера, наследуемого от DataArrayMD
	или другого класса с аналогичным интерфейсом

	См. \ref pg_FieldElement_Algorithm.
*/
class	AlgebraicAlgorithmsDataArrayMD
{
	public:
		template <class AT, class OP>
		static AT &A_Op_Assign(AT& array, const OP &unary_action)
		{
			Apply_A_MD_F1(array, unary_action);
			return array;
		}

		template <class AT, class OP>
		static AT A_Op_New(const AT& array, const OP &assign_action)
		{
			AT result(array.sizes());
			Apply_AA_MD_F2(result, array, assign_action);
			return result;
		}

		template <class AT1, class AT2, class OP>
		static AT1 &AA_Op_Assign(AT1& array_1, const AT2& array_2, const OP &binary_action)
		{
			Apply_AA_MD_F2(array_1, array_2, binary_action);
			return array_1;
		}

		template <class AT1, class T2, class OP>
		static AT1 &AS_Op_Assign(AT1& array_1, const T2& x, const OP &binary_action)
		{
			Apply_AS_MD_F2(array_1, x, binary_action);
			return array_1;
		}

		template <class AT1, class AT2, class OP>
		static AT1 AA_Op_New(const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			AT1 result(array_1.sizes());
			Apply_AAA_MD_F3(result, array_1, array_2, ternary_action);
			return result;
		}

		template <class AT1, class T2, class OP>
		static AT1 AS_Op_New(const AT1& array_1, const T2& x, const OP &ternary_action)
		{
			AT1 result(array_1.sizes());
			Apply_AAS_MD_F3(result, array_1, x, ternary_action);
			return result;
		}

		template <class AT1, class AT2>
		static bool AA_EqSize(const AT1& array_1, const AT2& array_2)
		{
			return EqSizes_AA_MD(array_1, array_2);
		}

		template <class AT1, class AT2, class OP>
		static bool AA_Any(const AT1& array_1, const AT2& array_2, const OP &binary_test)
		{
			return Apply_Any_AA_MD_RF2(array_1, array_2, binary_test);
		}

		template <class AT0, class AT1, class AT2, class OP>
		static AT0 &AAA_Op_Assign(AT0& array_0, const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			Apply_AAA_MD_F3(array_0, array_1, array_2, ternary_action);
			return array_0;
		}

		template <class AT0, class AT1, class ST, class OP>
		static AT0 &AAS_Op_Assign(AT0& array_0, const AT1& array_1, const ST& x, const OP &ternary_action)
		{
			Apply_AAS_MD_F3(array_0, array_1, x, ternary_action);
			return array_0;
		}
};

//--------------------------------------------------------------

}//namespace	AlgebraicStructures
XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_AlgebraicAlgorithmsMD_h
