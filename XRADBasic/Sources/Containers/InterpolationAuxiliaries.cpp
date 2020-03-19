#include "pre.h"
#include "InterpolationAuxiliaries.h"
#include <XRADBasic/LinearVectorTypes.h>

XRAD_BEGIN

//--------------------------------------------------------------
//	формулы стробирования

double	SincInterpolatorStrobeRadius(double filter_order)
{
	return 0.8 + (filter_order-1.)/20.;
	// эмпирическая формула, работает для интерполяторов с sinc
}

double	BesselInterpolatorStrobeRadius(size_t filter_order, double /*bessel_radius*/)
{
// коррекция стробирования в зависимости от радиуса фильтра
// формула эмпирическая
// double	r = double(filter_order)/(bessel_radius*sqrt(2));

	return SincInterpolatorStrobeRadius(filter_order);
//	return 0.8 + (r-1.)/20.;
}

double	BesselDerivativeInterpolatorStrobeRadius(size_t filter_order)
{
	const size_t N = 16;
	static	double	strobes[N] ={
		1.00000,	// filter_order = 1
		1.00000,	// filter_order = 2
		1.00000,	// filter_order = 3
		1.00000,	// filter_order = 4
		1.00000,	// filter_order = 5
		1.00000,	// filter_order = 6
		1.00000,	// filter_order = 7
		1.00000,	// filter_order = 8
		//
		0.9500000,	// filter_order = 9 +
		1.0165000,	// filter_order = 10 A
		1.0830000,	// filter_order = 11 +
		1.1415000,	// filter_order = 12 A
		1.2000000,	// filter_order = 13 +
		1.2500000,	// filter_order = 14 A
		1.3000000,	// filter_order = 15 +
		1.3150000	// filter_order = 16 A
	};

	return strobes[filter_order-1];
}

double	BesselDerivativeInterpolatorNormalizer(size_t filter_order)
{
	const size_t N = 16;
	static	double	divisors[N] ={
		1.00000,	// filter_order = 1 -
		1.00000,	// filter_order = 2 -
		1.00000,	// filter_order = 3 -
		1.00000,	// filter_order = 4 -
		1.00000,	// filter_order = 5 -
		1.00000,	// filter_order = 6 -
		1.00000,	// filter_order = 7 -
		1.00000,	// filter_order = 8 -
		//
		0.8302215,	// filter_order = 9 +
		0.9198965,	// filter_order = 10
		0.99867,	// filter_order = 11
		1.05776,	// filter_order = 12
		1.10725,	// filter_order = 13
		1.14240,	// filter_order = 14
		1.17152,	// filter_order = 15
		1.179170	// filter_order = 16
	};

	return 1./divisors[filter_order-1];
}



//--------------------------------------------------------------
//	формулы фильтров

real_interpolator_container CalculateBSpline(size_t spline_order, double x)
{
	real_interpolator_container	result(spline_order + 1);
	//TODO	есть универсальная функция, задействовать сюда ее.
	switch(spline_order)
	{
		case 0: result[0] = 1;
			break;
		case 1:
			// линейный интерполятор, самый быстрый
			// и с самой плохой гладкостью
			result[0] = (1.-x);
			result[1] = x;
			break;
		case 2:
			// сплайн второго порядка. из-за нечетного порядка фильтра приводит
			// к сдвигу интерполированного изображения на полотсчета. на этот случай
			// во всех алгоритмах интерполяции сделана соответствующая поправка
			{
				double	t0 = x-1.;
				double	t1 = x-0.5;
				double	t2 = x;

				result[0] = square(t0)/2;
				result[1] = 0.75-square(t1);
				result[2] = square(t2)/2;
			}
			break;
		case 3:
			{
				// кубический сплайн
				double	t1 = x;
				double	t2 = 1.-t1;

				double	a = 3;
				double	b= -6;
				double	d = 4;

				result[0] = cube(t2)/6;
				result[1] = (d + a*cube(t1) + b*square(t1))/6;
				result[2] = (d + a*cube(t2) + b*square(t2))/6;
				result[3] = cube(t1)/6;
			}
			break;
		default:
			ForceDebugBreak();
			throw invalid_argument(ssprintf("CalculateSpline(size_t spline_order, double x), invalid spline order = %d", spline_order));
	};

	return result;
}



