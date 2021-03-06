﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file i18n.h
//--------------------------------------------------------------
#ifndef XRAD__i18n_h
#define XRAD__i18n_h
/*!
	\file
	\brief Поддержка интернационализации: переключение языков, загрузка локализованного текста
*/
//--------------------------------------------------------------

#include "Config.h"
#include "BasicMacros.h"
#include <string>

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Получить текущий идентификатор языка

	Изначально возвращает значение по умолчанию для языка по умолчанию.

	\sa SetLanguageId(), SetDefaultLanguageId().
*/
string GetLanguageId();

//! \brief Получить идентификатор "en" (в нижнем регистре)
constexpr const char *GetStdLanguageIdEn() { return "en"; }

//! \brief Получить идентификатор "ru" (в нижнем регистре)
constexpr const char *GetStdLanguageIdRu() { return "ru"; }

/*!
	\brief Установить текущий идентификатор языка (настройка не сохраняется)

	При вызове с id="" устанавливает текущий язык равным языку по умолчанию.

	Формат идентификатора: в соответствии с RFC 5646 (IETF BCP 47).

	\sa GetLanguageId(), SetDefaultLanguageId(), SaveLanguageId().
*/
void SetLanguageId(const string &id);

//! \brief Установить идентификатор языка по умолчанию (должен быть не пустым)
void SetDefaultLanguageId(const string &id);

/*!
	\brief Получить локализованную строку

	Берет на вход русский и английский текст. Возвращает текст, соответствующий текущему языку.
	Ожидается, что аргументы являются константами, а не генерируются динамически.

	Функция берет на вход принципиально константы char*, а не string, поскольку концептуально
	она предназначена для получения строки из некоторого фиксированного набора строк.
	На выходе string, а не const char* для совместимости на будущее.
*/
string tr_ru_en(const char *text_ru, const char *text_en);

/*!
	\brief Получить локализованную строку

	Берет на вход русский и английский текст. Возвращает текст, соответствующий текущему языку.
	Ожидается, что аргументы являются константами, а не генерируются динамически.

	Функция берет на вход принципиально константы wchar_t*, а не wstring, поскольку концептуально
	она предназначена для получения строки из некоторого фиксированного набора строк.
	На выходе wstring, а не const wchar_t* для совместимости на будущее.
*/
wstring tr_ru_en(const wchar_t *text_ru, const wchar_t *text_en);

//! \brief Вспомогательная структура-тег
struct tr_copy_t {};
//! \brief Значение-тег для варианта tr_ru_en
constexpr tr_copy_t tr_copy = {};

//! \brief Вариант tr_ru_en для случая, когда русский и английский тексты совпадают.
//! В качестве второго параметра передавать tr_copy
inline string tr_ru_en(const char *text_ru, tr_copy_t)
{
	return tr_ru_en(text_ru, text_ru);
}

//! \brief Вариант tr_ru_en для случая, когда русский и английский тексты совпадают.
//! В качестве второго параметра передавать tr_copy
inline wstring tr_ru_en(const wchar_t *text_ru, tr_copy_t)
{
	return tr_ru_en(text_ru, text_ru);
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__i18n_h
