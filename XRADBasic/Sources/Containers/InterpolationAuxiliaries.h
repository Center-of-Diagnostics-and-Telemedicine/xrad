#ifndef __interpolation_auxiliaries_h
#define __interpolation_auxiliaries_h

#include <XRADBasic/MathFunctionTypes.h>

// сюда перенести все вспомогательные вычисления по интерполяции: стробы, сплайны и т.п.

XRAD_BEGIN



// формулы стробирования
double	SincInterpolatorStrobeRadius(double filter_order);
double	BesselInterpolatorStrobeRadius(size_t filter_order, double bessel_radius);
double	BesselDerivativeInterpolatorStrobeRadius(size_t filter_order);
double	BesselDerivativeInterpolatorNormalizer(size_t filter_order);

// формулы фильтров

typedef RealFunctionF64 real_interpolator_container;
typedef ComplexFunctionF64 complex_interpolator_container;

real_interpolator_container CalculateBSpline(size_t spline_order, double x);
real_interpolator_container CalculateISpline(size_t spline_order, double x);
real_interpolator_container CalculateSincInterpolator(size_t filter_order, double x);
real_interpolator_container CalculateSincDerivativeInterpolator(size_t filter_order, double x);
real_interpolator_container CalculateSincHilbertInterpolator(size_t filter_order, double x, ComplexNumberPart complex_number_part);
complex_interpolator_container CalculateSincDerivativeCarrierInterpolator(size_t filter_order, double x, double carrier);



double	CalculateQuasiSpline(double x, double nu);



XRAD_END

#endif  //__interpolation_auxiliaries_h
