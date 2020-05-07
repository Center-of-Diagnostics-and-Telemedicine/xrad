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

	bool save_json(const json& json_to_save, const wstring &filename);

	bool load_json(json& json_loaded, const wstring &filename);

	// flatten json to result string
	void json_flatten_string(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result);

	// parse json to list of dicom reference
	void json_parse_type1(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result,
		vector<json>& vec_json_dicom);

	// получить из json объекта значение тэга str_tag_discr, скопировать его в str_tag_value
	// проверяется, что это значение имеет тип string
	bool json_get_tag_value(const json& json_obj, const string & str_tag_discr, string & str_tag_value);

	bool json_get_tag_value(const json & json_obj, const string & str_tag_discr, uint64_t & tag_value);

}  // end namespace Dicom

XRAD_END

#endif // DicomClustering_h__
