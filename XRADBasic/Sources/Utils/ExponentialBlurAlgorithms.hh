#include <XRADBasic/SampleTypes.h>
#include <XRADBasic/Sources/SampleTypes/FixedPointSample.h>
#include <XRADBasic/Sources/Containers/MathFunctionMD.h>
#ifdef XRAD_USE_OPENCL
	#include <XRAD/OpenCL.h>
#endif

XRAD_BEGIN


//TODO вопросы по производительности экспоненциальных фильтров
//
// 1. цикл по итератору или по индексу? Есть предположение, что по индексу быстрее процентов на 10, но требуется еще проверка
//	меняем раскомментированием следующей строки
//
//#define CYCLES_BY_ITERATOR //иначе по индексу
//
// 2. вызывать inline функцию iir_one_point(*b1, *b0, a) или писать умножение прямо в строке? пока преимущество второго неясно до конца,
//	на стороне функции большая ясность кода и универсальность шаблона.
//	меняем раскомментированием одной из двух следующих строк
//
#define filter_action(result,b0,a) iir_one_point(result, b0, a)
//#define filter_action(result,b0,a) result += (result - b0)*a;
//
// 3. условие "i-- > 1" теоретически может быть хуже, чем равнозначное "--i > 0". выяснить




namespace	exponential_blur_algorithms
{
/*
TODO возможно, следующая реализация фильтра даст прирост быстродействия
template<class sample, class factor>
inline	void iir_one_point_div(sample &result, const sample &b0, const factor &a0, const factor &a1_a0)
{
	// еще более экономичный вариант предыдущего, не требуется
	// буферная переменная. вместо множителя a1 передается величина a1/a0
	result *= a1_a0;
	result += b0;
	result *= a0;
}

*/

//TODO: додумать еще организацию фильтра с осцилляцией (противоположный знак множителя, м.б., определить это через "отрицательный радиус")

//--------------------------------------------------------------
//
//	действие бих-фильтра 1-го порядка в одной точке
//	(переопределенные функции, с частичными реализациями шаблонов были проблемы)
//
inline	void iir_one_point(float &result, const float &b0, const float &a)
{
	result += (result - b0)*a;
}

inline	void iir_one_point(double &result, const double &b0, const float &a)
{
	result += (result - b0)*a;
}

inline	void iir_one_point(int32_t &result, const int32_t &b0, const int32_t &a)
{
	result += ((result - b0)*a) >> fixed_point_position<int32_t>();
}

inline	void iir_one_point(short &result, const short &b0, const int &a)
{
	result += ((result - b0)*a) >> fixed_point_position<int>();
}

// здесь нужен отдельный тип для скаляров, т.к. целочисленные данные умножаются в алгоритме
// на целое со сдвигом. а ST для них может быть с плавающей запятой
template<class T, class ST, class ST2>
inline	void iir_one_point(ComplexSample<T, ST> &result, const ComplexSample<T, ST> &b0, const ST2 &a)
{
	iir_one_point(result.re, b0.re, a);
	iir_one_point(result.im, b0.im, a);
}


template<class RGBT, class ST>
inline	void iir_one_point(RGBColorSample<RGBT> &result, const RGBColorSample<RGBT> &b0, const ST &a)
{
	iir_one_point(result.red(), b0.red(), a);
	iir_one_point(result.green(), b0.green(), a);
	iir_one_point(result.blue(), b0.blue(), a);
}

//--------------------------------------------------------------
//
// функтор-обертка для вычисления рекурсивного фильтра по массиву массивов
//

struct iir_one_point_functor
{
	template<class T0, class T1, class ST>
	void operator()(T0 &result, const T1 &b0, const ST &a) const
	{
		iir_one_point(result, b0, a);
	}
};

//--------------------------------------------------------------
//
// вычисление рекурсивного фильтра по "массиву массивов"
// такое вычисление предпочтительно перед обработкой каждой строки по отдельности
// в тех случаях, когда каждый элемент "массива массивов" занимает в памяти немного места,
// но смешиваемые объекты лежат далеко друг от друга. при поэлементном доступе
// требуется постоянно обращаться к далеко расположенным участкам памяти, кэш не работает как надо.
// для взвешенного смешивания пары "плоскостей" обе они помещаются в кэше.
//
// заметное приращение производительности это дало на трехмерных массивах, см. ниже
//
template<XRAD__MathFunction_template, XRAD__MathFunction_template1, class ST3>
inline	void iir_one_point(MathFunction<XRAD__MathFunction_template_args> &result, const MathFunction<XRAD__MathFunction_template_args1> &b0, const ST3 &a)
{
	Apply_AAS_1D_F3(result, b0, a, iir_one_point_functor());
}


template<class FT, class FT2, class ST>
inline	void iir_one_point(MathFunction2D<FT> &result, const MathFunction2D<FT2> &b0, const ST &a)
{
	Apply_AAS_2D_F3(result, b0, a, iir_one_point_functor());
}

//
//--------------------------------------------------------------



template<class sample_t, class index_t, class difference_t, class factor>
void	blur_1d_forward(sample_t *pdata,
						index_t data_size, difference_t data_step,
						factor a)
{
#ifndef CYCLES_BY_ITERATOR
	if(data_step==1)
	{
		for(size_t i=1; i<data_size; ++i)
		{
			iir_one_point(pdata[i], pdata[i-1], a);
		}
	}
	else
	{
		for(size_t i=1; i<data_size; ++i)
		{
			iir_one_point(pdata[i*data_step], pdata[(i-1)*data_step], a);
		}
	}

#else
	sample_t	*b0 = pdata;
	sample_t	*b1 = pdata + data_step;
	sample_t	*b_max = b0 + data_size*data_step;

	for(; b1 < b_max; b0+=data_step, b1+=data_step)
	{
		iir_one_point(*b1, *b0, a);
	}
#endif
}

template<class sample_t, class index_t, class difference_t, class factor>
void	blur_1d_reverse(sample_t *pdata,
						index_t data_size, difference_t data_step,
						factor a)
{
#ifndef CYCLES_BY_ITERATOR
	if(data_step==1)
	{
		for(size_t i=data_size; i-- > 1;)
		{
			iir_one_point(pdata[i-1], pdata[i], a);
		}
	}
	else
	{
		for(size_t i=data_size; i-- > 1;)
		{
			iir_one_point(pdata[(i-1)*data_step], pdata[i*data_step], a);
		}
	}

#else
	sample_t	*b0 = pdata + (data_size-2)*data_step;
	sample_t	*b1 = pdata + (data_size-1)*data_step;

	for(; b0 >= pdata; b0-=data_step, b1-=data_step)
	{
		iir_one_point(*b0, *b1, a);
	}
#endif
}

template<class sample_t, class index_t, class difference_t, class factor>
void	blur_1d_bidirectional(sample_t *pdata,
		index_t data_size, difference_t data_step,
		factor a)
{
	//float	af(a);// здесь приводить ко float нельзя, т.к. для целочисленных множителей другой алгоритм (умножение со сдвигом)
	// здесь использовать только шаблон iir_one_point, а не умножение в строке.
#ifdef CYCLES_BY_ITERATOR
	sample_t	*b0 = pdata;
	sample_t	*b1 = pdata + data_step;
	sample_t	*b_max = b0 + data_size*data_step;

	for(; b1 < b_max; b0+=data_step, b1+=data_step)
	{
		iir_one_point(*b1, *b0, a);
	}

	b0 -= data_step;
	b1 -= data_step;

	for(; b0 >= pdata; b0-=data_step, b1-=data_step)
	{
		iir_one_point(*b0, *b1, a);
	}
#else
	if(data_step==1)
	{
		for(size_t i=1; i<data_size; ++i)
		{
			iir_one_point(pdata[i], pdata[i-1], a);
		}
		for(size_t i=data_size; i-- > 1;)
		{
			iir_one_point(pdata[i-1], pdata[i], a);
		}
	}
	else
	{
		for(size_t i=1; i<data_size; ++i)
		{
			iir_one_point(pdata[i*data_step], pdata[(i-1)*data_step], a);
		}
		for(size_t i=data_size; i-- > 1;)
		{
			iir_one_point(pdata[(i-1)*data_step], pdata[i*data_step], a);
		}
	}


#endif
}

#if 1

template<class sample_t>
inline void	blur_1d_bidirectional(sample_t *pdata,
		size_t data_size, ptrdiff_t data_step,
		float af)
{
	if(data_step==1)
	{
		for(size_t i=1; i<data_size; ++i)
		{
			filter_action(pdata[i], pdata[i-1], af);
//			pdata[i] += (pdata[i] - pdata[i-1])*af;
		}
		for(size_t i=data_size; i-- > 1;)
		{
//			pdata[i-1] += (pdata[i-1] - pdata[i])*af;
			filter_action(pdata[i-1], pdata[i], af);
		}
	}
	else
	{
		for(size_t i=1; i<data_size; ++i)
		{
			filter_action(pdata[i*data_step], pdata[(i-1)*data_step], af);
//			pdata[i*data_step] += (pdata[i*data_step] - pdata[(i-1)*data_step])*af;
		}
		for(size_t i=data_size; i-- > 1;)
		{
//			pdata[(i-1)*data_step] += (pdata[(i-1)*data_step] - pdata[i*data_step])*af;
			filter_action(pdata[(i-1)*data_step], pdata[i*data_step], af);
		}
	}
}
#endif

template<class sample_t>
inline void	blur_1d_bidirectional(sample_t *pdata,
		size_t data_size, ptrdiff_t data_step,
		double a)
{
	// рост производительности, если исключить применение double
	float af(a);
	blur_1d_bidirectional(pdata, data_size, data_step, af);
}


} //namespace exponential_blur_algorithms


