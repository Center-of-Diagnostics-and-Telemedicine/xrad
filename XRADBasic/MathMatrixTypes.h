/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef MathMatrixTypes_h__
#define MathMatrixTypes_h__

/********************************************************************
	created:	2016/04/06
	created:	6:4:2016   13:41
	author:		kns
*********************************************************************/

#include "Sources/Containers/MathMatrix.h"
#include "Sources/SampleTypes/ComplexSample.h"
#include "LinearVectorTypes.h"

XRAD_BEGIN

typedef MathMatrix<double, double, AlgebraicStructures::FieldTagScalar> RealMatrixF64;
typedef MathMatrix<float, double, AlgebraicStructures::FieldTagScalar> RealMatrixF32;
typedef MathMatrix<complexF64, double, AlgebraicStructures::FieldTagComplex> ComplexMatrixF64;
typedef MathMatrix<complexF32, double, AlgebraicStructures::FieldTagComplex> ComplexMatrixF32;

typedef MathMatrix<int32_t, int, AlgebraicStructures::FieldTagScalar> RealMatrixI32;

XRAD_END

#endif // MathMatrixTypes_h__