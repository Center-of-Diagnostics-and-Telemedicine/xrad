#ifndef XRAD__File_MathFunctionTypes_h
#define XRAD__File_MathFunctionTypes_h
/*!
	\file
	\date 2014-04-15 16:53
	\author	kns

	\brief Заданы основные типы одномерных функций, которыми следует
	пользоваться в большинстве случаев
*/
//--------------------------------------------------------------

#include "Sources/Containers/RealFunction.h"
#include "Sources/Containers/ComplexFunction.h"
#include "Sources/SampleTypes/ColorSample.h"
#include "Sources/Containers/ColorContainer.h"

// TODO: Убрать эту зависимость?
#include "Sources/Containers/WindowFunction.h"
#include "Sources/Containers/UniversalInterpolation.h"

XRAD_BEGIN

//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с целым типом: f*x приводит к вычислению f[i]*int(x);
// работает быстрее, но возможна потеря точности.
// Пример: действие f *= 0.5 гарантированно обнулит функцию f.

typedef RealFunction<int8_t,int> RealFunctionI8;
typedef RealFunction<uint8_t,int> RealFunctionUI8;

typedef RealFunction<int16_t,int> RealFunctionI16;
typedef RealFunction<uint16_t,int> RealFunctionUI16;

typedef RealFunction<int32_t,int> RealFunctionI32;
typedef RealFunction<uint32_t,int> RealFunctionUI32;



//--------------------------------------------------------------
// Массивы целых чисел, для которых умножение на скаляр
// вычисляется с double: f*=x приводит к вычислению int(double(f[i])*double(x));
// медленнее, но выше точность.
// Пример: действие f *= 0.5 даст ненулевой результат для всех f[i]>=2.

typedef RealFunction<int8_t,double> RealFunctionI8F;
typedef RealFunction<uint8_t,double> RealFunctionUI8F;

typedef RealFunction<int16_t,double> RealFunctionI16F;
typedef RealFunction<uint16_t,double> RealFunctionUI16F;

typedef RealFunction<int32_t,double> RealFunctionI32F;
typedef RealFunction<uint32_t,double> RealFunctionUI32F;



//--------------------------------------------------------------
// Массивы действительных чисел с плавающей запятой

typedef RealFunction<float,double> RealFunctionF32;
typedef RealFunction<double,double> RealFunctionF64;



//--------------------------------------------------------------
// Массивы комплексных чисел с целочисленными компонентами

typedef ComplexFunction<complexI8,int> ComplexFunctionI8;
typedef ComplexFunction<complexI8F,double> ComplexFunctionI8F;
typedef ComplexFunction<complexI16,int> ComplexFunctionI16;
typedef ComplexFunction<complexI16F,double> ComplexFunctionI16F;
typedef ComplexFunction<complexI32,int> ComplexFunctionI32;
typedef ComplexFunction<complexI32F,double> ComplexFunctionI32F;



//--------------------------------------------------------------
// Массивы комплексных чисел с плавающей запятой

typedef ComplexFunction<complexF32,double> ComplexFunctionF32;
typedef ComplexFunction<complexF64,double> ComplexFunctionF64;



//--------------------------------------------------------------
// Шаблон для массивов цветных отсчетов

template <class RGB, class ST>
struct ColorFunctionHelper
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
 			MathFunction<RGB, ST, typename RGB::field_tag>,
//			LinearVector<RGB, ST, typename RGB::field_tag>,
			RealFunction<add_cv_from_sample_t<typename RGB::component_type, RGB>, ST>>;
};

//! \brief Шаблон для упрощения задания типов одномерных цветных контейнеров (см. примечания)
//!
//! Внимание! В качестве параметров шаблонных функций следует по возможности использовать шаблон
//! ColorContainer.
template <class RGB, class ST>
using ColorFunction = typename ColorFunctionHelper<RGB, ST>::type;



//--------------------------------------------------------------
// Массивы целочисленных цветных отсчетов

using ColorFunctionI32F = ColorFunction<ColorSampleI32, double>;
using ColorFunctionI16F = ColorFunction<ColorSampleI16, double>;
using ColorFunctionUI32F = ColorFunction<ColorSampleUI32, double>;
using ColorFunctionUI16F = ColorFunction<ColorSampleUI16, double>;
using ColorFunctionUI8F = ColorFunction<ColorSampleUI8, double>;

using ColorFunctionI32 = ColorFunction<ColorSampleI32, int>;
using ColorFunctionI16 = ColorFunction<ColorSampleI16, int>;
using ColorFunctionUI32 = ColorFunction<ColorSampleUI32, int>;
using ColorFunctionUI16 = ColorFunction<ColorSampleUI16, int>;
using ColorFunctionUI8 = ColorFunction<ColorSampleUI8, int>;



//--------------------------------------------------------------
// Массивы цветных отсчетов с плавающей запятой

using ColorFunctionF32 = ColorFunction<ColorSampleF32, double>;
using ColorFunctionF64 = ColorFunction<ColorSampleF64, double>;

//--------------------------------------------------------------

XRAD_END

#endif //XRAD__File_MathFunctionTypes_h
