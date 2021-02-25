#ifndef DataArrayHistogram2D_h__
#define DataArrayHistogram2D_h__
/*!
	\file
	\date 2018/02/16 15:46
	\author kulberg

	\brief Нахождение гистограммы изображения
*/
//--------------------------------------------------------------

#include "DataArrayAnalyze2D.h"

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Расчет гистограммы скалярного изображения, см. более новую ComputeHistogramRaw

	Крайние значения гистограммы задаются принудительно параметром values_range.
	Если они выходят за диапазон значений изображения, лишний диапазон гистограммы остается нулевым.
	Если они у́же, чем диапазон значений изображения, лишние значения плюсуются на краях диапазона.

	Диапазон values_range делится на histogram.size() интервалов.
*/
template <class ROW_T, class T>
void	ComputeHistogram(const DataArray2D<ROW_T> &img, DataArray<T> &histogram, const range1_F64 &values_range)
{
	if(!histogram.size()) return;

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
		throw invalid_argument(ssprintf("ComputeHistogram(const DataArray2D<ROW_T> &img, DataArray<T> &histogram, const range1_F64 &values_range)\n"
				"Invalid values range (%g, %g)", values_range.p1(), values_range.p2()));
	}

	histogram.fill(0);

// 	double	index_factor = double(histogram.size() - 1)/values_range.delta();
	double	index_factor = double(histogram.size())/values_range.delta();
	double	increment = 1./(img.vsize()*img.hsize());

	for(size_t i = 0; i < img.vsize(); ++i)
	{
		typename ROW_T::const_iterator it = img.row(i).begin(), ie = img.row(i).end();
		for(; it < ie; ++it)
		{
			size_t	index = range(ptrdiff_t(index_factor*(*it-values_range.p1())), 0, ptrdiff_t(histogram.size())-1);
			histogram[index] += increment;
		}
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
template <class Histogram = DataArray<size_t>, class ROW_T = void /* dummy default: always deduced */>
Histogram ComputeHistogramRaw(const DataArray2D<ROW_T> &array, const range1_F64 &values_range, size_t histogram_size)
{
	Histogram histogram(histogram_size, 0);
	size_t less_counter(0), greater_counter(0);
	Functors::acquire_histogram_functor<Histogram> hf(&histogram, &less_counter, &greater_counter,
			values_range.p1(), values_range.p2());
	Apply_A_2D_F1(array, hf);
	histogram[0] += (typename Histogram::value_type)less_counter;
	histogram[histogram.size() - 1] += (typename Histogram::value_type)greater_counter;
	return histogram;
}

/*!
	\brief Расчет гистограммы преобразованного массива

	Крайние значения гистограммы задаются принудительно параметром values_range.
	Если они выходят за диапазон значений изображения, лишний диапазон гистограммы остается нулевым.
	Если они у́же, чем диапазон значений изображения, лишние значения плюсуются на краях диапазона.

	Диапазон values_range делится на histogram.size() интервалов.

	Функтор должен быть монотонной функцией на интервале (in_minval, in_maxval),
	иначе результат непредсказуем. [Кажется, это требование лишнее. / @АБЕ]
*/
template <class ROW_T, class T, class F>
void	ComputeHistogramTransformed(const DataArray2D<ROW_T> &img, DataArray<T> &histogram, const range1_F64 &values_range, const F& function)
{
	if(!histogram.size()) return;

	if(!is_number(values_range.x1()) || !is_number(values_range.x2()) || values_range.x1() == values_range.x2())
	{
		//если концы диапазона не заданы, искать нечего.
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeHistogramTransformed(const DataArray2D<ROW_T> &img, DataArray<T> &histogram, const range1_F64 &values_range)\n"
				"Invalid values range (%g, %g)", values_range.p1(), values_range.p2()));
	}

	histogram.fill(0);

	double	index_factor = double(histogram.size() - 1)/values_range.delta();
	double	increment = 1./(img.vsize()*img.hsize());
// 	double	transformed;
	for(size_t i = 0; i < img.vsize(); ++i)
	{
		typename ROW_T::const_iterator it = img.row(i).begin(), ie = img.row(i).end();
		for(; it < ie; ++it)
		{
			auto transformed = function(*it);
			if(is_number(transformed))
			{
				transformed = range(transformed, values_range.x1(), values_range.x2());
				size_t	index = range(ptrdiff_t(index_factor*(transformed-values_range.x1())), ptrdiff_t(0), ptrdiff_t(histogram.size()-1));
				histogram[index] += increment;
			}
		}
	}
}



//--------------------------------------------------------------



