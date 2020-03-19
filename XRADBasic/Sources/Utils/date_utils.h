#ifndef XRAD__File_date_utils_h
#define XRAD__File_date_utils_h
/*!
	\file
	\date 2018/09/19 13:58
	\author kulberg
*/

#include <XRADBasic/Core.h>

XRAD_BEGIN



class date_t
{
public:
	enum weekday_t
	{
		e_sunday=1, e_monday, e_tuesday, e_wednesday, e_thursday, e_friday, e_saturday//доделать потом
	};
private:
	ptrdiff_t	m_year;
	size_t m_month;
	size_t m_day;
	weekday_t m_weekday;

public:

	// Единственный конструктор устанавливает дату на 01.01.1901. Во всех остальных ситуациях возможны исключения
	date_t();

	void load(ptrdiff_t in_year, size_t in_month, size_t in_day);
	void load(const wstring &d);
	void load(size_t xx_century_day);

	size_t	year() const { return m_year; }
	size_t	month() const { return m_month; }
	size_t	day() const { return m_day; }
	weekday_t	weekday() const { return m_weekday; }

	wstring export_wstring() const;
	size_t	xx_century_day() const;//день начиная с 1 января 1901 г.

};

wstring	weekday_name_ru(date_t::weekday_t);
ptrdiff_t years_age(const date_t &d1, const date_t &d2);

date_t	date_from_string(const wstring &date_string);
date_t	date_from_xx_century_day(size_t day);
date_t	today();

XRAD_END

#endif // XRAD__File_date_utils_h