inline double	ExponentialFlterCoefficient(double radius)
{
	static  double	log_level = -sqrt(2.);//log(edge_level);
		// уровень искомой экспоненты при аргументе, равном radius
		// при двусторонней фильтрации дисперсия получившегося пятна должна быть равна radius
	if(radius == 0) return 0;
	else return radius>0 ?
		-exp(log_level/radius):
		exp(log_level/(-radius));//второй вариант означает осциллирующий фильтр, который тоже экспоненциально затухает. может пригодиться
}



template<class ARR, class ST>
inline void	BiexpBlur1D(ARR &f, ST a)
{
	if (f.empty()) return;
	exponential_blur_algorithms::blur_1d_bidirectional(&f[0], f.size(), f.step(), a);
}

template<class ARR, class ST>
inline void	BiexpSharpen1D(ARR &f, ST a, double strength)
{
	if (f.empty()) return;
	ARR buffer(f);
	exponential_blur_algorithms::blur_1d_bidirectional(buffer.data(), buffer.size(), buffer.step(), a);
	f.mix(f, buffer, 1.+strength, -strength);
}


template<class ARR, class ST>
inline void	ExponentialBlur1D(ARR &f, ST a, exponential_blur_direction direction)
{
	if (f.empty()) return;
	switch(direction)
	{
		case 	exponential_blur_forward:
			exponential_blur_algorithms::blur_1d_forward(&f[0], f.size(), f.step(), a);
			break;
		case	exponential_blur_reverse:
			exponential_blur_algorithms::blur_1d_reverse(&f[0], f.size(), f.step(), a);
			break;
		case	biexponential_blur:
			//exponential_blur_algorithms::blur_1d_bidirectional(&f[0], f.size(), f.step(), a);
			BiexpBlur1D(f, a);
			break;
	}
}



