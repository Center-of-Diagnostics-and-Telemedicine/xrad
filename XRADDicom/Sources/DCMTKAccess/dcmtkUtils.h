/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef dcmtkUtils_h__
#define dcmtkUtils_h__

#include <XRADBasic/Core.h>
#include <dcmtk/dcmdata/dcfilefo.h>  // DcmFileFormat
#include <string>

XRAD_BEGIN

void checkOFResult(const OFCondition &, const std::string &);

//! DCMTK загружает строки из файла в переменную std::string, однако на практике эти строки закодированы в utf-8.
//! Это несоответствие типа содержимому следует устранять сразу же после загрузки данных с помощью следующих функций.
//! UPD Такие же ситуации стали возникать в других проектах в связи с JSON. Сделаны универсальные преобразования,
//!	частные реализации удаляем

// std::wstring convert_hidden_utf8_to_wstring(const std::string &str);
// std::string	convert_wstring_to_hidden_utf8(const std::wstring &str);

XRAD_END

#endif // dcmtkUtils_h__
