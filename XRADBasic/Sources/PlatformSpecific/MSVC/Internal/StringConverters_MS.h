/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef StringConverters_MS_h__
#define StringConverters_MS_h__
/*!
	\file
	\date 2017-10-24 17:26
	\author kulberg
*/

// Этот файл не должен использовать весь <Core.h> во избежание циклических зависимостей.
// Включаем только минимально необходимый набор заголовочных файлов.
#include <XRADBasic/Sources/Core/Config.h>
#include <XRADBasic/Sources/Core/BasicMacros.h>

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

//--------------------------------------------------------------

#include <string>

XRAD_BEGIN

//--------------------------------------------------------------

enum class CodePage
{
	cp_system = 0, //!< Системная кодовая страница (CP_ACP)
	cp_1251 = 1251, //!< Русская, для тестов однобайтовых кодовых страниц
	cp_54936 = 54936 //!< Китайская GB18030, для тестов многобайтовых кодовых страниц
};

//! \brief Получить глобальную кодовую страницу (по умолчанию это системная кодовая страница)
CodePage get_code_page_MS();

//! \brief Установить глобальную кодовую страницу, для тестов
void set_code_page_MS(CodePage code_page);

/*!
	\brief Преобразование строки из заданной кодовой страницы в UTF-16 (wchar_t) в MS Windows

	\note
	Замечание. В силу особенности WinAPI задание пользовательского default_char представляет
	серьезную техническую проблему и не реализовано. В качестве default_char здесь выступает '?', впрочем,
	возможно, это зависит от настроек Windows.
*/
wstring string_to_wstring_MS(const string &str, CodePage code_page, bool decode_literals, bool *error = nullptr);

/*!
	\brief Преобразование строки из UTF-16 (wchar_t) в заданную кодовую страницу в MS Windows

	\param default_char Символ по умолчанию. Должен быть ASCII-символом.
		Используется для замены отсутствующих в кодовой странице string символов, если encode_literals == false,
		а также для замены недопустимых кодов UTF-16 в исходной строке.
*/
string wstring_to_string_MS(const wstring &str, CodePage code_page, bool encode_literals, int default_char, bool *error = nullptr);

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_MS_VERSION

#endif // StringConverters_MS_h__
