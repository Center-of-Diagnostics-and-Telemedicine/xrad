/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_DataArrayAnalyzeMD_h
#define XRAD__File_DataArrayAnalyzeMD_h
/*!
	\file
	\date 2014-10-10 17:48
	\author kns
	\brief Общие инструмены для анализа многомерных массивов (DataArrayMD)
*/
//--------------------------------------------------------------

#include "DataArrayMD.h"
#include "BasicArrayInteractionsMD.h"
#include "IndexVector.h"
#include "DataArrayAnalyze2D.h"
#include "DataArrayHistogram2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

namespace Functors
{

template <class Predicate, class R>
class find_imd_v
{
	public:
		typedef typename std::remove_cv<R>::type nonconst_R;

		find_imd_v(index_vector *found_index, nonconst_R *found_value):
			found_index(found_index), found_value(found_value)
		{
		}
		// Оператор для поиска без индекса
		template <class T>
		void operator() (const T &v) const
		{
			if (!Predicate::test(*found_value, v))
				return;
			*found_value = v;
		}
		// Оператор для поиска с индексом
		template <class T>
		void operator() (const index_vector &index, const T &v) const
		{
			if (!Predicate::test(*found_value, v))
				return;
			found_index->MakeCopy(index);
			*found_value = v;
		}
	private:
		index_vector *found_index;
		nonconst_R *found_value;
};

template <class Predicate, class R, class Functor>
class find_transformed_imd_v
{
	public:
		typedef typename std::remove_cv<R>::type nonconst_R;

		find_transformed_imd_v(index_vector *found_index, nonconst_R *found_value, const Functor &functor):
			found_index(found_index), found_value(found_value), functor(functor)
		{
		}
		// Оператор для поиска без индекса
		template <class T>
		void operator() (const T &v) const
		{
			auto transformed_v = functor(v);
			if (!Predicate::test(*found_value, transformed_v))
				return;
			*found_value = transformed_v;
		}
		// Оператор для поиска с индексом
		template <class T>
		void operator() (const index_vector &index, T &&v) const
		{
			auto transformed_v = functor(v);
			if (!Predicate::test(*found_value, transformed_v))
				return;
			found_index->MakeCopy(index);
			*found_value = transformed_v;
		}
	private:
		index_vector *found_index;
		nonconst_R *found_value;
		Functor functor;
};

} // namespace Functors

//--------------------------------------------------------------
//
//	Простейший анализ содержимого
//


template<class FindFunctor, class A2T>
auto FindValue(const DataArrayMD<A2T> &array, index_vector *position_ptr = NULL)
	-> typename DataArrayMD<A2T>::value_type
{
	typedef typename std::remove_cv<typename DataArrayMD<A2T>::value_type>::type result_type;

	if (array.empty())
		throw invalid_argument("FindValue<>: empty array.");
	index_vector position(array.n_dimensions(), 0);
	result_type value(array.at(position));
	if (!position_ptr)
	{
		// Особый случай: индекс не нужен.
		Apply_A_MD_F1(array, FindFunctor(nullptr, &value));
		return value;
	}
	Apply_IA_MD_F2(array, FindFunctor(&position, &value));
	position_ptr->MakeCopy(position);
	return value;
}

template<class A2T>
auto MaxValue(const DataArrayMD<A2T> &array, index_vector *position_ptr = NULL)
	-> typename A2T::value_type
{
	typedef typename A2T::value_type result_type;
	return FindValue<Functors::find_imd_v<Functors::find_predicate_max, result_type>>(
			array, position_ptr);
}

template<class A2T>
auto MinValue(const DataArrayMD<A2T> &array, index_vector *position_ptr = NULL)
	-> typename A2T::value_type
{
	typedef typename A2T::value_type result_type;
	return FindValue<Functors::find_imd_v<Functors::find_predicate_min, result_type>>(
			array, position_ptr);
}

//--------------------------------------------------------------

template<class FindTransformedFunctor, class A2T, class F>
auto	FindValueTransformed(const DataArrayMD<A2T> &array, const F& function, index_vector *position_ptr = NULL)
	-> Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>;

	if (array.empty())
		throw invalid_argument("FindValueTransformed<>: empty array.");
	index_vector position(array.n_dimensions(), 0);
	result_type value(function(array.at(position)));
	if (!position_ptr)
	{
		// Особый случай: индекс не нужен.
		Apply_A_MD_F1(array, FindTransformedFunctor(nullptr, &value, function));
		return value;
	}
	Apply_IA_MD_F2(array, FindTransformedFunctor(&position, &value, function));
	position_ptr->MakeCopy(position);
	return value;
}

