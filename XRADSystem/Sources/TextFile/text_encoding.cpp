/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/03/19 15:44
	\author kulberg
*/
#include "pre.h"
#include "text_encoding.h"
#include <XRADBasic/Sources/Utils/ValuePredicates.h>
#include <map>

XRAD_BEGIN

using namespace std;

namespace text_encoding
{

//byte order mask
const string	s_utf8_bom("\xEF\xBB\xBF");
const string	s_utf16_le_bom("\xFF\xFE");
const string	s_utf16_be_bom("\xFE\xFF");
const string	s_empty_bom("");

const string &standard_bom(text_encoding::file_type enc)
{
	switch(enc)
	{
		case text_encoding::unknown:
		case text_encoding::char_8bit: return s_empty_bom;
		case text_encoding::utf8: return s_utf8_bom;
		case text_encoding::utf16_be: return s_utf16_be_bom;
		case text_encoding::utf16_le: return s_utf16_le_bom;

		default: throw invalid_argument("text_encoding::standard_bom, unknown text file format");
	}
}

//механизмы различения способов кодировки текста
auto	punctuation_1 = predicate::greater_or_equal(0x0000) & predicate::less_or_equal(0x0040);
auto	punctuation_2 = predicate::greater_or_equal(0x005B) & predicate::less_or_equal(0x0060);
auto	punctuation_3 = predicate::greater_or_equal(0x007B) & predicate::less_or_equal(0x007E);

auto	latin = predicate::greater_or_equal(0x0041) & predicate::less_or_equal(0x007A);
auto	latin_ext = predicate::greater_or_equal(0x0081) & predicate::less_or_equal(0x024F);

auto	cyrillic = predicate::greater_or_equal(0x0400) & predicate::less_or_equal(0x04FF);
auto	cyrillic_ext_a = predicate::greater_or_equal(0x2DE0) & predicate::less_or_equal(0x2DFF);
auto	cyrillic_ext_b = predicate::greater_or_equal(0xA640) & predicate::less_or_equal(0xA69F);
auto	cyrillic_ext_c = predicate::greater_or_equal(0x1C80) & predicate::less_or_equal(0x1C8F);

auto	punctuation = punctuation_1 | punctuation_2 | punctuation_3;
auto	cyrillic_ext = cyrillic_ext_a | cyrillic_ext_b | cyrillic_ext_c;


text_encoding::codepage_type	character_codepage(wchar_t c)
{
	if(c==default_ascii_character()) return codepage_other;

	if(punctuation(c)) return codepage_universal;
	if(latin(c)) return codepage_latin;
	if(latin_ext(c)) return codepage_latin_ext;
	if(cyrillic(c)) return codepage_cyrillic;
	if(cyrillic_ext(c)) return codepage_cyrillic_ext;

	return codepage_other;
}

//! Подсчет дефектов кодировки текста. Дефектом считается отклонение от преобладающей в тексте кодировки
size_t	encoding_defect(const wstring &s)
{
	map<text_encoding::codepage_type, size_t> encoding_map;
	for(auto c: s) encoding_map[character_codepage(c)]++;

	if(encoding_map.size()<=1) return 0;//наиболее однородный текст, одно слово без пробелов и пунктуации

	size_t	defect_value(s.size());	// считаем все символы "дефектными"
	defect_value -= encoding_map[codepage_universal];	// исключаем из счетчика дефектов пробелы, цифры и пунктуацию
	defect_value -= encoding_map[codepage_latin];		// исключаем стандартную латиницу, считая, что ее присутствие нормально в любом тексте: c'est comme il faut (Шишков, прости)

	if(encoding_map[codepage_cyrillic] > 0)
	{
		defect_value -= encoding_map[codepage_cyrillic];
		// если в тексте есть основная кириллица, не считаем дефектом и расширенную
		defect_value -= encoding_map[codepage_cyrillic_ext];
	}
	else if(encoding_map[codepage_latin_ext])
	{
		// если в тексте нет кириллицы, но есть обычная латынь, символы расширенной латыни не считаем дефектом
		defect_value -= encoding_map[codepage_latin_ext];
	}
	return defect_value;
}


text_encoding::file_type	recognize_by_content(const DataArray<char> &buffer)
{
	// могут встретиться как строки в 1251, так и utf-8 без BOM.
	// делается попытка интерпретировать текст в обеих кодировках, потом считается количество символов, выпадающих из основного тренда.
	//TODO добавить utf16
	wstring	char_8bit_s = convert_to_wstring(string((buffer.data())));
	wstring	utf8_s = convert_to_wstring(ustring(uchar_t::pointer_from_char(buffer.data())));

	return encoding_defect(char_8bit_s) < encoding_defect(utf8_s) ?
		char_8bit	:
		utf8;
}

file_type	recognize_by_bom(const DataArray<char> &buffer)
{
	auto	begins_with = [buffer](const string &bom)
	{
		if(!bom.size()) return false;
		if(buffer.size() < bom.size()) return false;
		for(size_t i = 0; i < bom.size(); ++i) if(buffer[i]!=bom[i]) return false;
		return true;
	};

	if(begins_with(standard_bom(text_encoding::utf8))) return text_encoding::utf8;
	if(begins_with(standard_bom(text_encoding::utf16_le))) return text_encoding::utf16_le;
	if(begins_with(standard_bom(text_encoding::utf16_be))) return text_encoding::utf16_be;

	return text_encoding::unknown;
}


}//namespace text_encoding




XRAD_END
