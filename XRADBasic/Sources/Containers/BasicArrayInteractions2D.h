/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file BasicArrayInteractions2D.h
//--------------------------------------------------------------
#ifndef XRAD__File_BasicArrayInteractions2D_h
#define XRAD__File_BasicArrayInteractions2D_h
/*!
	\file
	\brief Оптимизированные операции над двумерными массивами типа DataArray2D

	Данный файл не должен зависеть от DataArray2D. Он зависит только от соглашения
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
#include "BasicArrayInteractions1D.h"

XRAD_BEGIN

//--------------------------------------------------------------

template <int Dim, class Array>
size_t Size_A_2D(const Array &array)
{
	static_assert(Dim >= 0 && Dim <= 1, "Size_A_2D<Dim>: invalid Dim.");
	return array.sizes(Dim);
}

//--------------------------------------------------------------

/*!
	\details
	Этот шаблон может быть специализирован для типов массивов фиксированного размера,
	чтобы для них проверка выполнялась на стадии компиляции.
	Следует учесть, что Array1 и Array2 могут быть разными типами.
*/
template <class Array1, class Array2>
bool EqSizes_AA_2D(const Array1 &array_1, const Array2 &array_2)
{
	return Size_A_2D<0>(array_1) == Size_A_2D<0>(array_2) &&
			Size_A_2D<1>(array_1) == Size_A_2D<1>(array_2);
}

//--------------------------------------------------------------

