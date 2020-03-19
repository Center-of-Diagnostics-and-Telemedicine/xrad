#ifndef __DataArrayAnalyze_h
#define __DataArrayAnalyze_h
/*!
	\file
	\date 2014-10-10 17:33
	\author kns
	\brief Общие инструмены для анализа одномерных массивов (DataArray)
*/
//--------------------------------------------------------------

#include "DataArray.h"
#include "BasicArrayInteractions1D.h"
#include "ArrayAnalyzeFunctors.h"
#include "SpaceCoordinates.h"
#include <XRADBasic/Sources/SampleTypes/HomomorphSamples.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace Functors
{

template <class Predicate, class R, class Functor>
class find_transformed_iv
{
	public:
		typedef typename std::remove_cv<R>::type nonconst_R;

		find_transformed_iv(size_t *found_index, nonconst_R *found_value, const Functor &functor):
			found_index(found_index), found_value(found_value), functor(functor)
		{
		}
		template <class T>
		void operator() (size_t i, const T &v) const
		{
			auto transformed_v = functor(v);
			if (!Predicate::test(*found_value, transformed_v))
				return;
			*found_index = i;
			*found_value = transformed_v;
		}
	private:
		size_t *found_index;
		nonconst_R *found_value;
		Functor functor;
};

} // namespace Functors

//--------------------------------------------------------------
//
//	MaxValue, MinValue
//	Возвращаемый тип совпадает с элементом массива.
//

template <class FindFunctor, class T>
T	FindValue(const DataArray<T> &array, size_t *p = NULL)
{
	if (!array.size())
		throw invalid_argument("FindValue<>: array size is 0.");
	//эти функции не следует делать через transformed (возможна потеря быстродействия)
	typedef typename remove_cv<T>::type result_type;
	size_t position = 0;
	result_type value(array[0]);
	Apply_IA_1D_F2(array, FindFunctor(&position, &value), 1, array.size());

	if(p)
		*p = position;
	return value;
}

template <class T>
T	MaxValue(const DataArray<T> &array, size_t *p = NULL)
{
	typedef typename remove_cv<T>::type result_type;
	return FindValue<Functors::find_iv<Functors::find_predicate_max, result_type>>(array, p);
}

template <class T>
T	MinValue(const DataArray<T> &array, size_t *p = NULL)
{
	typedef typename remove_cv<T>::type result_type;
	return FindValue<Functors::find_iv<Functors::find_predicate_min, result_type>>(array, p);
}

//--------------------------------------------------------------

/*!
	\brief N-й элемент (обертка для функции stl). Возвращает N-й по величине элемент массива

	Возвращаемый тип совпадает с типом элемента массива.
*/
template <class T>
T NthElement(const DataArray<T> &array, size_t n)
{
	DataArray<T> buffer(array);
	nth_element(buffer.begin(), buffer.begin()+n, buffer.end());
	return buffer[n];
}


//--------------------------------------------------------------
//
//	MaxValueTransformed, MinValueTransformed
//	Возвращаемый тип определяется функтором.
//


template <class FindTransformedFunctor, class T, class F>
auto	FindValueTransformed(const DataArray<T> &array, const F& function, size_t *p = NULL) -> decltype(declval<F>()(declval<T>()))
{
	typedef decltype(declval<F>()(declval<T>())) result_type;
	typedef F functor_type;

	if (!array.size())
		throw invalid_argument("FindValueTransformed<>: array size is 0.");
	size_t position = 0;
	result_type value(function(array[0]));
	Apply_IA_1D_F2(array, FindTransformedFunctor(&position, &value, function), 1, array.size());

	if(p)
		*p = position;
	return value;
}

template <class T, class F>
auto	MaxValueTransformed(const DataArray<T> &array, const F& function, size_t *p = NULL) -> decltype(declval<F>()(declval<T>()))
{
	typedef decltype(declval<F>()(declval<T>())) result_type;
	return FindValueTransformed<Functors::find_transformed_iv<Functors::find_predicate_max, result_type, F>>(array, function, p);
}

template <class T, class F>
auto	MinValueTransformed(const DataArray<T> &array, const F& function, size_t *p = NULL) -> decltype(declval<F>()(declval<T>()))
{
	typedef decltype(declval<F>()(declval<T>())) result_type;
	return FindValueTransformed<Functors::find_transformed_iv<Functors::find_predicate_min, result_type, F>>(array, function, p);
}


//--------------------------------------------------------------
//
//	MaxAbsoluteValue, MinAbsoluteValue
//	Возвращаемый тип double (только так).
//


template <class T>
double	MaxAbsoluteValue(const DataArray<T> &array, size_t *p = NULL)
{
	return MaxValueTransformed(array, Functors::absolute_value(), p);
}

template <class T>
double	MinAbsoluteValue(const DataArray<T> &array, size_t *p = NULL)
{
	return MinValueTransformed(array, Functors::absolute_value(), p);
}

//--------------------------------------------------------------
//
//	Статистические функции, не члены класса
//

template <class T>
floating64_type<T> ElementSum(const DataArray<T> &array)
{
	typedef floating64_type<T> result_type;
	if (!array.size())
	{
		return result_type(0);
	}

	result_type	result = zero_value(array.at(0));
	Apply_A_1D_F1(array, Functors::acquire_sum<result_type>(&result));
	return result;
}

