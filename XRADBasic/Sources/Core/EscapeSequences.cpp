/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "EscapeSequences.h"
#include "HexChar.h"

XRAD_BEGIN

namespace
{

// enum
// 	{
// 	EStr_EscMaskNone = ' ',
// 	EStr_EscMaskEsc = '\\',
// 	EStr_EscMaskSymbol = '@',
// 	EStr_EscMaskNum = '#'
// 	};

template<class T>
constexpr size_t	n_hex_digits_per_value()
{
	return sizeof(T) * 2;//две на байт
}

template<class T>
char	nth_hex_digit(T c, size_t n)
{
	constexpr size_t	n_hex_digits = n_hex_digits_per_value<T>();
	size_t	offset = 4*(n_hex_digits - (n + 1));
	return	int_to_hex_char((c>>offset)&0xF);
}



template<class STRING_T>
STRING_T encode_escape_hex(typename STRING_T::value_type c)
{
	constexpr size_t	n_hex_digits = n_hex_digits_per_value<typename STRING_T::value_type>();
	STRING_T result(n_hex_digits+2, 0);
	result[0] = '\\';
	result[1] = 'x';

	for(size_t i = 0; i < n_hex_digits; ++i)
	{
		result[2+i] = nth_hex_digit(c, i);
	}
	return result;
}


template<class STRING_T>
STRING_T encode_escape_sequence(typename STRING_T::value_type c, const STRING_T &aux_symbols)
{
	switch(c)
	{
		case '\a':
			return STRING_T(1, '\\') + STRING_T(1, 'a');
		case '\b':
			return STRING_T(1, '\\') + STRING_T(1, 'b');
		case '\f':
			return STRING_T(1, '\\') + STRING_T(1, 'f');
		case '\n':
			return STRING_T(1, '\\') + STRING_T(1, 'n');
		case '\r':
			return STRING_T(1, '\\') + STRING_T(1, 'r');
		case '\t':
			return STRING_T(1, '\\') + STRING_T(1, 't');
		case '\v':
			return STRING_T(1, '\\') + STRING_T(1, 'v');
		case '\\':
			return STRING_T(1, '\\') + STRING_T(1, '\\');
	}

	if(c >= 0 && c < 0x20)
	{
		return encode_escape_hex<STRING_T>(c);
	}
	else if(aux_symbols.find_first_of(c) != string::npos)
	{
		return STRING_T(1, '\\') + c;
	}
	else
	{
		return STRING_T(1, c);
	}
}



template<class ITERATOR_T>
uint32_t decode_escape_sequence(ITERATOR_T &it, ITERATOR_T &ie)
{
	// раскодирует esc-последовательности вида
	//	\n abfrtv (спецсимволы консольного ввода-вывода)
	//	\xXXXX (шестнадцатеричные числа)
	//	\NN (восьмеричные числа

	if(*it != '\\' || ie <= it)
	{
		// esc-последовательности нет, смещаем итератор и возвращаем первый символ
		return *(it++);
	}
	else
	{
		// пропускаем '\' и переходим к следующему символу, если он есть.
		// иначе возвращаем слэш как есть,
		if(++it==ie) return '\\';
	}

	if(*it=='x' || *it == 'X')
	{
		// разбор шестнадцатеричного числа
		// TODO: Работа с количеством цифр не соответствует спецификации C.
		uint32_t	accumulator = 0;
		ITERATOR_T seq = it+1;//пропуск 'x'
		size_t	count = 0;

		for(; seq<ie && is_hex_char(*seq); ++seq, ++count)
		{
			accumulator *= 0x10;
			accumulator += hex_char_to_int(*seq);
		}
		if(count && count <= max_hex_digits_for_unicode)
		{
			// найдено шестнадцатеричное число, ставим итератор после него
			// и возвращаем результат дешифровки
			it = seq;
			return accumulator;
		}
		else
		{
			// литерал с ошибкой, возвращаем символы как есть
			return *(it-1);
		}
	}

	if(is_oct_char(*it))
	{
		// разбор восьмеричного числа
		// TODO: Работа с количеством цифр не соответствует спецификации C.
		uint32_t	accumulator = 0;
		ITERATOR_T seq = it;
		size_t	count = 0;

		for(; seq<ie && is_oct_char(*seq); ++seq, ++count)
		{
			accumulator *= 010;
			accumulator += oct_char_to_int(*seq);
		}
		if(count && count <= max_octal_digits_for_unicode)
		{
			// найдено восьмеричное число, ставим итератор после него
			// и возвращаем результат дешифровки
			it = seq;
			return accumulator;
		}
		else
		{
			// литерал с ошибкой, возвращаем символы как есть
			return *(it-1);
		}
	}

	// разбор остальных последовательностей.
	// все нестандартные сочетания вида \C преобразуются в символ C
	return esc_sequence_to_char(*(it++));
}

template<class STRING_1, class STRING_2>
STRING_1 decode_escape_sequences_template(const STRING_2 str)
{
	STRING_1 result;
	typename STRING_2::const_iterator	it = str.begin(), ie = str.end();
	for(; it<ie;)
	{
		// итератор прибавляется в Decode...
		result += decode_escape_sequence(it, ie);
	}
	return result;
}

template <class STRING_T>
STRING_T encode_escape_sequences_template(const STRING_T &str, const STRING_T &aux_symbols)
{
	STRING_T result;
	typename STRING_T::const_iterator	it = str.begin(), ie = str.end();
	for(;it<ie;++it)
	{
		result += encode_escape_sequence(*it, aux_symbols);
	}
	return result;
}

}//namespace



string encode_escape_sequences(const string &str, const string &aux_symbols)
{
	return encode_escape_sequences_template(str, aux_symbols);
}

wstring encode_escape_sequences(const wstring &str, const wstring &aux_symbols)
{
	return encode_escape_sequences_template(str, aux_symbols);
}

string decode_escape_sequences(const string &str)
{
	return decode_escape_sequences_template<string, string>(str);
}

wstring decode_escape_sequences(const wstring &str)
{
	return decode_escape_sequences_template<wstring, wstring>(str);
}



XRAD_END
