/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file ConsoleMain.cpp
//--------------------------------------------------------------
#include "pre.h"
#include <XRADBasic/Core.h>
#include <XRADSystem/System.h>
#include <windows.h>
#include <vector>
#include <string>

using namespace std;
XRAD_USING

//#define XRAD__LANG_DEBUG

//--------------------------------------------------------------

namespace
{
const char utf8bom[] = u8"\uFEFF";
constexpr size_t utf8bom_size = sizeof(utf8bom) - 1;
} // namespace

int wmain(int argn, wchar_t **args)
{
	ThreadSetup ts; (void)ts;
	try
	{
		// Установить кодовую страницу для консоли в ANSI для корректной работы printf, scanf.
		CPINFOEXW cp_info;
		memset(&cp_info, 0, sizeof(cp_info));
		GetCPInfoExW(CP_ACP, 0, &cp_info);
		SetConsoleOutputCP(cp_info.CodePage);
		SetConsoleCP(cp_info.CodePage);

		{
			// Получить идентификатор языка в формате ISO 639-1.
			// Пока поддерживаем только "en" и "ru", поэтому такого идентификатора достаточно.
			// См. также:
			// - LOCALE_SNAME, GetUserDefaultLocaleName(), Locale Names
			// - RFC 5646 (https://tools.ietf.org/html/rfc5646)
			constexpr size_t lang_id_max_size = LOCALE_NAME_MAX_LENGTH;
			wchar_t lang_id_buf[lang_id_max_size + 1];
			memset(lang_id_buf, 0, (lang_id_max_size + 1)*sizeof(*lang_id_buf));
			GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_SISO639LANGNAME,
					lang_id_buf, lang_id_max_size);
			if (*lang_id_buf)
			{
				SetLanguageId(convert_to_string(lang_id_buf));
#ifdef XRAD__LANG_DEBUG
				printf("System LangId: \"%s\"\n",
						EnsureType<const char*>(convert_to_string(lang_id_buf).c_str()));
#endif
			}
		}

		// Установить язык по умолчанию из файла *.*.lang.
		FILE *lang_fs = _wfopen((wstring(args[0]) + L".lang").c_str(), L"rb");
		if (lang_fs)
		{
			// Читаем файл.
			constexpr size_t lang_id_max_size = 100;
			char lang_id_buf[lang_id_max_size + 1];
			memset(lang_id_buf, 0, lang_id_max_size + 1);
			fread(lang_id_buf, 1, lang_id_max_size, lang_fs);
			fclose(lang_fs);
			// Убираем опциональный UTF-8 BOM.
			char *lang_id = lang_id_buf;
			if (!strncmp(lang_id, utf8bom, utf8bom_size))
				lang_id += utf8bom_size;
			// Обрезаем пробельные символы.
			const char *delimiters = " \t\r\n";
			while (*lang_id)
			{
				size_t lang_len = strcspn(lang_id, delimiters);
				if (lang_len)
				{
					lang_id[lang_len] = 0;
					break;
				}
				++lang_id;
			}
			// Устанавливаем язык.
			SetLanguageId(lang_id);
#ifdef XRAD__LANG_DEBUG
			printf("Manifest LangId: \"%s\"\n", EnsureType<char*>(lang_id));
#endif
		}
#ifdef XRAD__LANG_DEBUG
		printf("Lang: %s\n", EnsureType<const char*>(convert_to_string(
				tr_ru_en(L"Русский", L"English")).c_str()));
#endif

		// Перекодировать аргументы в char* с учетом Юникода.
		vector<vector<char>> args_c_s;
		args_c_s.reserve(argn);
		vector<char*> args_c;
		args_c.reserve(argn);
		for (int i = 0; i < argn; ++i)
		{
			auto s = convert_to_string(wstring(args[i]));
			args_c_s.push_back(vector<char>(s.c_str(), s.c_str() + s.size() + 1));
			args_c.push_back(args_c_s.back().data());
		}

		SetApplicationArg0(args[0]);

		// Вызвать xrad_main.
		return xrad_main(argn, args_c.data());
	}
	catch(...)
	{
		printf("Unhandled exception in main.\n%s\n",
				EnsureType<const char*>(GetExceptionString().c_str()));
		return 3;
	}
}

//--------------------------------------------------------------
