#ifndef DicomDirectoryIndexJson_h__
#define DicomDirectoryIndexJson_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявления функций для создания-записи-считывания json объектов с информацией о файлах в директории
*/

#include "DicomFileIndex.h"
#include "SingleDirectoryIndex.h"

XRAD_BEGIN

namespace Dicom
{

/// загрузить json файл, предполагаем наш формат
SingleDirectoryIndex load_parse_json(const wstring& json_fname);

/// записать json файл, вернуть имя json файла
wstring	save_to_jsons(const SingleDirectoryIndex& dcmDirectoryIndex, index_file_type json_type);

/// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
bool	test_write_load_json(SingleDirectoryIndex& dcmDirectoryIndex);

} // end namespace Dicom
XRAD_END

#endif // DicomDirectoryIndexJson_h__