template <class T>
floating64_type<T> AverageValue(const DataArray<T> &array)
{
	if (!array.size())
	{
		// Может быть, нужно кидать исключение?
		return floating64_type<T>(0);
	}

	return floating64_type<T>(ElementSum(array))/array.size();
}

template <class T, class F>
auto ElementSumTransformed(const DataArray<T> &array, const F& function) -> decltype(declval<F>()(declval<T>()))
{
	//TODO инициализация результата не до конца продумана.
	typedef decltype(declval<F>()(declval<T>())) result_type;
	if (!array.size())
	{
		return result_type(0);
	}
	result_type	result(0);
	Apply_A_1D_F1(array, Functors::acquire_transformed_sum<result_type, F>(&result, function));
	return result;
}

template <class T, class F>
auto	AverageValueTransformed(const DataArray<T> &array, const F& function) -> decltype(declval<F>()(declval<T>()))
{
	return ElementSumTransformed(array, function)/array.size();
}

template <class T>
floating64_type<T> SquareElementSum(const DataArray<T> &array)
{
	typedef floating64_type<T> result_type;
	return ElementSumTransformed(array, [](const T &v) { return (result_type)square(v); });
}

template <class T>
floating64_type<T> AverageSquare(const DataArray<T> &array)
{
	return floating64_type<T>(SquareElementSum(array))/array.size();
}

template <class T>
double StandardDeviation(const DataArray<T> &array)
{
	if(!array.size())
	{
		return 0;
	}

	floating64_type<T> average_value = AverageValue(array);
	return sqrt(AverageValueTransformed(array,
			[&average_value](const T &v) { return quadratic_norma(v - average_value); }));
}

//--------------------------------------------------------------

/*!
	\brief Расчет статистических моментов

	Количество рассчитываемых моментов определяется размером moments.

	Параметр central = true задает расчет центрированных моментов.

	moments[0] -- среднее
	moments[1] -- СКО^2 (если central = true)

	\todo Следует сделать произвольное количество моментов.
	Начальных сколько угодно, центральных два первых.

	\todo С комплексными данными как-то не так это все работает.
*/
template <class T, class position_type>
void WeightMoments(const DataArray<T> &array, DataArray<position_type> &moments, bool central)
{
	if(!moments.size()) return;
	moments.fill(0);
	T divisor(0);

	for(size_t i = 0; i < array.size(); ++i)
	{
		moments[0] += array[i]*double(i);
		divisor += array[i];
	}
	moments[0] /= divisor;
	for(size_t i = 0; i < array.size(); ++i)
	{
		double x_factor = central ? double(i)-moments[0] : double(i);
		double x = x_factor;
		for(size_t m = 1; m < moments.size(); ++m)
		{
			x*=x_factor;
			moments[m] += array[i]*x/divisor;
		}
	}
}

//--------------------------------------------------------------
//
// Статистические моменты покомпонентно
//

template <class T>
double AverageComponentValue(const DataArray<T> &array)
{
	double result = 0;
	const size_t n_data_components = n_components(array.at(0));
	const size_t s = array.size();

#if 0
	typedef acquire_sum_functor<double, size_t> functor_t;
	SA_1D_OpEq(result, array, componentwise_analyzer<functor_t >(functor_t(), n_data_components));

#else
	for(size_t i = 0; i < s; ++i)
	{
		for(size_t c = 0; c < n_data_components; ++c)
		{
			result += component(array[i], c);
		}
	}
#endif

	return result/(n_data_components*s);
}

template <class T>
double MaxComponentValue(const DataArray<T> &array, size_t *p)
{
	double result = component(array.at(0), 0);
	size_t position = 0;
	const size_t n_data_components = n_components(array.at(0));

#if 0
	typedef analyze_max_functor<double, size_t> functor_t;
	SA_1D_OpEq(result, array, componentwise_analyzer<functor_t>(functor_t(position), n_data_components));
#else

	const size_t s = array.size();
	for(size_t i = 0; i < s; ++i)
	{
		for(size_t c = 0; c < n_data_components; ++c)
		{
			double comp = component(array[i], c);
			if(comp > result) result = comp, position=i;
		}
	}
#endif

	if(p) *p = position;
	return result;
}

template <class T>
double MinComponentValue(const DataArray<T> &array, size_t *p)
{
	double result = component(array.at(0), 0);
	size_t position = 0;
	const size_t n_data_components = n_components(array.at(0));

#if 0
	typedef analyze_min_functor<double, size_t> functor_t;
	SA_1D_OpEq(result, array, componentwise_analyzer<functor_t>(functor_t(position), n_data_components));
#else

	const size_t s = array.size();
	for(size_t i = 0; i < s; ++i)
	{
		for(size_t c = 0; c < n_data_components; ++c)
		{
			double comp = component(array[i], c);
			if(comp < result) result = comp, position=i;
		}
	}
#endif

	if(p) *p = position;
	return result;
}

XRAD_END

//--------------------------------------------------------------
#endif //__DataArrayAnalyze_h