template<class A2T, class F>
auto MaxValueTransformed(const DataArrayMD<A2T> &array, const F& function, index_vector *position_ptr = NULL)
	-> Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>;
	return FindValueTransformed<Functors::find_transformed_imd_v<Functors::find_predicate_max, result_type, F>>(
			array, function, position_ptr);
}

template<class A2T, class F>
auto MinValueTransformed(const DataArrayMD<A2T> &array, const F& function, index_vector *position_ptr = NULL)
	-> Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArrayMD<A2T>::value_type>;
	return FindValueTransformed<Functors::find_transformed_imd_v<Functors::find_predicate_min, result_type, F>>(
			array, function, position_ptr);
}

//--------------------------------------------------------------

template<class A2T>
double	MaxAbsoluteValue(const DataArrayMD<A2T> &array, index_vector *position_ptr = NULL)
{
	typedef typename A2T::value_type value_type;
	return MaxValueTransformed(array, Functors::absolute_value(), position_ptr);
}

template<class A2T>
double	MinAbsoluteValue(const DataArrayMD<A2T> &array, index_vector *position_ptr = NULL)
{
	typedef typename A2T::value_type value_type;
	return MinValueTransformed(array, Functors::absolute_value(), position_ptr);
}


template<class A2T>
floating64_type<typename A2T::value_type>	ElementSum(const DataArrayMD<A2T> &array)
{
	typedef typename A2T::value_type value_type;
	typedef floating64_type<value_type> result_type;

	if (array.empty())
		return result_type(0);

	result_type result = zero_value(value_type(0));
	Apply_A_MD_F1(array, Functors::acquire_sum<result_type>(&result));
	return result;
}

template<class A2T>
floating64_type<typename A2T::value_type>	AverageValue(const DataArrayMD<A2T> &array)
{
	return ElementSum(array)/array.element_count();
}

template<class A2T, class F>
auto	ElementSumTransformed(const DataArrayMD<A2T> &array, const F& function)
	-> Functors::functor_result_type_t<F, typename A2T::value_type>
{
	//TODO инициализация результата не до конца продумана.
	typedef Functors::functor_result_type_t<F, typename A2T::value_type> result_type;
	if (array.empty())
		return result_type(0);

	result_type	result(0);
	Apply_A_MD_F1(array, Functors::acquire_transformed_sum<result_type, F>(&result, function));
	return result;
}

template<class A2T, class F>
auto AverageValueTransformed(const DataArrayMD<A2T> &array, F& function)
	-> Functors::functor_result_type_t<F, typename A2T::value_type>
{
	return ElementSumTransformed(array, function)/array.element_count();
}

template<class A2T>
floating64_type<typename A2T::value_type>	SquareElementSum(const DataArrayMD<A2T> &array)
{
	typedef floating64_type<typename A2T::value_type> result_type;
	return ElementSumTransformed(array, [](const typename A2T::value_type &v)
			{ return (result_type)square(v); });
}

template<class A2T>
floating64_type<typename A2T::value_type>	AverageSquare(const DataArrayMD<A2T> &array)
{
	return SquareElementSum(array)/array.element_count();
}

//
//--------------------------------------------------------------



//! \brief Расчет гистограммы. См. двумерную функцию ComputeHistogram, см. более новую ComputeHistogramRaw
template <class A2T, class T>
void	ComputeHistogram(const DataArrayMD<A2T> &array, DataArray<T> &histogram, const range1_F64 &values_range)
{
	size_t histogram_size = histogram.size();
	if(!histogram_size) return;



	if(array.n_dimensions() > 2)
	{
		histogram.fill(0);
		DataArray<T> histogram_increment(histogram_size);
		typename DataArrayMD<A2T>::invariable	subset;
		index_vector	iv(array.n_dimensions());

		for(size_t j = 0; j < array.n_dimensions()-1; ++j)
		{
			iv[j+1] = slice_mask(j);
		}
		iv[0] = 0;
		for(size_t i = 0; i < array.sizes(0); ++iv[0], ++i)
		{
			array.GetSubset(subset, iv);
			ComputeHistogram(subset, histogram_increment, values_range);
			Apply_AA_1D_F2(histogram, histogram_increment, Functors::plus_assign());
		}
		size_t div = array.sizes(0);
		Apply_A_1D_F1(histogram, [div](T &v) { v /= div; });
	}
	else if(array.n_dimensions() == 2)
	{
		typename DataArrayMD<A2T>::slice_type::invariable	slice;
		array.GetSlice(slice, {slice_mask(0), slice_mask(1)});
		ComputeHistogram(slice, histogram, values_range);
	}
}

