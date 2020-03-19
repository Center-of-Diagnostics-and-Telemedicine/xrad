#include "pre.h"
#include "UniversalInterpolation2D.h"
#include <XRADBasic/Sources/Math/SpecialFunctions.h>
#include "InterpolationAuxiliaries.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	Принципы работы двумерных интерполяторов
//
//	Точно такая же концепция используется в одномерном случае (UniversalInterpolation.h).
//
//	1.
//	Интерполятор представляет собой набор ких-фильтров, каждый из которых соответствует интерполяционному
//	сдвигу на некоторую величину от 0 до 1. Интерполяторы используются функцией in класса MathFunction2D<>.
//
//	2.
//	У любого интерполятора есть функция GetNeededFilter(double v, double h), которая
//	возвращает фильтр, нужный для получения интерполированного значения в точке (v,h).
//
//	3.
//	Интерполятор создается с помощью функции GenerateInterpolator(n_divisions, generator).
//	Здесь n_divisions -- максимальная дискретность выходных данных по отношению к входным (то есть, на 1
//	отсчет входных данных можно получить n_divisions отсчетов выходных). По умолчанию используется
//	величина default_interpolator_division, которая равна 32.
//
//	4.
//	Generator представляет собой указатель на объект класса, наследуемого от абстрактного класса
//	InterpolationFilterGenerator2D. У него есть пустой виртуальный метод GenerateFilter(FIRFilter&,...).
//
//	5.
//	Для каждого конкретного вида интерполяции пишется свой класс-генератор. Особенности создания
//	нужного фильтра выписываются в методе GenerateFilter(). А параметры инициализации конкретного
//	типа фильтра пишутся в конструкторе.
//
//	6.
//	Примеры использования:
//	RealFunction2D_F64	r(n);
//	MathFunction2D_CF64 c(n);
//
//	RealInterpolator2D biquadratic;
//	RealInterpolator2D bisinc;
//	biquadratic.InitFilters(8, &BSplineFilterGenerator2D<FIRFilter2DReal>(2)); // инициализируется сплайн 2 порядка
//	bisinc.InitFilters(16, &SincFilterGenerator2D<FIRFilter2DReal>(8,8));	// фильтр на основе sinc 8 порядка
//
//	double	a = r.in(2.3, 1.2 &biquadratic); // квадратичная сплайн-интерполяция действительной функции
//	complexF64 b = c.in(10.5, 2.3, &bisinc);	// спектральная интерполяция комплексной функции (используется действительный фильтр)
//
//	7.
//	Для комплексных данных со смещенной центральной частотой существует специальный генератор,
//	который создает комплексную версию любого действительного фильтра, меняя в нем только
//	частоту осцилляции. Конструктор этого класса использует ссылку на любой действительный генератор,
//	а также частоты осцилляции:
//
//	ComplexInterpolator2D biquadratic_oscillation(SplineComplexFilterGenerator2D(2, 0, 0.5));
//		// по второй координате осцилляция с частотой, равной 0.5 частоты дискретизации
//	b = c.in(5.5, 7.5, &biquadratic_oscillation);
//
//--------------------------------------------------------------

template<class FILTER>
void	InterpolationFilterGenerator2D<FILTER>::SetOffsetCorrection(double &offset_correction_v, double &offset_correction_h) const
{
	// При нечетном порядке фильтра возникает сдвиг на полотсчета,
	// который нужно скорректировать.

	if(filter_order_v & 0x00000001) offset_correction_v = 0.5;
	else offset_correction_v = 0;

	if(filter_order_h & 0x00000001) offset_correction_h = 0.5;
	else offset_correction_h = 0;
}



//--------------------------------------------------------------
//
//	Двумерные сплайн-интерполяторы

template<class FILTER>
double	BSplineFilterGenerator2D<FILTER>::GenerateFilter(FILTER &filter, double v, double h) const
{
	RealFunctionF64	fv = CalculateBSpline(spline_order, v);
	RealFunctionF64	fh = CalculateBSpline(spline_order, h);
	filter.realloc(filter_order_v, filter_order_h);
	//calculate a filter
	double	divisor = 0;
	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	value = fv[i] * fh[j];
			divisor += value;
			filter.at(i,j) = value;
		}
	}
	Apply_AS_2D_F2(filter, divisor, Functors::divide_assign()); // filters/=divisor;
	return 1;
}

