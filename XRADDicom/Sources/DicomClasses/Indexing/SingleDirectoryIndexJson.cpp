/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2019/10/21 10:00
	\author novik

	\brief Реализация функций для создания-записи-считывания json объектов с информацией о файлах
	в директории
*/
#include "pre.h"
#include "SingleDirectoryIndexJson.h"

#include "DicomClustering.h"
#include "DicomCatalogIndex.h"
#include "DicomFileIndexJson.h"

#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/xray_image.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice_siemens.h>

#include <XRADDicom/XRADDicom.h>

#include <XRADBasic/ThirdParty/nlohmann/json.hpp>
#include <typeinfo>

/// директива для возможности тестирования записанных и считанных json объектов
#define TEST_JSON

XRAD_BEGIN

namespace Dicom
{



using json = nlohmann::json;

namespace
{

constexpr const char *index_type_id_hierarchical() { return "hierarchical"; }
constexpr const char *index_type_id_plain() { return "plain"; }

index_file_type interpret_index_file_type(const string &s)
{
	if(s == index_type_id_plain())
		return index_file_type::plain;
	if(s == index_type_id_hierarchical())
		return index_file_type::hierarchical;
	return index_file_type::unknown;
}

constexpr const char *index_id_key() { return "id"; }
constexpr const char *index_id_value() { return "XRAD DICOM catalog"; }

constexpr const char *index_type_key() { return "type"; }

constexpr const char *index_filelist_key() { return "file_list"; }
constexpr const char *index_dicomlist_key() { return "dicom_list"; }



// из списка тэгов для каждого файла сгенерировать json файл type1 (sample1.json)
void dir_info_to_json_type1(const SingleDirectoryIndex& dcmDirectoryIndex, json& json_type1)
{
	// заполнить обязательные поля
	json_type1[index_id_key()] = string(index_id_value());
	json_type1[index_type_key()] = string(index_type_id_hierarchical());

	// выделить уникальные исследования по 5-ти первым тэгам
	json json_dicom_files; // хранение информации о файлах в древовидной структуре первых 5-тэгов
	json json_not_dicom; // информация о не-DICOM файлах (идентично записи в json type 2)

	// Для определенности сохраняем элементы в порядке "возрастания" имени файла с учетом регистра
	// в utf-8.
	map<string, const DicomFileIndex*> ordered_index;
	for (auto &di: dcmDirectoryIndex)
	{
		ordered_index[convert_to_string8(di.get_file_name())] = &di;
	}

	for (auto &odi: ordered_index)
	{
		auto &file_tags = *odi.second;
		if (file_tags.is_dicom())
		{
			json json_inner_file_tag;
			if(to_json_type1_inner_block(file_tags, json_inner_file_tag))		// записать все тэги, характеризующие дайком файл во внутренний блок
			{
				json *refInnerJson = &json_dicom_files;
				for(size_t i = NFIELDS_TYPE_1; i-- > 0;)			// в json файл заполнить поля об первых 5 тэгах, характеризующих исследование
				{
					string str_tag_value = convert_to_string8(file_tags.get_dicom_tags_value(i));
					refInnerJson = &(*refInnerJson)[str_tag_value];
				}
				refInnerJson->push_back(std::move(json_inner_file_tag));
			}
		}
		else
		{
			// заполнить информацию о не dicom файлах
			json json_file_tag;
			// записать все тэги, характеризующие дайком файл, во внутренний блок
			if(to_json_type1_inner_block(file_tags, json_file_tag))
				json_not_dicom.push_back(std::move(json_file_tag));
		}
	}

	// если не пусто, записать древовидную информацию dicom файлав в тэг "dicomlist"
	if (!json_dicom_files.empty())
		json_type1[index_dicomlist_key()] = std::move(json_dicom_files);

	// записать не-DICOM файлы, это идентично записи в json type 2
	if (!json_not_dicom.empty())
		json_type1[index_filelist_key()] = std::move(json_not_dicom);
}



// из списка тэгов для каждого файла сгенерировать json файл type2 (sample2.json)
void dir_info_to_json_type2(const SingleDirectoryIndex& dcmDirectoryIndex, json& json_type2)
{
	if (!dcmDirectoryIndex.size())
		return;

	// заполнить обязательные поля
	json_type2[index_id_key()] = string(index_id_value());
	json_type2[index_type_key()] = string(index_type_id_plain());

	// Для определенности сохраняем элементы в порядке "возрастания" имени файла с учетом регистра
	// в utf-8.
	map<string, const DicomFileIndex*> ordered_index;
	for (auto &di: dcmDirectoryIndex)
	{
		ordered_index[convert_to_string8(di.get_file_name())] = &di;
	}

	for (auto &odi: ordered_index)
	{
		auto &dicom_file_Tags = *odi.second;
		json json_file_tag;
		if(to_json_type2(dicom_file_Tags, json_file_tag))	 // записать все тэги, характеризующие дайком файл
		{
			json_type2[index_filelist_key()].push_back(std::move(json_file_tag));
		}
	}
}



// загрузить json файл с деревом исследований Dicom файлов type 1
void load_json_type1_tree(SingleDirectoryIndex& result, const json &json_type1_dicom_section)
{
	XRAD_ASSERT_THROW(!json_type1_dicom_section.is_null());   // проверить, есть ли информация в json объекте

	vector<string> reference_string;
	vector<vector<string>> flatten_result;
	vector<json> vec_json_dicom;
	json_parse_type1(reference_string, json_type1_dicom_section, flatten_result, vec_json_dicom);

	// для считывания этой древовидной структуры
	XRAD_ASSERT_THROW (flatten_result.size() != 0)  // если парсинг дал пустой результат

	for (size_t i = 0; i < flatten_result.size(); i++)
	{
		vector<string> fields1 = flatten_result[i];
		if (fields1.size() != NFIELDS_TYPE_1)
			break;
		DicomFileIndex fileindex_base;
		for (size_t i = NFIELDS_TYPE_1; i-- > 0;)			// в json файл заполнить поля об первых 5 тэгах, характеризующих исследование
		{
			//Dicom::tag_t tag_value = fileindex_base.m_dicom_tags[i];
			fileindex_base.set_dicom_tags_value(i, convert_to_wstring(fields1[NFIELDS_TYPE_1 - 1 - i]));
		}

		for (auto& el : vec_json_dicom[i])  // для каждого элемента array
		{
			try
			{
				DicomFileIndex fileindex = fileindex_base;
				fileindex.append(from_json_get_file_index(el, index_file_type::hierarchical));
				if(fileindex.is_dicom()) // взять остальные поля
				{
					result.add_file_index(std::move(fileindex));  // после функции move объект fileindex_from_json уже не хранит информации
				}
			}
			catch(...)
			{
			}
		}

	}
}

} // namespace



SingleDirectoryIndex load_parse_json(const wstring& json_fname, ErrorReportMode erm)
{
	auto report_error = [&json_fname, erm](const string &message, size_t *err_counter = nullptr)
	{
		switch (erm)
		{
			default:
			case ErrorReportMode::log_and_recover:
				if (err_counter)
				{
					if (*err_counter > 3)
						return;
					++*err_counter;
				}
				fprintf(stderr, "Error loading XRAD DICOM index file \"%s\":\n%s"
						"\n",
						EnsureType<const char*>(convert_to_string(json_fname).c_str()),
						EnsureType<const char*>(message.c_str()));
				break;
			case ErrorReportMode::throw_exception:
				throw runtime_error(ssprintf("Error loading XRAD DICOM index file \"%s\":\n%s",
						EnsureType<const char*>(convert_to_string(json_fname).c_str()),
						EnsureType<const char*>(message.c_str())));
				break;
		}
	};

	SingleDirectoryIndex result(RemoveTrailingPathSeparator(file_path(json_fname)));

	try
	{
		const json json_from_file = load_json(json_fname);

		// проверить тип json файла
		auto it_type = json_from_file.find(index_type_key());
		if (it_type == json_from_file.end())
		{
			report_error(ssprintf("Value not found: %s.", EnsureType<const char*>(index_type_key())));
			return result;
		}
		index_file_type json_type = interpret_index_file_type(*it_type);
		if (json_type == index_file_type::unknown)
		{
			report_error(ssprintf("Invalid index type: \"%s\".", EnsureType<const char*>(index_type_key())));
			return result;
		}

		if (json_type == index_file_type::hierarchical)
		{
			// json файл тип type 1
			// Парсинг type 1 нужно переписать полностью. Теперешний код, хотя и работает, содержит много натянутых зависимостей, которые усложняют сопровождение
			//Error("json type 1 index is not tested");

			auto it = json_from_file.find(index_dicomlist_key());
			if (it != json_from_file.end())
			{
				// В индексе есть DICOM-файлы.
				load_json_type1_tree(result, *it);
			}
		}

		// json файл тип type 1 или 2
		// Формат хранения о не-DICOM файлах в type 1 совпадает с форматом хранения обо всех файлах
		// type 2.
		auto it = json_from_file.find(index_filelist_key());
		if (it != json_from_file.end())
		{
			const json& json_filelist = *it;
			size_t err_counter = 0;
			for (size_t i = 0; i < json_filelist.size(); i++)
			{
				try
				{
					DicomFileIndex fileindex = from_json_get_file_index(json_filelist[i], index_file_type::plain);
					result.add_file_index(fileindex);
				}
				catch(...)
				{
					report_error(GetExceptionStringOrRethrow(), &err_counter);
				}
			}
		}
	}
	catch (...)
	{
		report_error(GetExceptionStringOrRethrow());
	}
	return result;
}



void save_to_jsons(const SingleDirectoryIndex& dcmDirectoryIndex, const wstring& json_fname,
		index_file_type json_type)
{
	// для каждого уникального кластера (директории с dicom файлами) сформировать json объект
	// json объект формируется согласно примерам образцов json файлов: sample1.json и sample2.json
	json json_to_save;
	switch (json_type)
	{
		case index_file_type::hierarchical:
			dir_info_to_json_type1(dcmDirectoryIndex, json_to_save);
			break;
		case index_file_type::plain:
			dir_info_to_json_type2(dcmDirectoryIndex, json_to_save);
			break;
		default:
			throw invalid_argument("save_to_jsons: invalid json_type.");
	}

	save_json(json_to_save, json_fname);
}



// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
bool test_write_load_json(SingleDirectoryIndex& dcmDirectoryIndex)
{
	// если некуда писать и откуда читать
	if (dcmDirectoryIndex.get_path().size() == 0)
		return true;

	try
	{
		auto wstr_json_fname1 = MergePath(dcmDirectoryIndex.get_path(), index_filename_type1());
		save_to_jsons(dcmDirectoryIndex, wstr_json_fname1, index_file_type::hierarchical);
		SingleDirectoryIndex dir_index_from_json1 = load_parse_json(wstr_json_fname1,
				ErrorReportMode::throw_exception); // если проблемы чтения json файла

		auto wstr_json_fname2 = MergePath(dcmDirectoryIndex.get_path(), index_filename_type2());
		save_to_jsons(dcmDirectoryIndex, wstr_json_fname2, index_file_type::plain);
		SingleDirectoryIndex dir_index_from_json2 = load_parse_json(wstr_json_fname2,
				ErrorReportMode::throw_exception); // если проблемы чтения json файла

		bool is_equal01 = dcmDirectoryIndex == dir_index_from_json1;
		bool is_equal02 = dcmDirectoryIndex == dir_index_from_json2;
		bool is_equal12 = dir_index_from_json1 == dir_index_from_json2;
		bool res = is_equal01 && is_equal02 && is_equal12;
		return res;
	}
	catch(...)
	{
		return false;
	}
}



} // namespace Dicom

XRAD_END
