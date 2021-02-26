/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file StringEncodeTest.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "StringEncodeTest.h"

XRAD_BEGIN

namespace StringEncodeTest
{

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

struct TableItem
{
	enum { f_good = 0, f_bad_8 = 1, f_bad_16 = 2, f_bad_32 = 3 };
	int flag;

	//! \brief Идентификатор для локализации ошибки при тестировании
	const char *id;

	//! \brief Строка UTF-8, содержит подстановки, см. parse()
	const char *utf8;
	//! \brief Строка UTF-16, содержит подстановки, см. parse()
	const char16_t *utf16;
	//! \brief Строка UTF-32, содержит подстановки, см. parse()
	const char32_t *utf32;

	static constexpr char default_character() { return '~'; }

	/*!
		\brief Выполнить подстановки в строке

		Особые символы:
		- '~' считается символом по умолчанию (он подставляется вместо недопустимых символов);
		- '@' заменяется нулевым символом;
		- '|' разбивает строку на основную часть и "хвост", для проверки выхода за границы строки.

		Замечания

		- Последовательности \xHH в строке u8"..." дают не сырые байты, а символы Unicode,
		которые затем кодируются по правилам кодировки UTF-8. Например, u8"\xFF" дает не байт 0xFF,
		а последовательность 0xC3, 0xBF. Т.е. \xHH получается почти эквивалентным \uHHHН
		за исключением того, что \xHH позволяет кодировать запрещенный диапазон кодов суррогатных пар.
		Значения больше 0x10FFFF приводят к ошибке компиляции. (MSVC 2015)

		- Последовательности \xHH в строке u"..." дают сырые 16-битные значения char16_t. (MSVC 2015)

		- Последовательности \xHH в строке U"..." дают сырые 32-битные значения char32_t
		(не производится проверка на допустимость кода символа). (MSVC 2015)
	*/
	template <class char_t>
	static basic_string<char_t> parse(const char_t *src_str, basic_string<char_t> *tail = nullptr)
	{
		basic_string<char_t> str(src_str);
		if (str.find('@'))
		{
			replace(str.begin(), str.end(), char_t('@'), char_t('\0'));
		}
		size_t pos = str.find('|');
		if (pos == str.npos)
		{
			if (tail)
				tail->clear();
			return str;
		}
		if (tail)
			*tail = str.substr(pos + 1);
		return str.substr(0, pos);
	}
};

//--------------------------------------------------------------

const TableItem tests_utf[] =
{
	// Тест значений по диапазону символов

	// Некоторые произвольные символы
	{
		TableItem::f_good, u8"Symbols0",
		u8"012abc",
		u"012abc",
		U"012abc"
	},

	// Граничные значения кодировки UTF-8
	{
		TableItem::f_good, u8"UTF-8 edge",
		u8"\u007F\u0080\u07FF\u0800\uD7FF\uE000\uFFFF\U00010000\U0010FFFF",
		u"\u007F\u0080\u07FF\u0800\uD7FF\uE000\uFFFF\U00010000\U0010FFFF",
		U"\u007F\u0080\u07FF\u0800\uD7FF\uE000\uFFFF\U00010000\U0010FFFF"
	},

	// Граничные значения кодировки UTF-16
	{
		TableItem::f_good, u8"UTF-16 edge",
		u8"\uD7FF\uE000\uFFFF\U00010000\U0010FFFF",
		u"\uD7FF\uE000\uFFFF\U00010000\U0010FFFF",
		U"\uD7FF\uE000\uFFFF\U00010000\U0010FFFF"
	},

	// Степени 2
	{
		TableItem::f_good, u8"UTF-8 2 powers",
		u8"\u0001\u0002\u0004\u0008\u0010\u0020\u0040\u0080\u0100\u0200\u0400\u0800\u1000\u2000\u4000\u8000\U00010000\U00100000",
		u"\u0001\u0002\u0004\u0008\u0010\u0020\u0040\u0080\u0100\u0200\u0400\u0800\u1000\u2000\u4000\u8000\U00010000\U00100000",
		U"\u0001\u0002\u0004\u0008\u0010\u0020\u0040\u0080\u0100\u0200\u0400\u0800\u1000\u2000\u4000\u8000\U00010000\U00100000"
	},

	// Степени 2 минус 1
	{
		TableItem::f_good, u8"UTF-8 2 powers - 1",
		u8"\u0001\u0002\u0003\u0007\u000F\u001F\u003F\u007F\u00FF\u01FF\u03FF\u07FF\u0FFF\u1FFF\u3FFF\u7FFF\U0000FFFF\U000FFFFF",
		u"\u0001\u0002\u0003\u0007\u000F\u001F\u003F\u007F\u00FF\u01FF\u03FF\u07FF\u0FFF\u1FFF\u3FFF\u7FFF\U0000FFFF\U000FFFFF",
		U"\u0001\u0002\u0003\u0007\u000F\u001F\u003F\u007F\u00FF\u01FF\u03FF\u07FF\u0FFF\u1FFF\u3FFF\u7FFF\U0000FFFF\U000FFFFF"
	},

	// Степени 2 + небольшие добавки
	{
		TableItem::f_good, u8"UTF-8 2 powers + small",
		u8"\u0001\u0002\u0004\u0008\u0015\u0025\u0045\u0085\u0155\u0255\u0455\u0855\u1555\u2555\u4555\u8555\U00015555\U00105555",
		u"\u0001\u0002\u0004\u0008\u0015\u0025\u0045\u0085\u0155\u0255\u0455\u0855\u1555\u2555\u4555\u8555\U00015555\U00105555",
		U"\u0001\u0002\u0004\u0008\u0015\u0025\u0045\u0085\u0155\u0255\u0455\u0855\u1555\u2555\u4555\u8555\U00015555\U00105555"
	},

	// Значения между степенями 2
	{
		TableItem::f_good, u8"UTF-8 2 powers intermediate",
		u8"\u0001\u0003\u0006\u000C\u0018\u0035\u0065\u00C5\u0185\u0355\u0655\u0C55\u1855\u3555\u6555\uC555\U00018555\U00108555",
		u"\u0001\u0003\u0006\u000C\u0018\u0035\u0065\u00C5\u0185\u0355\u0655\u0C55\u1855\u3555\u6555\uC555\U00018555\U00108555",
		U"\u0001\u0003\u0006\u000C\u0018\u0035\u0065\u00C5\u0185\u0355\u0655\u0C55\u1855\u3555\u6555\uC555\U00018555\U00108555"
	},



	// Проверка компилятора: escape-последовательность \x

	{
		TableItem::f_good, u8"Escape sequence test #1",
		u8"\x12345",
		u"\U00012345",
		U"\U00012345"
	},

	{
		TableItem::f_good, u8"Escape sequence test #2",
		u8"\U00012345",
		u"\xD808\xDF45", // 0x12345 - 0x10000 = 0x2345 = (0x8 << 10) | 0x345
		U"\U00012345"
	},

	{
		TableItem::f_good, u8"Escape sequence test #3",
		u8"\U00012345",
		u"\U00012345",
		U"\x00012345"
	},



	// Произвольные битовые наборы для UTF-8
	// (для проверки того, что значения из разных октетов UTF-8 комбинируются в правильном порядке)

	// Код длины 2: 0xD3, 0x95 = 110 10011, 10 010101 -> 100 1101 0101 = U+4D5
	{ TableItem::f_good, u8"UTF-8 any l=2",
		"\xD3\x95",
		u"\u04D5",
		U"\u04D5" },

	// Код длины 3: 0xEA, 0xB3, 0x87 = 1110 1010, 10 110011, 10 000111 -> 1010 1100 1100 0111 = U+ACC7
	{ TableItem::f_good, u8"UTF-8 any l=3",
		"\xEA\xB3\x87",
		u"\uACC7",
		U"\uACC7" },

	// Код длины 4: 0xF3, 0xAA, 0xB3, 0x87 = 11110 011, 10 101010, 10 110011, 10 000111 -> 0 1110 1010 1100 1100 0111 = U+0EACC7
	{ TableItem::f_good, u8"UTF-8 any l=4",
		"\xF3\xAA\xB3\x87",
		u"\U000EACC7",
		U"\U000EACC7" },

	// Код длины 4: 0xF4, 0x8A, 0xB3, 0x87 = 11110 100, 10 001010, 10 110011, 10 000111 -> 1 0000 1010 1100 1100 0111 = U+10ACC7
	{ TableItem::f_good, u8"UTF-8 any l=4 #2",
		"\xF4\x8A\xB3\x87",
		u"\U0010ACC7",
		U"\U0010ACC7" },



	// Произвольные битовые наборы для UTF-16
	// (для проверки того, что значения из разных частей суррогатной пары UTF-16 комбинируются в правильном порядке)

	{
		TableItem::f_good, u8"UTF-16 any",
		u8"\U000DCD55",
		u"\xDB33\xDD55", // 0xDB33, 0xDD55 <- 0x333.0x155 = 1100110011.0101010101 -> 0xCCD55 + 0x10000 = 0xDCD55
		U"\U000DCD55"
	},



	// Тест недопустимых кодов и близких к ним (соседних) допустимых кодов UTF-8

	// Слишком большое значение (минимальное):
	// 0xF4, 0x90, 0x80, 0x80 = 11110 100, 10 010000, 10 000000, 10 000000 -> 1 0001 0000 0000 0000 0000 = U+110000[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid big",
		"\xF4\x90\x80\x80",
		u"~",
		U"~" },