template<class FILTER>
double	ISplineFilterGenerator2D<FILTER>::GenerateFilter(FILTER &filter, double v, double h) const
{
	RealFunctionF64	fv = CalculateISpline(spline_order, v);
	RealFunctionF64	fh = CalculateISpline(spline_order, h);
	filter.realloc(filter_order_v, filter_order_h);
	//calculate a filter
	double	divisor = 0;
	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	value = fv[i] * fh[j];
			divisor += value;
			filter.at(i,j) = value;
		}
	}
	Apply_AS_2D_F2(filter, divisor, Functors::divide_assign()); // filters/=divisor;
	return 1;
}



//--------------------------------------------------------------
//
//	Интерполятор, основанный на функции sin(x)/x,
//	дает наилучшую спектральную точность, однако
//	нарушает изотропность входных данных, а также
//	приводит к появлению артефактов (эффект ѓиббса).

template<class FILTER>
SincFilterGenerator2D<FILTER>::SincFilterGenerator2D(int fov, int foh) : InterpolationFilterGenerator2D<FILTER>(fov, foh)
{
}

template<class FILTER>
double	SincFilterGenerator2D<FILTER>::GenerateFilter(FILTER &filter,
		double v,
		double h) const
{
	RealFunctionF64	fv = CalculateSincInterpolator(filter_order_v, v);
	RealFunctionF64	fh = CalculateSincInterpolator(filter_order_h, h);
	filter.realloc(filter_order_v, filter_order_h);

	//calculate a filter
	double	divisor = 0;
	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	value = fv[i] * fh[j];
			divisor += value;
			filter.at(i,j) = value;
		}
	}
	Apply_AS_2D_F2(filter, divisor, Functors::divide_assign()); // filters/=divisor;
	return 1;
}



//--------------------------------------------------------------
//
//	Интерполятор, основанный на функции J1(r)/r,
//	J1 -- функция бесселя первого рода первого порядка,
//	r = sqrt(x^2 + y^2)
//	сохраняет изотропность входных данных, однако дает
//	некоторую потерю резкости. Также как и sinc,
//	приводит к появлению артефактов (эффект ѓиббса).
//
//	У этого фильтра порядок по обеим координатам всегда
//	один и тот же, так как фильтр используется ради
//	сохранения изотропии изображения.

template<class FILTER>
BesselFilterGenerator<FILTER>::BesselFilterGenerator(int fo, double br) : InterpolationFilterGenerator2D<FILTER>(fo), bessel_radius(br)
{
	if(!br)
	{
		ForceDebugBreak();
		throw invalid_argument("BesselFilterGenerator -- zero filter radius");
	}
}

template<class FILTER>
double	BesselFilterGenerator<FILTER>::GenerateFilter(FILTER &filter,
		double v,
		double h) const
{

	double	strobe_radius = BesselInterpolatorStrobeRadius(filter_order_v, bessel_radius);

	double	s2 = double(filter_order_v)/2 - 1;
	double	divisor(0);
	//initialize filter parameters
	filter.realloc(filter_order_v, filter_order_h);

	//calculate a filter
	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	dv = (double)i - s2 - v;
			double	dh = (double)j - s2 - h;
			double	strobe = 	gauss(dh, strobe_radius)*
				gauss(dv, strobe_radius);

			double	r = std::hypot(dh, dv);

			if(!r) r = 1.e-5;
			double	argument = r*pi()*bessel_radius;

			if(r > s2)
			{
				filter.at(i, j) = 0;
			}
			else
			{
				double	value = 2.*strobe*Jn(argument, 1)/argument;
				filter.at(i, j) = value;
				divisor += value;
			}

		}
	}
	Apply_AS_2D_F2(filter, divisor, Functors::divide_assign()); // filters/=divisor;

	return 1;
}



BesselDerivativeFilterGenerator::BesselDerivativeFilterGenerator(int fo, double br, double dd) : InterpolationFilterGenerator2D<FIRFilter2DReal>(fo), bessel_radius(br), derivative_direction(dd)
{
	if(fo<9 || fo > 16)
	{
		ForceDebugBreak();
		throw invalid_argument("BesselDerivativeFilterGenerator::BesselDerivativeFilterGenerator -- invalid filter order");
		//порядок фильтра всегда нечетный, кроме случая линейной интерполяции
	}
};

