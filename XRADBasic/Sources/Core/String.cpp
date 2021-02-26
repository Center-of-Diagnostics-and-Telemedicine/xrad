/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file String.cpp
//	Created by ACS on 06.05.03
//--------------------------------------------------------------
#include "pre.h"

#include "String.h"
#include <XRADBasic/Containers.h>
#include <cstdarg>
#include <mutex>

//--------------------------------------------------------------

XRAD_BEGIN



//--------------------------------------------------------------
//
//	ssprintf, vssprintf, версии 8-bit и Unicode.
//	название одно, вариант написания wssprintf кажется громоздким
//
//--------------------------------------------------------------



ustring vssprintf(const uchar_t *format, va_list param)
{
	va_list param1;
	va_copy(param1, param);
	size_t buffer_size = size_t(vsnprintf(0, 0, uchar_t::pointer_to_char(format), param1));
	va_end(param1);
	DataArray<char> buffer(buffer_size+2, 0);

	size_t written = size_t(vsnprintf(buffer.data(), buffer_size+2,
			uchar_t::pointer_to_char(format), param));
	ustring result(buffer.data(), buffer.data()+written);
	return result;
}
//	printf в utf-8 строках не работает, хотя, кажется, должен бы.

string vssprintf(const char *format, va_list param)
{
	va_list param1;
	va_copy(param1, param);
	size_t buffer_size = size_t(vsnprintf(0, 0, format, param1));
	va_end(param1);
	DataArray<char> buffer(buffer_size+2, 0);

	size_t written = size_t(vsnprintf(buffer.data(), buffer_size+2, format, param));
	string result(buffer.data(), written);
	return result;
}

wstring vssprintf(const wchar_t *format, va_list param)
{
#if !defined(XRAD_USE_PREFER_STD_VERSION) && defined(XRAD_USE_MS_VERSION)
	va_list param1;
	va_copy(param1, param);
	size_t buffer_size = size_t(_vsnwprintf(0, 0, format, param1));
	va_end(param1);
	DataArray<wchar_t> buffer(buffer_size+2, 0);

	size_t written = size_t(_vsnwprintf(buffer.data(), buffer_size+2, format, param));
	wstring result(buffer.data(), written);
	return result;
#else
	size_t buffer_size = wcslen(format);
	DataArray<wchar_t> buffer(buffer_size + 2, 0);
	size_t written = 0;
	for (;;)
	{
		va_list param1;
		va_copy(param1, param);
		auto n = vswprintf(buffer.data(), buffer_size + 1, format, param1);
		va_end(param1);
		if (n >= 0)
		{
			written = (size_t)n;
			break;
		}
		buffer_size *= 2;
		buffer.realloc(buffer_size + 2);
		continue;
	}
	wstring result(buffer.data(), written);
	return result;
#endif
}

//
//--------------------------------------------------------------

string ssprintf_core(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	string result = vssprintf(format, args);
	va_end(args);
	return result;
}

wstring ssprintf_core(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	wstring result = vssprintf(format, args);
	va_end(args);
	return result;
}

ustring ssprintf_core(const uchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	ustring result = vssprintf(format, args);
	va_end(args);
	return result;
}
//	printf в utf-8 строках не работает, хотя, кажется, должен бы.

template<> string linefeed<string>(){ return "\n"; }
template<> wstring linefeed<wstring>(){ return L"\n"; }
template<> string tab<string>(){ return "\t"; }
template<> wstring tab<wstring>(){ return L"\t"; }

//--------------------------------------------------------------

vector<string> split(const string &str, char delimiter)
{
	vector<string> result;
	if (!str.length())
		return result;
	for (size_t pos = 0;;)
	{
		size_t delim_pos = str.find(delimiter, pos);
		if (delim_pos == str.npos)
		{
			result.push_back(str.substr(pos));
			break;
		}
		result.push_back(str.substr(pos, delim_pos - pos));
		pos = delim_pos + 1;
	}
	return result;
}

vector<wstring> split(const wstring &str, wchar_t delimiter)
{
	vector<wstring> result;
	if (!str.length())
		return result;
	for (size_t pos = 0;;)
	{
		size_t delim_pos = str.find(delimiter, pos);
		if (delim_pos == str.npos)
		{
			result.push_back(str.substr(pos));
			break;
		}
		result.push_back(str.substr(pos, delim_pos - pos));
		pos = delim_pos + 1;
	}
	return result;
}

string merge(const vector<string> &strings, const string &delimiter)
{
	if (!strings.size())
		return string();
	string result = strings[0];
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delimiter + strings[i];
	}
	return result;
}

wstring merge(const vector<wstring> &strings, const wstring &delimiter)
{
	if (!strings.size())
		return wstring();
	wstring result = strings[0];
	for (size_t i = 1; i < strings.size(); ++i)
	{
		result += delimiter + strings[i];
	}
	return result;
}

//--------------------------------------------------------------

XRAD_END
