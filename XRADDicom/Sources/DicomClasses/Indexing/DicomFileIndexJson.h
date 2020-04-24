#ifndef DicomFileIndexJson_h__
#define DicomFileIndexJson_h__
/*!
	\file
	\date 2019/10/17 13:00
	\author novik
	\brief Объявления функций для создания-записи-считывания json объектов с информацией о файле
*/

#include "DicomFileIndex.h"
#include <XRADBasic/ThirdParty/nlohmann/json.hpp>

XRAD_BEGIN

namespace Dicom
{
	using json = nlohmann::json;

	/// из json объекта заданного типа сформировать DicomFileIndex объект
	/// универсальная функция для двух типов json объектов
	bool from_json_get_file_index(DicomFileIndex& dcmFileIndex, const json& json_file_tag, const int json_type);

	/// из json объекта type1 сформировать DicomFileIndex объект
	bool from_json_type1_inner_block(DicomFileIndex& dcmFileIndex, const json& json_file_tag);

	/// из json объекта сгенерировать DicomFileIndex объект
	bool from_json_type2(DicomFileIndex& dcmFileIndex, const json& json_file_tag);

	/// в json объекта type1 записать инф-цию о файле во внутренний блок
	bool to_json_type1_inner_block(const DicomFileIndex& dcmFileIndex, json& json_file_tag);

	/// сгенерировать json объект type1
	bool to_json_type1(const DicomFileIndex& dcmFileIndex, json& json_unique_type1);

	/// сгенерировать json объект type2
	bool to_json_type2(const DicomFileIndex& dcmFileIndex, json& json_file_tag);

}   // end namespace Dicom

XRAD_END

#endif // DicomFileIndexJson_h__
