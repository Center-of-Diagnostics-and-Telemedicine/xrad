// file BasicArrayInteractionsMD.h
//--------------------------------------------------------------
#ifndef XRAD__File_BasicArrayInteractionsMD_h
#define XRAD__File_BasicArrayInteractionsMD_h
/*!
	\file
	\brief Оптимизированные операции над многомерными массивами типа DataArrayMD

	Данный файл не должен зависеть от DataArrayMD. Он зависит только от соглашения
	об _интерфейсе_ класса.

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
#include "BasicArrayInteractions2D.h"
#include "DataArrayAnalyze.h"
#include <vector>

XRAD_BEGIN

//--------------------------------------------------------------

template <class OpNameF, class Array>
void ECheckDimensions3_A_MD(const Array &array)
{
	if (array.n_dimensions() < 3)
	{
		throw runtime_error(ssprintf("%s: invalid array dimensions: %zu.",
				EnsureType<const char*>(OpNameF::name()),
				EnsureType<size_t>(array.n_dimensions())));
	}
}

//--------------------------------------------------------------

template <class OpNameF, class Array1, class Array2>
void ECheckDimensions_AA_MD(const Array1 &array_1, const Array2 &array_2)
{
	ECheckDimensions3_A_MD<OpNameF>(array_1);
	if (array_1.n_dimensions() != array_2.n_dimensions())
	{
		throw runtime_error(ssprintf("%s: array dimensions do not match: %zu and %zu",
				EnsureType<const char*>(OpNameF::name()),
				EnsureType<size_t>(array_1.n_dimensions()),
				EnsureType<size_t>(array_2.n_dimensions())));
	}
}

//--------------------------------------------------------------

template <class Array1, class Array2>
bool EqSizes_AA_MD(const Array1 &array_1, const Array2 &array_2)
{
	return array_1.sizes() == array_2.sizes();
}

//--------------------------------------------------------------

template <class OpNameF, class Array1, class Array2>
void ECheckSizes_AA_MD(const Array1 &array_1, const Array2 &array_2)
{
	ECheckDimensions3_A_MD<OpNameF>(array_1);
	if (!EqSizes_AA_MD(array_1, array_2))
	{
		throw runtime_error(ssprintf("%s: array sizes do not match: %s",
				EnsureType<const char*>(OpNameF::name()),
				EnsureType<const char*>(MDAAuxiliaries::size_compare_string(array_1.sizes(), array_2.sizes()).c_str())));
	}
}

//--------------------------------------------------------------

namespace MDAT_aux
{

/*!
	\brief Выбор среза многомерного массива размерностью на 1 меньше оригинала

	\param scan_dimension Номер исключенного измерения.
	\param subset_no Номер среза.
*/
template<class Array>
index_vector	GetSubsetMask(const Array &array, size_t scan_dimension, size_t subset_no)
{
	size_t	n_dimensions = array.n_dimensions();
	index_vector	subset_mask(array.sizes());

	for(size_t i = 0, dim = 0; i < n_dimensions; ++i)
	{
		if(i != scan_dimension)
		{
			subset_mask[i] =slice_mask(dim++);
		}
		else
		{
			subset_mask[i] = subset_no;
		}
	}
	return subset_mask;
}

template <class Array>
struct constness_types
{
	using array_type = Array;
	using slice_type = typename Array::slice_type;
	using row_type = typename Array::row_type;
};

template <class Array>
struct constness_types<const Array>
{
	using array_type = typename Array::invariable;
	using slice_type = typename Array::slice_type_invariable;
	using row_type = typename Array::row_type_invariable;
};

} // namespace MDAT_aux

//--------------------------------------------------------------

struct Apply_A_MD_F1_name { static const char *name() { return "Apply_A_MD_F1"; } };

template <class Array, class Functor>
void Apply_A_MD_F1(Array &array, Functor functor)
{
	if(array.empty())
	{
		// аргумент пустой, выходим без последствий
		return;
	}
	ECheckDimensions3_A_MD<Apply_A_MD_F1_name>(array);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array.steps(), &scan_dimension);
	const size_t	scan_size = array.sizes(scan_dimension);
	size_t	n_dimensions = array.n_dimensions();
	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array, scan_dimension, i);
		if(n_dimensions > 3)
		{
			typename MDAT_aux::constness_types<Array>::array_type subset;
			array.GetSubset(subset, subset_mask);
			// рекурсия
			Apply_A_MD_F1(subset, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array>::slice_type slice;
			array.GetSlice(slice, subset_mask);
			// конец рекурсии, обработка двумерного среза
			Apply_A_2D_F1(slice, functor);
		}
	}
}