template <class OpNameF, class Array1, class Array2>
void ECheckSizes_AA_2D(const Array1 &array_1, const Array2 &array_2)
{
	if (!EqSizes_AA_2D(array_1, array_2))
	{
		throw runtime_error(ssprintf("%s: array sizes do not match: (%zu, %zu) and (%zu, %zu).",
				EnsureType<const char*>(OpNameF::name()),
				EnsureType<size_t>(Size_A_2D<0>(array_1)), EnsureType<size_t>(Size_A_2D<1>(array_1)),
				EnsureType<size_t>(Size_A_2D<0>(array_2)), EnsureType<size_t>(Size_A_2D<1>(array_2))));
	}
}

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_A_2D_F1(Array &&array, Functor functor)
{
	if(array.steps(1) < array.steps(0))
	{
		for (size_t i = 0; i < array.sizes(0); ++i)
		{
			Apply_A_1D_F1(array.row(i), functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array.sizes(1); ++i)
		{
			Apply_A_1D_F1(array.col(i), functor);
		}
	}
}

//--------------------------------------------------------------

namespace Functors
{

template <class Functor>
class IA_2D_F3_1D_AdapterH
{
	public:
		IA_2D_F3_1D_AdapterH(size_t iv, const Functor &functor): iv(iv), functor(functor) {}

		template <class T>
		inline void operator() (size_t ih, T &&v) const
		{
			functor(iv, ih, v);
		}
	private:
		size_t iv;
		const Functor &functor;
};

template <class Functor>
class IA_2D_F3_1D_AdapterV
{
	public:
		IA_2D_F3_1D_AdapterV(size_t ih, const Functor &functor): ih(ih), functor(functor) {}

		template <class T>
		inline void operator() (size_t iv, T &&v) const
		{
			functor(iv, ih, v);
		}
	private:
		size_t ih;
		const Functor &functor;
};

} // namespace Functors

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_IA_2D_F3(Array &&array, Functor functor)
{
	if(array.steps(1) < array.steps(0))
	{
		for (size_t i = 0; i < array.sizes(0); ++i)
		{
			Apply_IA_1D_F2(array.row(i), Functors::IA_2D_F3_1D_AdapterH<Functor>(i, functor));
		}
	}
	else
	{
		for (size_t i = 0; i < array.sizes(1); ++i)
		{
			Apply_IA_1D_F2(array.col(i), Functors::IA_2D_F3_1D_AdapterV<Functor>(i, functor));
		}
	}
}

//--------------------------------------------------------------

template <class Array, class Functor>
void Apply_A_2D_RF1(Array &&array, Functor functor)
{
	if(array.steps(1) < array.steps(0))
	{
		for (size_t i = 0; i < array.sizes(0); ++i)
		{
			Apply_A_1D_RF1(array.row(i), functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array.sizes(1); ++i)
		{
			Apply_A_1D_RF1(array.col(i), functor);
		}
	}
}

//--------------------------------------------------------------

template <class Array, class Scalar, class Functor>
void Apply_AS_2D_F2(Array &&array, Scalar &&scalar, Functor functor)
{
	if(array.steps(1) < array.steps(0))
	{
		for (size_t i = 0; i < array.sizes(0); ++i)
		{
			Apply_AS_1D_F2(array.row(i), scalar, functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array.sizes(1); ++i)
		{
			Apply_AS_1D_F2(array.col(i), scalar, functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AA_2D_F2_name { static const char *name() { return "Apply_AA_2D_F2"; } };

template <class Array1, class Array2, class Functor>
void Apply_AA_2D_F2(Array1 &&array_1, Array2 &&array_2, Functor functor)
{
	ECheckSizes_AA_2D<Apply_AA_2D_F2_name>(array_1, array_2);

	if(array_1.steps(1) < array_1.steps(0))
	{
		for (size_t i = 0; i < array_1.sizes(0); ++i)
		{
			Apply_AA_1D_F2(array_1.row(i), array_2.row(i), functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array_1.sizes(1); ++i)
		{
			Apply_AA_1D_F2(array_1.col(i), array_2.col(i), functor);
		}
	}
}

//--------------------------------------------------------------

template <class Array1, class Array2, class Functor>
void Apply_AA_2D_Different_F2(Array1 &&array_1, Array2 &&array_2, Functor functor, extrapolation::method ex = extrapolation::by_zero)
{
	size_t size1 = Size_A_2D<0>(array_1);
	if (!size1 || !Size_A_2D<1>(array_1))
		return; // Пустой выходной массив
	size_t size2 = Size_A_2D<0>(array_2);
	if (size1 == size2)
	{
		if (Size_A_2D<1>(array_1) == Size_A_2D<1>(array_2))
		{
			Apply_AA_2D_F2(std::forward<Array1>(array_1), std::forward<Array2>(array_2), functor);
		}
		else
		{
			for (size_t i = 0; i < size1; ++i)
			{
				Apply_AA_1D_Different_F2(array_1.row(i), array_2.row(i), functor, ex);
			}
		}
		return;
	}
	if (!size2 || !Size_A_2D<1>(array_2))
	{
		// size1 > 0
		throw runtime_error("Apply_AA_2D_Different_F2: source array size is 0, destination array size > 0.");
	}

	size_t size_min = min(size1, size2);
	for (size_t i = 0; i < size_min; ++i)
	{
		Apply_AA_1D_Different_F2(array_1.row(i), array_2.row(i), functor, ex);
	}

	if (size1 <= size2)
		return;
	switch(ex)
	{
		case extrapolation::none:
			ForceDebugBreak();
			throw out_of_range("Apply_AA_2D_Different_F2, extrapolation is not allowed");

		case extrapolation::by_zero:
		{
			auto &sample_value = array_2.at(0,0);
			for (size_t i = size_min; i < size1; ++i)
			{
				Apply_AS_1D_F2(array_1.row(i), zero_value(sample_value), functor);
			}
			break;
		}

		case extrapolation::by_last_value:
		{
			auto &last_value = array_2.row(size_min-1);
			for (size_t i = size_min; i < size1; ++i)
			{
				Apply_AA_1D_Different_F2(array_1.row(i), last_value, functor, ex);
			}
			break;
		}

		case extrapolation::cyclic:
		{
			for (size_t i = size_min; i < size1; ++i)
			{
				Apply_AA_1D_Different_F2(array_1.row(i), array_2.row(i%size2), functor, ex);
			}
			break;
		}
	}
}

//--------------------------------------------------------------

struct Apply_Any_AA_2D_RF2_name { static const char *name() { return "Apply_Any_AA_2D_RF2"; } };

template <class Array1, class Array2, class Functor>
bool Apply_Any_AA_2D_RF2(Array1 &&array_1, Array2 &&array_2, Functor functor)
{
	ECheckSizes_AA_2D<Apply_Any_AA_2D_RF2_name>(array_1, array_2);

	if(array_1.steps(1) < array_1.steps(0))
	{
		for (size_t i = 0; i < array_1.sizes(0); ++i)
		{
			if (Apply_Any_AA_1D_RF2(array_1.row(i), array_2.row(i), functor))
				return true;
		}
		return false;
	}
	else
	{
		for (size_t i = 0; i < array_1.sizes(1); ++i)
		{
			if (Apply_Any_AA_1D_RF2(array_1.col(i), array_2.col(i), functor))
				return true;
		}
		return false;
	}
}

//--------------------------------------------------------------

struct Apply_AAA_2D_F3_name { static const char *name() { return "Apply_AAA_2D_F3"; } };

template <class Array1, class Array2, class Array3, class Functor>
void Apply_AAA_2D_F3(Array1 &&array_1, Array2 &&array_2, Array3 &&array_3, Functor functor)
{
	ECheckSizes_AA_2D<Apply_AAA_2D_F3_name>(array_1, array_2);
	ECheckSizes_AA_2D<Apply_AAA_2D_F3_name>(array_2, array_3);

	if(array_1.steps(1) < array_1.steps(0))
	{
		for (size_t i = 0; i < array_1.sizes(0); ++i)
		{
			Apply_AAA_1D_F3(array_1.row(i), array_2.row(i), array_3.row(i), functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array_1.sizes(1); ++i)
		{
			Apply_AAA_1D_F3(array_1.col(i), array_2.col(i), array_3.col(i), functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AAS_2D_F3_name { static const char *name() { return "Apply_AAS_2D_F3"; } };

template <class Array1, class Array2, class Scalar, class Functor>
void Apply_AAS_2D_F3(Array1 &&array_1, Array2 &&array_2, Scalar &&scalar, Functor functor)
{
	ECheckSizes_AA_2D<Apply_AAS_2D_F3_name>(array_1, array_2);

	if(array_1.steps(1) < array_1.steps(0))
	{
		for (size_t i = 0; i < array_1.sizes(0); ++i)
		{
			Apply_AAS_1D_F3(array_1.row(i), array_2.row(i), scalar, functor);
		}
	}
	else
	{
		for (size_t i = 0; i < array_1.sizes(1); ++i)
		{
			Apply_AAS_1D_F3(array_1.col(i), array_2.col(i), scalar, functor);
		}
	}
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_BasicArrayInteractions2D_h
