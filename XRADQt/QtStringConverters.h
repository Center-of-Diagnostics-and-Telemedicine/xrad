#ifndef XRAD__File_QtStringConverters_h
#define XRAD__File_QtStringConverters_h
/*!
	\file
	\date 2017/10/24 17:47
	\author kulberg
*/

#include <XRADBasic/Core.h>
#include <string>
#include <QtCore/QString>

XRAD_BEGIN

QString	wstring_to_qstring(const wstring &in_wstring);
QString	string_to_qstring(const string &in_string);
wstring	qstring_to_wstring(const QString &qstr);
string	qstring_to_string(const QString &qstr);

// то же между списками строк; только в одну сторону (другая, видимо, будет не нужна)
vector<QString> string_list_to_qstring_list(const vector<string> &in_list);
vector<QString> wstring_list_to_qstring_list(const vector<wstring> &in_wlist);

XRAD_END

#endif // XRAD__File_QtStringConverters_h