	// Слишком большое значение (произвольное, которое может быть закодировано 4 байтами):
	// 0xF4, 0xAA, 0xB3, 0x87 = 11110 100, 10 101010, 10 110011, 10 000111 -> 1 0010 1010 1100 1100 0111 = U+12ACC7[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid big max",
		"\xF4\xAA\xB3\x87",
		u"~",
		U"~" },

	// Код длины 5 по устаревшему RFC 2279, недопустимый сейчас:
	// 0xF8, 0x88, 0x80, 0x80, 0x80 = 111110 00, 10 001000, 10 000000, 10 000000, 10 000000 -> 00 0010 0000 0000 0000 0000 0000 = U+200000[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid old-5",
		"\xF8\x88\x80\x80\x80",
		u"~~~~~",
		U"~~~~~" },

	// Код длины 5 по устаревшему RFC 2279, недопустимый сейчас:
	// 0xFB, 0xBF, 0xBF, 0xBF, 0xBF = 111110 11, 10 111111, 10 111111, 10 111111, 10 111111 -> 11 1111 1111 1111 1111 1111 1111 = U+3FFFFFF[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid old-5 #2",
		"\xFB\xBF\xBF\xBF\xBF",
		u"~~~~~",
		U"~~~~~" },

	// Код длины 6 по устаревшему RFC 2279, недопустимый сейчас:
	// 0xFC, 0x84, 0x80, 0x80, 0x80, 0x80 = 1111110 0, 10 000100, 10 000000, 10 000000, 10 000000, 10 000000 -> 000 0100 0000 0000 0000 0000 0000 0000 = U+4000000[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid old-6",
		"\xFC\x84\x80\x80\x80\x80",
		u"~~~~~~",
		U"~~~~~~" },

	// Код длины 6 по устаревшему RFC 2279, недопустимый сейчас:
	// 0xFD, 0xBF, 0xBF, 0xBF, 0xBF, 0xBF = 1111110 1, 10 111111, 10 111111, 10 111111, 10 111111, 10 111111 -> 111 1111 1111 1111 1111 1111 1111 1111 = U+7FFFFFFF[!]
	{ TableItem::f_bad_8, u8"UTF-8 invalid old-6 #2",
		"\xFD\xBF\xBF\xBF\xBF\xBF",
		u"~~~~~~",
		U"~~~~~~" },

	// Не минимальный код длины 2 (должен кодироваться 1 байтом):
	// 0xC0, 0x80 = 110 00000, 10 00000000 -> 000 0000 0000 = U+0[2]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=2 U+0",
		"\xC0\x80",
		u"~",
		U"~" },

	// Не минимальный код длины 2 (должен кодироваться 1 байтом):
	// 0xC0, 0xBF = 110 00000, 10 111111 -> 000 0111 1111 = U+7F[2]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=2 U+7F",
		"\xC0\xBF",
		u"~",
		U"~" },

	// Не минимальный код длины 3 (должен кодироваться 1 байтом):
	// 0xE0, 0x80, 0x80 = 1110 0000, 10 000000, 10 000000 -> 0000 0000 0000 0000 = U+0[3]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=3 U+0",
		"\xE0\x80\x80",
		u"~",
		U"~" },

	// Не минимальный код длины 3 (должен кодироваться 2 байтами):
	// 0xE0, 0x9F, 0xBF = 1110 0000, 10 011111, 10 111111 -> 0000 0111 1111 1111 = U+7FF[3]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=3 U+7FF",
		"\xE0\x9F\xBF",
		u"~",
		U"~" },

	// Граничное допустимое значение перед диапазоном суррогатных пар:
	// 0xED, 0x9F, 0xBF = 1110 1101, 10 011111, 10 111111 -> 1101 0111 1111 1111 = U+D7FF
	{ TableItem::f_good, u8"UTF-8 surrogate pre",
		"\xED\x9F\xBF",
		u"\uD7FF",
		U"\uD7FF" },
	// Минимальное значение из диапазона суррогатных пар (недопустимое):
	// 0xED, 0xA0, 0x80 = 1110 1101, 10 100000, 10 000000 -> 1101 1000 0000 0000 = U+D800[!]
	{ TableItem::f_bad_8, u8"UTF-8 surrogate min",
		"\xED\xA0\x80",
		u"~",
		U"~" },
	// Максимальное значение из диапазона суррогатных пар (недопустимое):
	// 0xED, 0xBF, 0xBF = 1110 1101, 10 111111, 10 111111 -> 1101 1111 1111 1111 = U+DFFF[!]
	{ TableItem::f_bad_8, u8"UTF-8 surrogate max",
		"\xED\xBF\xBF",
		u"~",
		U"~" },
	// Граничное допустимое значение после диапазона суррогатных пар:
	// 0xEE, 0x80, 0x80 = 1110 1110, 10 000000, 10 000000 -> 1110 0000 0000 0000 = U+E000
	{ TableItem::f_good, u8"UTF-8 surrogate post",
		"\xEE\x80\x80",
		u"\uE000",
		U"\uE000" },

	// Не минимальный код длины 4 (должен кодироваться 1 байтом):
	// 0xF0, 0x80, 0x80, 0x80 = 11110 000, 10 000000, 10 000000, 10 000000 -> 0 0000 0000 0000 0000 0000 = U+0[4]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=4 U+0",
		"\xF0\x80\x80\x80",
		u"~",
		U"~" },

	// Не минимальный код длины 4 (должен кодироваться 3 байтами):
	// 0xF0, 0x8F, 0xBF, 0xBF = 11110 000, 10 001111, 10 111111, 10 111111 -> 0 0000 1111 1111 1111 1111 = U+FFFF[4]
	{ TableItem::f_bad_8, u8"UTF-8 not minimal l=4 U+FFFF",
		"\xF0\x8F\xBF\xBF",
		u"~",
		U"~" },



	// Неполные последовательности UTF-8

	// Код длины 2: 0xD3, 0x95 = 110 10011, 10 010101 -> 100 1101 0101 = U+4D5
	{ TableItem::f_good, u8"UTF-8 incomplete l=2 good",
		"+\xD3\x95-", u"+\u04D5-", U"+\u04D5-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=2->1 + ascii", // 0x95 = 10 010101 -> 00 010101 = 0x15
		"+\xD3\x15-", u"+~\u0015-", U"+~\u0015-"},
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=2->1 + broken", // 0x95 = 10 010101 -> 11 010101 = 0xD5
		"+\xD3\xD5-", u"+~~-", U"+~~-"},
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=2->1 eos",
		"+\xD3|\x95", u"+~", U"+~"},

	// "Хвост":
	{ TableItem::f_bad_8, u8"UTF-8 incomplete tail",
		"+\x95-", u"+~-", U"+~-"},
	{ TableItem::f_bad_8, u8"UTF-8 incomplete tail eos",
		"+\x95", u"+~", U"+~"},
	{ TableItem::f_bad_8, u8"UTF-8 incomplete tail #2",
		"+\xD3\x95\x95-", u"+\u04D5~-", U"+\u04D5~-" },

	// Код длины 3: 0xEA, 0xB3, 0x87 = 1110 1010, 10 110011, 10 000111 -> 1010 1100 1100 0111 = U+ACC7
	{ TableItem::f_good, u8"UTF-8 incomplete l=3 good",
		"+\xEA\xB3\x87-", u"+\uACC7-", U"+\uACC7-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->2 + ascii", // 0x87 = 10 000111 -> 00 000111 = 0x07
		"+\xEA\xB3\x07-", u"+~\u0007-", U"+~\u0007-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->2 + broken", // 0x87 = 10 000111 -> 11 0001111 = 0xC7
		"+\xEA\xB3\xC7-", u"+~~-", U"+~~-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->2 eos",
		"+\xEA\xB3|\x87", u"+~", U"+~" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->1 + ascii", // 0xB3 = 10 110011 -> 00 110011 = 0x33; 0x07
		"+\xEA\x33\x07-", u"+~\u0033\u0007-", U"+~\u0033\u0007-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->1 + broken", // 0xB3 = 10 110011 -> 11 110011 = 0xF3; 0xC7
		"+\xEA\xF3\xC7-", u"+~~~-", U"+~~~-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=3->1 eos",
		"+\xEA|\xB3\x87", u"+~", U"+~" },

	// Код длины 4: 0xF3, 0xAA, 0xB3, 0x87 = 11110 011, 10 101010, 10 110011, 10 000111 -> 0 1110 1010 1100 1100 0111 = U+0EACC7
	{ TableItem::f_good, u8"UTF-8 incomplete l=4 good",
		"+\xF3\xAA\xB3\x87-", u"+\U000EACC7-", U"+\U000EACC7-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->3 + ascii", // 0x87 = 10 000111 -> 00 000111 = 0x07
		"+\xF3\xAA\xB3\x07-", u"+~\u0007-", U"+~\u0007-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->3 + broken", // 0x87 = 10 000111 -> 11 000111 = 0xC7
		"+\xF3\xAA\xB3\xC7-", u"+~~-", U"+~~-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->3 eos",
		"+\xF3\xAA\xB3|\x87", u"+~", U"+~" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->2 + ascii", // 0xB3 = 10 110011 -> 00 110011 = 0x33; 0x07
		"+\xF3\xAA\x33\x07-", u"+~\u0033\u0007-", U"+~\u0033\u0007-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->2 + broken", // 0xB3 = 10 110011 -> 11 110011 = 0xF3; 0xC7
		"+\xF3\xAA\xF3\xC7-", u"+~~~-", U"+~~~-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->2 eos",
		"+\xF3\xAA|\xB3\x87", u"+~", U"+~" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->1 + ascii", // 0xAA = 10 101010 -> 00 101010 = 0x2A; 0x33; 0x07
		"+\xF3\x2A\x33\x07-", u"+~\u002A\u0033\u0007-", U"+~\u002A\u0033\u0007-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->1 + broken", // 0xAA = 10 101010 -> 11 101010 = 0xEA; 0xF3; 0xC7
		"+\xF3\xEA\xF3\xC7-", u"+~~~~-", U"+~~~~-" },
	{ TableItem::f_bad_8, u8"UTF-8 incomplete l=4->1 eos",
		"+\xF3|\xAA\xB3\x87", u"+~", U"+~" },



	// Неполные последовательности UTF-16

	// 0xDB33, 0xDD55 <- 0x333.0x155 = 1100110011.0101010101 -> 0xCCD55 + 0x10000 = 0xDCD55
	{ TableItem::f_good, u8"UTF-16 incomplete good",
		u8"+\U000DCD55-", u"+\xDB33\xDD55-", U"+\U000DCD55-" },
	{ TableItem::f_bad_16, u8"UTF-16 incomplete l=2->1 + ascii",
		u8"+~\u0020-", u"+\xDB33\u0020-", U"+~\u0020-" },
	{ TableItem::f_bad_16, u8"UTF-16 incomplete l=2->1 + surrogate",
		u8"+~\U00010000-", u"+\xDB33\xD800\xDC00-", U"+~\U00010000-" }, // U+00010000 -> 0xD800, 0xDC00
	{ TableItem::f_bad_16, u8"UTF-16 incomplete l=2->1 + non-surrogate hi",
		u8"+~\uF000-", u"+\xDB33\uF000-", U"+~\uF000-" },
	{ TableItem::f_bad_16, u8"UTF-16 incomplete l=2->1 eos",
		u8"+~", u"+\xDB33|\xDD55", U"+~" },
	{ TableItem::f_bad_16, u8"UTF-16 incomplete tail + ascii",
		u8"+~\u0020-", u"+\xDD55\u0020-", U"+~\u0020-" },
	{ TableItem::f_bad_16, u8"UTF-16 incomplete tail + tail + eos",
		u8"+~~", u"+\xDD55\xDD55|-", U"+~~" },



	// Обработка нулевого символа

	// Одиночный нулевой символ
	{
		TableItem::f_good, u8"Null symbol alone",
		u8"@",
		u"@",
		U"@"
	},

	// Нулевой символ в тексте
	{
		TableItem::f_good, u8"Null symbol in a text",
		u8"12@34\u07FF@\uFFFF@\U0010FFFF@.|.",
		u"12@34\u07FF@\uFFFF@\U0010FFFF@.|.",
		U"12@34\u07FF@\uFFFF@\U0010FFFF@.|."
	},



	// Тест недопустимых кодов UTF-32

	// Код начала диапазона суррогатных пар
	{ TableItem::f_bad_32, u8"UTF-32 surrogate min",
		"~",
		u"~",
		U"\xD800" },
	// Код конца диапазона суррогатных пар
	{ TableItem::f_bad_32, u8"UTF-32 surrogate max",
		"~",
		u"~",
		U"\xDFFF" },
	// Число, на 1 большее максимально допустимого кода символа
	{ TableItem::f_bad_32, u8"UTF-32 big: max + 1",
		"~",
		u"~",
		U"\x00110000" },
	// Максимальное 31-битное число
	{ TableItem::f_bad_32, u8"UTF-32 big: max 31-bit",
		"~",
		u"~",
		U"\x7FFFFFFF" },
	// Максимальное 32-битное число
	{ TableItem::f_bad_32, u8"UTF-32 big: max 32-bit",
		"~",
		u"~",
		U"\xFFFFFFFF" },



	// Конец таблицы

	{ TableItem::f_good, nullptr, nullptr, nullptr, nullptr }
};

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

