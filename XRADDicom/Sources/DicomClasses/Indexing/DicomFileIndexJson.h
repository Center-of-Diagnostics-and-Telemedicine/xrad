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


//! \brief Типы индексных файлов, связанные с полем "type" в json индексе
//!
//!	Значения поля type определяются словарем в cpp file.
enum class index_file_type
{
	unknown = 0,
	hierarchical = 1,
	plain = 2
};

//! \brief Число полей, включаемых в древовидную структуру json файла index_file_type::hierarchical
const size_t NFIELDS_TYPE_1 = 5;

/// из json объекта заданного типа сформировать DicomFileIndex объект
/// универсальная функция для двух типов json объектов
DicomFileIndex  from_json_get_file_index(const json& json_file_tag, index_file_type json_type);

/// в json объекта type1 записать инф-цию о файле во внутренний блок
bool to_json_type1_inner_block(const DicomFileIndex& dcmFileIndex, json& json_file_tag);

/// сгенерировать json объект type2
bool to_json_type2(const DicomFileIndex& dcmFileIndex, json& json_file_tag);

} // namespace Dicom

XRAD_END

#endif // DicomFileIndexJson_h__
