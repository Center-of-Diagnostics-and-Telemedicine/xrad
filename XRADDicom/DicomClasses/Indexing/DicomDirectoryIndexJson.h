#ifndef DicomDirectoryIndexJson_h__
#define DicomDirectoryIndexJson_h__


/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявления функций для создания-записи-считывания json объектов с информацией о файлах в директории
*/


#include <XRADDicom/DicomClasses/Indexing/DicomFileIndex.h>
#include <XRADDicom/DicomClasses/Indexing/DicomDirectoryIndex.h>
#include <XRADBasic/ThirdParty/nlohmann/json.hpp>

XRAD_BEGIN


namespace Dicom
{

using json = nlohmann::json;

/// загрузить json файл, предполагаем наш формат
bool	load_parse_json(DicomDirectoryIndex& dcmDirectoryIndex, const wstring &json_fname);

/// загрузить json файл с деревом исследованием Dicom файлов
bool	load_json_type1_tree(DicomDirectoryIndex& dcmDirectoryIndex, json& json_dicom_files);

enum class JsonType
{
	type_1 = 1,
	type_2 = 2
};

/// записать json файл, вернуть имя json файла
wstring	save_to_jsons(const DicomDirectoryIndex& dcmDirectoryIndex, JsonType json_type);

/// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
bool	test_write_load_json(DicomDirectoryIndex& dcmDirectoryIndex);

/// из списка тэгов для каждого файла сгенерировать json файл type1 (sample1.json)
void	dir_info_to_json_type1(const DicomDirectoryIndex& dcmDirectoryIndex, json& json_type);

/// из списка тэгов для каждого файла сгенерировать json файл type2 (sample2.json)
void	dir_info_to_json_type2(const DicomDirectoryIndex& dcmDirectoryIndex, json& json_type);

/// проверить актуальность инф-ции быстрого сканирования в DicomDirectoryIndex и информации из json файла m_filename_json
/// на совпадание имена, размер, время создания
void check_actuality_json(DicomDirectoryIndex& dcmDirectoryIndex);

} // end namespace Dicom
XRAD_END

#endif // DicomDirectoryIndexJson_h__