template<class AR2D>
void	BiexpBlur2D_cpu(AR2D &data, double radius_v, double radius_h)
{
	if (data.empty()) return;

	static	const typename AR2D::scalar_type	normalizer_factor =
			multiply_normalizer(typename AR2D::scalar_type());

	if(radius_v)
	{
		typename AR2D::scalar_type av = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius_v)*normalizer_factor);
		for(size_t j = 0; j < data.hsize(); j++)
		{
		//exponential_blur_algorithms::blur_1d_bidirectional(&data.col(j).at(0), data.vsize(), data.vstep(), av);
			BiexpBlur1D(data.col(j), av);
		}
	}


	if(radius_h)
	{
		typename AR2D::scalar_type ah = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius_h)*normalizer_factor);
		for(size_t i = 0; i < data.vsize(); i++)
		{
// 			exponential_blur_algorithms::blur_1d_bidirectional(&data.row(i).at(0), data.hsize(), data.hstep(), ah);
			BiexpBlur1D(data.row(i), ah);
		}
	}
}

template<class AR2D>
inline void BiexpBlur2D(AR2D &data, double radius_v, double radius_h)
{
	if (data.empty()) return;
#ifdef XRAD_USE_OPENCL
	if (BiexpBlur2D_OpenCL(data, radius_v, radius_h))
		return;
#endif
	BiexpBlur2D_cpu(data, radius_v, radius_h);
}