//--------------------------------------------------------------

namespace MDAT_aux
{

template <class Array, class Functor>
void Apply_IA_MD_F2_helper(Array &array, const Functor &functor_1d,
		index_vector *index,
		const vector<size_t> &ordered_dimensions,
		size_t ordered_dimension_index)
{
	size_t dimension = ordered_dimensions[ordered_dimension_index];
	if (ordered_dimension_index + 1 < ordered_dimensions.size())
	{
		size_t scan_size = array.sizes(dimension);
		for (size_t i = 0; i < scan_size; ++i)
		{
			(*index)[dimension] = i;
			Apply_IA_MD_F2_helper(array, functor_1d, index, ordered_dimensions, ordered_dimension_index + 1);
		}
	}
	else
	{
		// Записываем в (*index)[dimension] флаг slice_mask(0) для получения одномерного скана по измерению dimension.
		(*index)[dimension] = slice_mask(0);
		typename MDAT_aux::constness_types<Array>::row_type row;
		array.GetRow(row, *index);
		// Здесь (*index)[dimension] будет перезаписываться при обращениях к functor_1d.
		Apply_IA_1D_F2(row, functor_1d);
	}
}

template <class Functor>
class IA_MD_F2_1D_Adapter
{
	public:
		IA_MD_F2_1D_Adapter(const Functor &functor, index_vector *index_md, size_t run_dimension):
			functor(functor), index_md(index_md), run_index(&(*index_md)[run_dimension]) {}

		template <class T>
		inline void operator() (size_t i, T &&v) const
		{
			*run_index = i;
			functor(*index_md, v);
		}
	private:
		const Functor &functor;
		index_vector *index_md;
		index_vector::value_type *run_index;
};

} // namespace MDAT_aux

struct Apply_IA_MD_F2_name { static const char *name() { return "Apply_IA_MD_F2"; } };

template <class Array, class Functor>
void Apply_IA_MD_F2(Array &array, Functor functor)
{
	ECheckDimensions3_A_MD<Apply_IA_MD_F2_name>(array);
	// Выполняем проход по измерениям массива в порядке убывания шага по измерению для оптимизации обращения к памяти.
	size_t	n_dimensions = array.n_dimensions();
	vector<pair<size_t, size_t>> sort_index(n_dimensions);
	auto steps = array.steps();
	for (size_t i = 0; i < n_dimensions; ++i)
		sort_index[i] = make_pair(steps[i], i);
	sort(sort_index.begin(), sort_index.end());
	vector<size_t> ordered_dimensions(n_dimensions);
	for (size_t i = 0; i < n_dimensions; ++i)
		ordered_dimensions[i] = sort_index[n_dimensions - 1 - i].second;
	index_vector index(array.n_dimensions(), 0);
	MDAT_aux::Apply_IA_MD_F2_helper(array,
			MDAT_aux::IA_MD_F2_1D_Adapter<Functor>(functor, &index, ordered_dimensions.back()),
			&index, ordered_dimensions, 0);
}

//--------------------------------------------------------------

struct Apply_A_MD_RF1_name { static const char *name() { return "Apply_A_MD_RF1"; } };

