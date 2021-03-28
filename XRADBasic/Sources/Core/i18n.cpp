/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file i18n.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "i18n.h"

#include "StringEncode.h"
#include <mutex>

//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

//--------------------------------------------------------------

/*!
	\brief Вернуть код ISO 639-1 в нижнем регистре

	Используется для оптимизации сравнения идентификаторов языка:
	сейчас используется только код языка.

	На вход принимает строки типа "en-us", "mn-Cyrl-MN".
	Возвращает подстроку до первого "-" в нижнем регистре.

	См. также:
	- RFC 5646 (https://tools.ietf.org/html/rfc5646)
	- RFC 4647 (https://tools.ietf.org/html/rfc4647)
*/
string SimplifyLanguageId(const string &id);

//--------------------------------------------------------------

string LanguageId;
string LanguageIdSimplified;
string DefaultLanguageId = GetStdLanguageIdEn();
string DefaultLanguageIdSimplified = SimplifyLanguageId(GetStdLanguageIdEn());
//! \brief Мьютекс, защищающий доступ к LanguageId, DefaultLanguageId, *Simplified
std::mutex LanguageIdMutex;

//--------------------------------------------------------------

string SimplifyLanguageId(const string &id)
{
	auto pos = id.find('-');
	string lang_code;
	if (pos != id.npos)
		lang_code = id.substr(0, pos);
	else
		lang_code = id;
	return get_lower(lang_code);
}

//--------------------------------------------------------------

string GetLanguageIdSimplified()
{
	std::unique_lock<std::mutex> ll(LanguageIdMutex);
	if (!LanguageIdSimplified.empty())
		return LanguageIdSimplified;
	return DefaultLanguageIdSimplified;
}

//--------------------------------------------------------------

} // namespace

//--------------------------------------------------------------

string GetLanguageId()
{
	std::unique_lock<std::mutex> ll(LanguageIdMutex);
	if (!LanguageId.empty())
		return LanguageId;
	return DefaultLanguageId;
}

//--------------------------------------------------------------

void SetLanguageId(const string &id)
{
	std::unique_lock<std::mutex> ll(LanguageIdMutex);
	LanguageId = id;
	LanguageIdSimplified = SimplifyLanguageId(id);
}

//--------------------------------------------------------------

void SetDefaultLanguageId(const string &id)
{
	if (id.empty())
		throw invalid_argument("SetDefaultLanguageId(): Language id must not be empty.");
	std::unique_lock<std::mutex> ll(LanguageIdMutex);
	DefaultLanguageId = id;
	DefaultLanguageIdSimplified = SimplifyLanguageId(id);
}

//--------------------------------------------------------------

/*!
	- Если текущий язык "ru-*", возвращет русский текст.
	- Для любого другого языка возвращает английский текст.
*/
string tr_ru_en(const char *text_ru, const char *text_en)
{
	string lang_id = GetLanguageIdSimplified();
	if (lang_id == GetStdLanguageIdRu())
		return text_ru;
	return text_en;
}

//--------------------------------------------------------------

/*!
	- Если текущий язык "ru-*", возвращет русский текст.
	- Для любого другого языка возвращает английский текст.
*/
wstring tr_ru_en(const wchar_t *text_ru, const wchar_t *text_en)
{
	string lang_id = GetLanguageIdSimplified();
	if (lang_id == GetStdLanguageIdRu())
		return text_ru;
	return text_en;
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
