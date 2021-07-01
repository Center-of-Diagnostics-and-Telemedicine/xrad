/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FIRFilterKernelTypes2D.h
//--------------------------------------------------------------
#ifndef XRAD__File_FIRFilterKernelTypes2D_h
#define XRAD__File_FIRFilterKernelTypes2D_h
//--------------------------------------------------------------

#include "Sources/Containers/FIRFilterKernel2D.h"
#include "MathFunctionTypes.h"

XRAD_BEGIN

//--------------------------------------------------------------

// typedef FIRFilter2DConvolve<FilterKernelReal> FIRFilter2DReal;
// typedef FIRFilter2DConvolve<FilterKernelComplex> FIRFilter2DComplex;
// зависимость от kernel была надуманной, лучше как ниже:

typedef FIRFilterKernel2DConvolve<RealFunctionF64> FIRFilter2DReal;
typedef FIRFilterKernel2DConvolve<ComplexFunctionF64> FIRFilter2DComplex;

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FIRFilterKernelTypes2D_h