template <class Array, class Functor>
void Apply_A_MD_RF1(Array &array, Functor functor)
{
	if(array.empty())
	{
		// аргумент пустой, выходим без последствий
		return;
	}
	ECheckDimensions3_A_MD<Apply_A_MD_RF1_name>(array);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array.steps(), &scan_dimension);
	const size_t	scan_size = array.sizes(scan_dimension);
	size_t	n_dimensions = array.n_dimensions();
	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array, scan_dimension, i);
		if(n_dimensions > 3)
		{
			typename MDAT_aux::constness_types<Array>::array_type subset;
			array.GetSubset(subset, subset_mask);
			// рекурсия
			Apply_A_MD_RF1(subset, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array>::slice_type slice;
			array.GetSlice(slice, subset_mask);
			// конец рекурсии, обработка двумерного среза
			Apply_A_2D_RF1(slice, functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AS_MD_F2_name { static const char *name() { return "Apply_AS_MD_F2"; } };

template <class Array, class Scalar, class Functor>
void Apply_AS_MD_F2(Array &array, Scalar &scalar, Functor functor)
{
	if(array.empty())
	{
		// аргумент пустой, выходим без последствий
		return;
	}
	ECheckDimensions3_A_MD<Apply_AS_MD_F2_name>(array);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array.steps(), &scan_dimension);
	const size_t	scan_size = array.sizes(scan_dimension);
	size_t	n_dimensions = array.n_dimensions();
	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array, scan_dimension, i);
		if(n_dimensions > 3)
		{
			typename MDAT_aux::constness_types<Array>::array_type subset;
			array.GetSubset(subset, subset_mask);
			// рекурсия
			Apply_AS_MD_F2(subset, scalar, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array>::slice_type slice;
			array.GetSlice(slice, subset_mask);
			// конец рекурсии, обработка двумерного среза
			Apply_AS_2D_F2(slice, scalar, functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AA_MD_F2_name { static const char *name() { return "Apply_AA_MD_F2"; } };

template <class Array1, class Array2, class Functor>
void Apply_AA_MD_F2(Array1 &array_1, Array2 &array_2, Functor functor)
{
	if(array_1.empty() && array_2.empty())
	{
		// оба аргумента пустые, выходим без последствий
		return;
	}
	ECheckSizes_AA_MD<Apply_AA_MD_F2_name>(array_1, array_2);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array_1.steps(), &scan_dimension);
	const size_t	scan_size = array_1.sizes(scan_dimension);

	size_t	n_dimensions = array_1.n_dimensions();

	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);

		if(n_dimensions>3)
		{
			typename MDAT_aux::constness_types<Array1>::array_type	subset_1;
			typename MDAT_aux::constness_types<Array2>::array_type	subset_2;

			array_1.GetSubset(subset_1, subset_mask);
			array_2.GetSubset(subset_2, subset_mask);

			// рекурсия
			Apply_AA_MD_F2(subset_1, subset_2, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
			typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;

			array_1.GetSlice(slice_1, subset_mask);
			array_2.GetSlice(slice_2, subset_mask);

			// конец рекурсии, обработка двумерного среза
			Apply_AA_2D_F2(slice_1, slice_2, functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AA_MD_Different_F2_name { static const char *name() { return "Apply_AA_MD_Different_F2"; } };

template <class Array1, class Array2, class Functor>
void Apply_AA_MD_Different_F2(Array1 &array_1, Array2 &array_2, Functor functor, extrapolation::method ex = extrapolation::by_zero)
{
	if(array_1.empty() && array_2.empty())
	{
		// оба аргумента пустые, выходим без последствий
		return;
	}
	ECheckDimensions_AA_MD<Apply_AA_MD_Different_F2_name>(array_1, array_2);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array_1.steps(), &scan_dimension);
	const size_t	scan_size = array_1.sizes(scan_dimension);
	const size_t	s_min = min(scan_size, array_2.sizes(scan_dimension));
	size_t	n_dimensions = array_1.n_dimensions();
	for(size_t i = 0; i < s_min; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);

		if(n_dimensions > 3)
		{
			// рекурсия
			typename MDAT_aux::constness_types<Array1>::array_type subset_1;
			typename MDAT_aux::constness_types<Array2>::array_type subset_2;

			array_1.GetSubset(subset_1, subset_mask);
			array_2.GetSubset(subset_2, subset_mask);

			Apply_AA_MD_Different_F2(subset_1, subset_2, functor, ex);
		}
		else
		{
			typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
			typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;

			array_1.GetSlice(slice_1, subset_mask);
			array_2.GetSlice(slice_2, subset_mask);

			// конец рекурсии
			Apply_AA_2D_Different_F2(slice_1, slice_2, functor, ex);
		}
	}
	for(size_t i = s_min; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);
		switch(ex)
		{
			case extrapolation::none:
				ForceDebugBreak();
				throw out_of_range("Apply_AA_MD_Different_F2, extrapolation is not allowed");

			case extrapolation::by_zero:
			{
				auto	zero = zero_value(array_1.at(index_vector(n_dimensions, 0)));
				if(n_dimensions > 3)
				{
					typename MDAT_aux::constness_types<Array1>::array_type subset_1;
					array_1.GetSubset(subset_1, subset_mask);
					subset_1.fill(zero);
				}
				else
				{
					typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
					array_1.GetSlice(slice_1, subset_mask);
					slice_1.fill(zero);
				}
			}
			break;

			case extrapolation::by_last_value:
			case extrapolation::cyclic:
			{
				index_vector	subset_mask2;
				if(ex==extrapolation::by_last_value)
					subset_mask2 = MDAT_aux::GetSubsetMask(array_2, scan_dimension, s_min-1);
				else
					subset_mask2 = MDAT_aux::GetSubsetMask(array_2, scan_dimension, i%s_min);

				if(n_dimensions > 3)
				{
					typename MDAT_aux::constness_types<Array1>::array_type subset_1;
					typename MDAT_aux::constness_types<Array2>::array_type subset_2;
					array_1.GetSubset(subset_1, subset_mask);
					array_2.GetSubset(subset_2, subset_mask2);
					// рекурсия
					Apply_AA_MD_Different_F2(subset_1, subset_2, functor, ex);
				}
				else
				{
					typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
					typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;
					array_1.GetSlice(slice_1, subset_mask);
					array_2.GetSlice(slice_2, subset_mask2);
					// конец рекурсии
					Apply_AA_2D_Different_F2(slice_1, slice_2, functor, ex);
				}
			}
		}
	}
}

//--------------------------------------------------------------

struct Apply_Any_AA_MD_RF2_name { static const char *name() { return "Apply_Any_AA_MD_RF2"; } };

template <class Array1, class Array2, class Functor>
bool Apply_Any_AA_MD_RF2(Array1 &&array_1, Array2 &&array_2, Functor functor)
{
	if(array_1.empty() && array_2.empty())
	{
		return false;
	}
	ECheckSizes_AA_MD<Apply_AA_MD_F2_name>(array_1, array_2);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array_1.steps(), &scan_dimension);
	const size_t	scan_size = array_1.sizes(scan_dimension);

	size_t	n_dimensions = array_1.n_dimensions();

	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);

		if(n_dimensions>3)
		{
			typename MDAT_aux::constness_types<Array1>::array_type subset_1;
			typename MDAT_aux::constness_types<Array2>::array_type subset_2;

			array_1.GetSubset(subset_1, subset_mask);
			array_2.GetSubset(subset_2, subset_mask);

			// рекурсия
			if (Apply_Any_AA_MD_RF2(subset_1, subset_2, functor))
				return true;
		}
		else
		{
			typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
			typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;

			array_1.GetSlice(slice_1, subset_mask);
			array_2.GetSlice(slice_2, subset_mask);

			// конец рекурсии, обработка двумерного среза
			if (Apply_Any_AA_2D_RF2(slice_1, slice_2, functor))
				return true;
		}
	}
	return false;
}

//--------------------------------------------------------------

struct Apply_AAA_MD_F3_name { static const char *name() { return "Apply_AAA_MD_F3"; } };

template <class Array1, class Array2, class Array3, class Functor>
void Apply_AAA_MD_F3(Array1 &array_1, Array2 &array_2, Array3 &array_3, Functor functor)
{
	if(array_1.empty() && array_2.empty() && array_3.empty())
	{
		// все аргументы пустые, выходим без последствий
		return;
	}
	ECheckSizes_AA_MD<Apply_AAA_MD_F3_name>(array_1, array_2);
	ECheckSizes_AA_MD<Apply_AAA_MD_F3_name>(array_2, array_3);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array_1.steps(), &scan_dimension);
	const size_t	scan_size = array_1.sizes(scan_dimension);

	size_t	n_dimensions = array_1.n_dimensions();

	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);

		if(n_dimensions>3)
		{
			typename MDAT_aux::constness_types<Array1>::array_type subset_1;
			typename MDAT_aux::constness_types<Array2>::array_type subset_2;
			typename MDAT_aux::constness_types<Array3>::array_type subset_3;

			array_1.GetSubset(subset_1, subset_mask);
			array_2.GetSubset(subset_2, subset_mask);
			array_3.GetSubset(subset_3, subset_mask);

			// рекурсия
			Apply_AAA_MD_F3(subset_1, subset_2, subset_3, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
			typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;
			typename MDAT_aux::constness_types<Array3>::slice_type	slice_3;

			array_1.GetSlice(slice_1, subset_mask);
			array_2.GetSlice(slice_2, subset_mask);
			array_3.GetSlice(slice_3, subset_mask);

			// конец рекурсии, обработка двумерного среза
			Apply_AAA_2D_F3(slice_1, slice_2, slice_3, functor);
		}
	}
}

