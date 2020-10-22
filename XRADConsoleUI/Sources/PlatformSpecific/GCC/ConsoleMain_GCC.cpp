//--------------------------------------------------------------
#include "pre.h"
#include <XRADBasic/Core.h>
#include <XRADSystem/System.h>
#include <string>
#include <cstring>

using namespace std;
XRAD_USING

//#define XRAD__CONSOLE_MAIN_LANG_DEBUG

//--------------------------------------------------------------

namespace
{
const char utf8bom[] = u8"\uFEFF";
constexpr size_t utf8bom_size = sizeof(utf8bom) - 1;
} // namespace

int main(int argn, char **args)
{
	ThreadSetup ts; (void)ts;
	try
	{
		{
			// TODO: Получить идентификатор языка в формате ISO 639-1.
			// Пока поддерживаем только "en" и "ru".
			// См. также:
			// - RFC 5646 (https://tools.ietf.org/html/rfc5646)
#ifdef XRAD__CONSOLE_MAIN_LANG_DEBUG
			const char *loc_name = setlocale(LC_ALL, nullptr);
			printf("System LangId: \"%s\"\n",
					EnsureType<const char*>(loc_name));
#endif
		}

		// Установить язык по умолчанию из файла *.*.lang.
		FILE *lang_fs = xrad_fopen((string(args[0]) + ".lang").c_str(), "rb");
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
#ifdef XRAD__CONSOLE_MAIN_LANG_DEBUG
			printf("Manifest LangId: \"%s\"\n", EnsureType<char*>(lang_id));
#endif
		}
#ifdef XRAD__CONSOLE_MAIN_LANG_DEBUG
		printf("Lang: %s\n", EnsureType<const char*>(convert_to_string(
				tr_ru_en(L"Русский", L"English")).c_str()));
#endif

		SetApplicationArg0(convert_to_wstring(args[0]));

		// Вызвать xrad_main.
		return xrad_main(argn, args);
	}
	catch(...)
	{
		printf("Unhandled exception in main.\n%s\n",
				EnsureType<const char*>(GetExceptionString().c_str()));
		return 3;
	}
}

//--------------------------------------------------------------