void Test(ErrorReporter *error_reporter)
{
	auto report_error = [error_reporter](const string &error_string)
	{
		// Debugger breakpoint can be set here.
		error_reporter->ReportError(error_string);
	};

	for (size_t i = 0; tests_utf[i].utf8 && tests_utf[i].utf16; ++i)
	{
		const auto &test = tests_utf[i];
		int flag = test.flag;
		if (test.utf8 && test.utf32)
		{
			ustring str8_tail;
			ustring str8_sample = test.parse(uchar_t::pointer_from_char(test.utf8), &str8_tail);
			u32string str32_sample = test.parse(test.utf32);
			if (flag == TableItem::f_good || flag == TableItem::f_bad_8)
			{
				bool error_sample = flag == TableItem::f_bad_8;
				bool error = false;
				u32string str32;
				if (str8_tail.length())
				{
					ustring buf8 = str8_sample + str8_tail;
					str32 = ustring_to_u32string(buf8.c_str(), str8_sample.length(), TableItem::default_character(), &error);
				}
				else
				{
					str32 = ustring_to_u32string(str8_sample, TableItem::default_character(), &error);
				}
				if (str32 != str32_sample)
				{
					report_error(ssprintf("UTF-8 -> UTF-32 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-8 -> UTF-32 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			if (flag == TableItem::f_good || flag == TableItem::f_bad_32)
			{
				bool error_sample = flag == TableItem::f_bad_32;
				bool error = false;
				auto str8 = u32string_to_ustring(str32_sample, TableItem::default_character(), &error);
				if (str8 != str8_sample)
				{
					report_error(ssprintf("UTF-32 -> UTF-8 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-32 -> UTF-8 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}

		if (test.utf16 && test.utf32)
		{
			u16string str16_sample = test.parse(test.utf16);
			u32string str32_sample = test.parse(test.utf32);
			if (flag == TableItem::f_good || flag == TableItem::f_bad_32)
			{
				bool error_sample = flag == TableItem::f_bad_32;
				bool error = false;
				auto str16 = u32string_to_u16string(str32_sample, TableItem::default_character(), &error);
				if (str16 != str16_sample)
				{
					report_error(ssprintf("UTF-32 -> UTF-16 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-32 -> UTF-16 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			if (flag == TableItem::f_good || flag == TableItem::f_bad_16)
			{
				bool error_sample = flag == TableItem::f_bad_16;
				bool error = false;
				auto str32 = u16string_to_u32string(str16_sample, TableItem::default_character(), &error);
				if (str32 != str32_sample)
				{
					report_error(ssprintf("UTF-16 -> UTF-32 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-16 -> UTF-32 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}

		if (test.utf8 && test.utf16)
		{
			ustring str8_sample = test.parse(uchar_t::pointer_from_char(test.utf8));
			u16string str16_sample = test.parse(test.utf16);
			if (flag == TableItem::f_good || flag == TableItem::f_bad_8)
			{
				bool error_sample = flag == TableItem::f_bad_8;
				bool error = false;
				auto str16 = ustring_to_u16string(str8_sample, TableItem::default_character(), &error);
				if (str16 != str16_sample)
				{
					report_error(ssprintf("UTF-8 -> UTF-16 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-8 -> UTF-16 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			if (flag == TableItem::f_good || flag == TableItem::f_bad_16)
			{
				bool error_sample = flag == TableItem::f_bad_16;
				bool error = false;
				auto str8 = u16string_to_ustring(str16_sample, TableItem::default_character(), &error);
				if (str8 != str8_sample)
				{
					report_error(ssprintf("UTF-16 -> UTF-8 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("UTF-16 -> UTF-8 test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}
	}
}

//--------------------------------------------------------------

} // namespace StringEncodeTest

XRAD_END

//--------------------------------------------------------------
