#include "pre.h"
#include "DicomCatalogIndex.h"

#include "DicomClustering.h"
#include "DicomCatalogIndex.h"
#include "SingleDirectoryIndex.h"
#include "SingleDirectoryIndexJson.h"
#include <XRADSystem/System.h>


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

void DicomCatalogIndex::FillFromJsonAndFileInfo(const wstring &path,
	const DirectoryContentInfo& directory_tree,
	ProgressProxy pp)
{
	RandomProgressBar	progress(pp);
	progress.start("");

	vector<pair<wstring, const vector<FileInfo>*>> all_dirs;
	unroll_directories(path, directory_tree, &all_dirs);
	progress.set_position(0.01);

	ProgressBar	progress1(progress.subprogress(0.01, 1));
	progress1.start("", all_dirs.size());

	auto index_filename_n = CmpNormalizeFilename(index_filename_type2());
	for (auto &dir_data : all_dirs)
	{
		auto &files = *dir_data.second;
		auto it = find_if(files.begin(), files.end(),
			[&index_filename_n](const FileInfo &fi)
			{
				return CmpNormalizeFilename(fi.filename) == index_filename_n;
			});
		if (it != files.end())
		{
			auto filename = MergePath(dir_data.first, it->filename);
			SingleDirectoryIndex loaded_index = load_parse_json(filename);
			if (loaded_index.Update(files))
			{
				if (loaded_index.empty())
				{
					DeleteFile(filename);
				}
				else
				{
					save_to_jsons(loaded_index, index_file_type::hierarchical);
					save_to_jsons(loaded_index, index_file_type::raw);
				}
			}
			if (!loaded_index.empty())
				m_data.push_back(std::move(loaded_index));
		}
		else
		{
			// Директория без файла индекса.
			SingleDirectoryIndex new_index(dir_data.first);
			if (new_index.Update(files))
			{
				save_to_jsons(new_index, index_file_type::hierarchical);
				save_to_jsons(new_index, index_file_type::raw);
				m_data.push_back(std::move(new_index));
			}
		}
		++progress1;
	}
	progress1.end();
	progress.end();
}

void DicomCatalogIndex::FillFromJsonInfo(const wstring &path,
	const DirectoryContentInfo& directory_tree,
	ProgressProxy pp)
{
	RandomProgressBar	progress(pp);
	progress.start("");

	vector<pair<wstring, const vector<FileInfo>*>> all_dirs;
	unroll_directories(path, directory_tree, &all_dirs);
	progress.set_position(0.01);

	ProgressBar	progress1(progress.subprogress(0.01, 1));
	progress1.start("", all_dirs.size());

	auto index_filename_n = CmpNormalizeFilename(index_filename_type2());
	for (auto &dir_data : all_dirs)
	{
		auto &files = *dir_data.second;
		auto it = find_if(files.begin(), files.end(),
			[&index_filename_n](const FileInfo &fi)
			{
				return CmpNormalizeFilename(fi.filename) == index_filename_n;
			});
		if (it != files.end())
		{
			SingleDirectoryIndex loaded_index = load_parse_json(MergePath(dir_data.first, it->filename));
			loaded_index.CheckUpToDate(files);
			m_data.push_back(std::move(loaded_index));
		}
		else
		{
			// Директория без файла индекса.
			SingleDirectoryIndex empty_index(dir_data.first);
			empty_index.CheckUpToDate(files);
		}
		++progress1;
	}
	progress1.end();
	progress.end();
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

void DicomCatalogIndex::PerformCatalogIndexing(const datasource_folder &src_folder,
		ProgressProxy pp)
{
	switch(src_folder.mode())
	{
		default:
		case decltype(src_folder.mode())::read_and_update_index:
			PerformCatalogIndexingUpdate(src_folder, pp);
			break;

		case decltype(src_folder.mode())::read_index_as_is:
			PerformCatalogIndexingReadFast(src_folder, pp);
			break;
	}
}

void DicomCatalogIndex::PerformCatalogIndexingUpdate(const datasource_folder& src_folder,
		ProgressProxy pp)
{
	if (m_b_show_info)
	{
		printf("DICOM index (update): root path: \"%s\"\n",
				EnsureType<const char*>(convert_to_string(src_folder.path()).c_str()));
		fflush(stdout);
	}
	ProgressIndicatorScheduler	scheduler({ 15, 5, 80 });

	TimeProfiler scan_catalog_tp;
	scan_catalog_tp.Start();
	RandomProgressBar	progress(pp);
	progress.start("Scanning catalog", scheduler.n_steps());
	auto file_info_vector = GetDirectoryFilesDetailed(
			src_folder.path(),
			L"", true,
			progress.subprogress(scheduler.operation_boundaries(0)));
	scan_catalog_tp.Stop();
	if (m_b_show_info)
	{
		printf("DICOM index (update): file list: %g sec\n",
				EnsureType<double>(scan_catalog_tp.LastElapsed().sec()));
	}

	TimeProfiler fill_from_fileinfo_tp;
	fill_from_fileinfo_tp.Start();
	// Загрузить данные из json (при наличии), актуализировать их, сохранить новые json в случае
	// изменений данных.
	FillFromJsonAndFileInfo(
			src_folder.path(),
			file_info_vector,
			progress.subprogress(scheduler.operation_boundaries(1)));
	fill_from_fileinfo_tp.Stop();
	if (m_b_show_info)
	{
		printf("DICOM index (update): fill from fileinfo: %g sec, number of files: %zu, "
				"number of directories: %zu\n",
				EnsureType<double>(fill_from_fileinfo_tp.LastElapsed().sec()),
				EnsureType<size_t>(file_info_vector.files.size()),
				EnsureType<size_t>(file_info_vector.directories.size()));
		fflush(stdout);
	}
}

void DicomCatalogIndex::PerformCatalogIndexingReadFast(const datasource_folder& src_folder,
		ProgressProxy pp)
{
	if (m_b_show_info)
	{
		printf("DICOM index (fast): root path: \"%s\"\n",
				EnsureType<const char*>(convert_to_string(src_folder.path()).c_str()));
		fflush(stdout);
	}
	ProgressIndicatorScheduler scheduler({ 15, 5 });

	TimeProfiler scan_catalog_tp;
	scan_catalog_tp.Start();
	RandomProgressBar	progress(pp);
	progress.start("Scanning catalog", scheduler.n_steps());
	auto file_info_vector = GetDirectoryFilesDetailed(
			src_folder.path(),
			L"", true,
			progress.subprogress(scheduler.operation_boundaries(0)));
	scan_catalog_tp.Stop();
	if (m_b_show_info)
	{
		printf("DICOM index (fast): file list: %g sec\n",
				EnsureType<double>(scan_catalog_tp.LastElapsed().sec()));
	}

	TimeProfiler fill_from_jsoninfo_tp;
	fill_from_jsoninfo_tp.Start();
	FillFromJsonInfo(
			src_folder.path(),
			file_info_vector,
			progress.subprogress(scheduler.operation_boundaries(1)));
	fill_from_jsoninfo_tp.Stop();
	if (m_b_show_info)
	{
		printf("DICOM index (fast): fill_from_jsoninfo: %g sec, number of files: %zu, "
				"number of directories: %zu\n",
				EnsureType<double>(fill_from_jsoninfo_tp.LastElapsed().sec()),
				EnsureType<size_t>(file_info_vector.files.size()),
				EnsureType<size_t>(file_info_vector.directories.size()));
		fflush(stdout);
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