double	BesselDerivativeFilterGenerator::BesselDerivative(double r0, double x0) const
{
	// Вычисляет функцию
	//
	//	d
	//	---(J1(r)/r)
	//	dx
	//
	// в точке (x, y=sqrt(r*r-x*x))
	double	r = r0*pi()*bessel_radius;
	double	x = x0*pi()*bessel_radius;
	double	factor_1 = x/cube(r);
	double	factor_2 = J1prim1(r)*r - Jn(r, 1);

	return (factor_1*factor_2)*(pi()*bessel_radius);
}


double	BesselDerivativeFilterGenerator::GenerateFilter(FIRFilter2DReal &filter,
		double v,
		double h) const

		// derivative_direction значит направление дифференцирования.
		// 0 значит d/dv. Положительная производная означает рост сверху вниз.
		// pi()/2	значит d/dh. Положительная производная означает рост слева направо.
		// Увеличение аргумента означает поворот направления дифференцирования против часовой стрелки.
{
	const double	strobe_radius = BesselDerivativeInterpolatorStrobeRadius(filter_order_v);
	const double	amplification = BesselDerivativeInterpolatorNormalizer(filter_order_v);

	double	s2 = double(filter_order_v)/2 - 1;
	filter.realloc(filter_order_v, filter_order_h);

	double	average = 0;
	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	dv = (double)i - s2 - v;
			double	dh = (double)j - s2 - h;
			double	r = std::hypot(dh, dv);

			if(!r) r = 1.e-7;

			if(r > s2)
			{
				filter.at(i, j) = 0;
			}
			else
			{
				double	strobe = gauss(dh, strobe_radius) * gauss(dv, strobe_radius);
				double	value = -strobe*amplification*
					(BesselDerivative(r, dv)*cos(derivative_direction) +
					BesselDerivative(r, dh)*sin(derivative_direction));

				filter.at(i, j) = value;
				average += value;
			}
		}
	}

	average /= (filter_order_v*filter_order_h);
	Apply_AS_2D_F2(filter, average, Functors::minus_assign()); // filters/=divisor;
	return 1;
}



//--------------------------------------------------------------
//

template<class FILTER>
QuasiSplineIsotropicFilterGenerator2D<FILTER>::QuasiSplineIsotropicFilterGenerator2D(double bo) : bessel_order(bo), InterpolationFilterGenerator2D<FILTER>(round(bo))
{
}

template<class FILTER>
double	QuasiSplineIsotropicFilterGenerator2D<FILTER>::GenerateFilter(FILTER &filter,
		double v,
		double h) const
{
	double	s2 = double(filter_order_v)/2 - 1;
	double	argument_scale = double(filter_order_v)/2;

	double	divisor(0);
	//initialize filter parameters
	filter.realloc(filter_order_v, filter_order_h);

	//calculate a filter
	const double	strobe_radius = SincInterpolatorStrobeRadius(filter_order_v);

	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	dv = (double)i - s2 - v;
			double	dh = (double)j - s2 - h;

			double	r = std::hypot(dh, dv);

			if(r >= argument_scale)
			{
				filter.at(i, j) = 0;
			}
			else
			{
				double	strobe = gauss(r, strobe_radius);
				double	value = strobe * CalculateQuasiSpline(r/argument_scale, bessel_order);
				filter.at(i, j) = value;
				divisor += value;
			}

		}
	}
	if(divisor)
		Apply_AS_2D_F2(filter, divisor, Functors::divide_assign()); // filters/=divisor;
	divisor = 1;

	return 1;
}



//--------------------------------------------------------------
//
//	Единственный специфичный метод класса ComplexInterpolatorGenerator2D
//
//--------------------------------------------------------------