/*!
	\brief Расчет гистограммы

	Область значений данных разбивается на интервалы [first + i*step, first + (i+1)*step),
	first = values_range.p1(), step = (values_range.p2()-values_range.p1())/histogram_size.

	Каждый i-й элемент гистограммы содержит количество элементов массива array, значения которых
	попадают в i-й интервал.

	Количество элементов со значениями вне диапазона, слева и справа,
	прибавляется соответственно к первому и последнему элементам гистограммы.

	\note
	Можно сделать учет количества элементов вне диапазона в отдельных переменных,
	а не прибавлять их к первому и последнему элементам гистограммы.

	\todo Сделать вариант Transformed.

	\todo Можно сделать также функцию ComputeHistogramNormalized, которая будет
	возвращать DataArray<float> (или от double) с гистограммой, нормированной на количество элементов массива.

	\todo Эта функция должна заменить старую ComputeHistogram.
*/
template <class Histogram = DataArray<size_t>, class A2T = void /* dummy default: always deduced */>
Histogram ComputeHistogramRaw(const DataArrayMD<A2T> &array, const range1_F64 &values_range, size_t histogram_size)
{
	Histogram histogram(histogram_size, 0);
	size_t less_counter(0), greater_counter(0);
	Functors::acquire_histogram_functor<Histogram> hf(&histogram, &less_counter, &greater_counter,
			values_range.p1(), values_range.p2());
	Apply_A_MD_F1(array, hf);
	histogram[0] += (typename Histogram::value_type)less_counter;
	histogram[histogram.size() - 1] += (typename Histogram::value_type)greater_counter;
	return histogram;
}

//! \brief Расчет гистограммы преобразованного массива. См. двумерную функцию ComputeHistogramTransformed
template <class A2T, class T, class F>
void	ComputeHistogramTransformed(const DataArrayMD<A2T> &array, DataArray<T> &histogram, const range1_F64 &values_range, const F& function)
{
	size_t histogram_size = histogram.size();
	if(!histogram_size) return;

	if(array.n_dimensions() > 2)
	{
		histogram.fill(0);
		DataArray<T> histogram_increment(histogram_size);
		typename DataArrayMD<A2T>::invariable	subset;
		index_vector	iv(array.n_dimensions());

		for(size_t j = 0; j < array.n_dimensions()-1; ++j)
		{
			iv[j+1] = slice_mask(j);
		}
		iv[0] = 0;
		for(size_t i = 0; i < array.sizes(0); ++iv[0], ++i)
		{
			array.GetSubset(subset, iv);
			ComputeHistogramTransformed(subset, histogram_increment, values_range, function);
			Apply_AA_1D_F2(histogram, histogram_increment, Functors::plus_assign());
		}
		size_t div = array.sizes(0);
		Apply_A_1D_F1(histogram, [div](T &v) { v /= div; });
	}
	else if(array.n_dimensions() == 2)
	{
		typename DataArrayMD<A2T>::slice_type::invariable	slice;
		array.GetSlice(slice, {slice_mask(0), slice_mask(1)});
		ComputeHistogramTransformed(slice, histogram, values_range, function);
	}
}

/*!
	\brief Нахождение гистограммы для изображений с векторным элементом
	(цветных или комплексных), покомпонентно.
	См. двумерную функцию ComputeComponentsHistogram
*/
template <class A2T, class HISTOGRAM_ROW_T>
void	ComputeComponentsHistogram(const DataArrayMD<A2T> &array, DataArray2D<HISTOGRAM_ROW_T> &histogram, const range1_F64 &values_range)
{
	typedef typename A2T::value_type value_type;
	typedef typename HISTOGRAM_ROW_T::value_type histogram_value_type;

	if(array.n_dimensions() > 2)
	{
		histogram.fill(0);
		DataArray2D<HISTOGRAM_ROW_T> histogram_increment(histogram.vsize(), histogram.hsize());
		typename DataArrayMD<A2T>::invariable	subset;
		index_vector	iv(array.n_dimensions());

		for(size_t j = 0; j < array.n_dimensions()-1; ++j)
		{
			iv[j+1] = slice_mask(j);
		}
		iv[0] = 0;
		for(size_t i = 0; i < array.sizes()[0]; ++iv[0], ++i)
		{
			array.GetSubset(subset, iv);
			ComputeComponentsHistogram(subset, histogram_increment, values_range);
			Apply_AA_2D_F2(histogram, histogram_increment, Functors::plus_assign());
		}
		size_t div = array.sizes(0);
		Apply_A_2D_F1(histogram, [div](histogram_value_type &v) { v /= div; });
	}
	else if(array.n_dimensions() == 2)
	{
		typename DataArrayMD<A2T>::slice_type::invariable	slice;
		array.GetSlice(slice, {slice_mask(0), slice_mask(1)});
		ComputeComponentsHistogram(slice, histogram, values_range);
	}
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif //XRAD__File_DataArrayAnalyzeMD_h
