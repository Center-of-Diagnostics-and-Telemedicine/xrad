/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef DicomClustering_h__
#define DicomClustering_h__
/*!
	\file
	\date 2019/09/24 10:30
	\author novik
	\brief  Объявление вспомогательных функции кластеризации имён файлов и работы с json файлами
*/

#include <XRADDicom/XRADDicom.h>
#include <XRADBasic/ThirdParty/nlohmann/json.hpp>

XRAD_BEGIN

namespace Dicom
{
/// json объект из  json.hpp
using json = nlohmann::json;

	// вернуть список уникальных директорий
	vector<wstring>	get_unique_paths(const vector<wstring>& vector_paths);

	void save_json(const json& json_to_save, const wstring &filename);

	json load_json(const wstring &filename);

	// flatten json to result string
	void json_flatten_string(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result);

	// parse json to list of dicom reference
	void json_parse_type1(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result,
		vector<json>& vec_json_dicom);

	// получить из json объекта значение тэга label, скопировать его в str_tag_value
	// проверяется, что это значение имеет тип string
	wstring json_get_tag_string(const json& json_obj, const string &label);

	uint64_t json_get_tag_uint(const json & json_obj, const string &label);

}  // end namespace Dicom

XRAD_END

#endif // DicomClustering_h__
