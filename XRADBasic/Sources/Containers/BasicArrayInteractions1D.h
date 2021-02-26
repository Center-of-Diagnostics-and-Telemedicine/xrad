// file BasicArrayInteractions1D.h
//--------------------------------------------------------------
#ifndef XRAD__File_BasicArrayInteractions1D_h
#define XRAD__File_BasicArrayInteractions1D_h
/*!
	\file
	\brief Оптимизированные операции над одномерными массивами типа DataArray

	Данный файл не должен зависеть от DataArray. Он зависит только от соглашения
	об _интерфейсе_ класса.

	Некоторые функции могут иметь специализации для конкретных типов.
	Такие специализации должны быть реализованы в файлах этих типов, а не здесь.

	В функциях ниже используются шаблоны с универальными ссылками вида:

	~~~~
	template <class T>
	void Op(T &&x);
	~~~~

	Здесь T &&x означает не rvalue-ссылку, а универсальную ссылку, см. стандарт C++11.
	Такие шаблоны могут связываться со всеми типами ссылок: const, не const,
	volatile и т.д. Возможность связывания с rvalue-ссылками здесь не так важна.

	Эти шаблоны позволяют реализовать операции, модифицирующие не обязательно
	первый массив, или не модифицирующие массивы вовсе (все массивы могут быть const).

	\todo На самом деле достаточно просто ссылок?
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "ContainersBasic.h"

XRAD_BEGIN

//--------------------------------------------------------------

template <class Array>
size_t Size_A_1D(const Array &array)
{
	return array.sizes(0);
}

//--------------------------------------------------------------

template <class Array1, class Array2>
bool EqSizes_AA_1D(const Array1 &array_1, const Array2 &array_2)
{
	return Size_A_1D(array_1) == Size_A_1D(array_2);
}

//--------------------------------------------------------------

/*!
	\brief Шаблон, определяющий, является ли тип массивом с фиксированным размером,
	определяет constexpr bool value = true или false

	Общий шаблон возвращает false. Специализация для массивов фиксированного размера возвращает true.

	Признак фиксированного размера определяется взаимно однозначно
	по наличию объявления функции FixedSize_A_1D от _указателя_ на массив заданного типа.

	Возвращаемое значение функции FixedSize_A_1D должно иметь тип integral_constant<size_t, N>,
	где N — размер массива. При этом тело функции определять не нужно.
*/
template <class Array, class = void>
class is_fixed_size_a_1d: public std::false_type
{
};

//! \brief Специализация, задающая true для массивов, от указателя на которые
//! определена функция FixedSize_A_1D.
template <class Array>
class is_fixed_size_a_1d<Array,
	void_t<
			decltype(FixedSize_A_1D(std::declval<std::remove_cv_t<Array>*>()))
			>>:
	public std::true_type
{
};

//! \brief Вспомогательный класс для реализации ECheckSizes_AA_1D. Имеет две специализации:
//! для is_fixed = true и для is_fixed = false. Специализации определяют статический метод Check
template <bool is_fixed>
struct ECheckSizes_AA_1D_helper;

//! \brief Специализация для массивов фиксированного размера. Метод Check вызывает ошибку компиляции,
//! если размеры массивов не совпадают
//!
//! Для этих массивов должна быть определена функция FixedSize_A_1D от указателя на массив,
//! тип возвращаемого значения — integral_constant<size_t, N>, где N — размер массива. Сама функция
//! не вызывается, используется только её тип.
template <>
struct ECheckSizes_AA_1D_helper<true>
{
	template <class OpNameF, class Array1, class Array2>
	static void Check(const Array1 &array_1, const Array2 &array_2)
	{
		static_assert(decltype(FixedSize_A_1D(std::declval<std::remove_cv_t<Array1>*>()))::value ==
				decltype(FixedSize_A_1D(std::declval<std::remove_cv_t<Array2>*>()))::value,
				"Sizes of two arrays do not match.");
		(void)array_1;
		(void)array_2;
	}
};

//! \brief Специализация для массивов динамического размера. Метод Check вызывает исключение при исполнении,
//! если размеры массивов не совпадают
template <>
struct ECheckSizes_AA_1D_helper<false>
{
	template <class OpNameF, class Array1, class Array2, class = void>
	static void Check(const Array1 &array_1, const Array2 &array_2)
	{
		if (!EqSizes_AA_1D(array_1, array_2))
		{
			throw runtime_error(ssprintf("%s: array sizes do not match: %zu and %zu.",
					EnsureType<const char*>(OpNameF::name()),
					EnsureType<size_t>(Size_A_1D(array_1)),
					EnsureType<size_t>(Size_A_1D(array_2))));
		}
	}
};

/*!
	\brief Функция проверки совпадения размеров массивов. При несовпадении размеров,
	если оба массива фиксированного размера, вызывает ошибку при компиляции, иначе исключение при исполнении

	Признак фиксированного размера массива определяется шаблоном is_fixed_size_a_1d.
*/
template <class OpNameF, class Array1, class Array2>
void ECheckSizes_AA_1D(const Array1 &array_1, const Array2 &array_2)
{
	ECheckSizes_AA_1D_helper<is_fixed_size_a_1d<Array1>::value && is_fixed_size_a_1d<Array2>::value>::
			template Check<OpNameF>(array_1, array_2);
}

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_A_1D_F1(Array &&array, Functor functor)
{
	auto it = array.begin();
	auto ie = array.end();

	for(; it != ie; ++it)
	{
		functor(*it);
	}
}

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_A_1D_RF1(Array &&array, Functor functor)
{
	auto it = array.begin();
	auto ie = array.end();

	for(; it != ie; ++it)
	{
		*it = functor(*it);
	}
}

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_IA_1D_F2(Array &&array, Functor functor, size_t start_index, size_t end_index)
{
	if (end_index <= start_index)
		return;
	auto it = array.begin();
	if (start_index)
		it += start_index;

	for (size_t i = start_index; i != end_index; ++i, ++it)
	{
		functor(i, *it);
	}
}

