#ifndef XRAD__algebraic_algorithms_fixed_size_array_h
#define XRAD__algebraic_algorithms_fixed_size_array_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Операции над одномерными массивами типа FixedSizeArray для FieldElement

	Данный файл не должен зависеть от FixedSizeArray. Он зависит только от соглашения
	об _интерфейсе_ класса.
*/
//! @} <!-- ^group gr_Algebra -->
//--------------------------------------------------------------

#include <XRADBasic/Sources/Containers/BasicArrayInteractions1D.h>

XRAD_BEGIN

namespace AlgebraicStructures
{

//--------------------------------------------------------------
/*!
	\brief Алгоритмы для FieldElement для произвольного одномерного контейнера, наследуемого от
	FixedSizeArray или другого класса с аналогичным интерфейсом

	См. \ref pg_FieldElement_Algorithm.
*/
class AlgebraicAlgorithmsFixedSizeArray
{
	public:
		template <class AT, class OP>
		static AT &A_Op_Assign(AT& array, const OP &unary_action)
		{
			Apply_A_1D_F1(array, unary_action);
			return array;
		}

		template <class AT1, class AT2, class OP>
		static AT1 &AA_Op_Assign(AT1& array_1, const AT2& array_2, const OP &binary_action)
		{
			Apply_AA_1D_F2(array_1, array_2, binary_action);
			return array_1;
		}

		template <class AT1, class T2, class OP>
		static AT1 &AS_Op_Assign(AT1& array_1, const T2& x, const OP &binary_action)
		{
			Apply_AS_1D_F2(array_1, x, binary_action);
			return array_1;
		}

		template <class AT, class OP>
		static AT A_Op_New(const AT& array, const OP &assign_action)
		{
			AT result;
			Apply_AA_1D_F2(result, array, assign_action);
			return result;
		}

		template <class AT1, class AT2, class OP>
		static AT1 AA_Op_New(const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			AT1 result;
			Apply_AAA_1D_F3(result, array_1, array_2, ternary_action);
			return result;
		}

		template <class AT1, class T2, class OP>
		static AT1 AS_Op_New(const AT1& array_1, const T2& x, const OP &ternary_action)
		{
			AT1 result;
			Apply_AAS_1D_F3(result, array_1, x, ternary_action);
			return result;
		}

		template <class AT1, class AT2>
		static typename std::enable_if<AT1::fixed_size == AT2::fixed_size, bool>::type AA_EqSize(const AT1&, const AT2&)
		{
			return true;
		}

		template <class AT1, class AT2>
		static typename std::enable_if<AT1::fixed_size != AT2::fixed_size, bool>::type AA_EqSize(const AT1&, const AT2&)
		{
			return false;
		}

		template <class AT1, class AT2, class OP>
		static bool AA_Any(const AT1& array_1, const AT2& array_2, const OP &binary_test)
		{
			return Apply_Any_AA_1D_RF2(array_1, array_2, binary_test);
		}

		template <class AT0, class AT1, class AT2, class OP>
		static AT0 &AAA_Op_Assign(AT0& array_0, const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			Apply_AAA_1D_F3(array_0, array_1, array_2, ternary_action);
			return array_0;
		}

		template <class AT0, class AT1, class ST, class OP>
		static AT0 &AAS_Op(AT0& array_0, const AT1& array_1, const ST& x, const OP &ternary_action)
		{
			Apply_AAS_1D_F3(array_0, array_1, x, ternary_action);
			return array_0;
		}
};

//--------------------------------------------------------------

} // namespace AlgebraicStructures
XRAD_END

//--------------------------------------------------------------
#endif // XRAD__algebraic_algorithms_fixed_size_array_h
