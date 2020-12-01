/*!
	\file
	\date 2019/09/26 14:00
	\author novik

	\brief 	Имплементация функционала SingleDirectoryIndex - структуры для работы с файлами в одной директории

	Функции работы с json файлом вынесены в файл DicomDirectoryIndexJson.cpp
*/
#include "pre.h"
#include "SingleDirectoryIndex.h"

#include <XRADDicom/XRADDicom.h>
#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/xray_image.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice_siemens.h>

#include <typeinfo>

XRAD_BEGIN

namespace Dicom
{

void	SingleDirectoryIndex::add_file_index(const DicomFileIndex& dcmFileIndex)
{
	push_back(dcmFileIndex);
}

namespace
{

void throw_mismatch(const wstring &path, const wstring &filename)
{
	throw runtime_error(ssprintf("DICOM index is out-of-date for \"%s\".\n"
			"Mismatch at file \"%s\".",
			EnsureType<const char*>(convert_to_string(path).c_str()),
			EnsureType<const char*>(convert_to_string(filename).c_str())));
}

} // namespace

void SingleDirectoryIndex::CheckUpToDate(const vector<FileInfo>& file_infos,
		const vector<wstring> &reserved_filenames) const
{
	map<wstring, const DicomFileIndex*> current_entries;
	for (auto &di: *this)
	{
		current_entries[CmpNormalizeFilename(di.get_file_name())] = &di;
	}
	set<wstring> reserved_filename_set;
	for (auto &f: reserved_filenames)
	{
		reserved_filename_set.insert(CmpNormalizeFilename(f));
	}
	for (auto &fi: file_infos)
	{
		auto filename_n = CmpNormalizeFilename(fi.filename);
		if (!reserved_filename_set.count(filename_n) &&
				may_be_dicom_filename(fi.filename))
		{
			auto it = current_entries.find(filename_n);
			if (it == current_entries.end())
				throw_mismatch(m_path, fi.filename);
			auto &di = *it->second;
			if (di.get_file_size() != fi.size ||
					di.get_file_mtime() != DicomFileIndex::FormatTime(&fi.time_write))
			{
				throw_mismatch(m_path, fi.filename);
			}
			current_entries.erase(it);
		}
	}
	for (auto &dii: current_entries)
	{
		auto &filename_n = dii.first;
		if (!reserved_filename_set.count(filename_n) &&
				may_be_dicom_filename(dii.second->get_file_name()))
		{
			throw_mismatch(m_path, dii.second->get_file_name());
		}
	}
}

bool SingleDirectoryIndex::Update(const vector<FileInfo>& file_infos,
		const vector<wstring> &reserved_filenames,
		UpdateStat *stat)
{
	map<wstring, const FileInfo*> fs_entries;
	for (auto &fi: file_infos)
	{
		fs_entries[CmpNormalizeFilename(fi.filename)] = &fi;
	}
	// Если индекс загружен из файла, построенного на другой файловой системе (в другой операционной
	// системе), то возможна следующая ситуация. Индекс построен в Linux на файловой системе,
	// чувствительной к регистру символов. На момент постройки в директории было два файла,
	// имена которых различаются только регистром (это могут быть не-DICOM файлы, попадающие
	// в индекс). Сейчас программа работает в Windows на файловой системе, не чувствительной
	// к регистру символов. Получается, что в *this будет два элемента, у которых
	// CmpNormalizeFilename(.) будет совпадать. На файловой системе для них будет не больше одного
	// файла.
	map<wstring, list<size_t>> current_entries;
	for (size_t i = 0; i < size(); ++i)
	{
		auto &di = (*this)[i];
		current_entries[CmpNormalizeFilename(di.get_file_name())].push_back(i);
	}
	set<wstring> reserved_filename_set;
	for (auto &f: reserved_filenames)
	{
		reserved_filename_set.insert(CmpNormalizeFilename(f));
	}

	bool modified = false;

	// Элементы для удаления перемещаем в конец, на них будет указывать it_end.
	vector<size_t> items_to_delete;
	for (auto &ci: current_entries)
	{
		auto filename_n = ci.first;
		if (reserved_filename_set.count(filename_n) ||
				!may_be_dicom_filename(filename_n))
		{
			// Этого файла не должно быть в индексе.
			for (auto i: ci.second)
			{
				if (stat)
				{
					if ((*this)[i].is_dicom())
						++stat->deleted_dicoms;
					else
						++stat->deleted_non_dicoms;
				}

				items_to_delete.push_back(i);
			}
			modified = true;
		}
		else
		{
			auto fi_it = fs_entries.find(filename_n);
			if (fi_it == fs_entries.end())
			{
				// Такого файла больше нет.
				for (auto i: ci.second)
				{
					if (stat)
					{
						if ((*this)[i].is_dicom())
							++stat->deleted_dicoms;
						else
							++stat->deleted_non_dicoms;
					}

					items_to_delete.push_back(i);
				}
				modified = true;
			}
			else
			{
				auto &fi = *fi_it->second;
				size_t di_index = 0;
				bool need_update = false;
				if (ci.second.size() == 1)
				{
					di_index = ci.second.front();
					auto &di = (*this)[di_index];
					need_update = di.get_file_size() != fi.size ||
							di.get_file_mtime() != DicomFileIndex::FormatTime(&fi.time_write);
				}
				else
				{
					// Выбираем наиболее подходящий элемент из нескольких.
					XRAD_ASSERT_THROW(ci.second.size() > 1); // не 0
					need_update = true;
					int score = 0;
					for (auto i: ci.second)
					{
						auto &di = (*this)[i];
						// Считаем, что совпадение имени с учетом регистра — наиболее важный показатель
						// в "рейтинге", потом идет совпадение размера, потом — даты.
						// При неточном совпадении имени файла (при том, что кандидатов несколько), требуем
						// полную проверку файла (need_update = true), даже если размер и время совпадают.
						int current_score = 1 +
							(di.get_file_size() == fi.size ? 2: 0) +
							(di.get_file_mtime() == DicomFileIndex::FormatTime(&fi.time_write) ? 1: 0) +
							(di.get_file_name() == fi.filename ? 4: 0);
						constexpr int max_score = 8; // значение current_score при полном совпадении
						if (current_score > score)
						{
							score = current_score;
							di_index = i;
							if (current_score == max_score)
							{
								need_update = false;
								break;
							}
						}
					}
					// Удаляем остальные элементы (кроме di_index).
					for (auto i: ci.second)
					{
						if (i == di_index)
							continue;
						if (stat)
						{
							if ((*this)[i].is_dicom())
								++stat->deleted_dicoms;
							else
								++stat->deleted_non_dicoms;
						}

						items_to_delete.push_back(i);
					}
				}
				// Если регистр имени файла изменился, но не требуется обновление по содержимому файла,
				// игнорируем такое отличие.
				if (need_update)
				{
					// Файл изменился.
					// Если обновляем информацию по содержимому файла, то все данные о файле обновляем.
					auto &di = (*this)[di_index];
					DicomFileIndex current_file_tags;
					if (!current_file_tags.fill_filetags_from_file(get_path(), fi.filename))
					{
						throw runtime_error(ssprintf("Error updating DICOM index data for file \"%s\".",
								EnsureType<const char*>(convert_to_string(MergePath(get_path(), fi.filename))
										.c_str())));
					}
					if (stat)
					{
						if (di.is_dicom())
						{
							if (current_file_tags.is_dicom())
							{
								++stat->modified_dicoms;
							}
							else
							{
								++stat->deleted_dicoms;
								++stat->added_non_dicoms;
							}
						}
						else
						{
							if (current_file_tags.is_dicom())
							{
								++stat->deleted_non_dicoms;
								++stat->added_dicoms;
							}
							else
							{
								++stat->modified_non_dicoms;
							}
						}
					}
					di = std::move(current_file_tags);
					modified = true;
				}
				// Удаляем элемент из fs_entries.
				fs_entries.erase(fi_it);
			}
		}
	}

	// Удаляем элементы, индексы которых находятся в items_to_delete.
	sort(items_to_delete.begin(), items_to_delete.end());
	auto it_end = end();
	for (auto i: items_to_delete)
	{
		--it_end;
		auto it = begin() + i;
		if (it != it_end)
		{
			using std::swap;
			swap(*it, *it_end);
		}
	}
	erase(it_end, end());

	for (auto &fsi: fs_entries)
	{
		auto &filename_n = fsi.first;
		if (!reserved_filename_set.count(filename_n) &&
				may_be_dicom_filename(fsi.second->filename))
		{
			// Новый файл.
			DicomFileIndex current_file_tags;
			if (!current_file_tags.fill_filetags_from_file(get_path(), fsi.second->filename))
			{
				throw runtime_error(ssprintf("Error updating DICOM index data for file \"%s\".",
						EnsureType<const char*>(convert_to_string(MergePath(get_path(), fsi.second->filename))
								.c_str())));
			}
			if (stat)
			{
				if (current_file_tags.is_dicom())
					++stat->added_dicoms;
				else
					++stat->added_non_dicoms;
			}
			push_back(std::move(current_file_tags));
			modified = true;
		}
	}
	return modified;
}

} //namespace Dicom

XRAD_END
