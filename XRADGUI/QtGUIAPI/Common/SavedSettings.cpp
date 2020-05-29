#include "pre.h"
#include "SavedSettings.h"
/*!
	\file
	\date 2018/03/16 11:42
	\author kulberg

	\brief
*/

#include <XRADQt/QtStringConverters.h>

namespace XRAD_GUI
{

XRAD_USING;



template <>
void GUISaveParameter(QString function_name, QString setting_name, const QString &value)
{
	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);

	settings.beginGroup(function_name);
	settings.setValue(setting_name, value);
	settings.endGroup();
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const wstring &value)
{
	GUISaveParameter(function_name, setting_name, wstring_to_qstring(value));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const string &value)
{
	GUISaveParameter(function_name, setting_name, string_to_qstring(value));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const bool &value)
{
	GUISaveParameter<int>(function_name, setting_name, value? 1: 0);
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned int &value)
{
	string value_str = ssprintf("%u", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned long &value)
{
	string value_str = ssprintf("%lu", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned long long &value)
{
	string value_str = ssprintf("%llu", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const int &value)
{
	string value_str = ssprintf("%i", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const long &value)
{
	string value_str = ssprintf("%li", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const long long &value)
{
	string value_str = ssprintf("%lli", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const double &value)
{
	string value_str = ssprintf("%.16le", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}

template <>
void GUISaveParameter(QString function_name, QString setting_name, const float &value)
{
	string value_str = ssprintf("%.7e", value);
	GUISaveParameter(function_name, setting_name, string_to_qstring(value_str));
}



template <>
QString GUILoadParameter(QString function_name, QString setting_name, const QString &default_value,
		bool *loaded)
{
	try
	{
		QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
		QSettings settings("XRAD", appname);

		settings.beginGroup(function_name);
		QVariant value = settings.value(setting_name);
		settings.endGroup();
		if (!value.isValid() || value.isNull())
			return default_value;
		if (loaded)
			*loaded = true;
		return value.toString();
	}
	catch (...)
	{
		return default_value;
	}
}

template <>
wstring GUILoadParameter(QString function_name, QString setting_name, const wstring &default_value,
		bool *loaded)
{
	return qstring_to_wstring(GUILoadParameter<QString>(function_name, setting_name,
			wstring_to_qstring(default_value), loaded));
}

template <>
string GUILoadParameter(QString function_name, QString setting_name, const string &default_value,
		bool *loaded)
{
	return qstring_to_string(GUILoadParameter<QString>(function_name, setting_name,
			string_to_qstring(default_value), loaded));
}

template <>
bool GUILoadParameter(QString function_name, QString setting_name,
		const bool &default_value, bool *loaded)
{
	bool local_loaded = false;
	auto value_i = GUILoadParameter<int>(function_name, setting_name, default_value, &local_loaded);
	if (value_i == 0)
	{
		if (local_loaded && loaded)
			*loaded = true;
		return false;
	}
	if (value_i == 1)
	{
		if (local_loaded && loaded)
			*loaded = true;
		return true;
	}
	return default_value;
}

template <>
unsigned int GUILoadParameter(QString function_name, QString setting_name,
		const unsigned int &default_value, bool *loaded)
{
	bool local_loaded = false;
	auto value_ll = GUILoadParameter<unsigned long long>(function_name, setting_name, default_value,
			&local_loaded);
	if (value_ll > numeric_limits<unsigned int>::max())
		return default_value;
	if (local_loaded && loaded)
		*loaded = true;
	return (unsigned int)value_ll;
}

template <>
unsigned long GUILoadParameter(QString function_name, QString setting_name,
		const unsigned long &default_value, bool *loaded)
{
	bool local_loaded = false;
	auto value_ll = GUILoadParameter<unsigned long long>(function_name, setting_name, default_value,
			&local_loaded);
	if (value_ll > numeric_limits<unsigned long>::max())
		return default_value;
	if (local_loaded && loaded)
		*loaded = true;
	return (unsigned long)value_ll;
}

template <>
unsigned long long GUILoadParameter(QString function_name, QString setting_name,
		const unsigned long long &default_value, bool *loaded)
{
	QString value_str = GUILoadParameter<QString>(function_name, setting_name, QString());
	if (value_str.isEmpty())
		return default_value;
	auto bytes_utf8 = value_str.toUtf8();
	const char *data = bytes_utf8.data();
	char *end = nullptr;
	// Читаем беззнаковое целое максимальной разрядности.
	unsigned long long value = strtoull(data, &end, 0);
	if (end != data + bytes_utf8.size())
		return default_value;
	if (loaded)
		*loaded = true;
	return value;
}

template <>
int GUILoadParameter(QString function_name, QString setting_name, const int &default_value,
		bool *loaded)
{
	bool local_loaded = false;
	auto value_ll = GUILoadParameter<long long>(function_name, setting_name, default_value,
			&local_loaded);
	if (value_ll < numeric_limits<int>::min() || value_ll > numeric_limits<int>::max())
		return default_value;
	if (local_loaded && loaded)
		*loaded = true;
	return (int)value_ll;
}

template <>
long GUILoadParameter(QString function_name, QString setting_name, const long &default_value,
		bool *loaded)
{
	bool local_loaded = false;
	auto value_ll = GUILoadParameter<long long>(function_name, setting_name, default_value,
			&local_loaded);
	if (value_ll < numeric_limits<long>::min() || value_ll > numeric_limits<long>::max())
		return default_value;
	if (local_loaded && loaded)
		*loaded = true;
	return (long)value_ll;
}

template <>
long long GUILoadParameter(QString function_name, QString setting_name,
		const long long &default_value, bool *loaded)
{
	QString value_str = GUILoadParameter<QString>(function_name, setting_name, QString());
	if (value_str.isEmpty())
		return default_value;
	auto bytes_utf8 = value_str.toUtf8();
	const char *data = bytes_utf8.data();
	char *end = nullptr;
	// Читаем знаковое целое максимальной разрядности.
	long long value = strtoll(data, &end, 0);
	if (end != data + bytes_utf8.size())
		return default_value;
	if (loaded)
		*loaded = true;
	return value;
}

template <>
double GUILoadParameter(QString function_name, QString setting_name, const double &default_value,
		bool *loaded)
{
	QString value_str = GUILoadParameter<QString>(function_name, setting_name, QString());
	if (value_str.isEmpty())
		return default_value;
	auto bytes_utf8 = value_str.toUtf8();
	const char *data = bytes_utf8.data();
	char *end = nullptr;
	double value = strtod(data, &end);
	if (end != data + bytes_utf8.size())
		return default_value;
	if (loaded)
		*loaded = true;
	return value;
}

template <>
float GUILoadParameter(QString function_name, QString setting_name, const float &default_value,
		bool *loaded)
{
	bool local_loaded = false;
	auto value_ll = GUILoadParameter<double>(function_name, setting_name, default_value,
			&local_loaded);
	if (value_ll < numeric_limits<float>::lowest() || value_ll > numeric_limits<float>::max())
		return default_value;
	if (local_loaded && loaded)
		*loaded = true;
	return (float)value_ll;
}

QString PathParamName(file_dialog_mode flag)
{
	switch(flag)
	{
		case file_open_dialog:
			return "LastOpenPath";
		case file_save_dialog:
			return "LastSavePath";
		default:
			throw invalid_argument("SavePathParamName(file_dialog_mode flag), invalid flag");
	}
}

void SaveDefaultPath(file_dialog_mode flag, QString path)
{
	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);

	settings.beginGroup("GetFileName");
	settings.setValue(PathParamName(flag), path);
	settings.endGroup();

	QDir::setCurrent(path);
}

QString GetDefaultPath(file_dialog_mode flag)
{
	QString	appname = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
	QSettings settings("XRAD", appname);
	settings.beginGroup("GetFileName");
	QString	currentPathPrevious = settings.value("currentPath", QCoreApplication::applicationDirPath()).toString();//для обратной совместимости с прежними записями в реестре
	QString currentPath = settings.value(PathParamName(flag), currentPathPrevious).toString();
	settings.endGroup();

	QDir::setCurrent(currentPath);

	return currentPath;
}



} //namespace XRAD_GUI
