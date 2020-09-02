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

bool SingleDirectoryIndex::operator==(const SingleDirectoryIndex& a) const
{
	for (const auto& el1 : *this)
	{
		bool is_equal_find = false;
		for (const auto& el2 : a)
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

void	SingleDirectoryIndex::add_file_index(const DicomFileIndex& dcmFileIndex)
{
	push_back(dcmFileIndex);
}

void SingleDirectoryIndex::update()
{
	for (auto& el : *this)
	{
		if (el.indexing_needed())
		{
			DicomFileIndex current_file_tags;
			if(current_file_tags.fill_filetags_from_file(get_path(),
			   convert_to_wstring(el.get_file_name())))
			{
				el = current_file_tags;			// обновили тэги
				el.set_indexing_needed(false);    // сняли метку необходимости индексации
			}
		}
	}
	// удалить индексаторы о файлах, по которым не удалось обновить информацию
	auto predicate = [](const DicomFileIndex &v) { return v.indexing_needed(); };
	erase(remove_if(begin(), end(), predicate), end());
}

bool SingleDirectoryIndex::fill_from_fileinfo(//const wstring &path,
		const vector<FileInfo>& file_infos)
{
//	m_path = path;
	for (auto el : file_infos)
	{
		if (el.filename == index_filename_type1())
		{
			m_filename_json_1 = el.filename;
		}
		else if (el.filename == index_filename_type2())
		{
			m_filename_json_2 = el.filename;
		}
		else if (may_be_dicom_filename(el.filename))
		{
			DicomFileIndex current_file_tags;
			current_file_tags.fill_from_fileinfo(el);
			push_back(std::move(current_file_tags));
				// после функции move объект current_file_tags уже не хранит информации
		}
	}
	return size() || !m_filename_json_1.empty() || !m_filename_json_2.empty();
}

} //namespace Dicom

XRAD_END
