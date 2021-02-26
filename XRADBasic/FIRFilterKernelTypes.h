/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FIRFilterKernelTypes.h
//--------------------------------------------------------------
#ifndef XRAD__File_FIRFilterKernelTypes_h
#define XRAD__File_FIRFilterKernelTypes_h
//--------------------------------------------------------------

#include "Sources/Containers/FIRFilterKernel.h"
#include "Sources/Containers/FIRFilterKernelFunctions.h"
#include "Sources/Algebra/ComplexFieldElement.h"

XRAD_BEGIN

//--------------------------------------------------------------

typedef FIRFilterKernel<double, double, AlgebraicStructures::FieldTagScalar> FilterKernelReal;
typedef FIRFilterKernel<complexF64, double, AlgebraicStructures::FieldTagComplex> FilterKernelComplex;

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FIRFilterKernelTypes_h
