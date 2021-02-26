/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__GUI_i18n_h
#define XRAD__GUI_i18n_h
/*!
	\file
	\brief Поддержка интернационализации: сохранение идентификатора текущего языка
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Загрузить сохраненный идентификатор языка (отличается от текущего при запуске приложения)
string LoadLanguageId();

//! \brief Сохранить идентификатор языка и сделать его текущим
void SaveLanguageId(const string &id);

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__GUI_i18n_h
