#ifndef DataArrayHistogram_h__
#define DataArrayHistogram_h__
/*!
	\file
	\date 2018/02/16 15:40
	\author kulberg

	\brief Функторы для анализа содержимого контейнеров
*/
//--------------------------------------------------------------

#include "DataArrayAnalyze.h"

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Расчет гистограммы, см. более новую ComputeHistogramRaw

	Крайние значения гистограммы задаются принудительно параметром values_range.
	Если они выходят за диапазон значений изображения, лишний диапазон гистограммы остается нулевым.
	Если они у́же, чем диапазон значений изображения, лишние значения плюсуются на краях диапазона.

	Диапазон values_range делится на histogram.size() интервалов.
*/
template <class T, class T2>
void	ComputeHistogram(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range)
{
	if(!histogram.size()) return;
	if(!row.size())
	{
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeHistogram(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range)\n"
							"Invalid row size (size = %d)", row.size()));
	}

	if(values_range.x1() == values_range.x2())
	{
		histogram.fill(0);
		histogram[histogram.size()/2] = 1;
		return;
	}

	if(!is_number(values_range.x1()) || !is_number(values_range.x2()))
	{
		//если концы диапазона не заданы, искать нечего.
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeHistogram(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range)\n"
							"Invalid values range (%g, %g)", values_range.p1(), values_range.p2()));
	}

	histogram.fill(0);

	double	index_factor = double(histogram.size())/values_range.delta();
	double	increment = 1./(row.size());

	typename DataArray<T>::const_iterator it = row.begin(), ie = row.end();
	for(; it < ie; ++it)
	{
		size_t	index = range(ptrdiff_t(index_factor*(*it-values_range.p1())), 0, ptrdiff_t(histogram.size())-1);
		histogram[index] += increment;
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
template <class Histogram = DataArray<size_t>, class T = void /* dummy default: always deduced */>
Histogram ComputeHistogramRaw(const DataArray<T> &array, const range1_F64 &values_range, size_t histogram_size)
{
	Histogram histogram(histogram_size, 0);
	size_t less_counter(0), greater_counter(0);
	Functors::acquire_histogram_functor<Histogram> hf(&histogram, &less_counter, &greater_counter,
			values_range.p1(), values_range.p2());
	Apply_A_1D_F1(array, hf);
	histogram[0] += (typename Histogram::value_type)less_counter;
	histogram[histogram.size() - 1] += (typename Histogram::value_type)greater_counter;
	return histogram;
}

/*!
	\brief Расчет гистограммы преобразованного массива [переделать тип function на result_type f(value)]

	Крайние значения гистограммы задаются принудительно параметром values_range.
	Если они выходят за диапазон значений изображения, лишний диапазон гистограммы остается нулевым.
	Если они у́же, чем диапазон значений изображения, лишние значения плюсуются на краях диапазона.

	Диапазон values_range делится на histogram.size() интервалов.

	Функтор должен быть монотонной функцией на интервале (in_minval, in_maxval),
	иначе результат непредсказуем. [Кажется, это требование лишнее. / @АБЕ]

	\todo Сейчас function -- двухаргументный функтор (f(&result, value)). Аналогичная двумерная функция
	расчета гистограммы принимает функтор, возвращающий значение (result_type f(value)).
	Сделать здесь, как в двумерной функции.
*/
template <class T, class T2, class F>
void	ComputeHistogramTransformed(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range, const F& function)
{
	if(!histogram.size()) return;
	if(!row.size())
	{
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeHistogramTransformed(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range)\n"
							"Invalid row sizes (size = %d)", row.size()));
	}

	if(!is_number(values_range.x1()) || !is_number(values_range.x2()) || values_range.x1() == values_range.x2())
	{
		//если концы диапазона не заданы, искать нечего.
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument("ComputeHistogramTransformed(const DataArray<T> &row, DataArray<T2> &histogram, const range1_F64 &values_range), invalid values range");
	}

	histogram.fill(0);

	double	index_factor = double(histogram.size() - 1)/values_range.delta();
	double	increment = 1./row.size();
	double	transformed;
	typename DataArray<T>::const_iterator it = row.begin(), ie = row.end();
	for(; it < ie; ++it)
	{
		function(transformed, *it);
		if(is_number(transformed))
		{
			transformed = range(transformed, values_range.x1(), values_range.x2());
			size_t	index = range(ptrdiff_t(index_factor*(transformed-values_range.x1())), 0, ptrdiff_t(histogram.size())-1);
			histogram[index] += increment;
		}
	}
}

//--------------------------------------------------------------

XRAD_END

#endif // DataArrayHistogram_h__