template<class CFILTER_GENERATOR>
void	ComplexInterpolatorGenerator2D<CFILTER_GENERATOR>::SetFilterCarrier(typename CFILTER_GENERATOR::filter_type &filter, double v, double h) const
{
	int	vs2 = filter_order_v/2;
	int	hs2 = filter_order_h/2;

	for(int i = 0; i < filter_order_v; i++)
	{
		for(int j = 0; j < filter_order_h; j++)
		{
			double	dv = (double)i - vs2 - v;
			double	dh = (double)j - hs2 - h;

			filter.at(i, j) *= polar(1., -two_pi()*(carrier_v*dv + carrier_h*dh));
		}
	}
}

//	конец генераторов



//--------------------------------------------------------------
//
//	Методы класса UniversalInterpolator2D
//
//--------------------------------------------------------------

//TODO int -> size_t
template<class FILTER>
void	UniversalInterpolator2D<FILTER>::InitFilters(int in_n_divisions_v, int in_n_divisions_h, const InterpolationFilterGenerator2D<FILTER> &generator)
{
	n_filters_v = in_n_divisions_v;
	n_filters_h = in_n_divisions_h;

	InterpolationFilters.realloc(n_filters_v, n_filters_h);
	double	average(0);

	for(int i = 0; i < n_filters_v; i++)
	{
		for(int j = 0; j < n_filters_h; j++)
		{
			double	v = double(i)/n_filters_v;
			double	h = double(j)/n_filters_h;

			average += generator.GenerateFilter(
				InterpolationFilters.at(i, j),
				v, h);
		}
	}
	average /= (n_filters_v*n_filters_h);

	// Ниже уточняем нормировку фильтра
	for(int i = 0; i < n_filters_v; i++)
	{
		for(int j = 0; j < n_filters_h; j++)
		{
			for(size_t k = 0; k < InterpolationFilters.at(i, j).vsize(); ++k)
			{
				for(size_t l = 0; l < InterpolationFilters.at(i, j).hsize(); ++l)
				{
					InterpolationFilters.at(i, j).at(k, l) /= average;
				}
			}
		}
	}
	generator.SetOffsetCorrection(v_offset, h_offset);
}



//--------------------------------------------------------------
//
//	Данные класса interpolators2D
//
//--------------------------------------------------------------

const	int	interpolators2D::default_interpolator_division = 16;
const	int	interpolators2D::default_complex_interpolator_division = 32;
const	int	interpolators2D::default_derivative_division = 32;
// "Дробность" интерполяторов, равная 16, может оказаться недостаточной
// для комплексных осциллирующих данных и для вычисления производных.

//const	int	interpolators2D::default_filter_size = 15;
const	double	interpolators2D::besselRadiusMIN_LOST = 1.1285;
const	double	interpolators2D::besselRadius_ISOTROPIC = 1./sqrt_2();

//	Real filters (are applicable to complex or rgb data too)

RealInterpolator2D interpolators2D::nearest_neighbour;
RealInterpolator2D interpolators2D::bilinear;
RealInterpolator2D interpolators2D::biquadratic;
RealInterpolator2D interpolators2D::bicubic;
RealInterpolator2D interpolators2D::ibicubic;

RealInterpolator2D interpolators2D::sinc;
RealInterpolator2D interpolators2D::bessel1_isotropic;
RealInterpolator2D interpolators2D::bessel1_min_lost;

RealInterpolator2D interpolators2D::quasi_spline_2;
RealInterpolator2D interpolators2D::quasi_spline_5;

UniversalInterpolator2D <FIRFilter2DReal>interpolators2D::bessel_ddx;
RealInterpolator2D interpolators2D::bessel_ddy;

//	Complex filters with carrier (are applicable to complex data only)

ComplexInterpolator2D	interpolators2D::complex_biquadratic;
ComplexInterpolator2D	interpolators2D::complex_sinc;
ComplexInterpolator2D	interpolators2D::complex_bessel;

ComplexInterpolator2D	interpolators2D::complex_biquadraticT;
ComplexInterpolator2D	interpolators2D::complex_sincT;
ComplexInterpolator2D	interpolators2D::complex_besselT;

ComplexInterpolator2D	interpolators2D::complex_bessel_2osc;
ComplexInterpolator2D	interpolators2D::complex_bessel_2oscT;



