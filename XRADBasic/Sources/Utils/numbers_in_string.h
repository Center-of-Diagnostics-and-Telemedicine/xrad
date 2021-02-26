/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_numbers_in_string_h
#define XRAD__File_numbers_in_string_h
/*!
	\file
	\date 2018/04/02 14:12
	\author kulberg
*/

#include <XRADBasic/Core.h>

XRAD_BEGIN

//! \brief Округление чисел, содержащих длинные последовательности нулей или девяток после запятой. Число вида 0.01239999999 должно преобразоваться в 0.0124. Число вида 1.20000005 должно преобразоваться в 1.2
//! \param s Ссылка на строковое представление округляемого числа
//! \param count Длина последовательности нулей или девяток, подлежащей округлению

void	smart_round(wstring &s, size_t count = 5);

//	вспомогательные процедуры для функций преобразования строки в число
template<class T>
inline bool is_digit(const T &symbol) { return in_range(symbol, int('0'), int('9')); };
//std::isdigit вылетало с неизвестным исключением на строке, начинавшейся словами "исследование содержит"

template<class C>
bool symbol_found(const C &symbol, const C *str)
{
	if(!str) return false;
	for(int i = 0; str[i]; ++i)
	{
		if(symbol==str[i])
			return true;
	}
	return false;
}



inline void convert_i(long long &result, const char *str, char ** end, int radix){ result = strtoll(str, end, radix); }
inline void convert_i(unsigned long long &result, const char *str, char ** end, int radix){ result = strtoull(str, end, radix); }
inline void convert_i(long &result, const char *str, char ** end, int radix){ result = strtol(str, end, radix); }
inline void convert_i(unsigned long &result, const char *str, char ** end, int radix){ result = strtoul(str, end, radix); }
inline void convert_i(int &result, const char *str, char ** end, int radix){ result = strtol(str, end, radix); }
inline void convert_i(unsigned int &result, const char *str, char ** end, int radix){ result = strtoul(str, end, radix); }

inline void convert_f(long double &result, const char *str, char ** end){ result = strtold(str, end); }
inline void convert_f(double &result, const char *str, char ** end){ result = strtod(str, end); }
inline void	convert_f(float &result, const char *str, char ** end){ result = strtof(str, end); }

template<class T>
const T *default_commas();

template<>
inline const char *default_commas(){ return ".,"; }

template<>
inline const wchar_t *default_commas(){ return L".,"; }

template<class IT>
inline string string_for_number_conversion(IT begin, IT end,
				const typename IT::value_type *commas,
				const typename IT::value_type *symbols_to_ignore,
				bool *error_ptr)
{
	string	buffer;

	for(auto it = begin; it < end; ++it)
	{
		typename IT::value_type c = *it;
		if(symbol_found(c, commas)) buffer.push_back('.');
		else if(!symbol_found(c, symbols_to_ignore))
		{
			if(!in_range(c, 0, 127))
			{
				*error_ptr = true;
				break;
			}
			buffer.push_back(c);
		}
	}

	return buffer;
}


//!	\brief Преобразование строки в целое число.
//! \param begin итератор, указывающий на начало строкового представления целого числа. Допустимое представление ddd, +ddd, -ddd
//! \param end итератор, указывающий на конец строкового представления целого числа
//! \param symbols_to_ignore Символы, которые могут быть разделителями групп чисел.
//! \param error_ptr Адрес, по которому в случае неудачи пишется true. Иначе остается неизменным.
//! \return Преобразованное число. В случае неудачи 0. Неудачу можно определить по содержимому error_ptr

// 	Вызвана следующими проблемами в стандартных функциях:
// 	1. stoi выдает исключение, которое сильно снижает производительность.
// 	2. strtoi требует проверки errno, что тоже не всегда удобно.

template<class RESULT_T = long long, class IT>
inline RESULT_T	string_to_integral(IT begin, IT end,
						const typename IT::value_type *symbols_to_ignore = nullptr,
						bool *error_ptr = nullptr)
{
	bool	error = false;
	string	buffer = string_for_number_conversion(begin, end, nullptr, symbols_to_ignore, &error);

	int errno_buf = errno;
	errno = 0;

	RESULT_T	result;
	char	*search_end;
	convert_i(result, buffer.c_str(), &search_end, 0);

	if(search_end != buffer.c_str() + buffer.size())
	{
		error = true;
	}
	if(error_ptr)
	{
		if(error || errno)
		{
			*error_ptr = true;
		}
	}

	errno = errno_buf;

	return result;
}

//!	\brief Преобразование строки в число с плавающей запятой.
//! \param begin итератор, указывающий на начало строкового представления целого числа. Допустимое представление ddd, +ddd, -ddd
//! \param end итератор, указывающий на конец строкового представления целого числа
//! \param commas символы, которые могут использоваться в качестве десятичной точки
//! \param symbols_to_ignore Символы, которые могут быть разделителями групп чисел.
//! \param error_ptr Адрес, по которому в случае неудачи пишется true. Иначе остается неизменным.
//! \return Преобразованное число. Если не расшифровано ни одной цифры, nan. В случае прочих ошибок (переполнения и т.д) совпадает с strtold. Неудачу можно определить по содержимому error_ptr

// 	Вызвана следующими проблемами в стандартных функциях:
// 	1. stod выдает исключение, которое сильно снижает производительность.
// 	2. strtod требует проверки errno, что тоже не всегда удобно.
// 	3. Обе не учитывают возможные различия десятичных точек и разделителей групп чисел.


template<class RESULT_T/* = long double*/, class IT>
inline RESULT_T	string_to_floating(IT begin, IT end,
					const typename IT::value_type *commas = default_commas<typename IT::value_type>(),
					const typename IT::value_type *symbols_to_ignore = nullptr,
					bool *error_ptr = nullptr)
{
	bool	error;
	string	buffer = string_for_number_conversion(begin, end, commas, symbols_to_ignore, &error);


	int errno_buf = errno;
	errno = 0;

	RESULT_T	result;
	char	*search_end;
	convert_f(result, buffer.c_str(), &search_end);

	if(search_end != buffer.c_str() + buffer.size())
	{
		error = true;
	}

	if(search_end==buffer.c_str())
	{
		error = true;
		result = nan(nullptr);
	}

	if(error_ptr)
	{
		if(error || errno)
		{
			*error_ptr = true;
		}
	}

	errno = errno_buf;
	return result;
}

template<class RESULT_T/* = int64_t*/, class STRING>
inline RESULT_T	string_to_integral(STRING s,
		const typename STRING::value_type *symbols_to_ignore = nullptr,
		bool *error_ptr = nullptr)
{
	return string_to_integral<RESULT_T>(s.begin(), s.end(), symbols_to_ignore, error_ptr);
}

template<class RESULT_T/* = long double*/, class STRING>
inline RESULT_T	string_to_floating(STRING s,
		const typename STRING::value_type *commas = default_commas<typename STRING::value_type>(),
		const typename STRING::value_type *symbols_to_ignore = nullptr,
		bool *error_ptr = nullptr)
{
	return string_to_floating<RESULT_T>(s.begin(), s.end(), commas, symbols_to_ignore, error_ptr);
}



XRAD_END

#endif // XRAD__File_numbers_in_string_h
