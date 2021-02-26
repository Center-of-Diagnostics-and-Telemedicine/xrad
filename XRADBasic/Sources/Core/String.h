/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file String.h
//	Created by ACS on 25.10.01
//--------------------------------------------------------------
#ifndef XRAD__File_String_h
#define XRAD__File_String_h
//--------------------------------------------------------------

//	(?) Внутренний файл библиотеки.

//--------------------------------------------------------------
//
// функции из этого файла еще не утверждены и не везде реализованы.
// возможно, параметры и названия будут изменены.
//
//--------------------------------------------------------------

//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"
#include <string>
#include <vector>

XRAD_BEGIN



using namespace std;

//! \brief Значение, используемое для замены недопустимых символов. Значение в диапазоне [32, 127].
inline int default_ascii_character(){ return '~'; }

//typedef	uint8_t	uchar_t;

/*!
	\brief utf-8 char - отдельный тип, размер символа тот же, но сделан отдельным классом,
	чтобы исключить преобразование указателей

	Должен удовлетворять требованиям is_trivial && is_standard_layout, чтобы его можно было
	использовать в basic_string.
*/
class uchar_t
{
	public:
		/*!
			\brief Конструктор по умолчанию = default: обеспечиваем is_trivial && is_standard_layout

			Такое объявление позволяет создавать неинициализированные переменные и массивы при объявлениях
			вида `uchar_t x; uchar_t x[10];` и инициализированные нулем объекты при вызовах `uchar_t()`.
		*/
		uchar_t() = default;
		uchar_t(char c) : m_char(c){}

		operator char&(){ return m_char; }
		operator const char&() const { return m_char; }

		static const char *pointer_to_char(const uchar_t *pointer)
		{
			static_assert(sizeof(uchar_t) == sizeof(char), "Invalid uchar_t type.");
			return reinterpret_cast<const char*>(pointer);
		}
		static const uchar_t *pointer_from_char(const char *pointer)
		{
			static_assert(sizeof(uchar_t) == sizeof(char), "Invalid uchar_t type.");
			return reinterpret_cast<const uchar_t*>(pointer);
		}
	private:
		char	m_char;
};

//! \brief Тип, содержащий utf-8 строку и неприводимый к std::string
using ustring = std::basic_string<uchar_t, char_traits<uchar_t>, allocator<uchar_t> >;

//! \brief Тип, содержащий utf-8 строку и совпадающий с std::string. Дублирующее название, вводится для ясности кода.
//! UPD: во избежание путаницы с похожими, но разными именами типов (ustring, u8string) префикс u8 сделан постфиксом 8.
//!	Таким образом, префиксное написание означает совсем другой тип, постфиксное -- тот же тип, но с другим содержимым
using string8 = string;




//! \brief Безопасное преобразование char* к string, при котором
//! исключена ситуация, когда на вход конструктора std::string(char *)
//! попадает нулевой указатель (в этом случае стандартная реакция -- исключение)
inline string safe_string(const char *c){return c? c: "";}
inline wstring safe_string(const wchar_t *c){return c? c: L"";}

//! \brief Форматированный вывод по правилам vsprintf, результат помещается в возвращаемую строку
string	vssprintf(const char *format, va_list param);
wstring	vssprintf(const wchar_t *format, va_list param);
ustring	vssprintf(const uchar_t *format, va_list param);// массив uchar_t это начинка строки utf8

//! \brief Форматированный вывод по правилам sprintf, результат помещается в возвращаемую строку
//	2019_03_15 Доработано по идеям из статьи:
//	https://msdn.microsoft.com/ru-ru/magazine/dn913181.aspx
//	Использование ssprintf("%s", string("string")) корректно работает без добавления c_str()

//TODO	Представляет интерес обратная задача: получение предупреждений компилятора из-за некорректных аргументов функци printf.
//	warning C4477: format string '%d' requires an argument of type 'int', but variadic argument 1 has type 'double'
//	Подробнее:
//	https://blogs.msdn.microsoft.com/vcblog/2015/06/22/format-specifiers-checking/


//! \brief Вспомогательные функции, которые ранее были основными.
string	ssprintf_core(const char *format, ...);
wstring ssprintf_core(const wchar_t *format, ...);
ustring ssprintf_core(const uchar_t *format, ...);

//! \brief Преобразование аргумента к виду, приемлемому для sprintf_core
template <typename T>
const T &ssprintf_arg(T const & value) noexcept { return value; }
template <typename T>
const T * ssprintf_arg(const std::basic_string<T> & value) noexcept { return value.c_str(); }

//! \brief Основные функции вывода
template <class T, typename ... Args>
basic_string<T> ssprintf(const basic_string<T> &format, Args const & ... args) noexcept
{
	return ssprintf_core(format.c_str(), ssprintf_arg(args) ...);
}

