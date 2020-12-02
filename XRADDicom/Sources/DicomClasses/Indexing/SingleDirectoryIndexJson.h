#ifndef DicomDirectoryIndexJson_h__
#define DicomDirectoryIndexJson_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявления функций для создания-записи-считывания json объектов с информацией о файлах в директории
*/

#include "DicomFileIndexJson.h"
#include "SingleDirectoryIndex.h"

XRAD_BEGIN

namespace Dicom
{

//! \brief Имя json файла (тип 1)
inline const wchar_t *index_filename_type1() { return L".xrad-dicom-cat-v1"; }

//! \brief Имя json файла (тип 2)
inline const wchar_t *index_filename_type2() { return L".xrad-dicom-cat-v2"; }

enum class ErrorReportMode
{
	log_and_recover = 0,
	throw_exception
};

//! \brief Загрузить json файл, предполагаем наш формат
SingleDirectoryIndex load_parse_json(const wstring& json_fname, ErrorReportMode erm);

//! \brief Записать json файл, вернуть имя json файла
void save_to_jsons(const SingleDirectoryIndex& dcmDirectoryIndex, const wstring& json_fname,
		index_file_type json_type);

//! \brief Проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах (debug)
bool	test_write_load_json(SingleDirectoryIndex& dcmDirectoryIndex);

} // end namespace Dicom

XRAD_END

#endif // DicomDirectoryIndexJson_h__