real_interpolator_container CalculateISpline(size_t spline_order, double x)
{
	real_interpolator_container	result(spline_order + 1);

	switch(spline_order)
	{
		case 0: result[0] = 1;
			break;
		case 1:
			// линейный интерполятор, самый быстрый
			// и с самой плохой гладкостью
			result[0] = (1.-x);
			result[1] = x;
			break;
		case 3:
			{
				// кубический интерполирующий сплайн
				static const RealVectorF64	coefficients0({1.,	0.0,	-4.5,	7.5});
				static const RealVectorF64	coefficients1({0.,	-0.75,	3.0,	-4.5});

				double	t1 = x;
				double	t2 = 1.-t1;

				result[0] = polynom_factorial(coefficients1, t1);
				result[1] = polynom_factorial(coefficients0, t1);
				result[2] = polynom_factorial(coefficients0, t2);
				result[3] = polynom_factorial(coefficients1, t2);
			}
			break;
		default:
			ForceDebugBreak();
			throw invalid_argument(ssprintf("CalculateSpline(size_t spline_order, double x), invalid spline order = %d", spline_order));
	};

	return result;
}
//--------------------------------------------------------------
//	формулы фильтров

double	CalculateQuasiSpline(double x, double nu)
{
	if(fabs(x)>1) return 0;
	else return pow((1. - x*x), nu-1);
}


//--------------------------------------------------------------

real_interpolator_container CalculateSincInterpolator(size_t filter_order, double x)
{
	real_interpolator_container	result(filter_order);
	double	s2 = double(filter_order)/2 - 1;
	double	strobe_radius = SincInterpolatorStrobeRadius(filter_order);
	//TODO сужение полосы сделать во всех sinc-based  и bessel-based фильтрах по нижеследующему образцу
	//TODO определить, какой строб все-таки лучше в общем случае: ранее использовавшийся гауссов или Наттолла?
	double	band_shrink_factor = 1.-4./filter_order;	// результат подбора. для идеального интерполяторе должен быть равен 1, но тогда при конечном ядре остаются неподавленные частоты.
	//nuttall_win	window_function;

	for(size_t i = 0; i < filter_order; i++)
	{
		double	dx = double(i) - s2 - x;
		double	strobe = gauss(dx, strobe_radius);
		//double	strobe = window_function((double(i+1) - x)*1000, int(filter_order*1000));
		double	value = strobe*sinc(dx*pi()*band_shrink_factor);

		//value = strobe;

		result[i] = value;
	}

	return result;
}



real_interpolator_container CalculateSincDerivativeInterpolator(size_t filter_order, double x)
{
	real_interpolator_container	result(filter_order);
	double	s2 = double(filter_order)/2 - 1;
	double	strobe_radius = SincInterpolatorStrobeRadius(filter_order);

	for(size_t i = 0; i < filter_order; i++)
	{
		double	dx = double(i) - s2 - x;
		double	strobe = gauss(dx, strobe_radius);

		result[i] = -strobe*pi()*sinc_derivative(dx*pi());
	}

	return result;
}

complex_interpolator_container CalculateSincDerivativeCarrierInterpolator(size_t filter_order, double x, double carrier)
{
	ComplexFunctionF64	result(filter_order);
	double	s2 = double(filter_order)/2 - 1;
	double	strobe_radius = SincInterpolatorStrobeRadius(filter_order);

	for(size_t i = 0; i < filter_order; i++)
	{
		double	dx = double(i) - s2 - x;
		double	strobe = gauss(dx, strobe_radius);

		// считается производная от sinc(pi*x) * exp(2*pi*i*carrier*x)
		complexF64 phasor = polar(1., -two_pi()*carrier*dx);
		result[i] = -strobe*pi()*phasor*(sinc_derivative(dx*pi()) - 2*carrier*sinc(dx*pi())*complexF64(0, 1));
	}

	return result;
}


