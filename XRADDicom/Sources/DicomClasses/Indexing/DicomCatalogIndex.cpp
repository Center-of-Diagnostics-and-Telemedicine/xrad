#include "pre.h"
#include "DicomCatalogIndex.h"

#include "DicomClustering.h"
#include "DicomCatalogIndex.h"
#include "SingleDirectoryIndex.h"
#include "SingleDirectoryIndexJson.h"


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
//const size_t IndexFileCountMax = 1000000;
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

	for (auto &dir_data: all_dirs)
	{
		SingleDirectoryIndex current_directory_info(dir_data.first);
		if (current_directory_info.fill_from_fileinfo(*dir_data.second))
		{
			m_data.push_back(std::move(current_directory_info));  // после функции move объект current_directory_info уже не хранит информации
		}
		++progress1;
	}
}

void DicomCatalogIndex::fill_from_json_info(const wstring &path,
	const DirectoryContentInfo& directory_tree,
	ProgressProxy pp)
{
	RandomProgressBar	progress(pp);
	progress.start("prepare fill_from_json_info");

	vector<pair<wstring, const vector<FileInfo>*>> all_dirs;
	unroll_directories(path, directory_tree, &all_dirs);
	progress.set_position(0.5);

	ProgressBar	progress1(progress.subprogress(0.5, 1));
	progress1.start("", all_dirs.size());

	for (auto &dir_data : all_dirs)
	{
		SingleDirectoryIndex current_directory_info(dir_data.first);
		current_directory_info.fill_from_fileinfo(*dir_data.second);
		const wstring& json_name = current_directory_info.get_path_json_2();

		if (!json_name.empty())
		{
			SingleDirectoryIndex loaded_index = load_parse_json(json_name);

			//	if (current_directory_info.fill_from_fileinfo(*dir_data.second))
			//	{
			m_data.push_back(std::move(loaded_index));  // после функции move объект current_directory_info уже не хранит информации
	//	}
		}
		++progress1;
	}
}

void DicomCatalogIndex::clear()
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
			if (el1 == el2)
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

//	CatalogIndexing->PerformCatalogIndexing
//	Предлагаю в имена функций всегда включать глагол, выражающий суть выполняемых действий. КНС

void DicomCatalogIndex::PerformCatalogIndexing(const datasource_folder &src_folder, ProgressProxy pp )
{
	TimeProfiler	scan_catalog_tp, fill_from_fileinfo_tp, check_actuality_tp;
	if (m_b_show_info)
	{
		printf("%s : root_path \n", convert_to_string(src_folder.path()).c_str());
		fflush(stdout);
	}
	scan_catalog_tp.Start();

	RandomProgressBar	progress(pp);
	ProgressIndicatorScheduler	scheduler({ 15, 5, 80 });
	progress.start("Scanning catalog", scheduler.n_steps());
	auto file_info_vector = GetDirectoryFilesDetailed(
		src_folder.path(),
		L"", true,
		progress.subprogress(scheduler.operation_boundaries(0)));
	scan_catalog_tp.Stop();


	fill_from_fileinfo_tp.Start();
	// заполнить для каждого файла информацию о размере файла и дате создания из структур fileinfo
	switch(src_folder.mode())
	{
	default:
	case decltype(src_folder.mode())::read_and_update_index:
		fill_from_fileinfo(
			src_folder.path(),
			file_info_vector,
			progress.subprogress(scheduler.operation_boundaries(1)));
		break;

	case decltype(src_folder.mode())::read_index_as_is:
		fill_from_json_info(
			src_folder.path(),
			file_info_vector,
			progress.subprogress(scheduler.operation_boundaries(1)));
		break;
	}

	fill_from_fileinfo_tp.Stop();

	// проверить актуальность информации из json файлов и сохранить json файлы только обновлённых директорий

	check_actuality_tp.Start();
	check_actuality_and_update(progress.subprogress(scheduler.operation_boundaries(2)));
	check_actuality_tp.Stop();

	if (m_b_show_info)
	{
		printf("%s : root_path \n", convert_to_string(src_folder.path()).c_str());
		printf("1) %g sec: file list\n", scan_catalog_tp.LastElapsed().sec());

		printf("2) %g s: fill_from_fileinfo  %zu: number of files  %zu: number of directories \n",
			fill_from_fileinfo_tp.LastElapsed().sec(),
			file_info_vector.files.size(),
			file_info_vector.directories.size());

		printf("3) %g sec: check_actuality_and_update \n%zu: number of files \n ",
			check_actuality_tp.LastElapsed().sec(),
			file_info_vector.files.size());
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
		if(m_check_consistency)
		{
			check_index_actuality(current_dir_index);
			if(current_dir_index.indexing_needed() && m_update) // если индексация нужна
			{
				current_dir_index.update();
				// заполнять полную информацию о файлах с диска и сохранить её в json файл
				save_to_jsons(current_dir_index, index_file_type::hierarchical);
				save_to_jsons(current_dir_index, index_file_type::raw);
			}
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

size_t DicomCatalogIndex::n_items() const
{
	size_t	result = 0;
	for(auto &directory: m_data) result += directory.size();
	return result;
}

} //namespace Dicom

XRAD_END