template <class Array, class Functor>
void Apply_IA_1D_F2(Array &&array, Functor functor)
{
	Apply_IA_1D_F2(array, functor, 0, array.size());
}

//--------------------------------------------------------------

template <class Array, class Scalar, class Functor>
void Apply_AS_1D_F2(Array &&array, Scalar &&scalar, Functor functor)
{
	auto it = array.begin();
	auto ie = array.end();

	for(; it != ie; ++it)
	{
		functor(*it, scalar);
	}
}

//--------------------------------------------------------------

struct Apply_AA_1D_F2_name { static const char *name() { return "Apply_AA_1D_F2"; } };

template <class Array1, class Array2, class Functor>
void Apply_AA_1D_F2(Array1 &&array_1, Array2 &&array_2, Functor functor)
{
	ECheckSizes_AA_1D<Apply_AA_1D_F2_name>(array_1, array_2);

	auto it1 = array_1.begin();
	auto ie1 = array_1.end();
	auto it2 = array_2.begin();

	for(; it1 != ie1; ++it1, ++it2)
	{
		functor(*it1, *it2);
	}
}

//--------------------------------------------------------------

template <class Array1, class Array2, class Functor>
void Apply_AA_1D_Different_F2(Array1 &&array_1, Array2 &&array_2, Functor functor, extrapolation::method ex = extrapolation::by_zero)
{
	size_t size1 = Size_A_1D(array_1);
	size_t size2 = Size_A_1D(array_2);
	if (size1 == size2)
	{
		Apply_AA_1D_F2(array_1, array_2, functor);
		return;
	}
	if (size2 == 0)
	{
		// size1 > 0
		throw runtime_error("Apply_AA_1D_Different_F2: source array size is 0, destination array size > 0.");
	}
	auto it1 = array_1.begin();
	auto it2 = array_2.begin();
	size_t size_min = min(size1, size2);

	for(size_t i = 0; i != size_min; ++it1, ++it2, ++i)
	{
		functor(*it1, *it2);
	}

	if (size1 <= size2)
		return;
	switch(ex)
	{
		case extrapolation::none:
			ForceDebugBreak();
			throw out_of_range("Apply_AA_1D_Different_F2, extrapolation is not allowed");

		case extrapolation::by_zero:
		{
			// it2 == array_2.end(), size2 > 0
			--it2;
			auto &last_value = *it2;
			for(size_t i = size_min; i < size1; ++i, ++it1)
			{
				functor(*it1, zero_value(last_value));
			}
			break;
		}

		case extrapolation::by_last_value:
		{
			// it2 == array_2.end(), size2 > 0
			--it2;
			auto &last_value = *it2;
			for(size_t i = size_min; i < size1; ++i, ++it1)
			{
				functor(*it1, last_value);
			}
			break;
		}

		case extrapolation::cyclic:
		{
			for(size_t i = size_min; i < size1; ++i, ++it1)
			{
				functor(*it1, array_2[i%size2]);
			}
			break;
		}
	}
}

//--------------------------------------------------------------

struct Apply_Any_AA_1D_RF2_name { static const char *name() { return "Apply_Any_AA_1D_RF2"; } };

template <class Array1, class Array2, class Functor>
bool Apply_Any_AA_1D_RF2(Array1 &&array_1, Array2 &&array_2, Functor functor)
{
	ECheckSizes_AA_1D<Apply_Any_AA_1D_RF2_name>(array_1, array_2);

	auto it1 = array_1.begin();
	auto ie1 = array_1.end();
	auto it2 = array_2.begin();

	for(; it1 != ie1; ++it1, ++it2)
	{
		if (functor(*it1, *it2))
			return true;
	}
	return false;
}

//--------------------------------------------------------------

struct Apply_AAA_1D_F3_name { static const char *name() { return "Apply_AAA_1D_F3"; } };

template <class Array1, class Array2, class Array3, class Functor>
void Apply_AAA_1D_F3(Array1 &&array_1, Array2 &&array_2, Array3 &&array_3, Functor functor)
{
	ECheckSizes_AA_1D<Apply_AAA_1D_F3_name>(array_1, array_2);
	ECheckSizes_AA_1D<Apply_AAA_1D_F3_name>(array_2, array_3);

	auto it1 = array_1.begin();
	auto ie1 = array_1.end();
	auto it2 = array_2.begin();
	auto it3 = array_3.begin();

	for(; it1 != ie1; ++it1, ++it2, ++it3)
	{
		functor(*it1, *it2, *it3);
	}
}

//--------------------------------------------------------------

struct Apply_AAS_1D_F3_name { static const char *name() { return "Apply_AAS_1D_F3"; } };

template <class Array1, class Array2, class Scalar, class Functor>
void Apply_AAS_1D_F3(Array1 &&array_1, Array2 &&array_2, Scalar &&scalar, Functor functor)
{
	ECheckSizes_AA_1D<Apply_AAS_1D_F3_name>(array_1, array_2);

	auto it1 = array_1.begin();
	auto ie1 = array_1.end();
	auto it2 = array_2.begin();

	for(; it1 != ie1; ++it1, ++it2)
	{
		functor(*it1, *it2, scalar);
	}
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_BasicArrayInteractions1D_h
