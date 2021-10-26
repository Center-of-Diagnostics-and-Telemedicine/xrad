/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file DataIOEnum.h
//	Created by ACS on 29.04.03
//	рекомендуется включать посредством <DataIO.h>
//--------------------------------------------------------------
#ifndef XRAD__File_DataIOEnum_h
#define XRAD__File_DataIOEnum_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

namespace DataArrayIOAuxiliaries
{

//--------------------------------------------------------------
//
//	различные вспомогательные классы для определения типов хранения данных
//
//	не следует использовать эти классы в программах напрямую
//
//--------------------------------------------------------------



//--------------------------------------------------------------
//
// вспомогательные объявления
// эти классы (ioXxx) вообще не должны использоваться вовне
//
//--------------------------------------------------------------


// 2016.12.13 отсылки к платформе "mac, PC" заменяются на информацию о порядке байт: "BE, LE"
struct ioEndian
{
	enum{
		mask = 0xf000,
		big = 0x1000,
		little = 0x2000,
		text = 0x3000,
		pdp = 0x4000//TODO PDP11 (middle)endian: стоит предусмотреть, такие наборы данных могут встретиться
	};
};

//--------------------------------------------------------------

struct ioNumberFormat
{
	enum {
		mask = 0x0f00,
		invalid = 0,
		I8 = 0x0100,
		UI8 = 0x0200,
		I16 = 0x0300,
		UI16 = 0x0400,
		I32 = 0x0500,
		UI32 = 0x0600,
		F32 = 0x0700,
		F64 = 0x0800,

		I64 = 0x0900,
		UI64 = 0x0a00
	};
};

//--------------------------------------------------------------

struct ioNumberType
{
	enum
	{
		mask = 0x00f0,
		scalar = 0x0000, // обязательно 0, можно опускать
		grayscale = scalar,
		complex = 0x0010,
		rgb_color = 0x0030,
		rgba_color = 0x0040,
		lab_color = 0x0050,
		hls_color = 0x0060,
		cmyk_color = 0x0070,
		vector = 0x0080
	};
};

//--------------------------------------------------------------

struct ioOptionsReserved
{
	enum
	{
		mask = 0xfff0
	};
};

}//namespace DataArrayIOAuxiliaries



//--------------------------------------------------------------
//
//	ioNumberOptions -- типы данных для ввода/вывода чисел
//	значение ioInvalid зарезервировано для обозначения недопустимого формата
//
//	Внимание! Ни в коем случае не сохраняйте данные значения в файлах,
//	т.к. числовое значение констант может в будущем меняться. Используйте
//	текстовое представление с помощью функций io_to_str/str_to_io.
//
//--------------------------------------------------------------

enum ioNumberOptions
{
// special
	ioInvalid = -1,
// scalar types
	ioI8 = DataArrayIOAuxiliaries::ioNumberFormat::I8,
	ioUI8 = DataArrayIOAuxiliaries::ioNumberFormat::UI8,
	//
	ioI16_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::I16,
	ioI16_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::I16,
	ioUI16_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	ioUI16_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	//
	ioI32_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::I32,
	ioI32_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::I32,
	ioUI32_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI32,
	ioUI32_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI32,
	//
	ioI64_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::I64,
	ioI64_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::I64,
	ioUI64_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI64,
	ioUI64_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI64,
	//
	ioF32_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioF32_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioF64_LE = DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F64,
	ioF64_BE = DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F64,
	//
	ioScalarText = DataArrayIOAuxiliaries::ioEndian::text | DataArrayIOAuxiliaries::ioNumberFormat::F64,
	// точность максимальная

// complex types
	ioComplexF32_LE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioComplexF32_BE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F32,

	ioComplexI8 = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioNumberFormat::I8,

	ioComplexI16_LE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::I16,
	ioComplexI16_BE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::I16,

	ioComplexI32_LE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::I32,
	ioComplexI32_BE = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::I32,

	ioComplexText = DataArrayIOAuxiliaries::ioNumberType::complex | DataArrayIOAuxiliaries::ioEndian::text | DataArrayIOAuxiliaries::ioNumberFormat::F64,

// pixel types
// порядок байтов во всех случаях должен соответствовать
// стандарту Photoshop RAW (от платформы не зависит)

	ioGrayscale8 = DataArrayIOAuxiliaries::ioNumberType::grayscale | DataArrayIOAuxiliaries::ioNumberFormat::UI8,
	ioGrayscale16_LE = DataArrayIOAuxiliaries::ioNumberType::grayscale | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	ioGrayscale16_BE = DataArrayIOAuxiliaries::ioNumberType::grayscale | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI16,

	ioRGB_UI8 = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI8,
	ioRGB_UI16_LE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	ioRGB_UI16_BE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	ioRGB_UI32_LE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI32,
	ioRGB_UI32_BE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI32,
	ioRGB_F32_LE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioRGB_F32_BE = DataArrayIOAuxiliaries::ioNumberType::rgb_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F32,

	//порядок байт: BE=RGBA, LE = ABGR
	ioRGBA_UI8_LE = DataArrayIOAuxiliaries::ioNumberType::rgba_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI8,
	ioRGBA_UI8_BE = DataArrayIOAuxiliaries::ioNumberType::rgba_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI8,

	//
	ioCMYK_UI8 = DataArrayIOAuxiliaries::ioNumberType::cmyk_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI8,
	ioCMYK_UI16_LE = DataArrayIOAuxiliaries::ioNumberType::cmyk_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	ioCMYK_UI16_BE = DataArrayIOAuxiliaries::ioNumberType::cmyk_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::UI16,
	//
	ioLAB_F32_LE = DataArrayIOAuxiliaries::ioNumberType::lab_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioLAB_F32_BE = DataArrayIOAuxiliaries::ioNumberType::lab_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	//
	ioHLS_F32_LE = DataArrayIOAuxiliaries::ioNumberType::hls_color | DataArrayIOAuxiliaries::ioEndian::little | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	ioHLS_F32_BE = DataArrayIOAuxiliaries::ioNumberType::hls_color | DataArrayIOAuxiliaries::ioEndian::big | DataArrayIOAuxiliaries::ioNumberFormat::F32,
	// dummy
	ioDummyTypeEnumEnd = 0
	// можно реализовать чтение/запись
	// типа "ioDummy=~0", имеет длину 0, в него можно писать всё, что угодно, и
	// читать из него всё, что угодно (скаляры и комплексные)
};

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_DataIOEnum_h