void	interpolators2D::Init(ProgressProxy pproxy)
{
	ProgressBar	progress(pproxy);
	progress.start("Initializing 2D interpolators", 14);
	sinc.InitFilters(default_interpolator_division, default_interpolator_division, SincFilterGenerator2D<FIRFilter2DReal>(8, 8));

	++progress;
	nearest_neighbour.InitFilters(default_interpolator_division, default_interpolator_division, BSplineFilterGenerator2D<FIRFilter2DReal>(0));
	bilinear.InitFilters(default_interpolator_division, default_interpolator_division, BSplineFilterGenerator2D<FIRFilter2DReal>(1));
	biquadratic.InitFilters(default_interpolator_division, default_interpolator_division, BSplineFilterGenerator2D<FIRFilter2DReal>(2));
	bicubic.InitFilters(default_interpolator_division, default_interpolator_division, BSplineFilterGenerator2D<FIRFilter2DReal>(3));
	ibicubic.InitFilters(default_interpolator_division, default_interpolator_division, ISplineFilterGenerator2D<FIRFilter2DReal>(3));

	++progress;
	bessel1_isotropic.InitFilters(default_interpolator_division, default_interpolator_division, BesselFilterGenerator<FIRFilter2DReal>(10, besselRadius_ISOTROPIC));
	++progress;
	bessel1_min_lost.InitFilters(default_interpolator_division, default_interpolator_division, BesselFilterGenerator<FIRFilter2DReal>(10, besselRadiusMIN_LOST));

	quasi_spline_2.InitFilters(default_interpolator_division, default_interpolator_division, QuasiSplineIsotropicFilterGenerator2D<FIRFilter2DReal>(2));
	quasi_spline_5.InitFilters(default_interpolator_division, default_interpolator_division, QuasiSplineIsotropicFilterGenerator2D<FIRFilter2DReal>(5));

	++progress;

	// Для производных порядок увеличиваем, так как эта операция
	// более чувствительна к ошибке.
	bessel_ddx.InitFilters(default_interpolator_division, default_derivative_division, BesselDerivativeFilterGenerator(12, besselRadius_ISOTROPIC, pi()/2));
	++progress;

	bessel_ddy.InitFilters(default_derivative_division, default_interpolator_division, BesselDerivativeFilterGenerator(12, besselRadius_ISOTROPIC, 0));
	++progress;

	// Для комплексных данных увеличивается количество
	// делений по осциллирующей координате.
	complex_biquadratic.InitFilters(default_interpolator_division, default_complex_interpolator_division, SplineComplexFilterGenerator2D(2, 0, 0.5));
	++progress;
	complex_sinc.InitFilters(default_interpolator_division, default_complex_interpolator_division, SincComplexFilterGenerator2D(10, 10, 0, 0.5));
	++progress;
	complex_bessel.InitFilters(default_interpolator_division, default_complex_interpolator_division, BesselComplexFilterGenerator(10, besselRadius_ISOTROPIC, 0, 0.5));
	++progress;

	complex_biquadraticT.InitFilters(default_interpolator_division, default_complex_interpolator_division, SplineComplexFilterGenerator2D(2, 0, -0.5));
	++progress;
	complex_sincT.InitFilters(default_interpolator_division, default_complex_interpolator_division, SincComplexFilterGenerator2D(10, 10, 0, -0.5));
	++progress;
	complex_besselT.InitFilters(default_interpolator_division, default_complex_interpolator_division, BesselComplexFilterGenerator(10, besselRadius_ISOTROPIC, 0, -0.5));
	++progress;

	complex_bessel_2osc.InitFilters(default_complex_interpolator_division, default_complex_interpolator_division, BesselComplexFilterGenerator(10, besselRadius_ISOTROPIC, 0.5, 0.5));
	++progress;
	complex_bessel_2oscT.InitFilters(default_complex_interpolator_division, default_complex_interpolator_division, BesselComplexFilterGenerator(10, besselRadius_ISOTROPIC, -0.5, -0.5));
//	pproxy->EndProgress();
}



//--------------------------------------------------------------
//
//	Главная процедура инициализации всех фильтров, которую следует вызывать
//	до того, как ими пользоваться. Обычно это делается в начале main().
//
//--------------------------------------------------------------

void	Init2DInterpolators(ProgressProxy progress)
{
	interpolators2D::Init(progress);
}

XRAD_END
