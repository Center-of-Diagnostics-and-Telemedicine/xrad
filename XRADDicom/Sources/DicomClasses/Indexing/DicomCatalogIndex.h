#ifndef DicomCatalogIndex_h__
#define DicomCatalogIndex_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявление функций и полей класса DicomCatalogIndex, предназначенных для работы с файлами
	и директориями каталога

	Класс DicomCatalogIndex  предназначен для обработки всех поддиректорий некоторого каталога.
*/
#include <XRADBasic/Sources/Utils/TimeProfiler.h>
#include "SingleDirectoryIndex.h"
#include "SingleDirectoryIndexJson.h"
#include <XRADDicom/Sources/DicomClasses/DataContainers/datasource.h>

XRAD_BEGIN

using namespace Dicom;

namespace Dicom
{

//! \brief Класс для работы и хранения информации о каталоге с файлами
class DicomCatalogIndex
{
	private:
		/// требуется ли вывод вспомогательной информации в stdout
		const bool	m_b_show_info;
		/// вектор информации о директориях
		vector<SingleDirectoryIndex>	m_data;

	public:
		DicomCatalogIndex(bool show_info) : m_b_show_info(show_info)
		{
		}

	private:
		/*!
			\brief Заполнить содержимое на основании содержания файлов json в дереве каталогов
				 с проверкой актуальности сведений по directory_tree, обновить данные для измененных
				 файлов, удалить лишние элементы
		*/
		void FillFromJsonAndFileInfo(const wstring &path,
			const DirectoryContentInfo& directory_tree,
			ProgressProxy pp);

		/*!
			\brief Заполнить содержимое на основании содержания файлов json в дереве каталогов
				 с проверкой актуальности сведений по directory_tree (exception при несоответствии)
		*/
		void FillFromJsonInfo(const wstring &path,
			const DirectoryContentInfo& directory_tree,
			ProgressProxy pp);

	public:
		/// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
		bool test_json_write_load();

	public:
		/// проверка равенства двух DicomFileIndex объектов
		bool operator== (const DicomCatalogIndex& a)  const;
		size_t	n_items() const;

		/// индексировать все файлы в каталоге root_path: несколько шагов
		/// 1) составить список всех файлов и разбить их на директории
		/// 2) проверить адекватность информации в json файлах с проверяемыми файлами:
		/// 3) если информация из json файла неактуальна - её нужно индексировать: "update"
		/// 4) сгенерировать json файлы для оставшихся файлов
		/// \param root_path [in] путь к анализируемому каталогу
		/// \param show_info [in] выводить вспомогательную информацию

		void PerformCatalogIndexing(const datasource_folder& src_folder,
				ProgressProxy pp = VoidProgressProxy());

		vector<SingleDirectoryIndex> &data() { return m_data; }

	private:
		void PerformCatalogIndexingUpdate(const datasource_folder& src_folder, ProgressProxy pp);
		void PerformCatalogIndexingReadFast(const datasource_folder& src_folder, ProgressProxy pp);
};



} //namespace Dicom

XRAD_END

#endif // DicomCatalog_h__
