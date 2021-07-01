/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_text_encoding_h
#define XRAD__File_text_encoding_h
/*!
	\file
	\date 2018/03/19 15:44
	\author kulberg
*/

#include <XRADBasic/Sources/Containers/DataArray.h>

XRAD_BEGIN

//! Обозначение кодировок, которые могут быть в текстовом файле. Различия 8-битных кодировок не учитываются
namespace text_encoding
{

enum file_type
{
	unknown,
	char_8bit,
	utf8,
	utf16_le,
	utf16_be,

#if XRAD_ENDIAN==XRAD_BIG_ENDIAN
	utf16_native = utf16_be,
	utf16_reverse = utf16_le
#elif XRAD_ENDIAN==XRAD_LITTLE_ENDIAN
	utf16_native = utf16_le,
	utf16_reverse = utf16_be
#else
#error	unknown endianness
#endif
};


//! Обозначение кодовых наборов Unicode
enum codepage_type
{
	codepage_universal,// Цифры, пунктуация, пробелы и т.п. Формально подмножество latin, но используется совместно с символами всех других кодовых страниц
	codepage_latin,
	codepage_latin_ext,
	codepage_cyrillic,
	codepage_cyrillic_ext,
	codepage_other
};

file_type	recognize_by_bom(const DataArray<char> &buffer);
file_type	recognize_by_content(const DataArray<char> &buffer);


enum recognize_method
{
	recognize_encoding_none, recognize_encoding_bom, recognize_encoding_content//TODO плохо продуманы имена объектов
};

const string	&standard_bom(text_encoding::file_type enc);
inline size_t	standard_bom_size(text_encoding::file_type enc) { return standard_bom(enc).size(); }


}//namespace text_encoding



XRAD_END

#endif // XRAD__File_text_encoding_h
