/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file DisplaySampleType.h
//--------------------------------------------------------------
#ifndef XRAD__File_DisplaySampleType_h
#define XRAD__File_DisplaySampleType_h
//--------------------------------------------------------------

#include <XRADBasic/SampleTypes.h>

XRAD_BEGIN

enum display_sample_type
	{
	gray_sample_ui8,
	indexed_color_sample_ui8,

	gray_sample_i16,
	gray_sample_ui16,
	gray_sample_i32,
	gray_sample_ui32,
	gray_sample_f32,
	gray_sample_f64,

	rgba_sample_ui8,
	rgb_sample_f32,

	complex_sample_f32,
	complex_sample_f64,
	complex_sample_i32,
	complex_sample_i16
	};

namespace internal
{

template<class T> display_sample_type DisplaySampleTypeUtil() = delete;

template<> inline display_sample_type DisplaySampleTypeUtil<uint8_t>() { return gray_sample_ui8; }
template<> inline display_sample_type DisplaySampleTypeUtil<int16_t>() { return gray_sample_i16; }
template<> inline display_sample_type DisplaySampleTypeUtil<uint16_t>() { return gray_sample_ui16; }

template<> inline display_sample_type DisplaySampleTypeUtil<int32_t>() { return gray_sample_i32; }
template<> inline display_sample_type DisplaySampleTypeUtil<uint32_t>() { return gray_sample_ui32; }

template<> inline display_sample_type DisplaySampleTypeUtil<float>() { return gray_sample_f32; }
template<> inline display_sample_type DisplaySampleTypeUtil<double>() { return gray_sample_f64; }

template<> inline display_sample_type DisplaySampleTypeUtil<ColorPixel>() { return rgba_sample_ui8; }
template<> inline display_sample_type DisplaySampleTypeUtil<ColorSampleF32>() { return rgb_sample_f32; }

template<> inline display_sample_type DisplaySampleTypeUtil<complexF32>() { return complex_sample_f32; }
template<> inline display_sample_type DisplaySampleTypeUtil<complexF64>() { return complex_sample_f32; }
template<> inline display_sample_type DisplaySampleTypeUtil<complexI32F>() { return complex_sample_i32; }
template<> inline display_sample_type DisplaySampleTypeUtil<complexI16F>() { return complex_sample_i16; }// здесь предполагается использование избыточного хранилища. чтобы не умножать строк без нужды. но возможно, это придется потом сделать
template<> inline display_sample_type DisplaySampleTypeUtil<complexI32>() { return complex_sample_i32; }
template<> inline display_sample_type DisplaySampleTypeUtil<complexI16>() { return complex_sample_i16; }// здесь предполагается использование избыточного хранилища. чтобы не умножать строк без нужды. но возможно, это придется потом сделать

} // namespace internal

template<class T> inline display_sample_type DisplaySampleType() { return internal::DisplaySampleTypeUtil<typename remove_cv<T>::type>(); }

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_DisplaySampleType_h
