/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"

#include "QtStringConverters_Test.h"
#include <XRADQt/QtStringConverters.h>

XRAD_BEGIN

namespace QtStringConverters_Test
{

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

struct TableItem
{
	enum { f_good = 0 };
	int flag_unused;

	//! \brief Идентификатор для локализации ошибки при тестировании
	const char *id;

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
		TableItem::f_good, u8"ASCII-symbols",
		// Некоторые произвольные символы
		L"012abc",
		U"012abc"
	},

	{
		TableItem::f_good, u8"Different symbols",
		// +ÀΣﬁ🌍🌠🍌🎄{\U0010FFFF}- Эти символы покрывают разные диапазоны Юникода.
		L"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-",
		U"+\u00C0\u03A3\uFB01\U0001F30D\U0001F320\U0001F34C\U0001F384\U0010FFFF-"
	},

	// Обработка нулевого символа

	// Одиночный нулевой символ
	{
		TableItem::f_good, u8"Null symbol alone",
		L"@",
		U"@"
	},

	// Нулевой символ в тексте
	{
		TableItem::f_good, u8"Null symbol in a text",
		L"12@34\u07FF@\uFFFF@\U0010FFFF@.|.",
		U"12@34\u07FF@\uFFFF@\U0010FFFF@.|."
	},

	// Конец таблицы
	{ TableItem::f_good, nullptr, nullptr, nullptr }
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

	for (size_t i = 0; tests_cp[i].wstring; ++i)
	{
		const auto &test = tests_cp[i];

		if (test.wstring && test.utf32)
		{
			wstring str_w_sample = test.parse(test.wstring);
			u32string str32_sample_ = test.parse(test.utf32);
			QString str_q_sample = QString::fromUcs4(str32_sample_.c_str(), (int)str32_sample_.length());
			{
				QString str_q = wstring_to_qstring(str_w_sample);
				if (str_q != str_q_sample)
				{
					report_error(ssprintf("WideChar -> QString test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}

			{
				auto str_w = qstring_to_wstring(str_q_sample);
				if (str_w != str_w_sample)
				{
					report_error(ssprintf("QString -> WideChar test error at %zu: %s (value mismatch).\n",
							EnsureType<size_t>(i), EnsureType<const char*>(test.id)));
				}
			}
		}
	}
}

//--------------------------------------------------------------

} // namespace QtStringConverters_Test

XRAD_END

//--------------------------------------------------------------