template <class T, typename ... Args>
basic_string<T> ssprintf(const T *format, Args const & ... args) noexcept
{
	return ssprintf_core(format, ssprintf_arg(args) ...);
}




//! \brief Короткие строковые константы для разных типов строк
template<class T = wstring> T linefeed() = delete;
template<class T = wstring> T tab() = delete;

template<> string linefeed<string>();
template<> wstring linefeed<wstring>();
template<> string tab<string>();
template<> wstring tab<wstring>();



//! \brief Замена всех вхождений find_string на replace_string в заданном тексте
template<class STRING>
STRING	replace(const STRING &text, const STRING &find_string, const STRING &replace_string)
{
	STRING	result;

	size_t position = 0;
	while(position != string::npos)
	{
		size_t found = text.find(find_string, position);
		if(found!=string::npos)
		{
			result += text.substr(position, found-position);
			result += replace_string;
			position = found + find_string.size();
		}
		else
		{
			result += text.substr(position);
			position = found;
		}
	}
	return result;
}

//! \brief Сдвиг всех строк текста на одну табуляцию вправо
template<class STRING>
STRING	tabify(const STRING &text)
{
	return tab<STRING>() + replace(text, linefeed<STRING>(), linefeed<STRING>() + tab<STRING>());
}

//! \brief Сдвиг всех строк текста на одну табуляцию влево
template<class STRING>
STRING	detabify(const STRING &text)
{
	if(text.empty()) return text;
	return replace(text[0] == '\t' ? text.substr(1, string::npos) : text, linefeed<STRING>() + tab<STRING>(), linefeed<STRING>());
}

//--------------------------------------------------------------

//! \brief Разделить строку на части по заданному разделителю
vector<string> split(const string &str, char delimiter);

//! \brief Разделить строку на части по заданному разделителю
vector<wstring> split(const wstring &str, wchar_t delimiter);

//! \brief Склеить из частей одну строку, части соединить заданным разделителем
string merge(const vector<string> &strings, const string &delimiter);

//! \brief Склеить из частей одну строку, части соединить заданным разделителем
wstring merge(const vector<wstring> &strings, const wstring &delimiter);

//--------------------------------------------------------------

enum class LineEndKind { N, RN };

//--------------------------------------------------------------

template <class CharT>
struct LineEnds
{
	static const CharT line_end_n[];
	static const CharT line_end_rn[];
	static const CharT *get(LineEndKind line_end_kind, size_t *o_length = nullptr)
	{
		switch (line_end_kind)
		{
			default:
			case LineEndKind::N:
				if (o_length)
					*o_length = 1;
				return line_end_n;
			case LineEndKind::RN:
				if (o_length)
					*o_length = 2;
				return line_end_rn;
		}
	}
};

template <class CharT>
const CharT LineEnds<CharT>::line_end_n[] = {'\n', 0};

template <class CharT>
const CharT LineEnds<CharT>::line_end_rn[] = {'\r', '\n', 0};

//--------------------------------------------------------------

template <class String>
String normalize_line_ends(const String &text, LineEndKind line_end_kind)
{
	// Нормировать концы строк: "\n".
	using value_type = typename String::value_type;
	const value_type *source = text.c_str();
	size_t source_length = text.length();
	size_t line_end_length = 0;
	const value_type *line_end = LineEnds<value_type>::get(line_end_kind, &line_end_length);
	size_t result_buffer_length = source_length;
	if (line_end_length > 1)
	{
		size_t out_i = 0;
		bool prev_cr = false;
		for (size_t i = 0; i < source_length; ++i)
		{
			wchar_t c = source[i];
			if (c == (value_type)'\n')
			{
				if (!prev_cr)
					out_i += line_end_length;
				prev_cr = false;
			}
			else if (c == (value_type)'\r')
			{
				out_i += line_end_length;
				prev_cr = true;
			}
			else
			{
				++out_i;
				prev_cr = false;
			}
		}
		result_buffer_length = out_i;
	}
	vector<value_type> result_buf(result_buffer_length, 0);
	value_type *result = result_buf.data();
	size_t out_i = 0;
	bool prev_cr = false;
	for (size_t i = 0; i < source_length; ++i)
	{
		wchar_t c = source[i];
		if (c == (value_type)'\n')
		{
			if (!prev_cr)
			{
				for (size_t j = 0; j < line_end_length; ++j)
					result[out_i++] = line_end[j];
			}
			prev_cr = false;
		}
		else if (c == (value_type)'\r')
		{
			for (size_t j = 0; j < line_end_length; ++j)
				result[out_i++] = line_end[j];
			prev_cr = true;
		}
		else
		{
			result[out_i++] = c;
			prev_cr = false;
		}
	}
	return String(result, out_i);
}



//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_String_h
