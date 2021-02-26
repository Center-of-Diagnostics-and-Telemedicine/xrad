/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "UniversalInterpolation.h"
#include "InterpolationAuxiliaries.h"
#include "DataArrayAnalyze.h"


XRAD_BEGIN


#pragma message ("interpolators, to do (not urgent): see comment")
//! \file
//! \todo interpolators, to do (not urgent): see comment
//--------------------------------------------------------------
//	1.сделать еще дифференцирующий фильтр для осциллирующих данных -- сделано
//	2.гильберта настроить

template<class FILTER>
double	BSplineFilterGenerator<FILTER>::GenerateFilter(FILTER &filter, double x) const
{
	// x = 0...1;
	filter.realloc(filter_order);

	//calculate a filter
	real_interpolator_container	f = CalculateBSpline(spline_order, x);
	CopyData(filter, f);
	return 1;
}

template<class FILTER>
double	ISplineFilterGenerator<FILTER>::GenerateFilter(FILTER &filter, double x) const
{
	// x = 0...1;
	filter.realloc(filter_order);

	//calculate a filter
	real_interpolator_container	f = CalculateISpline(spline_order, x);
	CopyData(filter, f);
	return 1;
}



//--------------------------------------------------------------
//
//	Интерполятор, основанный на функции sin(x)/x,
//	дает наилучшую спектральную точность, однако
//	приводит к появлению артефактов (эффект Гиббса)

template<class FILTER>
SincFilterGenerator<FILTER>::SincFilterGenerator(int fo) : InterpolationFilterGenerator<FILTER>(fo)
{
}

template<class FILTER>
double	SincFilterGenerator<FILTER>::GenerateFilter(FILTER &filter,
		double x) const
{
	filter.realloc(filter_order);

	real_interpolator_container	f = CalculateSincInterpolator(filter_order, x);
	f /= (AverageValue(f)*f.size());
	// Сумма всех элементов интерполятора должна быть равна 1.
	// Это условие имеет решающее влияние на качество интерполяции.
	// Нормировку нельзя делать внутри функции CalculateSincInterpolator,
	// так как эта же функция используется при построении двумерных
	// интерполяторов, которые должны иметь единую нормировку по обеим
	// координатам.

	CopyData(filter, f);
	return 1;
}



//--------------------------------------------------------------
//
//	Дифференцирующий интерполятор, основанный на производной
//	функции sin(x)/x
//

SincDerivativeFilterGenerator::SincDerivativeFilterGenerator(int fo) : InterpolationFilterGenerator<FilterKernelReal>(fo)
{
}

double	SincDerivativeFilterGenerator::GenerateFilter(FilterKernelReal &filter,
		double x) const
{
	filter.realloc(filter_order);

	real_interpolator_container	f = CalculateSincDerivativeInterpolator(filter_order, x);
	f -= AverageValue(f);
	// В отличие от простого интерполятора, условие нормировки
	// здесь -- нулевая сумма всех элементов.

	CopyData(filter, f);
	return 1;
}

SincDerivativeComplexFilterGenerator::SincDerivativeComplexFilterGenerator(int fo, double in_carrier) : InterpolationFilterGenerator<FilterKernelComplex>(fo), carrier(in_carrier)
{
}

double	SincDerivativeComplexFilterGenerator::GenerateFilter(FilterKernelComplex &filter,
		double x) const
{
	filter.realloc(filter_order);

	complex_interpolator_container	f = CalculateSincDerivativeCarrierInterpolator(filter_order, x, carrier);
	f -= AverageValue(f);
	// В отличие от простого интерполятора, условие нормировки
	// здесь -- нулевая сумма всех элементов.

	CopyData(filter, f);
	return 1;
}



//--------------------------------------------------------------
//
//	Фильтр для преобразования Гильберта, основанный на
//	функции sin(x)/x
//

HilbertFilterGenerator::HilbertFilterGenerator(int fo, ComplexNumberPart cp) : InterpolationFilterGenerator<FilterKernelReal>(fo), complex_number_part(cp)
{
}

double	HilbertFilterGenerator::GenerateFilter(FilterKernelReal &filter, double x) const
{
	filter.realloc(filter_order);

	real_interpolator_container	f = CalculateSincHilbertInterpolator(filter_order, x, complex_number_part);
	//f -= AverageValue(f);
	// В отличие от простого интерполятора, условие нормировки
	// здесь -- нулевая сумма всех элементов.

	CopyData(filter, f);
	return 1;
}



//--------------------------------------------------------------
//
//	Изменение несущей частоты комплексного интерполирующего фильтра.
//	Единственный специфичный метод класса ComplexInterpolatorGenerator
//
//--------------------------------------------------------------

template<class CFILTER_GENERATOR>
void	ComplexInterpolatorGenerator<CFILTER_GENERATOR>::SetFilterCarrier(typename CFILTER_GENERATOR::filter_type &filter, double x) const
{
	int	s2 = double(filter_order)/2 - 1;

	for(int i = 0; i < filter_order; i++)
	{
		double	dx = (double)i - s2 - x;

		filter.at(i) *= polar(1., -two_pi()*carrier*dx);
	}
}



//--------------------------------------------------------------
//
//	Методы класса UniversalInterpolator
//
//--------------------------------------------------------------



