#include "pre.h"
#include "WindowFunction.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	Параметрические окна
//

double	gauss_window::operator()(double x) const
{
	return gauss(2*x-1, width);
}

double	tukey_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	// Написано вчерне, есть недочеты

	double	argument = 2*x-1;
//	double	x = fabs(2.*double(n)/double(N-1) - 1.);//четная функция


	if(argument <= flatness) return 1.;
	else
	{
		double	dx = (argument-flatness) / (1.-flatness);
		return (1. + cos(pi()*dx))/2.;
	}
}

//
//--------------------------------------------------------------



//--------------------------------------------------------------
//
//	Окна среднего и выского разрешения
//

double	triangular_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return 1. - fabs(2*x-1.);
}

double	raised_cosine_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return a0 - a1*cos(2.*pi()*x);
}

//
//--------------------------------------------------------------



//--------------------------------------------------------------
//
//	Окна с большим динамическим диапазоном
//

double	nuttall_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return 0.355768 - 0.487396*cos(2.*pi()*x) + 0.144232*cos(4.*pi()*x) - 0.012604*cos(6.*pi()*x);
}

double	blackman_harris_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return 0.35875 - 0.48829*cos(2.*pi()*x) + 0.14128*cos(4.*pi()*x) - 0.01168*cos(6.*pi()*x);
}

double	blackman_nuttall_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return 0.3635819 - 0.4891775*cos(2.*pi()*x) + 0.1365995*cos(4.*pi()*x) - 0.0106411*cos(6.*pi()*x);
}

double	flat_top_window::operator()(double x) const
{
	if(!in_range(x, 0, 1)) return 0;
	return 1. - 1.93*cos(2.*pi()*x) + 1.29*cos(4.*pi()*x) - 0.388*cos(6.*pi()*x) + 0.032*cos(8.*pi()*x);
}

//
//--------------------------------------------------------------



string	GetWindowFunctionName(window_function_e wfe)
{
	switch(wfe)
	{
		case e_constant_window:
			return string("Constant");
		case e_triangular_window:
			return string("Triangular");
		case e_cos2_window:
			return string("1+cos(x)/2");
		case e_hamming_window:
			return string("Hamming");
		case e_nuttall_window:
			return string("Nuttall");
		case e_blackman_harris_window:
			return string("Blackman-Harris");
		case e_blackman_nuttall_window:
			return string("Blackman-Nuttall");
		case e_flat_top_window:
			return string("Flat top");
		default:
			return string("Unknown window");
	};
}

shared_ptr<window_function>	GetWindowFunctionByEnum(window_function_e wfe)
{
	switch(wfe)
	{
		case e_constant_window:
			return shared_ptr<window_function>(new constant_window);
		case e_triangular_window:
			return shared_ptr<window_function>(new triangular_window);
		case e_cos2_window:
			return shared_ptr<window_function>(new cos2_window);
		case e_hamming_window:
			return shared_ptr<window_function>(new hamming_window);
		case e_nuttall_window:
			return shared_ptr<window_function>(new nuttall_window);
		case e_blackman_harris_window:
			return shared_ptr<window_function>(new blackman_harris_window);
		case e_blackman_nuttall_window:
			return shared_ptr<window_function>(new blackman_nuttall_window);
		case e_flat_top_window:
			return shared_ptr<window_function>(new flat_top_window);
		default:
			throw invalid_argument("GetWindowFunctionByEnum, unknown argument");
	};
}

XRAD_END
