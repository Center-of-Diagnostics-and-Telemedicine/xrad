/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file DataIOTypesHelpers.h
//	Created by ACS on 29.04.03
//	!!! не включать непосредственно!!!
//--------------------------------------------------------------
#ifndef XRAD__File_DataIOTypesHelpers_h
#define XRAD__File_DataIOTypesHelpers_h

#include <XRADBasic/Sources/SampleTypes/ComplexSample.h>
#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------
//
//	различные вспомогательные классы для преобразования формата хранения данных
//	не предназначены для использования в пользовательском коде
//
//--------------------------------------------------------------

namespace DataArrayIOAuxiliaries
{

//--------------------------------------------------------------
//
//  преобразование порядка байт
//
//--------------------------------------------------------------


template<class T, class element = uint8_t>
void reverse_elements_back(element *result, const T& original)
{
	enum{ n_elements = sizeof(T)/sizeof(element) };
	const element* in = reinterpret_cast<const element*>(&original) + sizeof(T)/sizeof(element)-1;
	element* out = result;
	element* out_end = out + n_elements;

	for(;out<out_end;++out, --in) *out=*in;
}

template<class T, class element = uint8_t>
void reverse_elements_direct(T &result, const element *original)
{
	enum{ n_elements = sizeof(T)/sizeof(element) };
	const element* in = original + n_elements-1;
	element* out = reinterpret_cast<element*>(&result);
	element* out_end = out + n_elements;

	for(;out<out_end;++out, --in) *out=*in;
}

//--------------------------------------------------------------

template< class T>
class io_type_builtin
{
public:
	static size_t fsize() { return sizeof(T); };
	typedef T value_type;

	static T get(const uint8_t *data) { return *reinterpret_cast<const T*>(data); }
	static void put(uint8_t *data, const T &x) { *reinterpret_cast<T*>(data) = x; }
};


template< class T>
class io_type_reverse
{
public:
	static size_t fsize(){ return sizeof(T); };
	typedef T value_type;

	static T get(const uint8_t *data)
	{
		T result;
		reverse_elements_direct(result, data);
		return result;
	}
	static void put(uint8_t *data, const T &x)
	{
		reverse_elements_back(data, x);
	}
};

#if XRAD_ENDIAN==XRAD_BIG_ENDIAN
	#define io_type_le io_type_reverse
	#define io_type_be io_type_builtin
#elif  XRAD_ENDIAN==XRAD_LITTLE_ENDIAN
	#define io_type_le io_type_builtin
	#define io_type_be io_type_reverse
#else
	#error unknown endianness
#endif


//--------------------------------------------------------------
//
//  для бинарных комплексных чисел
//
//--------------------------------------------------------------


template <class CT>
class io_type_complex
{
public:
	typedef ComplexSample<typename CT::value_type, double> value_type;
	static size_t fsize(){ return sizeof(value_type); }

	static complexF64 get(const uint8_t *data)
	{
		float re = CT::get(data), im = CT::get(data + CT::fsize());
		return complexF64(re, im);
	}

	static void put(uint8_t *data, const complexF64& x)
	{
		CT::put(data, real(x));
		CT::put(data + CT::fsize(), imag(x));
	}
};

//--------------------------------------------------------------
//
//  обмен между текстовым файлом и объектом любого встроенного
//  скалярного типа <T> без буфера. длина чисел произвольная
//  count увеличивается на 1, если удалось прочесть число
//  функции get и put вызываются аналогично вышенаписанным,
//  только вместо указателя на буфер дается указатель FILE *
//  (исключается возможность поиска по файлу, но это в данном
//  случае неважно).
//--------------------------------------------------------------

class	scalar_text_iotype
{
public:
	typedef double value_type;

