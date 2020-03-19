#ifndef __BSplines_h
#define __BSplines_h

//	created:	2014/11/18
//	created:	18.11.2014   13:08
//	author:		kns

#include <XRADBasic/Core.h>

XRAD_BEGIN

double	b_spline(int n, double x);
double	b_spline_integral(int n, double x);

XRAD_END


#endif //__BSplines_h