#ifndef XRAD__algebraic_algorithms_data_array_h
#define XRAD__algebraic_algorithms_data_array_h
/*!
	\addtogroup gr_Algebra
	@{

	\file
	\brief Операции над одномерными массивами типа DataArray для FieldElement

	Данный файл не должен зависеть от DataArray. Он зависит только от соглашения
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
	\brief Алгоритмы для FieldElement для произвольного одномерного контейнера, наследуемого от DataArray
	или другого класса с аналогичным интерфейсом

	Данная реализация является эталонной для алгоритмов для FieldElement. Здесь дается документация
	по всем (публичным) методам класса с точки зрения предоставляемого интерфейса.

	См. \ref pg_FieldElement_Algorithm.
*/
class AlgebraicAlgorithmsDataArray
{
	public:
		/*!
			\brief Проверяет, совпадают ли размеры array_1 и array_2

			\return
			- true - размеры совпадают
			- false - размеры не совпадают
		*/
		template <class AT1, class AT2>
		static bool AA_EqSize(const AT1& array_1, const AT2& array_2)
		{
			return EqSizes_AA_1D(array_1, array_2);
		}

		/*!
			\brief Применяет функтор f(x) к элементам array,
			возвращает ссылку на array

			array берется по const или не const ссылке, может модифицироваться.
		*/
		template <class AT, class OP>
		static AT &A_Op_Assign(AT& array, const OP &unary_action)
		{
			Apply_A_1D_F1(array, unary_action);
			return array;
		}

		/*!
			\brief Создает новый массив result такого же размера, как array,
			применяет функтор f(&r, x) к парам соответствующих элементов result и array,
			возвращает result

			array берется по const ссылке, не модифицируется.
		*/
		template <class AT, class OP>
		static AT A_Op_New(const AT& array, const OP &assign_action)
		{
			AT result(array.sizes(0));
			Apply_AA_1D_F2(result, array, assign_action);
			return result;
		}

		/*!
			\brief Применяет функтор f(x, y) к парам из элементов array_1 и значения x,
			возвращает ссылку на array_1

			array_1 берется по const или не const ссылке, может модифицироваться.

			x берется по const ссылке, не модифицируется.
		*/
		template <class AT1, class T2, class OP>
		static AT1 &AS_Op_Assign(AT1& array_1, const T2& x, const OP &binary_action)
		{
			Apply_AS_1D_F2(array_1, x, binary_action);
			return array_1;
		}

		/*!
			\brief Создает новый массив result такого же размера, как array_1,
			применяет функтор f(&r, x, y) к тройкам из соответствующих элементов result и array_1 и значения x,
			возвращает result

			array_1, x берутся по const ссылке, не модифицируются.
		*/
		template <class AT1, class T2, class OP>
		static AT1 AS_Op_New(const AT1& array_1, const T2& x, const OP &ternary_action)
		{
			AT1 result(array_1.sizes(0));
			Apply_AAS_1D_F3(result, array_1, x, ternary_action);
			return result;
		}

		/*!
			\brief Применяет функтор f(x, y) к парам соответствующих элементов array_1 и array_2,
			возвращает ссылку на array_1

			Размеры array_1 и array_2 должны совпадать.

			array_1 берется по const или не const ссылке, может модифицироваться.

			array_2 берется по const ссылке, не модифицируется.
		*/
		template <class AT1, class AT2, class OP>
		static AT1 &AA_Op_Assign(AT1& array_1, const AT2& array_2, const OP &binary_action)
		{
			Apply_AA_1D_F2(array_1, array_2, binary_action);
			return array_1;
		}

		/*!
			\brief Создает новый массив result такого же размера, как array_1,
			применяет функтор f(&r, x, y) к тройкам из соответствующих элементов result, array_1 и array_2,
			возвращает result

			Размеры array_1 и array_2 должны совпадать.

			array_1, array_2 берутся по const ссылке, не модифицируются.
		*/
		template <class AT1, class AT2, class OP>
		static AT1 AA_Op_New(const AT1& array_1, const AT2& array_2, const OP &ternary_action)
		{
			AT1 result(array_1.sizes(0));
			Apply_AAA_1D_F3(result, array_1, array_2, ternary_action);
			return result;
		}

		/*!
			\brief Применяет функтор f(x, y, z) к тройкам из соответствующих элементов array_1 и array_2 и значения x,
			возвращает ссылку на array_1

			Размеры array_1 и array_2 должны совпадать.

			array_1 берется по const или не const ссылке, может модифицироваться.

			array_2, x берутся по const ссылке, не модифицируются.
		*/
		template <class AT1, class AT2, class ST, class OP>
		static AT1 &AAS_Op_Assign(AT1& array_1, const AT2& array_2, const ST& x, const OP &ternary_action)
		{
			Apply_AAS_1D_F3(array_1, array_2, x, ternary_action);
			return array_1;
		}

		/*!
			\brief Применяет функтор f(x, y, z) к тройкам из соответствующих элементов array_1, array_2 и array_3,
			возвращает ссылку на array_1

			Размеры array_1, array_2 и array_3 должны совпадать.

			array_1 берется по const или не const ссылке, может модифицироваться.

			array_2, array_3 берутся по const ссылке, не модифицируются.
		*/
		template <class AT1, class AT2, class AT3, class OP>
		static AT1 &AAA_Op_Assign(AT1& array_1, const AT2& array_2, const AT3& array_3, const OP &ternary_action)
		{
			Apply_AAA_1D_F3(array_1, array_2, array_3, ternary_action);
			return array_1;
		}

		/*!
			\brief Проверяет, содержат ли array_1, array_2 пару соответствующих элементов (с одинаковыми индексами),
			для которых f(x, y) == true

			Размеры array_1 и array_2 должны совпадать.

			\return
			- true - нужная пара элементов существует
			- false - нужной пары элементов не существует
		*/
		template <class AT1, class AT2, class OP>
		static bool AA_Any(const AT1& array_1, const AT2& array_2, const OP &binary_test)
		{
			return Apply_Any_AA_1D_RF2(array_1, array_2, binary_test);
		}
};

//--------------------------------------------------------------

} // namespace AlgebraicStructures
XRAD_END

//--------------------------------------------------------------
#endif // XRAD__algebraic_algorithms_data_array_h
