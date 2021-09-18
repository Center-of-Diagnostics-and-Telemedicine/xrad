/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2017-10-24 17:55
	\author kulberg
*/
#include "pre.h"
#include "QtStringConverters.h"
#include <XRADGUI/Sources/Core/GUIConfig.h>

#include <QtCore/QChar>

XRAD_BEGIN

//--------------------------------------------------------------

#if defined(XRAD_USE_MS_VERSION)

//--------------------------------------------------------------

QString	wstring_to_qstring(const wstring &in_wstring)
{
	return QString::fromUtf16(reinterpret_cast<const char16_t *>(in_wstring.c_str()), int(in_wstring.length()));
}

QString	string_to_qstring(const string &in_string)
{
#if 0
	// если включена прагма про utf8 в обычных строках C
	return QString::fromUtf8(in_string.c_str(), in_string.length());
#else
	wstring	ws = string_to_wstring(in_string, e_decode_literals);
	return wstring_to_qstring(ws);
	//	return QObject::tr(in_string.c_str());
#endif
}

wstring	qstring_to_wstring(const QString &qstr)
{
	return qstr.toStdWString();
	// Замечание: ранее был написан комментарий, что в Qt5 не работает как надо toStdWString().
	// На данный момент недостатков в работе toStdWString() не замечено. [АБЕ 2019-01-30]
}

string	qstring_to_string(const QString &qstr)
{
	wstring	wresult = qstring_to_wstring(qstr);
	return	wstring_to_string(wresult, e_encode_literals);
}

//--------------------------------------------------------------

#elif defined(XRAD_USE_CHAR_UTF8_WCHAR_UTF32_VERSION)

//--------------------------------------------------------------

QString	wstring_to_qstring(const wstring &in_wstring)
{
	return string_to_qstring(convert_to_string(in_wstring));
}

QString	string_to_qstring(const string &in_string)
{
	return QString::fromUtf8(in_string.c_str(), in_string.length());
}

wstring	qstring_to_wstring(const QString &qstr)
{
	return convert_to_wstring(qstring_to_string(qstr));
}

string	qstring_to_string(const QString &qstr)
{
	return qstr.toStdString(); // вызывает QString::toUtf8()
}

//--------------------------------------------------------------

#else
#error Unsupported string encoding mode.
#endif

//--------------------------------------------------------------

vector<QString> string_list_to_qstring_list(const vector<string> &in_list)
{
	vector<QString> result(in_list.size());
	for(size_t i = 0; i < in_list.size(); ++i)
	{
		result[i] = string_to_qstring(in_list[i]);
	}
	return result;
}

vector<QString> wstring_list_to_qstring_list(const vector<wstring> &in_wlist)
{
	vector<QString> result(in_wlist.size());
	for(size_t i = 0; i < in_wlist.size(); ++i)
	{
		result[i] = wstring_to_qstring(in_wlist[i]);
	}
	return result;
}

//--------------------------------------------------------------

XRAD_END
