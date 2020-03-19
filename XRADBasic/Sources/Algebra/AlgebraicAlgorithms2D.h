// file AlgebraicAlgorithms2D.h
//--------------------------------------------------------------
#ifndef __AlgebraicAlgorithms2D_h
#define __AlgebraicAlgorithms2D_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Операции над двумерными массивами типа DataArray2D для FieldElement

	Данный файл не должен зависеть от DataArray2D. Он зависит только от соглашения
	об _интерфейсе_ класса.
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include <XRADBasic/Sources/Containers/BasicArrayInteractions2D.h>

XRAD_BEGIN

namespace AlgebraicStructures
{

//--------------------------------------------------------------
/*!
	\brief Алгоритмы для FieldElement для произвольного двумерного контейнера, наследуемого от DataArray2D
	или другого класса с аналогичным интерфейсом

	См. \ref pg_FieldElement_Algorithm.
*/
class AlgebraicAlgorithmsDataArray2D
{
	public:
		template <class AT, class OP>
		static AT &A_Op_Assign(AT& array, const OP &unary_action)
		{
			Apply_A_2D_F1(array, unary_action);
			return array;
		}

		template <class AT, class OP>
		static AT A_Op_New(const AT& array, const OP &assign_action)
		{
			AT result(array.vsize(), array.hsize());
			Apply_AA_2D_F2(result, array, assign_action);
			return result;
		}

		template <class AT1, class AT2, class OP>
		static AT1 &AA_Op_Assign(AT1& array_1, const AT2& array_2, const OP &binary_action)
		{
			Apply_AA_2D_F2(array_1, array_2, binary_action);
			return array_1;
		}

		template <class AT1, class T2, class OP>
		static AT1 &AS_Op_Assign(AT1& array_1, const T2& x, const OP &binary_action)
		{
			Apply_AS_2D_F2(array_1, x, binary_action);
			return array_1;
		}

		template <class AT1, class AT2, class OP>
		static AT1 AA_Op_New(const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			AT1 result(array_1.vsize(), array_1.hsize());
			Apply_AAA_2D_F3(result, array_1, array_2, ternary_action);
			return result;
		}

		template <class AT1, class T2, class OP>
		static AT1 AS_Op_New(const AT1& array_1, const T2& x, const OP &ternary_action)
		{
			AT1 result(array_1.vsize(), array_1.hsize());
			Apply_AAS_2D_F3(result, array_1, x, ternary_action);
			return result;
		}

		template <class AT1, class AT2>
		static bool AA_EqSize(const AT1& array_1, const AT2& array_2)
		{
			return EqSizes_AA_2D(array_1, array_2);
		}

		template <class AT1, class AT2, class OP>
		static bool AA_Any(const AT1& array_1, const AT2& array_2, const OP &binary_test)
		{
			return Apply_Any_AA_2D_RF2(array_1, array_2, binary_test);
		}

		template <class AT0, class AT1, class AT2, class OP>
		static AT0 &AAA_Op_Assign(AT0& array_0, const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			Apply_AAA_2D_F3(array_0, array_1, array_2, ternary_action);
			return array_0;
		}

		template <class AT0, class AT1, class ST, class OP>
		static AT0 &AAS_Op_Assign(AT0& array_0, const AT1& array_1, const ST& x, const OP &ternary_action)
		{
			Apply_AAS_2D_F3(array_0, array_1, x, ternary_action);
			return array_0;
		}
};

//--------------------------------------------------------------

} // namespace AlgebraicStructures
XRAD_END

//--------------------------------------------------------------
#endif // __AlgebraicAlgorithms2D_h
