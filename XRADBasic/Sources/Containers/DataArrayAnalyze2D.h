#ifndef __DataArray2DAnalyze_h
#define __DataArray2DAnalyze_h
/*!
	\file
	\date 2014-10-10 17:21
	\author kns
	\brief Общие инструмены для анализа двумерных массивов (DataArray2D)
*/
//--------------------------------------------------------------

#include "DataArray2D.h"
#include "BasicArrayInteractions2D.h"
#include "DataArrayAnalyze.h"

XRAD_BEGIN

//--------------------------------------------------------------

namespace Functors
{

template <class Predicate, class R>
class find_i2d_v
{
	public:
		typedef typename std::remove_cv<R>::type nonconst_R;

		find_i2d_v(size_t *found_index_v, size_t *found_index_h, nonconst_R *found_value):
			found_index_v(found_index_v), found_index_h(found_index_h), found_value(found_value)
		{
		}
		template <class T>
		void operator() (size_t iv, size_t ih, const T &v) const
		{
			if (!Predicate::test(*found_value, v))
				return;
			*found_index_v = iv;
			*found_index_h = ih;
			*found_value = v;
		}
	private:
		size_t *found_index_v;
		size_t *found_index_h;
		nonconst_R *found_value;
};

template <class Predicate, class R, class Functor>
class find_transformed_i2d_v
{
	public:
		find_transformed_i2d_v(size_t *found_index_v, size_t *found_index_h, R *found_value, const Functor &functor):
			found_index_v(found_index_v), found_index_h(found_index_h), found_value(found_value), functor(functor)
		{
		}
		template <class T>
		void operator() (size_t iv, size_t ih, T &&v) const
		{
			auto transformed_v = functor(v);
			if (!Predicate::test(*found_value, transformed_v))
				return;
			*found_index_v = iv;
			*found_index_h = ih;
			*found_value = transformed_v;
		}
	private:
		size_t *found_index_v;
		size_t *found_index_h;
		R *found_value;
		Functor functor;
};

} // namespace Functors



template<class FindTransformedFunctor, class ROW_T, class F>
auto	FindValueTransformed(const DataArray2D<ROW_T> &array, const F& function, size_t *v_position = NULL, size_t *h_position = NULL)
	-> Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>;
	if (!array.vsize() || !array.hsize())
		throw invalid_argument("FindValueTransformed<>: array size is 0.");
	size_t	vp = 0, hp = 0;
	result_type value = function(array.at(0, 0));
	Apply_IA_2D_F3(array, FindTransformedFunctor(&vp, &hp, &value, function));

	if(v_position)
		*v_position = vp;
	if(h_position)
		*h_position = hp;
	return value;
}

template<class ROW_T, class F>
auto	MaxValueTransformed(const DataArray2D<ROW_T> &array, const F& function, size_t *v_position = NULL, size_t *h_position = NULL)
	-> Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>;
	return FindValueTransformed<Functors::find_transformed_i2d_v<Functors::find_predicate_max, result_type, F>>(
			array, function, v_position, h_position);
}

template<class ROW_T, class F>
auto	MinValueTransformed(const DataArray2D<ROW_T> &array, const F& function, size_t *v_position = NULL, size_t *h_position = NULL)
	-> Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>
{
	using result_type = Functors::functor_result_type_t<F, typename DataArray2D<ROW_T>::value_type>;
	return FindValueTransformed<Functors::find_transformed_i2d_v<Functors::find_predicate_min, result_type, F>>(
			array, function, v_position, h_position);
}


//--------------------------------------------------------------
//
//	MaxValue, MinValue
//	Возвращаемый тип совпадает с элементом массива.
//

template<class FindFunctor, class ROW_T>
auto FindValue(const DataArray2D<ROW_T> &array, size_t *v_position = NULL, size_t *h_position = NULL)
	-> typename ROW_T::value_type
{
	typedef typename std::remove_cv<typename ROW_T::value_type>::type result_type;

	if (!array.vsize() || !array.hsize())
		throw invalid_argument("FindValue<>: array size is 0.");
	size_t	vp = 0, hp = 0;
	result_type	value(array.at(0, 0));
	Apply_IA_2D_F3(array, FindFunctor(&vp, &hp, &value));

	if(v_position)
		*v_position = vp;
	if(h_position)
		*h_position = hp;
	return value;
}

template<class ROW_T>
auto MaxValue(const DataArray2D<ROW_T> &array, size_t *v_position = NULL, size_t *h_position = NULL)
	-> typename ROW_T::value_type
{
	typedef typename ROW_T::value_type result_type;
	return FindValue<Functors::find_i2d_v<Functors::find_predicate_max, result_type>>(
			array, v_position, h_position);
}

template<class ROW_T>
auto MinValue(const DataArray2D<ROW_T> &array, size_t *v_position = NULL, size_t *h_position = NULL)
	-> typename ROW_T::value_type
{
	typedef typename ROW_T::value_type result_type;
	return FindValue<Functors::find_i2d_v<Functors::find_predicate_min, result_type>>(
			array, v_position, h_position);
}


//--------------------------------------------------------------
//
//	MaxAbsoluteValue, MinAbsoluteValue
//	Возвращаемый тип double (только так).
//

template<class ROW_T>
double MaxAbsoluteValue(const DataArray2D<ROW_T> &array, size_t *v_position = NULL, size_t *h_position = NULL)
{
	return MaxValueTransformed(array, Functors::absolute_value(), v_position, h_position);
}

