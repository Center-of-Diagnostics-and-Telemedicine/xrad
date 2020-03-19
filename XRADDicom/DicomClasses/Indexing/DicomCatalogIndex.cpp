#include "pre.h"

#include <XRADDicom/DicomClasses/Indexing/DicomClustering.h>
#include <XRADDicom/DicomClasses/Indexing/DicomCatalogIndex.h>
#include <XRADDicom/DicomClasses/Indexing/DicomDirectoryIndex.h>
#include <XRADDicom/DicomClasses/Indexing/DicomCatalogIndex.h>
#include <XRADDicom/DicomClasses/Indexing/DicomDirectoryIndexJson.h>


/// define TEST_JSON для тестирования работы индексатора
#define TEST_JSON0


/*!
	\file
	\date 2019/10/21 11:00
	\author novik

	\brief  Имплиментация (реализация) функций и полей класса DicomCatalogIndex

	Класс DicomCatalogIndex  предназначен для обработки всех поддиректорий
*/

XRAD_BEGIN

namespace Dicom
{

namespace
{
//! \brief Максимальное число индексируемых файлов
const size_t IndexFileCountMax = 1000000;
} // namespace

namespace
{
void unroll_directories(const wstring &path,
		const DirectoryContentInfo& directory_tree,
		vector<pair<wstring, const vector<FileInfo>*>> *all_dirs)
{
	all_dirs->emplace_back(path, &directory_tree.files);
	for (auto &sub: directory_tree.directories)
	{
		unroll_directories(path + wpath_separator() + sub.filename, sub.content, all_dirs);
	}
}
} // namespace

void DicomCatalogIndex::fill_from_fileinfo(const wstring &path,
		const DirectoryContentInfo& directory_tree,
		ProgressProxy pp)
{
	RandomProgressBar	progress(pp);
	progress.start("prepare fill_from_fileinfo");

	vector<pair<wstring, const vector<FileInfo>*>> all_dirs;
	unroll_directories(path, directory_tree, &all_dirs);
	progress.set_position(0.5);

	ProgressBar	progress1(progress.subprogress(0.5, 1));
	progress1.start("", all_dirs.size());

	size_t file_count = 0;
	for (auto &dir_data: all_dirs)
	{
		DicomDirectoryIndex current_dir_info;
		if (current_dir_info.fill_from_fileinfo(dir_data.first, *dir_data.second))
		{
			if (file_count + current_dir_info.m_FilesIndex.size() > IndexFileCountMax)
				break;
			m_data.push_back(std::move(current_dir_info));  // после функции move объект current_dir_info уже не хранит информации
			file_count += current_dir_info.m_FilesIndex.size();
		}
		++progress1;
	}
}

void DicomCatalogIndex::сlear()
{
	m_data.clear();
	m_data.shrink_to_fit();
}

bool DicomCatalogIndex::operator==(const DicomCatalogIndex & a) const
{
	if (this->m_data.size() != a.m_data.size())
		return false;
	for (const auto& el1 : m_data)
	{
		bool is_equal_find = false;
		for (const auto& el2 : a.m_data)
		{
			if (el1.is_equal(el2))
			{
				is_equal_find = true;
				break;
			}
		}
		if (!is_equal_find)  // если в "a" не найдены одинаковые елементы "el1"
			return false;
	}
	return true;

}

void DicomCatalogIndex::CatalogIndexing(const wstring& root_path, bool show_info, ProgressProxy pp)
{
	//this->clear();
	m_b_show_info = show_info;
	if (m_b_show_info)
	{
		printf("%s : root_path \n", convert_to_string(root_path).c_str());
		fflush(stdout);
	}
	auto begin = std::chrono::high_resolution_clock::now();

	RandomProgressBar	progress(pp);
	ProgressIndicatorScheduler	scheduler({ 15, 5, 80 });
	progress.start("Scanning catalog.", scheduler.n_steps());
	auto file_info_vector = GetDirectoryFilesDetailed(root_path, L"", true,
			progress.subprogress(scheduler.operation_boundaries(0)));

	auto end1 = std::chrono::high_resolution_clock::now();
	// заполнить для каждого файла информацию о размере файла и дате создания из структур fileinfo
	fill_from_fileinfo(root_path, file_info_vector,
			progress.subprogress(scheduler.operation_boundaries(1)));

	auto end2 = std::chrono::high_resolution_clock::now();
	if (m_b_show_info)
	{
		printf("%s : root_path \n", convert_to_string(root_path).c_str());
		printf("1) %g s: file list \n2) %g s: fill_from_fileinfo  %zu: number of files  %zu: number of directories \n",
			1.e-9*(end1 - begin).count(), 1.e-9*(end2 - end1).count(), file_info_vector.files.size(),
			file_info_vector.directories.size());
		fflush(stdout);
	}

	// проверить актуальность информации из json файлов и сохранить json файлы только обновлённых
	// директорий
	check_actuality_and_update(progress.subprogress(scheduler.operation_boundaries(2)));

	if (m_b_show_info)
	{
		auto end3 = std::chrono::high_resolution_clock::now();
		printf("3) %g s: check_actuality_and_update \n%zu: number of files \n ",
			1.e-9*(end3 - end2).count(), file_info_vector.files.size());
		fflush(stdout);
	}
}



void DicomCatalogIndex::check_actuality_and_update(ProgressProxy pp)
{
	ProgressBar progress(pp);
	progress.start("", m_data.size());

	// для каждой директории с файлами
	for (auto& current_dir_index : m_data)
	{
		check_actuality_json(current_dir_index);
		if (current_dir_index.get_isneed_indexing()) // если индексация нужна
		{
			current_dir_index.update();
			// заполнять полную информацию о файлах с диска и сохранить её в json файл
			save_to_jsons(current_dir_index, JsonType::type_1);
			save_to_jsons(current_dir_index, JsonType::type_2);
		}
		++progress;
	}
}


bool DicomCatalogIndex::test_json_write_load()
{
	auto n_subdirs = m_data.size();
	wstring prompt{ L"Testing write_load json files" };
	//GUIProgressBar progress;
	//progress.start(prompt, n_subdirs);
	// для каждой директории с файлами
	for (uint32_t i_subdir = 0; i_subdir < n_subdirs; i_subdir++)
	{
		if (!test_write_load_json(m_data[i_subdir])) // при отрицательной проверке сразу выходит
			return false;
		//progress++;
	}
	return true;
}

} //namespace Dicom

XRAD_END