//--------------------------------------------------------------

struct Apply_AAS_MD_F3_name { static const char *name() { return "Apply_AAS_MD_F3"; } };

template <class Array1, class Array2, class Scalar, class Functor>
void Apply_AAS_MD_F3(Array1 &array_1, Array2 &array_2, Scalar &scalar, Functor functor)
{
	if(array_1.empty() && array_2.empty())
	{
		// оба аргумента пустые, выходим без последствий
		return;
	}
	ECheckSizes_AA_MD<Apply_AAS_MD_F3_name>(array_1, array_2);

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MaxValue(array_1.steps(), &scan_dimension);
	const size_t	scan_size = array_1.sizes(scan_dimension);

	size_t	n_dimensions = array_1.n_dimensions();

	for(size_t i = 0; i < scan_size; ++i)
	{
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array_1, scan_dimension, i);

		if(n_dimensions>3)
		{
			typename MDAT_aux::constness_types<Array1>::array_type subset_1;
			typename MDAT_aux::constness_types<Array2>::array_type subset_2;

			array_1.GetSubset(subset_1, subset_mask);
			array_2.GetSubset(subset_2, subset_mask);

			// рекурсия
			Apply_AAS_MD_F3(subset_1, subset_2, scalar, functor);
		}
		else
		{
			typename MDAT_aux::constness_types<Array1>::slice_type	slice_1;
			typename MDAT_aux::constness_types<Array2>::slice_type	slice_2;

			array_1.GetSlice(slice_1, subset_mask);
			array_2.GetSlice(slice_2, subset_mask);

			// конец рекурсии, обработка двумерного среза
			Apply_AAS_2D_F3(slice_1, slice_2, scalar, functor);
		}
	}
}

