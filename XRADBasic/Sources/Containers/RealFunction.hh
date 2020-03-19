// file RealFunction.hh
//--------------------------------------------------------------
#ifndef __RealFunction_cc
#define __RealFunction_cc
//--------------------------------------------------------------

#include "UniversalInterpolation.h"

XRAD_BEGIN

//--------------------------------------------------------------
//
//	Интерполяция
//
//--------------------------------------------------------------

template<XRAD__RealFunction_template>
auto RealFunction<XRAD__RealFunction_template_args>::in(double x) const -> floating64_type<value_type>
{
	interpolators::icubic.ApplyOffsetCorrection(x);
	const	FilterKernelReal	*filter = interpolators::icubic.GetNeededFilter(x);
	return filter->Apply((*this), integral_part(x));
}

template<XRAD__RealFunction_template>
auto RealFunction<XRAD__RealFunction_template_args>::d_dx(double x) const -> floating64_type<value_type>
{
	interpolators::sinc_derivative.ApplyOffsetCorrection(x);
	const	FilterKernelReal	*filter = interpolators::sinc_derivative.GetNeededFilter(x);
	return filter->Apply((*this), integral_part(x));
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // __RealFunction_cc
