#ifndef DicomDirectoryIndex_h__
#define DicomDirectoryIndex_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief  объявление функций и полей класса DicomDirectoryIndex

	Класс DicomDirectoryIndex  предназначен для обработки файлов в одной директории.
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
\brief класс для обработки и хранения инф-ции о dicom файлах в одной директории
*/
class DicomDirectoryIndex
{
	private:
		/// путь к директории с файлами
		wstring		m_path;

		/// имя json файла типа 1 (внутри директории)
		wstring		m_filename_json_1;

		/// имя json файла типа 2 (внутри директории)
		wstring		m_filename_json_2;

		/// требуется ли индексация, если true - то индексация требуется
		bool		m_b_indexing_needed = true;

	public:
		/// вектор инф-ции об отдельных файлах
		vector<DicomFileIndex>		m_FilesIndex;

		//! \brief Заполнить значения размера файла и даты создания для каждого файла из file_infos
		//! \param path [in] Абсолютный путь к директории с файлами file_infos
		bool	fill_from_fileinfo(const wstring &path, const vector<FileInfo>& file_infos);

		/// проверка равенства двух DicomDirectoryIndex объектов
		bool	operator == (const DicomDirectoryIndex& a) const;

		/// получить путь к директории m_path
		/// \return путь к директории
		wstring get_path() const
		{
			return m_path;
		}

		/// установить путь к директории m_path
		/// \param path [in] путь к директории
		void	set_path(const wstring& path)
		{
			m_path = path;
		}

		/// получить путь к json файлу
		/// \return путь к json файлу
		wstring get_path_json_2() const
		{
			if (m_filename_json_2.empty())
			{
				return L"";
			}
			else
			{
				return m_path + wpath_separator() + m_filename_json_2;
			}
		}

		/// получить значение m_b_indexing_needed, необходимость в индексации
		/// \return требуется ли индексация
		bool	get_isneed_indexing() const
		{
			return m_b_indexing_needed;
		}

		/// установить значение m_b_indexing_needed
		/// \param b_value [in] установить флаг необходимости индексации
		void	set_need_indexing(bool b_value)
		{
			m_b_indexing_needed = b_value;
		}

		/// обновить информацию для элементов с m_b_indexing_needed == true
		void	update();

		/// добавить структуру dcmFileIndex в контейнер директории
		void	add_file_index(const DicomFileIndex& dcmFileIndex);

		/// очистить контейнер m_FilesIndex
		void	clear()
		{
			m_FilesIndex.clear();
			m_FilesIndex.shrink_to_fit();
		}
};

} //namespace Dicom

XRAD_END

#endif // DicomDirectoryIndex_h__