template<class AR2D>
void	BiexpSharpen2D(AR2D &data, double radius_v, double radius_h, double strength)
{
	if (data.empty()) return;
	static	const typename AR2D::scalar_type	normalizer_factor =
			multiply_normalizer(typename AR2D::scalar_type());

	if(radius_v)
	{
		typename AR2D::scalar_type av = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius_v)*normalizer_factor);
		for(size_t j = 0; j < data.hsize(); j++)
		{
			BiexpSharpen1D(data.col(j), av, strength);
		}
	}


	if(radius_h)
	{
		typename AR2D::scalar_type ah = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius_h)*normalizer_factor);
		for(size_t i = 0; i < data.vsize(); i++)
		{
			BiexpSharpen1D(data.row(i), ah, strength);
		}
	}
}

template<class AR2D>
void	BiexpBlur2D(AR2D &data, const point2_F64 &radius)
{
	BiexpBlur2D(data, radius.y(), radius.x());
}

template<class AR2D>
void	ExponentialBlur2D(AR2D &data, const point2_F64 &radius, const blur_directions_2 &directions)
{
	if (data.empty()) return;
	static	const typename AR2D::scalar_type	normalizer_factor =
			multiply_normalizer(typename AR2D::scalar_type());

	if(radius.y() && directions.y())
	{
		typename AR2D::scalar_type av = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius.y())*normalizer_factor);
		switch(directions.y())
		{
			case 	exponential_blur_forward:
				for(size_t j = 0; j < data.hsize(); j++) exponential_blur_algorithms::blur_1d_forward(&data.col(j).at(0), data.vsize(), data.vstep(), av);
				break;

			case	exponential_blur_reverse:
				for(size_t j = 0; j < data.hsize(); j++) exponential_blur_algorithms::blur_1d_reverse(&data.col(j).at(0), data.vsize(), data.vstep(), av);
				break;

			case	biexponential_blur:
				for(size_t j = 0; j < data.hsize(); j++)
				{
				//exponential_blur_algorithms::blur_1d_bidirectional(&data.col(j).at(0), data.vsize(), data.vstep(), av);
					BiexpBlur1D(data.col(j), av);
				}
				break;
		}
	}


	if(radius.x() && directions.x())
	{
		typename AR2D::scalar_type ah = typename AR2D::scalar_type(
				ExponentialFlterCoefficient(radius.x())*normalizer_factor);

		switch(directions.x())
		{
			case 	exponential_blur_forward:
				for(size_t i = 0; i < data.vsize(); i++) exponential_blur_algorithms::blur_1d_forward(&data.row(i).at(0), data.hsize(), data.hstep(), ah);
				break;

			case	exponential_blur_reverse:
				for(size_t i = 0; i < data.vsize(); i++) exponential_blur_algorithms::blur_1d_reverse(&data.row(i).at(0), data.hsize(), data.hstep(), ah);
				break;

			case	biexponential_blur:
				for(size_t i = 0; i < data.vsize(); i++)
				{
// 					exponential_blur_algorithms::blur_1d_bidirectional(&data.row(i).at(0), data.hsize(), data.hstep(), ah);
					BiexpBlur1D(data.row(i), ah);
				}
				break;
		}
	}
}


