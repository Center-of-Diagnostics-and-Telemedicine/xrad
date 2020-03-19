#ifndef __MathFunctionTypesMD_h
#define __MathFunctionTypesMD_h
/*!
	\file
	\date 2014-09-29 10:17
	\author KNS
*/
//--------------------------------------------------------------

#include "Sources/Containers/MathFunctionMD.h"
#include "Sources/Containers/ComplexFunctionMD.h"
#include "MathFunctionTypes2D.h"

XRAD_BEGIN

//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с целым типом: f*x приводит к вычислению f[i]*int(x);
// работает быстрее, но возможна потеря точности.
// Пример: действие f *= 0.5 гарантированно обнулит функцию f.

typedef MathFunctionMD<RealFunction2D_I8> RealFunctionMD_I8;
typedef MathFunctionMD<RealFunction2D_UI8> RealFunctionMD_UI8;

typedef MathFunctionMD<RealFunction2D_I16> RealFunctionMD_I16;
typedef MathFunctionMD<RealFunction2D_UI16> RealFunctionMD_UI16;

typedef MathFunctionMD<RealFunction2D_I32> RealFunctionMD_I32;
typedef MathFunctionMD<RealFunction2D_UI32> RealFunctionMD_UI32;



//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с double: f*=x приводит к вычислению int(double(f[i])*double(x));
// медленнее, но выше точность.
// Пример: действие f *= 0.5 даст ненулевой результат для всех f[i]>=2.

typedef MathFunctionMD<RealFunction2D_I8F> RealFunctionMD_I8F;
typedef MathFunctionMD<RealFunction2D_UI8F> RealFunctionMD_UI8F;

typedef MathFunctionMD<RealFunction2D_I16F> RealFunctionMD_I16F;
typedef MathFunctionMD<RealFunction2D_UI16F> RealFunctionMD_UI16F;

typedef MathFunctionMD<RealFunction2D_I32F> RealFunctionMD_I32F;
typedef MathFunctionMD<RealFunction2D_UI32F> RealFunctionMD_UI32F;



//--------------------------------------------------------------
// Массивы действительных чисел с плавающей запятой

typedef MathFunctionMD<RealFunction2D_F32> RealFunctionMD_F32;
typedef MathFunctionMD<RealFunction2D_F64> RealFunctionMD_F64;



//--------------------------------------------------------------
// Массивы цветных отсчетов

using ColorImageMD_UI8 = ColorContainer<MathFunctionMD<ColorImageUI8>, RealFunctionMD_UI8F>;

using ColorImageMD_I16 = ColorContainer<MathFunctionMD<ColorImageI16>, RealFunctionMD_I16F>;
using ColorImageMD_UI16 = ColorContainer<MathFunctionMD<ColorImageUI16>, RealFunctionMD_UI16F>;

using ColorImageMD_I32 = ColorContainer<MathFunctionMD<ColorImageI32>, RealFunctionMD_I32F>;
using ColorImageMD_UI32 = ColorContainer<MathFunctionMD<ColorImageUI32>, RealFunctionMD_UI32F>;

using ColorImageMD_F32 = ColorContainer<MathFunctionMD<ColorImageF32>, RealFunctionMD_F32>;
using ColorImageMD_F64 = ColorContainer<MathFunctionMD<ColorImageF64>, RealFunctionMD_F64>;



//--------------------------------------------------------------
// Массивы комплексных чисел с целочисленными компонентами

//typedef ComplexFunctionMD<complexI8, int> ComplexFunctionMD_I8;
	// Типы, такие как ComplexFunctionMD<complexI8, int>, требуют, вероятно,
	// использования ComplexSample<int8_t, int>. Иначе не компилируется шаблон GenericAlgebraElement:
	// FieldElement требует соответствия scalar type у ComplexSamle и у самого FieldElement.
typedef ComplexFunctionMD<complexI8F, double> ComplexFunctionMD_I8F;
typedef ComplexFunctionMD<complexI16, int> ComplexFunctionMD_I16;
typedef ComplexFunctionMD<complexI16F, double> ComplexFunctionMD_I16F;
typedef ComplexFunctionMD<complexI32, int> ComplexFunctionMD_I32;
typedef ComplexFunctionMD<complexI32F, double> ComplexFunctionMD_I32F;



//--------------------------------------------------------------
// Массивы комплексных чисел с плавающей запятой

typedef ComplexFunctionMD<complexF32, double> ComplexFunctionMD_F32;
typedef ComplexFunctionMD<complexF64, double>  ComplexFunctionMD_F64;

//--------------------------------------------------------------

XRAD_END

#endif //__MathFunctionTypesMD_h
