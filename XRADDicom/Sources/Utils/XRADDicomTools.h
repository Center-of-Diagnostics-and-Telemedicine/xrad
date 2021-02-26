/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 26/08/2019 13:10
	\author kovbas
*/
#ifndef XRADDicomTools_h__
#define XRADDicomTools_h__

#include <XRADDicom/Sources/DicomClasses/dataelement.h>

XRAD_BEGIN

namespace Dicom
{

	enum class UUID_level : int
	{
		study,
		series,
		instance,
		other
	};
	/*!
		brief: Генерирует уникальные идентификаторы для исследований, серий, инстансев и другого
		prefix_p - задаваемый пользователем префикс. Если отсутствует, то будет использоваться стандартный префикс DCMTK (1.2.276.0.7230010.3)
				При желании использовать не DCMTK, но при этом действительный, можно использовать префикс 2.25
				Формат префикса: 1.2.3.4 или 1.2.680 и т.д. Проверка полученного префикса на корректность не производится.
		note: Определение функции лежит в файле dcmtkElementsTools.cpp, т.к. использует функцию DCMTK

		использование:
		1) в случае выбора уровня UUID_level::other можно подставить префикс любого уровня. Будет сгенерирована конечная часть
		2) при пустом параметре prefix_p и UUID_level::other функция вернёт корректный сгенерированный DCMTK UUID для таких случаев
	*/
	wstring generateUUID(UUID_level id_type_p, const wstring &prefix_p = L"");

	/*!
		brief: Возвращает имя для заданного тега
		note: Определение функции лежит в файле dcmtkElementsTools.cpp, т.к. использует функцию DCMTK
	*/
	wstring get_tagname(tag_t tag_p);
	wstring get_tagname(const wstring &tagnum);

	/*!
		brief: Возвращает тег, записанный строкой вида: (xxxx,xxxx)
		note: Определение функции лежит в файле dcmtkElementsTools.cpp, т.к. использует функцию DCMTK
	*/
	wstring get_tag_as_string(tag_t tag_p);


} //namespace Dicom


XRAD_END

#endif // XRADDicomTools_h__