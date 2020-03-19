/*!
	\file
	\date 2017-10-24 17:55
	\author kulberg
*/
#include "pre_GUI.h"
#include "StringConverters_Qt.h"
#include <QtCore/QChar>

XRAD_BEGIN



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

XRAD_END