//--------------------------------------------------------------

real_interpolator_container CalculateSincHilbertInterpolator(size_t filter_order, double x, ComplexNumberPart complex_number_part)
{
	// комментарий от старой версии фильтра (во многом не соответствует текущей, но оставляю,
	// так как что-то может пригодиться при точной настройке фильтра

	// параметр band_shrink_factor определяет сужение полосы в общем случае. оно складывается из двух
	// факторов.
	//
	// во-первых, спектр дискретизированной функции sinc не является чистым rect-ом
	// с учетом функции-окна (strob) можно считать, что дискретный спектр расширяется
	// примерно на 1 дискретный отсчет в каждую сторону. это не совсем точно (точно было бы,
	// если бы мы пользовались косинусным окном). однако косинусное окно дает худшую точность интерполяции.
	// оптимальный вид строба -- гауссоида с дисперсией strobDispersion (опыт). точно так же
	// ничего хорошего не выходит при попытках непосредственного стробирования спектра каким-либо окном.
	// в общем, это уширение спектра компенсируется параметром shrink1.
	//
	// во-вторых, полоса реального сигнала может быть уже, чем просто половина рабочей полосы.
	// так, например, в данных тайсона, где она составляет примерно треть или четверть от нее.
	// мы здесь предполагаем (пока), что эта полоса простирается все же от нулевой частоты
	// до какой-то максимальной, так что carrier равна половине максимальной частоты.
	// в этом случае, по очевидным соображениям, также следует сузить полосу фильтрации,
	// так как иначе будет пролезание в область отрицательных частот. это параметр shrink2.
	//
	// band_shrink_factor умножается на аргумент sinc и на аргумент функции-строба. на аргумент
	// фазора ее умножать не надо

	//TODO отлажена с использованием окна Наттолла и со значительным сужением полосы (0,25).
	//
	// можно попытаться расширить полосу оцифровки, при этом добавив подавление нулевой и макс. частоты путем вычитания "шапочки" из
	// добавить нормализатор как в обычном sinc интерполяторе?! в этом случае усреднять value
	// TODO исследовать разные формы окон, кроме Наттолла. Попробовать вернуть гауссово с расчетом через strobe_radius_factor

	real_interpolator_container	result(filter_order);
	//double	band_shrink_factor = 0.25;	// результат подбора. для идеального гильберта должен быть равен 0.5, но тогда при конечном ядре остаются неподавленные частоты.
	double	band_shrink_factor = 0.5*(1.-4./filter_order);	// результат подбора. для идеального гильберта должен быть равен 0.5, но тогда при конечном ядре остаются неподавленные частоты.
	double	carrier = 0.25;				// "несущая" равна четверти частоты оцифровки
	int	s2 = double(filter_order)/2 - 1;
	double	strobe_radius = SincInterpolatorStrobeRadius(double(filter_order));


	//blackman_nuttall_win	window_function;
	nuttall_window	window_function;
	//abstract_window_function	window_function;

	for(size_t i = 0; i < filter_order; i++)
	{
		double	dx = double(i) - s2 - x;
		double	strobe = gauss(dx, strobe_radius);
		//double	strobe = window_function((double(i+1) - x)*1000, int(filter_order*1000));
		double	magnitude = strobe*sinc(dx*band_shrink_factor*pi());
		double	phase = -two_pi()*carrier*dx;
		//double	phase = two_pi()*carrier*dx;

		switch(complex_number_part)
		{
			case real_part:
				result[i] = magnitude * cos(phase);
				break;
			case imag_part:
				result[i] = magnitude * sin(phase);
				break;
		};
	}
	return result;
}



XRAD_END
