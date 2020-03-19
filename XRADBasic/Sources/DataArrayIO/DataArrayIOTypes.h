//	file DataIOTypes.h
//	Created by ACS on 29.04.03
//	рекомендуется включать посредством <DataIO.h>
//--------------------------------------------------------------
#ifndef __DataIOTypes_h
#define __DataIOTypes_h
//--------------------------------------------------------------

#include "DataArrayIOTypesHelpers.h"
#include <XRADBasic/Core.h>

XRAD_BEGIN

namespace DataArrayIOAuxiliaries
{


//--------------------------------------------------------------
//
//	скалярные типы
//
//--------------------------------------------------------------

typedef io_type_le<int8_t> int8_iotype;
typedef io_type_le<uint8_t> uint8_iotype;


#if XRAD_ENDIAN==XRAD_BIG_ENDIAN

typedef io_type_le<int16_t> int16_be_iotype;
typedef io_type_le<uint16_t> uint16_be_iotype;
typedef io_type_le<int32_t> int32_be_iotype;
typedef io_type_le<uint32_t> uint32_be_iotype;
typedef io_type_le<float> float32_be_iotype;
typedef io_type_le<double> float64_be_iotype;

typedef io_type_be<int16_t> int16_le_iotype;
typedef io_type_be<uint16_t> uint16_le_iotype;
typedef io_type_be<int32_t> int32_le_iotype;
typedef io_type_be<uint32_t> uint32_le_iotype;
typedef io_type_be<float> float32_le_iotype;
typedef io_type_be<double> float64_le_iotype;

// typedef io_type_le<complexF32> complexF32_be_iotype;
// typedef io_type_le<complexI32> complexI32_be_iotype;
// typedef io_type_le<complexI16> complexI16_be_iotype;


#elif XRAD_ENDIAN==XRAD_LITTLE_ENDIAN

typedef io_type_le<int16_t> int16_le_iotype;
typedef io_type_le<uint16_t> uint16_le_iotype;
typedef io_type_le<int32_t> int32_le_iotype;
typedef io_type_le<uint32_t> uint32_le_iotype;
typedef io_type_le<float> float32_le_iotype;
typedef io_type_le<double> float64_le_iotype;

typedef io_type_be<int16_t> int16_be_iotype;
typedef io_type_be<uint16_t> uint16_be_iotype;
typedef io_type_be<int32_t> int32_be_iotype;
typedef io_type_be<uint32_t> uint32_be_iotype;
typedef io_type_be<float> float32_be_iotype;
typedef io_type_be<double> float64_be_iotype;

// 	typedef io_type_le<complexF32> complexF32_le_iotype;
// 	typedef io_type_le<complexI32> complexI32_le_iotype;
// 	typedef io_type_le<complexI16> complexI16_le_iotype;
//
// 	typedef io_type_complex<float32_be_iotype> complexF32_be_iotype;
// 	typedef io_type_complex<int32_be_iotype> complexI32_be_iotype;
// 	typedef io_type_complex<int16_be_iotype> complexI16_be_iotype;

#else
#error ("Unknown numerals endiannes")
#endif


typedef io_type_complex<float64_le_iotype> complexF64_le_iotype;
typedef io_type_complex<float32_le_iotype> complexF32_le_iotype;
typedef io_type_complex<int32_le_iotype> complexI32_le_iotype;
typedef io_type_complex<int16_le_iotype> complexI16_le_iotype;

typedef io_type_complex<float64_be_iotype> complexF64_be_iotype;
typedef io_type_complex<float32_be_iotype> complexF32_be_iotype;
typedef io_type_complex<int32_be_iotype> complexI32_be_iotype;
typedef io_type_complex<int16_be_iotype> complexI16_be_iotype;

typedef io_type_le<complexI8F> complexI8_iotype;


// преобразователи текстовых данных (от платформы не зависят, число знаков произвольное)
// разрядности для текстового входного int не определены.

// typedef	io_type_text_scalar floating_text_iotype
// typedef	io_type_text_scalar integral_text_iotype;

//TODO ниже, кажется, нарушено единообразие. нужен шаблон?
//typedef class io_type_text_complex complex_text_iotype;

//--------------------------------------------------------------



//--------------------------------------------------------------
//
//	ввод/вывод пикселов
//
//--------------------------------------------------------------

typedef io_type_rgb<uint8_iotype> rgbUI8_iotype;

typedef io_type_rgb<uint16_le_iotype> rgbUI16_le_iotype;
typedef io_type_rgb<uint16_be_iotype> rgbUI16_be_iotype;

typedef io_type_rgb<uint32_le_iotype> rgbUI32_le_iotype;
typedef io_type_rgb<uint32_be_iotype> rgbUI32_be_iotype;


typedef io_type_rgb<float32_be_iotype> rgbF32_be_iotype;
typedef io_type_rgb<float32_le_iotype> rgbF32_le_iotype;

typedef uint8_iotype gray8_iotype;
typedef uint16_le_iotype gray16_iotype;

//typedef io_type_rgba<uint8_iotype> rgbaUI8_iotype;

//inline pcFloat *StdConversion( const ColorSample *) { return 0;}


//--------------------------------------------------------------
//
//  фиктивные функции, которые подставляются в шаблон.
//  определяют некий формат по умолчанию для сохранения данных.
//  для функций без параметров, когда надо просто сохранить данные
//  и затем их прочитать, возможно, на другой машине
//
//--------------------------------------------------------------



//inline Uint8 *StdConversion( const uint8_t *) { return 0;}
//inline int32_be_iotype *StdConversion( const int *) { return 0;}
//inline macFloat *StdConversion( const float *) { return 0;}
//inline macComplexF *StdConversion( const ComplexF32 *) { return 0;}

/*
inline pcComplex *pcConversion(const ComplexF32 *value){return NULL;}
inline macComplex *macConversion(const ComplexF32 *value){return NULL;}
inline  complex_text_iotype *textConversion(const ComplexF32 *value){return NULL;}

inline  pcFloat *pcConversion(const float *value){return NULL;}
inline  macFloat *macConversion(const float *value){return NULL;}
inline  float32_text_iotype *textConversion(const float *value){return NULL;}
*/

//--------------------------------------------------------------

}//namespace DataArrayIOAuxiliaries

XRAD_END

#endif // __DataIOTypes_h