//--------------------------------------------------------------




template <class F2DT>
void ExponentialBlur3D(DataArrayMD<F2DT> &data, const point3_F64 &radius, const blur_directions_3 &directions, omp_usage_t omp_usage)
{
	if (data.empty()) return;
	size_t coord0;
	MaxValue(data.steps(), &coord0);

	size_t	coord1 = FilterMDAuxiliaries::GetCoordNo(coord0, 0);
	size_t	coord2 = FilterMDAuxiliaries::GetCoordNo(coord0, 1);
	index_vector	access_v(3);

	static	const typename F2DT::scalar_type	normalizer_factor =
			multiply_normalizer(typename F2DT::scalar_type());

	if(data.n_dimensions() !=3)
	{
		ForceDebugBreak();
		throw invalid_argument("BiexpBlur3D(DataArrayMD<F2DT> &data, const point3_F64 &radius, const blur_directions_3 &directions), invalid number of dimensions");
	}
	if(data.steps(coord1)>data.steps(coord2))
	{
		access_v[coord1] = slice_mask(0);
		access_v[coord2] = slice_mask(1);
	}
	else
	{
		access_v[coord1] = slice_mask(1);
		access_v[coord2] = slice_mask(0);
	}

	MathFunction<F2DT, double, typename F2DT::field_tag> slices(data.sizes(coord0));
	for(size_t i = 0; i < data.sizes(coord0); ++i)
	{
		access_v[coord0] = i;
		data.GetSlice(slices[i], access_v);
	}
	if(omp_usage == e_use_omp)
	{
		ThreadErrorCollector ec("ExponentialBlur3D");
		//ради omp используется индекс со знаком
#pragma omp parallel for schedule (guided)
		for(ptrdiff_t i = 0; i < ptrdiff_t(data.sizes(coord0)); ++i)
		{
			if (ec.HasErrors())
			{
#ifdef XRAD_COMPILER_MSC
				break;
#else
				continue;
#endif
			}
			ThreadSetup ts; (void)ts;
			try
			{
				ExponentialBlur2D(slices[i], point2_F64(radius.y(), radius.x()), blur_directions_2(directions.y(), directions.x()));
			}
			catch (...)
			{
				ec.CatchException();
			}
		}
		ec.ThrowIfErrors();
	}
	else
	{
		for(size_t i = 0; i < data.sizes(coord0); ++i)
		{
			ExponentialBlur2D(slices[i], point2_F64(radius.y(), radius.x()), blur_directions_2(directions.y(), directions.x()));
		}
	}

	if(radius.z()>0)
	{
		typename F2DT::scalar_type az = typename F2DT::scalar_type(
				ExponentialFlterCoefficient(radius.z())*normalizer_factor);
		ExponentialBlur1D(slices, az, directions.z());
	}
}


template <class F2DT>
void BiexpBlur3D(DataArrayMD<F2DT> &data, const typename F2DT::scalar_type &radius_z, const typename F2DT::scalar_type &radius_y, const typename F2DT::scalar_type &radius_x, omp_usage_t omp_usage)
{
	ExponentialBlur3D(data, point3_F64(radius_z, radius_y, radius_x), blur_directions_3(biexponential_blur, biexponential_blur, biexponential_blur), omp_usage);
}

template <class F2DT>
void	BiexpBlur3D(DataArrayMD<F2DT> &data, const point3_F64 &radius, omp_usage_t omp_usage)
{
	ExponentialBlur3D(data, radius, blur_directions_3(biexponential_blur, biexponential_blur, biexponential_blur), omp_usage);
}



XRAD_END
