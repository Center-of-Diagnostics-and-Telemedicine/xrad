/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2018/09/19 13:58
	\author kulberg
*/
#include "pre.h"
#include "date_utils.h"

#include <map>

XRAD_BEGIN

namespace
{
map<date_t::weekday_t, wstring>	weekdays_ru =
{
	{date_t::e_sunday, L"воскресенье"},
	{date_t::e_monday, L"понедельник"},
	{date_t::e_tuesday, L"вторник"},
	{date_t::e_wednesday, L"среда"},
	{date_t::e_thursday, L"четверг"},
	{date_t::e_friday, L"пятница"},
	{date_t::e_saturday, L"суббота"},
};

wstring	weekday_name_ru(date_t::weekday_t wd)
{
	return weekdays_ru[wd];
}

vector<date_t::weekday_t> weekdays{date_t::e_sunday, date_t::e_monday, date_t::e_tuesday, date_t::e_wednesday, date_t::e_thursday, date_t::e_friday, date_t::e_saturday};



date_t::weekday_t	xx_century_weekday0(size_t century_day)
{
	//01.01.1900 вторник. воскресенье считаем за 0
	return weekdays[(century_day+2) % 7];
}
}


date_t::date_t()
{
	m_day = 1;
	m_month = 1;
	m_year = 1900;

	m_weekday = xx_century_weekday0(0);
}

void	date_t::load(ptrdiff_t in_year, size_t in_month, size_t in_day)
{
	m_day = in_day;
	m_month = in_month;
	m_year = in_year;

	m_weekday = xx_century_weekday0(xx_century_day());
}




void date_t::load(const wstring &wdate)
{
//	в формате YYYYMMDD

	if(wdate.size() == 8)
	{
		wchar_t *end;
		m_year = wcstol(wstring(wdate.begin(), wdate.begin()+4).c_str(), &end, 10);
		m_month = wcstol(wstring(wdate.begin()+4, wdate.begin()+6).c_str(), &end, 10);
		m_day = wcstol(wstring(wdate.begin()+6, wdate.end()).c_str(), &end, 10);

		m_weekday = xx_century_weekday0(xx_century_day());
	}
	else
	{
		throw invalid_argument("invalid date");
	}
}


date_t date_from_xx_century_day(size_t day)
{
	date_t	result;
	result.load(day);
	return result;
}


date_t date_from_string(const wstring &date_string)
{
	date_t	result;
	result.load(date_string);
	return result;
}

date_t	today()
{
	time_t	t;
	time(&t);

	#pragma warning (push)
	#pragma warning (disable: 4996)
	tm *detailed_t = gmtime(&t);
	#pragma warning (pop)

	ptrdiff_t year = 1900 + detailed_t->tm_year;
	size_t month = 1 + detailed_t->tm_mon;
	size_t	day = detailed_t->tm_mday;//почему-то месяцы считает с 0, а дни с 1

	date_t	result;
	result.load(year, month, day);
	return result;
}




wstring date_t::export_wstring() const
{
	return ssprintf(L"%04i%02i%02i", m_year, m_month, m_day);
}


const vector<size_t> month_length_bissextile =	{31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const vector<size_t> month_length			=	{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


bool	check_proper_date(size_t year, size_t month, size_t day)
{
	if(!in_range(year, 1901, 2099)) return false;
	if(!in_range(month, 1, 12)) return false;
	if(year%4)
	{
		if(!in_range(day, 1, month_length[month-1])) return false;
	}
	else
	{
		if(!in_range(day, 1, month_length_bissextile[month-1])) return false;
	}
	return true;
}





constexpr	size_t quad_days(){ return 365*3 + 366; }

size_t month_last_day_bissextile(size_t month_no)
{
	static vector<size_t> mb (month_length_bissextile);

	do_once
	{
		for(size_t i = 1; i < mb.size(); ++i)
		{
			mb[i] += mb[i-1];
		}
	}
	return mb[month_no];
}

size_t &month_last_day(size_t month_no)
{
	static vector<size_t> m(month_length);

	do_once
	{
		for(size_t i = 1; i < m.size(); ++i)
		{
			m[i] += m[i-1];
		}
	}
	return m[month_no];
}



void	date_t::load(size_t xx_century_day)
{
	//счет дней и годов начинается с 0, перевод в календарные +1 в самом конце
	size_t	quad_start_year = 4*(xx_century_day/quad_days());
	size_t	day_in_quad = xx_century_day%quad_days();
	size_t	year_in_quad = day_in_quad / 365;
	size_t	day_in_year = day_in_quad%365;

	size_t year0 = quad_start_year + year_in_quad;
	size_t month0(0);
	size_t day0(day_in_year);

	if(year_in_quad==3)
	{
		while(day_in_year/month_last_day_bissextile(month0))
		{
			day0 = day_in_year%month_last_day_bissextile(month0++);
		}
	}
	else
	{
		while(day_in_year/month_last_day(month0))
		{
			day0 = day_in_year%month_last_day(month0++);
		}
	}


	m_year = 1901 + year0;
	m_month = 1 + month0;
	m_day = 1 + day0;
	m_weekday = xx_century_weekday0(xx_century_day);
}




size_t	xx_century_day_util(size_t year, size_t month, size_t day)
{
	if(!check_proper_date(year, month, day))
	{
		throw invalid_argument("invalid date");
	}

	size_t	result(0);

	size_t	quad_no = (year - 1901)/4;
	result += quad_no*quad_days();

	size_t	year_in_quad = (year - 1901)%4;
	result += year_in_quad*365;

	if(month>=2)
	{
		if((year_in_quad+1)%4)
		{
			result += month_last_day(month-2);
		}
		else
		{
			result += month_last_day_bissextile(month-2);
		}
	}

	result += day - 1;

	return result;
}

size_t	xx_century_weekday0(const size_t year, size_t month, size_t day)
{
	return xx_century_weekday0(xx_century_day_util(year, month, day));
}

size_t	date_t::xx_century_day() const
{
	return xx_century_day_util(m_year, m_month, m_day);
}

ptrdiff_t years_age(const date_t &date1, const date_t &date2)
{
	date_t d1, d2;
	int	sign;

	if(date1.xx_century_day() < date2.xx_century_day())
	{
		d1 = date1;
		d2 = date2;
		sign = 1;
	}
	else
	{
		d2 = date1;
		d1 = date2;
		sign = -1;
	}

	ptrdiff_t	result = d2.year() - d1.year();
	if(d2.month() < d1.month())
	{
		--result;
	}
	else
	{
		if(d2.month() == d1.month() && d2.day() < d1.day())
		{
			--result;
		}
	}
	return result*sign;
}



XRAD_END
