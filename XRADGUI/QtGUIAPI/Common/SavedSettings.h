#ifndef SavedSettings_h__
#define SavedSettings_h__
/*!
	\file
	\date 2018/03/16 11:43
	\author kulberg

	\brief  Функции сохранения последних выбранных значений в диалогах. Для каждого имени диалога запоминается свое значение.
*/
//--------------------------------------------------------------

#include "XRADGUIAPIDefs.h"
#include <XRADQt/QtStringConverters.h>

namespace XRAD_GUI
{

XRAD_USING;

//--------------------------------------------------------------



/*!
	\brief Общий шаблон функции сохранения значения в системном хранилище. Имеет только специализации

	Сохранение значений в Windows производится в реестре в ключе
	HKCU/XRAD/{application_name}/function_name/param_name.

	Сделан шаблон со специализациями, а не набор перегруженных функций, чтобы при несовпадении
	типа аргумента получить ошибку компилятора, а не преобразование типа.

	При появлении новых типов аргументов следует добавлять соответствующие специализации.
*/
template <class T>
void GUISaveParameter(QString function_name, QString setting_name, const T &value) = delete;

template <>
void GUISaveParameter(QString function_name, QString setting_name, const QString &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const wstring &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const string &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const bool &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned int &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned long &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const unsigned long long &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const int &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const long &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const long long &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const double &value);

template <>
void GUISaveParameter(QString function_name, QString setting_name, const float &value);

//! \brief Шаблон функции сохранения значения с именами опции типа wstring. Перенаправляет
//! на одноименную функцию с именами опции типа QString
template <class T>
void GUISaveParameter(const wstring &function_name, const wstring &setting_name, const T &value)
{
	GUISaveParameter(wstring_to_qstring(function_name), wstring_to_qstring(setting_name), value);
}

//--------------------------------------------------------------

/*!
	\brief Общий шаблон функции загрузки значения из системного хранилища. Имеет только специализации

	Сделан шаблон со специализациями, а не набор перегруженных функций, чтобы при несовпадении
	типа аргумента получить ошибку компилятора, а не преобразование типа.

	При появлении новых типов аргументов следует добавлять соответствующие специализации.
*/
template <class T>
T GUILoadParameter(QString function_name, QString setting_name, const T &default_value,
		bool *loaded = nullptr) = delete;

template <>
QString GUILoadParameter(QString function_name, QString setting_name, const QString &default_value,
		bool *loaded);

template <>
wstring GUILoadParameter(QString function_name, QString setting_name, const wstring &default_value,
		bool *loaded);

template <>
string GUILoadParameter(QString function_name, QString setting_name, const string &default_value,
		bool *loaded);

template <>
bool GUILoadParameter(QString function_name, QString setting_name, const bool &default_value,
		bool *loaded);

template <>
unsigned int GUILoadParameter(QString function_name, QString setting_name,
		const unsigned int &default_value,
		bool *loaded);

template <>
unsigned long GUILoadParameter(QString function_name, QString setting_name,
		const unsigned long &default_value,
		bool *loaded);

template <>
unsigned long long GUILoadParameter(QString function_name, QString setting_name,
		const unsigned long long &default_value,
		bool *loaded);

template <>
int GUILoadParameter(QString function_name, QString setting_name, const int &default_value,
		bool *loaded);

template <>
long GUILoadParameter(QString function_name, QString setting_name,
		const long &default_value,
		bool *loaded);

template <>
long long GUILoadParameter(QString function_name, QString setting_name,
		const long long &default_value,
		bool *loaded);

template <>
double GUILoadParameter(QString function_name, QString setting_name, const double &default_value,
		bool *loaded);

template <>
float GUILoadParameter(QString function_name, QString setting_name, const float &default_value,
		bool *loaded);

//! \brief Шаблон функции загрузки значения с именами опции типа wstring. Перенаправляет
//! на одноименную функцию с именами опции типа QString
template <class T>
T GUILoadParameter(const wstring &function_name, const wstring &setting_name,
		const T &default_value, bool *loaded = nullptr)
{
	return GUILoadParameter(wstring_to_qstring(function_name), wstring_to_qstring(setting_name),
			default_value, loaded);
}

//--------------------------------------------------------------

QString PathParamName(file_dialog_mode flag);
void	SaveDefaultPath(file_dialog_mode flag, QString path);
QString GetDefaultPath(file_dialog_mode flag);

//--------------------------------------------------------------

} //namespace XRAD_GUI

#endif // SavedSettings_h__
