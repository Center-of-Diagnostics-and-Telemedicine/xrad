// file Exceptions.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "Exceptions.h"
#include "String.h"
#include <ctime>
#include <cstring>

XRAD_BEGIN

//--------------------------------------------------------------
// TODO
// Все "разумные" типы исключений должны наследоваться от std::exception.
// Но стандарт не запрещает использовать в качестве исключений произвольные типы.
// При необходимости можно сюда добавить catch исключений других типов.
string GetExceptionString()
{
	try
	{
		throw;
	}
	catch (exception &e)
	{
		time_t	t;
		time(&t);
		char str_buf[1024];
		auto local_tm = localtime(&t);
		if (local_tm)
			str_buf[strftime(str_buf, sizeof(str_buf), "%F %T%z", local_tm)] = 0;
		else
			strcpy(str_buf, "???");
		return ssprintf("%s [%s at %s]", e.what(), typeid(e).name(), str_buf);
	}
	catch (...)
	{
		return """Unknown exception";
	}
}

//--------------------------------------------------------------

/*
	По поводу часового пояса в strftime(). Тест корректности работы strftime с параметром "%z"
	можно провести следующим образом:

	1. Установить в системе некоторый европейский часовой пояс с переходом на летнее время.
	Даты перехода для Берлина:
	- 2019-10-27 03:00->02:00 (для Лондона: 02:00->01:00)
	- 2020-03-29 02:00->03:00 (для Лондона: 01:00->02:00)

	2. Запустить примерно следующий код:
	tm tt;
	memset(&tt, 0, sizeof(tm));
	#if 0
	tt.tm_year = 119;
	tt.tm_mon = 9;
	tt.tm_mday = 27;
	#else
	tt.tm_year = 120;
	tt.tm_mon = 2;
	tt.tm_mday = 29;
	#endif
	tt.tm_hour = 1; // Для Лондона: 0; на 2 часа раньше времени перевода часов.
	tt.tm_min = 30;
	tt.tm_sec = 0;
	tt.tm_isdst = -1;
	time_t t = mktime(&tt);
	char str_buf[1024];
	auto local_tm = localtime(&t);
	size_t len = strftime(str_buf, sizeof(str_buf), "%F %T%z", localtime(&t));
	for (int i = 0; i < 3; ++i)
	{
		str_buf[len++] = '\n';
		t += 3600; // прибавляем 1 час; предполагаем, что time_t кодирует время в секундах
		len += strftime(str_buf + len, 100, "%F %T%z", localtime(&t));
	}

	3. Должны получить последовательность временных отметок с разницей в 1 час
	в местном часовом поясе с разными и при этом корректными смещениями относительно UTC.
	(Моменты "неоднозначного" времени при переводе часов назад функция strftime разрешает по полю
	tm_isdst.)

	MSVC2015 + Windows 10 (1607): Работает корректно.
*/

//--------------------------------------------------------------

XRAD_END