//--------------------------------------------------------------

#if 1
namespace MDAT_aux
{

//#error This roll procedure is buggy.
// Процедура на 4-мерном массиве дважды ротирует данные по некоторым измерениям.
// Как эта процедура будет работать при нечетном количестве измерений?

// как бы там ни было, на 3 измерениях она работала корректно, а в проектах эта процедура нужна сейчас

template <class A2DT>
void	roll_utility_md(DataArrayMD<A2DT> &array, const offset_vector &roll_vector, const index_vector &roll_priority)
{
	XRAD_ASSERT_THROW(!array.empty());
	size_t	n_dimensions = array.n_dimensions();
	if (n_dimensions<3)
	{
		throw invalid_argument(
			string("Invalid roll MD:") +
			string("\nInvalid array dimensions") +
			MDAAuxiliaries::index_string(array.sizes()) +
			string("\nOperand1 type =") + typeid(DataArrayMD<A2DT>).name());
	}

	// находим размерность с максимальным шагом, и ее исключаем в первую очередь. тогда есть надежда,
	// что окончательная обработка двумерных срезов будет хорошо оптимизирована
	size_t	scan_dimension;
	MinValue(roll_priority, &scan_dimension);
	const size_t	scan_size = array.sizes(scan_dimension);

	offset_vector	subset_roll_vector(n_dimensions-1);
	index_vector	subset_roll_priority(n_dimensions-1);
	index_vector	roll_priority_remaining(roll_priority);

	size_t	dim1, dim2;
	size_t	n_dim_remaining(0);
	if(MaxValue(roll_priority_remaining, &dim1)) roll_priority_remaining[dim1] = 0, ++n_dim_remaining;
	if(MaxValue(roll_priority_remaining, &dim2)) roll_priority_remaining[dim2] = 0, ++n_dim_remaining;
	if(!n_dim_remaining) return;

	for(size_t i = 0; i < scan_size; ++i)
	{
		//
		index_vector	subset_mask = MDAT_aux::GetSubsetMask(array, scan_dimension, i);
		for(size_t j = 0; j < n_dimensions; ++j)
		{
			size_t	sm = subset_mask[j];
			if(is_slice_mask(sm))
			{
				subset_roll_priority[dimension_no(sm)] = roll_priority[j];
				subset_roll_vector[dimension_no(sm)] = roll_priority[j] ? roll_vector[j]:0;
			}
		}

		if(n_dimensions > 3)
		{
			DataArrayMD<A2DT>	subset;
			array.GetSubset(subset, subset_mask);
			// рекурсия
			roll_utility_md(subset, subset_roll_vector, subset_roll_priority);
		}
		else
		{
			A2DT slice;
			array.GetSlice(slice, subset_mask);
			// конец рекурсии, обработка двумерного среза
			slice.roll(subset_roll_vector[0], subset_roll_vector[1]);
		}
	}
	//еще одна рекурсия
	roll_utility_md(array, roll_vector, roll_priority_remaining);
}

} // namespace MDAT_aux

//--------------------------------------------------------------

template <class A2DT>
void	roll(DataArrayMD<A2DT> &array, const offset_vector &in_roll_vector)
{
	index_vector	roll_priority(array.n_dimensions(), 0);
	index_vector	roll_vector(array.n_dimensions());
	for(size_t i = 0; i < array.n_dimensions(); ++i)
	{
		if(in_roll_vector[i]) roll_priority[i] = i+1;
		roll_vector[i] = (array.sizes(i) + in_roll_vector[i])%array.sizes(i);
	}
	MDAT_aux::roll_utility_md(array, roll_vector, roll_priority);
}
#endif

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_BasicArrayInteractionsMD_h
