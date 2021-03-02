/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file RealFunction.hh
//--------------------------------------------------------------
#ifndef XRAD__File_RealFunction_cc
#define XRAD__File_RealFunction_cc
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
#endif // XRAD__File_RealFunction_cc
