#ifndef __MathFunction2DTypes_h
#define __MathFunction2DTypes_h
/*!
	\file
	\date 2014-04-15 16:58
	\author kns

	\brief Заданы основные типы двумерных функций, которыми следует
	пользоваться в большинстве случаев
*/
//--------------------------------------------------------------

#include "Sources/Containers/MathFunction2D.h"
#include "Sources/Containers/ComplexFunction2D.h"
#include "MathFunctionTypes.h"
#include <XRADBasic/Sources/Containers/UniversalInterpolation2D.h>//определения интерполяторов должны становиться доступными сразу вместе с классами MathFunction2D

XRAD_BEGIN

//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с целым типом: f*x приводит к вычислению f[i]*int(x);
// работает быстрее, но возможна потеря точности.
// Пример: действие f *= 0.5 гарантированно обнулит функцию f.

typedef MathFunction2D<RealFunctionI8> RealFunction2D_I8;
typedef MathFunction2D<RealFunctionUI8> RealFunction2D_UI8;

typedef MathFunction2D<RealFunctionI16> RealFunction2D_I16;
typedef MathFunction2D<RealFunctionUI16> RealFunction2D_UI16;

typedef MathFunction2D<RealFunctionI32> RealFunction2D_I32;
typedef MathFunction2D<RealFunctionUI32> RealFunction2D_UI32;



//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с double: f*=x приводит к вычислению int(double(f[i])*double(x));
// медленнее, но выше точность.
// Пример: действие f *= 0.5 даст ненулевой результат для всех f[i]>=2.

typedef MathFunction2D<RealFunctionI8F> RealFunction2D_I8F;
typedef MathFunction2D<RealFunctionUI8F> RealFunction2D_UI8F;

typedef MathFunction2D<RealFunctionI16F> RealFunction2D_I16F;
typedef MathFunction2D<RealFunctionUI16F> RealFunction2D_UI16F;

typedef MathFunction2D<RealFunctionI32F> RealFunction2D_I32F;
typedef MathFunction2D<RealFunctionUI32F> RealFunction2D_UI32F;



//--------------------------------------------------------------
// Массивы действительных чисел с плавающей запятой

typedef MathFunction2D<RealFunctionF32> RealFunction2D_F32;
typedef MathFunction2D<RealFunctionF64> RealFunction2D_F64;



//--------------------------------------------------------------
// Массивы комплексных чисел с целочисленными компонентами

typedef ComplexFunction2D<complexI8, int> ComplexFunction2D_I8;
typedef ComplexFunction2D<complexI8F, double> ComplexFunction2D_I8F;
typedef ComplexFunction2D<complexI16, int> ComplexFunction2D_I16;
typedef ComplexFunction2D<complexI16F, double> ComplexFunction2D_I16F;
typedef ComplexFunction2D<complexI32, int> ComplexFunction2D_I32;
typedef ComplexFunction2D<complexI32F, double> ComplexFunction2D_I32F;



//--------------------------------------------------------------
// Массивы комплексных чисел с плавающей запятой

typedef ComplexFunction2D<complexF32, double> ComplexFunction2D_F32;
typedef ComplexFunction2D<complexF64, double>  ComplexFunction2D_F64;



//--------------------------------------------------------------
// Массивы цветных отсчетов, именуются не "Function2D", а "Image"

template <class RGB, class ST>
struct ColorImageHelper
{
	// Предполагается, что компоненты ColorFunction — скаляры. Проверка:
	static_assert(std::is_same<typename RGB::component_field_tag,
			AlgebraicStructures::FieldTagScalar>::value, "Invalid ColorFunction component type");

	template <class T, class Sample>
	using add_const_from_sample_t = std::conditional_t<std::is_const_v<Sample>,
			std::add_const_t<T>, T>;

	template <class T, class Sample>
	using add_volatile_from_sample_t = std::conditional_t<std::is_volatile_v<Sample>,
			std::add_volatile_t<T>, T>;

	template <class T, class Sample>
	using add_cv_from_sample_t = add_const_from_sample_t<
			add_volatile_from_sample_t<T, Sample>, Sample>;

	using type = ColorContainer<
			MathFunction2D<ColorFunction<RGB, ST>>,
			MathFunction2D<RealFunction<add_cv_from_sample_t<typename RGB::component_type, RGB>, ST>>>;
};

//! \brief Шаблон для упрощения задания типов двумерных цветных контейнеров (см. примечания)
//!
//! Внимание! В качестве параметров шаблонных функций следует по возможности использовать шаблон
//! ColorContainer.
template <class RGB, class ST>
using ColorImage = typename ColorImageHelper<RGB, ST>::type;



using ColorImageUI8 = ColorImage<ColorSampleUI8, double>;

using ColorImageI16 = ColorImage<ColorSampleI16, double>;
using ColorImageUI16 = ColorImage<ColorSampleUI16, double>;

using ColorImageI32 = ColorImage<ColorSampleI32, double>;
using ColorImageUI32 = ColorImage<ColorSampleUI32, double>;

using ColorImageF32 = ColorImage<ColorSampleF32, double>;
using ColorImageF64 = ColorImage<ColorSampleF64, double>;

//--------------------------------------------------------------

XRAD_END

#endif //__MathFunction2DTypes_h
