#include "pre.h"
#include "BSplines.h"
#include <XRADBasic/Sources/Math/SpecialFunctions.h>

XRAD_BEGIN

int	spline_calc_treshold = 21;

// функция неважного качества: работает примерно до n=25,
// дальше ряд расходится из-за потерь точности, поэтому
// приходится использовать аппроксимацию. макс. погрешность ~1e-3
// следует в более спокойной обстановке это переписать как следует

double	b_spline(int n, double x)
	{
	if(!in_range(x,0,n)) return 0;

	if(n>spline_calc_treshold)
		{
		double	sigma = sqrt(double(n)/12);
		return gauss(x-double(n)/2, sigma)/(sigma*sqrt(2.*pi()));
		}

	switch(n)
		{
		case 1: return 1;
			break;

		case 2:
			return x<1 ? x : 2.-x;
			break;

		case 3:
			{
			if(x<1) return square(x)/2;
			else if(x<2) return 0.75 - square(x-1.5);
			else return square(x-3)/2;
			}
			break;

		case 4:
			{
			if(x<1) return cube(x)/6;
			else if(x<2) return (4 + 3*cube(2-x) - 6*square(2-x))/6;
			else if(x<3) return (4 + 3*cube(x-2) - 6*square(x-2))/6;
			else return cube(4-x)/6;
			}
			break;

		default:
			{
			double result = 0;
			double	factor = 1;
			for(int k = 0; k <= n; ++k)
				{
				result += factor*C_nk(n,k)*pow(x-k, n-1)*sign(x-k);
				factor *= -1;
				}
			return result/(2.*factorial<double>(n-1));
			}
		};
	}
// функция распределения ирвина-холла (Irwin-Hall distribution)
double	b_spline_integral(int n, double x)
	{
	if(x<=0) return 0;
	if(x >= n) return 1;

	if(n>spline_calc_treshold)
		{
// 		gaussian_cdf
		double	sigma = sqrt(double(n)/12);
		double a = 1./(sqrt(2.*square(sigma)));
		return 0.5*(1.+SpecialFunctions::erf((x-double(n)/2)*a));
		}


	double result = 0;
	double	factor = 1;
	for(int k = 0; k <= n; ++k)
		{
		result += factor*C_nk(n,k)*pow(x-k, n)*sign(x-k);
		factor *= -1;
		}
	return 0.5 + result/(2.*factorial<double>(n));
	}

XRAD_END
