//	file StatisticUtils.cpp
//	Created by ACS on 28.05.01
//--------------------------------------------------------------
#include "pre.h"
#include "StatisticUtils.h"
#include "FibonacciRandoms.h"
#include <XRADBasic/Sources/Math/SpecialFunctions.h>


XRAD_BEGIN


namespace
{
const FibonacciRandomGenerator<double>	default_generator_F64(107, 378);
const FibonacciRandomGenerator<uint32_t>	default_generator_UI32(83, 258);
const FibonacciRandomGenerator<uint16_t>	default_generator_UI16(30, 127);
}//namespace

uint16_t	RandomUniformUI16()
{
	return	default_generator_UI16.Generate();
}

uint32_t	RandomUniformUI32()
{
	return	default_generator_UI32.Generate();
}

int16_t	RandomUniformI16()
{
	uint16_t	result = default_generator_UI16.Generate();

#if XRAD_COMPILER_MSC
	return result;
#else
	// встроенное преобразование unsigned->signed зависит от платформы.
	// неизвестно, даст ли оно правильное заполнение всего диапазона. то, что здесь написано,
	// как-то это делает. наверное, можно сделать лучше. подумать.
	// то же и для 32-битного числа ниже
	return	result < numeric_limits<int16_t>::max() ?
		static_cast<uint16_t>(result) :
		-static_cast<uint16_t>(result-numeric_limits<int16_t>::max());
#endif //XRAD_COMPILER_MSC
}

int32_t	RandomUniformI32()
{
	uint32_t	result = default_generator_UI32.Generate();

#if XRAD_COMPILER_MSC
	return result;
#else
	return	result < numeric_limits<int32_t>::max() ?
		static_cast<uint32_t>(result) :
		-static_cast<uint32_t>(result-numeric_limits<int32_t>::max());
#endif //XRAD_COMPILER_MSC
}


double RandomUniformF64()
{
	return default_generator_F64.Generate();
}

double RandomUniformF64(double min, double max)
{
	if(max==min)
	{
		return max;
	}
	if(max < min)
	{
		ForceDebugBreak();
		throw invalid_argument(ssprintf("RandomUniformF64: invalid range (max=%g, min=%g)", max, min));
	}

	return RandomUniformF64()*(max-min) + min;
}

//--------------------------------------------------------------

double RandomGaussianStandard()
{
// генерируется стандартная гауссова величина с дисперсией 1 через преобразование Бокса-Мюллера.
// чтобы избежать логарифмирования нуля, магнитуду порождаем с помощью дискретного генератора,
// а результат его увеличиваем на 1

	static const FibonacciRandomGenerator<uint32_t> &magnitude_generator(default_generator_UI32);
	static const double	factor = 1./(double(magnitude_generator.MaxValue()) + 1.);

	static double	r_exp, r, fi;
	static bool generate = true;

	if(generate)
	{
		r_exp = (double(magnitude_generator.Generate()) + 1.) * factor;
		r = sqrt(-2.*log(r_exp));
		fi = two_pi() * RandomUniformF64();

		generate = false;
		return cos(fi) * r;
	}
	else
	{
		generate = true;
		return sin(fi) * r;
	}
}


double RandomGaussian(double sigma)
{
	return sigma*RandomGaussianStandard();
}

double RandomGaussian(double mean, double sigma)
{
	return mean + sigma*RandomGaussianStandard();
}


//--------------------------------------------------------------
//
//	функции плотности и функции распределения для основных законов
//	(можно добавлять по мере надобности)
//

// функция плотности и функция распределения гаусса

double	gaussian_pdf(double x, double average, double sigma)
{
	return (gauss(x-average, sigma))/(sigma*sqrt(2.*pi()));
}

double	gaussian_cdf(double x, double average, double sigma)
{
	double a = 1./(sqrt(2.*square(sigma)));
	return 0.5*(1.+SpecialFunctions::erf((x-average)*a));
}
// функция плотности пуассона
double	poisson_pdf(double x, double lambda)
{
	if(lambda<=0)
		throw invalid_argument(ssprintf("poisson_pdf(x, lambda), lambda is %lg, but must be positive",
							   lambda));
	return x<=0 ? 0 : exp(x*log(lambda) - lambda - log_gamma_function(x+1));
}

// функция плотности и функция распределения релея

double	rayleigh_pdf(double x, double sigma)
{
	if(x<=0) return 0;
	double a = x/square(sigma);
	return a*exp(-x*a/2);
}

double	rayleigh_cdf(double x, double sigma)
{
	if(x<=0) return 0;
	return 1.-exp(-0.5*square(x/sigma));
}

// функция плотности и функция распределения райса

double	rician_pdf(double x, double nu, double sigma)
{
	if(x<=0) return 0;
	double	sigma_square = sigma*sigma;
	return (x/sigma_square)*exp(-square(x-nu)/(2*sigma_square))*In_exp(x*nu/sigma_square, 0);
}

double	rician_cdf(double x, double nu, double sigma)
{
	return x <= 0 ? 0 : 1 - Qm(nu/sigma, x/sigma, 1);
}


double	log_rician_pdf(double x, double nu, double sigma)
{
	double sigma_square = square(sigma);
	return log(x/sigma_square) -
		((x*x + nu*nu)/(2*sigma_square)) +
		log_In(x*nu/sigma_square, 0);
}

//
//--------------------------------------------------------------

XRAD_END