template<class ROW_T>
double MinAbsoluteValue(const DataArray2D<ROW_T> &array, size_t *v_position = NULL, size_t *h_position = NULL)
{
	return MinValueTransformed(array, Functors::absolute_value(), v_position, h_position);
}




//! \todo Переделать через 2D алгоритм.
template<class ROW_T>
floating64_type<typename ROW_T::value_type> ElementSum(const DataArray2D<ROW_T> &array)
{
	size_t	vs = array.vsize();
	floating64_type<typename ROW_T::value_type>	result(0);
	if(!vs) return result;

	for(size_t i = 0; i < vs; i++) result += ElementSum(array.row(i));
	return result;
}

//! \todo Переделать через 2D алгоритм.
template <class ROW_T, class F>
auto ElementSumTransformed(const DataArray2D<ROW_T> &array, const F& function) -> decltype(declval<F>()(declval<typename ROW_T::value_type>()))
{
	typedef decltype(declval<F>()(declval<typename ROW_T::value_type>())) result_type;
	size_t	vs = array.vsize();
	result_type	result(0);
	if(!vs) return result;

	for(size_t i = 0; i < vs; i++) result += ElementSumTransformed(array.row(i), function);
	return result;
}

//! \todo Переделать через 2D алгоритм.
template<class ROW_T>
floating64_type<typename ROW_T::value_type> SquareElementSum(const DataArray2D<ROW_T> &array)
{
	size_t	vs = array.vsize();
	typename ROW_T::value_type_variable	result(0);
	if(!vs) return result;

	for(size_t i = 0; i < vs; i++) result += SquareElementSum(array.row(i));
	return result;
}


template<class ROW_T>
floating64_type<typename ROW_T::value_type> AverageValue(const DataArray2D<ROW_T> &array)
{
	double	s = array.vsize()*array.hsize();
	if(!s) return typename ROW_T::value_type(0);

	// Здесь при делении нужно быть аккуратным с типами значений.
	// Сейчас оба аргумента имеют тип числа с плавающей точкой, поэтому неприятностей нет.
	// Раньше использовались типы из исходных массивов, в т.ч. целочисленные.
	// Если value_type -- signed (sizeof() <= sizeof(s)), а тип s -- unsigned (size_t),
	// то результат деления unsigned. В этом случае результат получается неверный,
	// если ElementSum() < 0.
	// При подсчете среднего значения гистограммы получалось отрицательное дробное значение,
	// и вместо -0.8 выдавало 17529.
	return ElementSum(array)/s;
}

template <class ROW_T, class F>
auto AverageValueTransformed(const DataArray2D<ROW_T> &array, const F& function) -> decltype(declval<F>()(declval<typename ROW_T::value_type>()))
{
	typedef decltype(declval<F>()(declval<typename ROW_T::value_type>())) result_type;
	double	s = array.vsize()*array.hsize();
	if(!s) return result_type(0);
	return ElementSumTransformed(array, function)/s;

}


template<class ROW_T>
floating64_type<typename ROW_T::value_type> AverageSquare(const DataArray2D<ROW_T> &array)
{
	size_t	s = array.vsize()*array.hsize();
	if(!s) return typename ROW_T::value_type(0);;

	return SquareElementSum(array)/s;
}

//--------------------------------------------------------------
//
//	Покомпонентно
//
template<class ROW_T>
double MaxComponentValue(const DataArray2D<ROW_T> &array, size_t *v_position=NULL, size_t *h_position=NULL)
{
	// \todo Выбрать измерение с меньшим шагом или переделать (FindValueTransformed).
	size_t	vs = array.vsize();
	size_t v_position_buffer = 0, h_position_buffer = 0;
	double	buffer = MaxComponentValue(array.row(0), &h_position_buffer);
	double	result = buffer;
	for(size_t i = 1; i < vs; i++)
	{
		size_t	j;
		buffer = MaxComponentValue(array.row(i), &j);
		if(buffer > result) result = buffer, h_position_buffer = j, v_position_buffer = i;
	};
	if(v_position)
		*v_position = v_position_buffer;
	if(h_position)
		*h_position = h_position_buffer;
	return result;
}

template<class ROW_T>
double MinComponentValue(const DataArray2D<ROW_T> &array, size_t *v_position=NULL, size_t *h_position=NULL)
{
	// \todo Выбрать измерение с меньшим шагом или переделать (FindValueTransformed).
	size_t	vs = array.vsize();
	size_t v_position_buffer = 0, h_position_buffer = 0;
	double	buffer = MinComponentValue(array.row(0), &h_position_buffer);
	double	result = buffer;
	for(size_t i = 1; i < vs; i++)
	{
		size_t	j;
		buffer = MinComponentValue(array.row(i), &j);
		if(buffer < result) result = buffer, h_position_buffer = j, v_position_buffer = i;
	};
	if(v_position)
		*v_position = v_position_buffer;
	if(h_position)
		*h_position = h_position_buffer;
	return result;
}

template<class ROW_T>
double AverageComponentValue(const DataArray2D<ROW_T> &array)
{
	// \todo Выбрать измерение с меньшим шагом или переделать (FindValueTransformed).
	size_t	vs = array.vsize();
	double	result = 0;
	for(size_t i = 0; i < vs; i++) result += AverageComponentValue(array.row(i));
	return result/vs;
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif //__DataArray2DAnalyze_h