	static	double get(FILE* theFile, size_t &count)
	{
		double buffer;
		if(fscanf(theFile, "%lg", &buffer) > 0)
		{ // конец файла возвращает EOF = -1, 0 -- не прочиталось по каким-либо другим причинам
			count++;
			return buffer;
		}
		else return double(0);
	}
	static	bool put(FILE *theFile, const double &x)
	{
		double	buff = double(x);
		if(fprintf(theFile, "%lg, ", buff) > 0) return true;
		else return false;
	}
};

//--------------------------------------------------------------
//
//	обмен между встроенным complexF64 и текстовым файлом
//	читает все числа подряд, игнорируя разделители
//	выводит в стандартной записи (real, im) по одному числу в строку.
//	count увеличивается на 1, если удалось инициализировать
//	хотя бы одну часть комплексного числа
//
//--------------------------------------------------------------

class	complex_text_iotype
{
public:
	typedef complexF64 value_type;

	static	complexF64 get(FILE *theFile, size_t &count)
	{
		size_t	success(0);
		complexF64 result;
		real(result) = scalar_text_iotype::get(theFile, success);
		imag(result) = scalar_text_iotype::get(theFile, success);
		if(success) count++;
		return result;
	}

	static bool put(FILE *theFile, const complexF64 &x)
	{
		size_t	success(0);
		if(fprintf(theFile, "(%lg, ", real(x)) > 0) success++;
		if(fprintf(theFile, "%lg)\n", imag(x)) > 0) success++;

		if(success) return true;
		else return false;
	}
};

//--------------------------------------------------------------
//
//  ввод/вывод пикселов
//  порядок байтов определяется стандартом Photoshop RAW
//  пока не помню как там, буду находить опытным путем
//
//--------------------------------------------------------------


template <class storeType> // int16 или int8
class io_type_rgb
{
public:
	static size_t fsize(){ return 3*storeType::fsize(); }
	typedef ColorSampleF64 value_type;

	static ColorSampleF64 get(const uint8_t *data)
	{
		double	r = storeType::get(data);
		double	g = storeType::get(data + storeType::fsize());
		double	b = storeType::get(data + storeType::fsize()*2);

		return ColorSampleF64(r, g, b);
	}

	static void put(uint8_t *data, const ColorSampleF64 &x)
	{
		storeType::put(data, x.red());
		storeType::put(data + storeType::fsize(), x.green());
		storeType::put(data + storeType::fsize() * 2, x.blue());
	}
};

template <class storeType> // int16 или int8
class io_type_rgba : public io_type_rgb<storeType>
{
	PARENT(io_type_rgb);
public:
	static size_t fsize(){ return 4*storeType::fsize(); }
	typedef ColorPixel value_type;

	static ColorSampleF64 get(const uint8_t* data)
	{
		double	r = storeType::get(data);
		double	g = storeType::get(data + storeType::fsize());
		double	b = storeType::get(data + storeType::fsize()*2);
		double	a = store_type::get(data + store_type::fsize()*3);

		return ColorPixel(r, g, b, a);
	}

	static void put(uint8_t* data, const ColorSampleF64& x)
	{
		storeType::put(data, x.red());
		storeType::put(data + storeType::fsize(), x.green());
		storeType::put(data + storeType::fsize() * 2, x.blue());
		storeType::put(data + storeType::fsize() * 3, x.alpha());
	}
};


//
// template <class storeType> // int16 или int8
// class io_type_rgba
// {
// public:
// 	static size_t fsize(){ return 4*storeType::fsize(); }
// 	typedef ColorSampleF64 value_type;
//
// 	static ColorSampleF64 get(const uint8_t *data)
// 	{
// 		storeType::get(data);
// 		float r = storeType::get(data);
// 		float g = storeType::get(data+storeType::fsize());
// 		float b = storeType::get(data+storeType::fsize()*2);
// 		return ColorSampleF64(r, g, b);
// 	}
// 	static void put(uint8_t *data, const ColorSampleF64 &x)
// 	{
// 		storeType::put(data, 0);//dummy alpha-channel
// 		storeType::put(data, x.red());
// 		storeType::put(data+storeType::fsize(), x.green());
// 		storeType::put(data+storeType::fsize() * 2, x.blue());
// 	}
// };

//--------------------------------------------------------------

}//namespace DataArrayIOAuxiliaries

XRAD_END

#endif // XRAD__File_DataIOTypesHelpers_h
