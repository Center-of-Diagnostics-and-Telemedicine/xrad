/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file StringConverters_MS_Test.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "StringConverters_MS_Test.h"

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <XRADBasic/Sources/PlatformSpecific/MSVC/Internal/StringConverters_MS.h>

XRAD_BEGIN

namespace StringConverters_MS_Test
{

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

struct TableItem
{
	enum { f_good = 0, f_bad_c, f_bad_w, f_bad_32,
		f_one_way_c, f_one_way_unicode };
	int flag;

	//! \brief Идентификатор для локализации ошибки при тестировании
	const char *id;

	//! \brief Кодовая страница для "простой" строки
	CodePage code_page;

	bool decode_literals;

	//! \brief Строка в кодировке code_page, содержит подстановки, см. parse()
	const char *cstring;
	//! \brief Строка UTF-16, содержит подстановки, см. parse()
	const wchar_t *wstring;
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

const TableItem tests_cp[] =
{
	// Тест значений по диапазону символов

	{
		TableItem::f_good, u8"CP-1251: ASCII-symbols",
		CodePage::cp_1251,
		true,
		// Некоторые произвольные символы
		"012abc",
		L"012abc",
		U"012abc"
	},

	{
		TableItem::f_good, u8"CP-1251: Specific chars",
		CodePage::cp_1251,
		true,
		// +АБЮЯабюя-+«»“”-
		// (*1522185218) Кодирование U+401, U+411.
		"+\xC0\xC1\xDE\xDF\xE0\xE1\xFE\xFF-+\xAB\xBB\x93\x94-",
		L"+\u0410\u0411\u042E\u042F\u0430\u0431\u044E\u044F-+\u00AB\u00BB\u201C\u201D-",
		U"+АБЮЯабюя-+\u00AB\u00BB\u201C\u201D-"
	},

	{
		TableItem::f_good, u8"CP-1251: Specific chars #2",
		CodePage::cp_1251,
		true,
		// +\x98­­­­­\xAD- Эти коды в CP1251, возможно, являются недопустимыми.
		// Но конвертация проходит без ошибок (Win10.1607).
		"+\x98\xAD-",
		L"+\u0098\u00AD-",
		U"+\u0098\u00AD-"
	},

	{
		TableItem::f_good, u8"CP-1251: Out of the code page coverage",
		CodePage::cp_1251,
		true,
		// +ÀΣﬁ🌍🌠🍌🎄{\U0010FFFF}- Эти символы покрывают разные диапазоны Юникода.
		// (*1522185760) Кодирование U+303.
		"+{\\uC0}{\\u3A3}{\\uFB01}{\\u1F30D}{\\u1F320}{\\u1F34C}{\\u1F384}{\\u10FFFF}-",
		L"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-",
		U"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Encoded ASCII chars",
		CodePage::cp_1251,
		true,
		// +{\x01}01- Закодированные символы ASCII: такое кодирование недопустимо.
		"+\x01{\\u30}{\\u31}-",
		L"+\u0001{\\u30}{\\u31}-",
		U"+\u0001{\\u30}{\\u31}-"
	},



	// Необратимые преобразования

	{
		TableItem::f_one_way_c, u8"CP-1251: Encoded covered chars (irreversible)",
		CodePage::cp_1251,
		true,
		// +АБ- Закодированные символы Юникода из набора символов кодокой страницы:
		// преобразование из Юникода в кодовую страницу преобразует их в символы.
		// См. (*1522185218) для обратного преобразования.
		"+{\\u410}{\\u411}-",
		L"+\u0410\u0411-",
		U"+\u0410\u0411-"
	},

	{
		TableItem::f_one_way_c, u8"CP-1251: Irreversible encoded sequence (small)",
		CodePage::cp_1251,
		true,
		// +Σ- Кодовая последовательность с буквами в нижнем регистре
		// (при преобразовании в кодовую страницу для кодовых последовательностей
		// используется верхний регистр).
		// См. (*1522185760) для обратного преобразования.
		"+{\\u3a3}-",
		L"+\u03A3-",
		U"+\u03A3-"
	},

	{
		TableItem::f_one_way_c, u8"CP-1251: Irreversible encoded sequence (zero padding 1)",
		CodePage::cp_1251,
		true,
		// +Σ- Кодовая последовательность с дополнением нулями слева до 4 символов.
		// См. (*1522185760) для обратного преобразования.
		"+{\\u03A3}-",
		L"+\u03A3-",
		U"+\u03A3-"
	},

	{
		TableItem::f_one_way_c, u8"CP-1251: Irreversible encoded sequence (zero padding 2)",
		CodePage::cp_1251,
		true,
		// +Σ- Кодовая последовательность с дополнением нулями слева до 5 символов.
		// См. (*1522185760) для обратного преобразования.
		"+{\\u003A3}-",
		L"+\u03A3-",
		U"+\u03A3-"
	},

	{
		TableItem::f_one_way_c, u8"CP-1251: Irreversible encoded sequence (zero padding 3)",
		CodePage::cp_1251,
		true,
		// +Σ- Кодовая последовательность с дополнением нулями слева до 6 символов.
		// См. (*1522185760) для обратного преобразования.
		"+{\\u0003A3}-",
		L"+\u03A3-",
		U"+\u03A3-"
	},



	// Строки, при преобразовании которых устанавливается признак ошибки

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (begin 0.1)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности: нет цифр.
		"+{\\u",
		L"+{\\u",
		U"+{\\u"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (begin 0.2)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности: нет цифр.
		"+{\\u-",
		L"+{\\u-",
		U"+{\\u-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (begin 0.3)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности: нет цифр.
		"+{\\u--",
		L"+{\\u--",
		U"+{\\u--"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (begin 0.4)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности: нет цифр.
		"+{\\u---",
		L"+{\\u---",
		U"+{\\u---"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 0.1)",
		CodePage::cp_1251,
		true,
		// Недопустимые кодовые последовательности: нет цифр.
		"+{\\u}-",
		L"+{\\u}-",
		U"+{\\u}-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 0.2)",
		CodePage::cp_1251,
		true,
		// Недопустимые кодовые последовательности: нет цифр.
		"+{\\u}--",
		L"+{\\u}--",
		U"+{\\u}--"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 1.1)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности с 1 значащим символом.
		"+{\\u0-",
		L"+{\\u0-",
		U"+{\\u0-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 1.2)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности с 1 значащим символом.
		"+{\\u1-",
		L"+{\\u1-",
		U"+{\\u1-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 2)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности с 2 значащими символами.
		"+{\\uFF-",
		L"+{\\uFF-",
		U"+{\\uFF-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (trunc 6)",
		CodePage::cp_1251,
		true,
		// Неоконченные кодовые последовательности с 6 значащими символами.
		"+{\\u0003A3-",
		L"+{\\u0003A3-",
		U"+{\\u0003A3-"
	},

	{
		TableItem::f_bad_c, u8"CP-1251: Bad encoded sequences (too many digits)",
		CodePage::cp_1251,
		true,
		// Недопустимые кодовые последовательности: с 7 цифрами.
		"+{\\u00003A3}-",
		L"+{\\u00003A3}-",
		U"+{\\u00003A3}-"
	},



	// Строки, для которых преобразование из Unocode в CP и обратно в Unicode дает результат,
	// отличный от исходной строки

	{
		TableItem::f_one_way_unicode, u8"CP-1251: Encoded sequence in wstring (1)",
		CodePage::cp_1251,
		true,
		// +{\\u410}{\\u411}- Кодовые последовательности в wstring.
		// См. (*1522185218) для обратного преобразования.
		"+{\\u410}{\\u411}-",
		L"+{\\u410}{\\u411}-",
		U"+{\\u410}{\\u411}-"
	},

	{
		TableItem::f_one_way_unicode, u8"CP-1251: Encoded sequence in wstring (2)",
		CodePage::cp_1251,
		true,
		// +{\\u3a3}- Кодовая последовательность в wstring.
		// См. (*1522185760) для обратного преобразования.
		"+{\\u3a3}-",
		L"+{\\u3a3}-",
		U"+{\\u3a3}-"
	},



	// Преобразования с отключенным использованием кодовых последовательностей

	{
		TableItem::f_good, u8"CP-1251: Encoded sequences disabled (1)",
		CodePage::cp_1251,
		false,
		// Без декодирования последовательностей
		"+{\\u3A3}-",
		L"+{\\u3A3}-",
		U"+{\\u3A3}-"
	},
	{
		TableItem::f_good, u8"CP-1251: Encoded sequences disabled (2)",
		CodePage::cp_1251,
		false,
		// Без декодирования последовательностей
		"+{\\u3a3}-",
		L"+{\\u3a3}-",
		U"+{\\u3a3}-"
	},



	// Тесты для многобайтовой кодовой страницы

	{
		TableItem::f_good, u8"CP-54936: ASCII-symbols",
		CodePage::cp_54936,
		true,
		// Произвольные ASCII-символы
		"012abc",
		L"012abc",
		U"012abc"
	},

	{
		TableItem::f_good, u8"CP-54936: Different symbols (1)",
		CodePage::cp_54936,
		true,
		// +Σ- 2-байтовый символ.
		"+\xA6\xB2-",
		L"+\u03A3-",
		U"+\u03A3-"
	},

	{
		TableItem::f_good, u8"CP-54936: Different symbols (2)",
		CodePage::cp_54936,
		true,
		// +À- 4-байтовый символ.
		"+\x81\x30\x86\x38-",
		L"+\u00C0-",
		U"+\u00C0-"
	},

	{
		TableItem::f_good, u8"CP-54936: Different symbols (3)",
		CodePage::cp_54936,
		true,
		// +ÀΣﬁ🌍🌠🍌🎄{\U0010FFFF}- Эти символы покрывают разные диапазоны Юникода.
		"+\x81\x30\x86\x38\xA6\xB2\x84\x30\xB2\x33\x94\x39\xB1\x31\x94\x39\xB3\x30\x94\x39\xB7\x34\x94\x39\xBD\x30\xE3\x32\x9A\x35-",
		L"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-",
		U"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-"
	},

	{
		TableItem::f_bad_c, u8"CP-54936: Truncated MBCS",
		CodePage::cp_54936,
		true,
		// +<часть кода À>- Недопустимый код.
		"+\x81\x30-",
		L"+?0-",
		U"+?0-"
	},



	// Неполные последовательности UTF-16 (дают ошибку при преобразовании в другие кодировки)

	// 0xDB33, 0xDD55 <- 0x333.0x155 = 1100110011.0101010101 -> 0xCCD55 + 0x10000 = 0xDCD55
	{ TableItem::f_good, u8"UTF-16 incomplete good",
		CodePage::cp_1251, true,
		"+{\\uDCD55}-", L"+\xDB33\xDD55-", U"+\U000DCD55-" }, // Исходный материал — правильные коды
	{ TableItem::f_bad_w, u8"UTF-16 incomplete l=2->1 + ascii",
		CodePage::cp_1251, true,
		"+~\x20-", L"+\xDB33\u0020-", U"+~\u0020-" },
	{ TableItem::f_bad_w, u8"UTF-16 incomplete l=2->1 + surrogate",
		CodePage::cp_1251, true,
		"+~{\\u10000}-", L"+\xDB33\xD800\xDC00-", U"+~\U00010000-" }, // U+00010000 -> 0xD800, 0xDC00
	{ TableItem::f_bad_w, u8"UTF-16 incomplete l=2->1 + non-surrogate hi",
		CodePage::cp_1251, true,
		"+~{\\uF000}-", L"+\xDB33\uF000-", U"+~\uF000-" },
	{ TableItem::f_bad_w, u8"UTF-16 incomplete l=2->1 eos",
		CodePage::cp_1251, true,
		"+~", L"+\xDB33|\xDD55", U"+~" },
	{ TableItem::f_bad_w, u8"UTF-16 incomplete tail + ascii",
		CodePage::cp_1251, true,
		"+~\x20-", L"+\xDD55\u0020-", U"+~\u0020-" },
	{ TableItem::f_bad_w, u8"UTF-16 incomplete tail + tail + eos",
		CodePage::cp_1251, true,
		"+~~", L"+\xDD55\xDD55|-", U"+~~" },



	// Обработка нулевого символа

	// Одиночный нулевой символ
	{
		TableItem::f_good, u8"Null symbol alone",
		CodePage::cp_1251, true,
		"@",
		L"@",
		U"@"
	},

	// Нулевой символ в тексте
	{
		TableItem::f_good, u8"Null symbol in a text",
		CodePage::cp_1251, true,
		"12@34{\\u7FF}@{\\uFFFF}@{\\u10FFFF}@.|.",
		L"12@34\u07FF@\uFFFF@\U0010FFFF@.|.",
		U"12@34\u07FF@\uFFFF@\U0010FFFF@.|."
	},



	// Конец таблицы
	{ TableItem::f_good, nullptr, CodePage::cp_system, nullptr, nullptr, nullptr }
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

	for (size_t i = 0; tests_cp[i].cstring && tests_cp[i].wstring; ++i)
	{
		const auto &test = tests_cp[i];
		auto code_page = test.code_page;
		bool decode_literals = test.decode_literals;
		int flag = test.flag;
		if (test.cstring && test.wstring)
		{
			string str_c_sample = test.parse(test.cstring);
			wstring str_w_sample = test.parse(test.wstring);
			if (flag == TableItem::f_good || flag == TableItem::f_bad_c || flag == TableItem::f_one_way_c)
			{
				bool error_sample = flag == TableItem::f_bad_c;
				bool error = false;
				wstring str_w = string_to_wstring_MS(str_c_sample, code_page, decode_literals, &error);
				if (str_w != str_w_sample)
				{
					//string str_c2 = wstring_to_string_MS(str_w, code_page, decode_literals, TableItem::default_character(), nullptr);
					report_error(ssprintf("CodePage -> WideChar test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("CodePage -> WideChar test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			if (flag == TableItem::f_good || flag == TableItem::f_bad_w || flag == TableItem::f_one_way_unicode)
			{
				bool error_sample = flag == TableItem::f_bad_w;
				bool error = false;
				auto str_c = wstring_to_string_MS(str_w_sample, code_page, decode_literals, TableItem::default_character(), &error);
				if (str_c != str_c_sample)
				{
					report_error(ssprintf("WideChar -> CodePage test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
				if (error != error_sample)
				{
					report_error(ssprintf("WideChar -> CodePage test error at %zu: %s (error flag mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}

		if (test.cstring && test.utf32)
		{
			// Для реализации этой части используется механизм задания кодовой страницы для string_to_u32string() и обратной функции.
			// Это изменение влияет на все обращения к функциям преобразования кодировки.
			// Этот механизм не поддерживает многопоточное исполнение!
			auto save_cp = get_code_page_MS();
			try
			{
				string str_c_sample = test.parse(test.cstring);
				u32string str32_sample = test.parse(test.utf32);
				if (flag == TableItem::f_good || flag == TableItem::f_bad_c || flag == TableItem::f_one_way_c)
				{
					set_code_page_MS(code_page);
					u32string str32 = string_to_u32string(str_c_sample, decode_literals? e_decode_literals: e_ignore_literals_decoding);
					set_code_page_MS(save_cp);
					if (str32 != str32_sample)
					{
						report_error(ssprintf("CodePage -> UTF-32 test error at %zu: %s (value mismatch).\n",
								EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
					}
				}

				if (flag == TableItem::f_good || flag == TableItem::f_bad_32 || flag == TableItem::f_one_way_unicode)
				{
					set_code_page_MS(code_page);
					auto str_c = u32string_to_string(str32_sample, decode_literals? e_encode_literals: e_ignore_literals_encoding);
					set_code_page_MS(save_cp);
					if (str_c != str_c_sample)
					{
						report_error(ssprintf("UTF-32 -> CodePage test error at %zu: %s (value mismatch).\n",
								EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
					}
				}
			}
			catch (...)
			{
				set_code_page_MS(save_cp);
				throw;
			}
		}

		if (test.wstring && test.utf32)
		{
			wstring str_w_sample = test.parse(test.wstring);
			u32string str32_sample = test.parse(test.utf32);
			if (flag == TableItem::f_good || flag == TableItem::f_bad_w || flag == TableItem::f_one_way_unicode)
			{
				u32string str32 = wstring_to_u32string(str_w_sample);
				if (str32 != str32_sample)
				{
					report_error(ssprintf("WideChar -> UTF-32 test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			if (flag == TableItem::f_good || flag == TableItem::f_bad_32 || flag == TableItem::f_one_way_unicode)
			{
				auto str_w = u32string_to_wstring(str32_sample);
				if (str_w != str_w_sample)
				{
					report_error(ssprintf("UTF-32 -> WideChar test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}
	}
}

//--------------------------------------------------------------

} // namespace StringConverters_MS_Test

XRAD_END

#else // XRAD_USE_MS_VERSION

#include <XRAD/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_StringConverters_MS_Test() {}
XRAD_END

#endif // XRAD_USE_MS_VERSION

//--------------------------------------------------------------