template<class FILTER>
//void	UniversalInterpolator<FILTER>::InitFilters(int in_n_filters, const InterpolationFilterGenerator<FILTER>* generator)
void	UniversalInterpolator<FILTER>::InitFilters(int in_n_filters, const InterpolationFilterGenerator<FILTER> &generator)
{
	n_filters = in_n_filters;
	InterpolationFilters.realloc(n_filters);
	double	average(0);

	for(int i = 0; i < n_filters; i++)
	{
		double	x = double(i)/n_filters;

		//		average += generator->GenerateFilter(InterpolationFilters.at(i), x);
		average += generator.GenerateFilter(InterpolationFilters.at(i), x);
	}
	average /= n_filters;

	// Ниже уточняем нормировку фильтра
	for(int i = 0; i < n_filters; ++i)
	{
		for(int j = 0; j < generator.filter_order; ++j)
		{
			InterpolationFilters[i][j] /= average;
		}
	}
	generator.SetOffsetCorrection(x_offset);
}



UniversalInterpolator<FilterKernelReal> interpolators::nearest_neighbour;
UniversalInterpolator<FilterKernelReal> interpolators::linear;
UniversalInterpolator<FilterKernelReal> interpolators::quadratic;
UniversalInterpolator<FilterKernelReal> interpolators::cubic;
UniversalInterpolator<FilterKernelReal> interpolators::icubic;

UniversalInterpolator<FilterKernelReal> interpolators::sinc;
UniversalInterpolator<FilterKernelReal> interpolators::sinc_derivative;

// Прямая фаза, carrier = 0.5
UniversalInterpolator<FilterKernelComplex> interpolators::complex_icubic;
UniversalInterpolator<FilterKernelComplex> interpolators::complex_sinc;
UniversalInterpolator<FilterKernelComplex> interpolators::complex_sinc_derivative;

// Обратная фаза, carrier = -0.5
UniversalInterpolator<FilterKernelComplex> interpolators::complex_icubicT;
UniversalInterpolator<FilterKernelComplex> interpolators::complex_sincT;
UniversalInterpolator<FilterKernelComplex> interpolators::complex_sinc_derivativeT;

// Обратная фаза, carrier = -0.5
UniversalInterpolator<FilterKernelReal> interpolators::hilbert_re;
UniversalInterpolator<FilterKernelReal> interpolators::hilbert_im;
UniversalInterpolator<FilterKernelReal> interpolators::hilbert_re8;
UniversalInterpolator<FilterKernelReal> interpolators::hilbert_im8;

const	int	default_interpolation_factor = 128;

interpolators::interpolators()
{
	nearest_neighbour.InitFilters(default_interpolation_factor, BSplineFilterGenerator<FilterKernelReal>(0));
	linear.InitFilters(default_interpolation_factor, BSplineFilterGenerator<FilterKernelReal>(1));

	quadratic.InitFilters(default_interpolation_factor, BSplineFilterGenerator<FilterKernelReal>(2));
	cubic.InitFilters(default_interpolation_factor, BSplineFilterGenerator<FilterKernelReal>(3));
	icubic.InitFilters(default_interpolation_factor, ISplineFilterGenerator<FilterKernelReal>(3));

	sinc.InitFilters(default_interpolation_factor, SincFilterGenerator<FilterKernelReal>(8));
	sinc_derivative.InitFilters(default_interpolation_factor, SincDerivativeFilterGenerator(14));
		// 14 -- этому фильтру нужен порядок выше, так как на производных
		// погрешность более заметна.

	//	Интерполяторы с поворотом фазы для комплексных данных
	//
	//	По поводу параметра carrier при инициализации комплексных
	//	интерполяторов с несущей:
	//	-0.5 -- такая фаза в таблицах ATL (analytic.LUT);
	//	+0.5 -- соответствует правильной школьной формуле Фурье-преобразования.

	complex_icubic.InitFilters(default_interpolation_factor, ISplineComplexFilterGenerator(3, 0.5));
	complex_sinc.InitFilters(default_interpolation_factor, SincComplexFilterGenerator(8, 0.5));
	complex_sinc_derivative.InitFilters(default_interpolation_factor, SincDerivativeComplexFilterGenerator(14, 0.5));

	complex_icubicT.InitFilters(default_interpolation_factor, BSplineComplexFilterGenerator(3, -0.5));
	complex_sincT.InitFilters(default_interpolation_factor, SincComplexFilterGenerator(8, -0.5));
	complex_sinc_derivativeT.InitFilters(default_interpolation_factor, SincDerivativeComplexFilterGenerator(14, -0.5));

	//	Преобразование Гильберта (недописано)

	hilbert_re.InitFilters(default_interpolation_factor, HilbertFilterGenerator(16, real_part));
	hilbert_im.InitFilters(default_interpolation_factor, HilbertFilterGenerator(16, imag_part));

	hilbert_re8.InitFilters(default_interpolation_factor, HilbertFilterGenerator(8, real_part));
	hilbert_im8.InitFilters(default_interpolation_factor, HilbertFilterGenerator(8, imag_part));
}

namespace
{
// Помещается здесь ради вызова конструктора, в котором будут проинициализированы
// статические члены. В отличие от двумерных интерполяторов, он должен вычисляться
// мгновенно.

// Плохо это с точки зрения обработки исключений.

interpolators	BasicInterpolatorSet;
}



XRAD_END