/*!
	\brief Нахождение гистограммы для изображений с векторным элементом
	(цветных или комплексных), покомпонентно

	Диапазон values_range делится на histogram.size() интервалов.

	\todo С этим следует разобраться и результат вынести на отдельную страницу (\\page),
	во всех функциях расчета гистограмм вставить ссылки (\\ref) на неё:
	При анализе гистограммы привязку можно выполнять по левому краю
	или по центру интервала. Второе предпочтительнее, так и делается
	в интерфейсных функциях отображения гистограммы.
*/
template <class ROW_T, class F1D>
void	ComputeComponentsHistogram(const DataArray2D<ROW_T> &img, DataArray2D<F1D> &histogram, const range1_F64 &values_range)
{
	if(!img.vsize() || !img.hsize())
	{
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeComponentsHistogram(const DataArray2D<ROW_T> &img, DataArray2D<F1D> &histogram, const range1_F64 &values_range)\n"
				"Invalid image sizes (vsize = %d hsize = %d)", img.vsize(), img.hsize()));
	}
	const size_t n_data_components = n_components(img.at(0, 0));
	if(n_data_components != histogram.vsize())
	{
		ForceDebugBreak();// здесь уместен
		throw invalid_argument(ssprintf("ComputeComponentsHistogram(const DataArray2D<ROW_T> &img, DataArray2D<F1D> &histogram, const range1_F64 &values_range)\n"
				"Invalid histogram vsize = %d (n_components = %d)", histogram.vsize(), n_data_components));
	}
	if(!histogram.hsize()) return;
	if(!is_number(values_range.x1()) || !is_number(values_range.x2()) || values_range.x1() == values_range.x2())
	{
		//если концы диапазона не заданы, искать нечего.
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeComponentsHistogram(const DataArray2D<ROW_T> &img, DataArray2D<F1D> &histogram, const range1_F64 &values_range)\n"
				"Invalid values range (%g, %g)", values_range.p1(), values_range.p2()));
	}
	histogram.fill(0);

	double	index_factor = double(histogram.hsize())/values_range.delta();
//	double	increment = 1./(img.vsize()*img.hsize()*n_data_components);
	double	increment = 1./(img.vsize()*img.hsize());

	for(size_t i = 0; i < img.vsize(); ++i)
	{
		typename ROW_T::const_iterator it = img.row(i).begin(), ie = img.row(i).end();
		for(;it<ie;++it)
		{
			for(size_t k = 0; k < n_data_components; ++k)
			{
				double	value = component(*it, k);
				//все аргументы ниже со знаком, возможно появление отрицательного индекса и его последующее ограничение до 0
				ptrdiff_t	index = range(ptrdiff_t(index_factor*(value-values_range.x1())), 0, ptrdiff_t(histogram.hsize()) - 1);
				histogram.at(k, index) += increment;
			}
		}
	}
}

//--------------------------------------------------------------

/*!
	\brief Нахождение гистограммы для одномерных массивов (DataArray) с векторным элементом
	(цветных или комплексных), покомпонентно

	(Функция для анализа одномерных данных.
	Функция размещена здесь, т.к. на выходе дает двумерные данные).

	Диапазон values_range делится на histogram.size() интервалов.
*/
template <class T, class F1D>
void	ComputeComponentsHistogram(const DataArray<T> &row, DataArray2D<F1D> &histogram, const range1_F64 &values_range)
{
	if(!row.size())
	{
		//ForceDebugBreak();здесь этот останов неуместен
		throw invalid_argument(ssprintf("ComputeComponentsHistogram(const DataArray<T> &row, DataArray2D<F1D> &histogram, const range1_F64 &values_range)\n"
							"Invalid row sizes (size = %d)", row.size()));
	}
	const size_t n_data_components = n_components(row.at(0, 0));
	if(n_data_components != histogram.vsize())
	{
		ForceDebugBreak(); //здесь уместен
		throw invalid_argument(ssprintf("ComputeComponentsHistogram(const DataArray<T> &row, DataArray2D<F1D> &histogram, const range1_F64 &values_range)\n"
							"Invalid histogram vsize = %d (n_components = %d)", histogram.vsize(), n_data_components));
	}
	if(!histogram.hsize()) return;
	if(!is_number(values_range.x1()) || !is_number(values_range.x2()) || values_range.x1() == values_range.x2())
	{
	//если концы диапазона не заданы, искать нечего
		histogram.fill(1./histogram.hsize());
		return;
	}
	histogram.fill(0);

	double	index_factor = double(histogram.hsize())/values_range.delta();
	double	increment = 1./row.size();

	typename DataArray<T>::const_iterator it = row.begin(), ie = row.end();
	for(;it<ie;++it)
	{
		for(size_t k = 0; k < n_data_components; ++k)
		{
			double	value = component(*it, k);
			//все аргументы ниже со знаком, возможно появление отрицательного индекса и его последующее ограничение до 0
			ptrdiff_t	index = range(ptrdiff_t(index_factor*(value-values_range.x1())), 0, ptrdiff_t(histogram.hsize()) - 1);
			histogram.at(k, index) += increment;
		}
	}
}

//--------------------------------------------------------------

XRAD_END

#endif // DataArrayHistogram2D_h__
