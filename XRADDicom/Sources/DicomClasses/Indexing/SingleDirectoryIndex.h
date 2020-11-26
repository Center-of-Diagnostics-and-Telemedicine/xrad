#ifndef DicomDirectoryIndex_h__
#define DicomDirectoryIndex_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявление функций и полей класса SingleDirectoryIndex

	Класс SingleDirectoryIndex предназначен для обработки файлов в одной директории.
*/

#include "DicomFileIndex.h"

XRAD_BEGIN

namespace Dicom
{

//! \brief Имя json файла (тип 1)
inline const wchar_t *index_filename_type1() { return L".v1-xrad-dicom-cat"; }

//! \brief Имя json файла (тип 2)
inline const wchar_t *index_filename_type2() { return L".v2-xrad-dicom-cat"; }

/*!
	\brief Класс для обработки и хранения инф-ции о dicom файлах в одной директории

	\note
	Прежнее название DicomDirectoryIndex создавало путаницу: рядом находится класс DicomCatalogIndex,
	содержащий принципиально другую информацию, но по названию синонимичный этому (catalog=directory).
*/
class SingleDirectoryIndex : public vector<DicomFileIndex>
{
	private:
		/// путь к индексируемой директории
		wstring	m_path;

		PARENT(vector<DicomFileIndex>);

	public:
		SingleDirectoryIndex(const wstring &in_path) : m_path(in_path){}

		//! \brief Получить путь к директории m_path
		wstring get_path() const {return m_path;}

		//! \brief Добавить структуру dcmFileIndex в контейнер директории
		void	add_file_index(const DicomFileIndex& dcmFileIndex);

		//! \brief Проверить актуальность по размерам и датам файлов, исключение при несоответствии
		void CheckUpToDate(const vector<FileInfo>& file_infos) const;

		/*!
			\brief Проверить актуальность по размерам и датам файлов, актуализировать данные для
			изменившихся файлов, добавить новые, удалить несуществующие

			\return
				- true - есть изменения (требуется сохранение)
				- false - нет изменений
		*/
		bool Update(const vector<FileInfo>& file_infos);
};

} //namespace Dicom

XRAD_END

#endif // DicomDirectoryIndex_h__
