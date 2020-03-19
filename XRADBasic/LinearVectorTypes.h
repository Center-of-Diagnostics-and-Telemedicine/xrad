#ifndef LinearVectorTypes_h__
#define LinearVectorTypes_h__
/*!
	\file
	\date 2016-04-06 13:41
	\author kns
*/
//--------------------------------------------------------------

#include "Sources/Containers/LinearVector.h"
#include "Sources/SampleTypes/ComplexSample.h"
#include "Sources/Algebra/ComplexFieldElement.h"

XRAD_BEGIN

//--------------------------------------------------------------

typedef	LinearVector<double, double, AlgebraicStructures::FieldTagScalar> RealVectorF64;
typedef	LinearVector<float, double, AlgebraicStructures::FieldTagScalar> RealVectorF32;
typedef LinearVector<int32_t, int32_t, AlgebraicStructures::FieldTagScalar> RealVectorI32;

typedef	LinearVector<complexF64, double, AlgebraicStructures::FieldTagComplex> ComplexVectorF64;
typedef	LinearVector<complexF32, double, AlgebraicStructures::FieldTagComplex> ComplexVectorF32;

//--------------------------------------------------------------

XRAD_END

#endif // LinearVectorTypes_h__
