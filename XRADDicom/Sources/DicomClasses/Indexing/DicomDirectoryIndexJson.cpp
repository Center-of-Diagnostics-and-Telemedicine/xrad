#include "pre.h"
#include "DicomDirectoryIndexJson.h"

#include "DicomClustering.h"
#include "DicomCatalogIndex.h"
#include "DicomFileIndexJson.h"

#include <XRADDicom/Sources/DicomClasses/instances/ct_slice.h>
#include <XRADDicom/Sources/DicomClasses/instances/xray_image.h>
#include <XRADDicom/Sources/DicomClasses/instances/mr_slice.h>
#include <XRADDicom/Sources/DicomClasses/instances/mr_slice_siemens.h>

#include <XRADDicom/XRADDicom.h>

#include <typeinfo>

/// директива для возможности тестирования записанных и считанных json объектов
#define TEST_JSON


/*!
\file
\date 2019/10/21 10:00
\author novik

\brief Реализация функций для создания-записи-считывания json объектов с информацией о файлах в директории
*/

XRAD_BEGIN

namespace Dicom
{

static const map<string, string> map_header_json_type1 =
{
	{ "ID","purpose: Dicom catalog" },
	{ "version","0.0" },
	{ "type","type 1" }
};

static const map<string, string> map_header_json_type2 =
{
	{ "ID","purpose: Dicom catalog" },
	{ "version","0.0" },
	{ "type","type 2" }
};

// из списка тэгов для каждого файла сгенерировать json файл type1 (sample1.json)
void dir_info_to_json_type1(const DicomDirectoryIndex& dcmDirectoryIndex, json& json_type1)
{
	// выделить уникальные исследования по 5-ти первым тэгам
	vector<wstring> list_tags;
	json json_dicom_files;					// хранение информации о файлах в древовидной структуре первых 5-тэгов

	for (const auto& file_tags : dcmDirectoryIndex.m_FilesIndex)
	{
		if (!file_tags.is_dicom())
			continue;
		json json_inner_file_tag;
		if (to_json_type1_inner_block(file_tags, json_inner_file_tag))		// записать все тэги, характеризующие дайком файл во внутренний блок
		{
			vector<size_t> n_Size_Tages(NFIELDS_TYPE_1, 0);
			vector<string> vec_tag_names(NFIELDS_TYPE_1);
			json *refInnerJson = &json_dicom_files;
			for (size_t i = NFIELDS_TYPE_1; i-- > 0;)			// в json файл заполнить поля об первых 5 тэгах, характеризующих исследование
			{
				auto wstr_tag_value = file_tags.get_dicom_tags_value(i);
				string str_tag_value = convert_to_string8(wstr_tag_value);
				refInnerJson = &(*refInnerJson)[str_tag_value];
				n_Size_Tages[NFIELDS_TYPE_1 - 1 - i] = refInnerJson->size();
				vec_tag_names[NFIELDS_TYPE_1 - 1 - i] = str_tag_value;
			}
			refInnerJson->push_back(json_inner_file_tag);
		}
	}

	// записать древовидную информацию dicom файлав в тэг "dicomlist"
	if (json_dicom_files.size())                     // если не пусто
		json_type1["dicomlist"] = json_dicom_files;

	// заполнить обязательные поля о "ID", "version", "type" из map_header_json_type1
	for (auto& map_v : map_header_json_type1)
		json_type1[get<0>(map_v)] = get<1>(map_v);

	// записать "non Dicom" файлы, это идентично записи в json type 2
	{
		json json_not_dicom;
		// заполнить информацию о не dicom файлах полях
		for (auto &dicom_file_Tags: dcmDirectoryIndex.m_FilesIndex)
		{
			if (dicom_file_Tags.is_dicom())
				continue;
			json json_file_tag;
			// записать все тэги, характеризующие дайком файл во внутренний блок
			if (to_json_type1_inner_block(dicom_file_Tags, json_file_tag))
				json_not_dicom.push_back(std::move(json_file_tag));
		}
		if (json_not_dicom.size() > 0)
			json_type1["non Dicom"] = std::move(json_not_dicom);
	}

}



// из списка тэгов для каждого файла сгенерировать json файл type2 (sample2.json)
void dir_info_to_json_type2(const DicomDirectoryIndex& dcmDirectoryIndex, json& json_type2)
{
	if (!dcmDirectoryIndex.m_FilesIndex.size())
		return;

	// заполнить обязательные поля о "ID", "version", "type" из map_header_json_type1
	for (auto& map_v : map_header_json_type2)
		json_type2[get<0>(map_v)] = get<1>(map_v);

	// вначале записать dicom файлы
	for (auto &dicom_file_Tags: dcmDirectoryIndex.m_FilesIndex)
	{
		if (!dicom_file_Tags.is_dicom())
			continue;
		json json_file_tag;
		if (to_json_type2(dicom_file_Tags, json_file_tag))	 // записать все тэги, характеризующие дайком файл
			//json_type2["filelist"][json_type2["filelist"].size()] = json_file_tag;
			json_type2["filelist"].push_back(json_file_tag);
	}
	// записать не dicom файлы
	for (auto &dicom_file_Tags: dcmDirectoryIndex.m_FilesIndex)
	{
		if (dicom_file_Tags.is_dicom())
			continue;
		json json_file_tag;
		if (to_json_type2(dicom_file_Tags, json_file_tag))	 // записать все тэги, характеризующие дайком файл
			json_type2["filelist"].push_back(json_file_tag);
	}
}


// загрузить json файл с деревом исследований Dicom файлов type 1
bool load_json_type1_tree(DicomDirectoryIndex& dcmDirectoryIndex, json& json_dicom_files)
{
	if (json_dicom_files.is_null())   // проверить, есть ли информация в json объекте
		return false;

	//auto begin = std::chrono::high_resolution_clock::now();
	vector<string> reference_string;
	vector<vector<string>> flatten_result;
	vector<json> vec_json_dicom;
	json_parse_type1(reference_string, json_dicom_files, flatten_result, vec_json_dicom);

	//auto end1 = std::chrono::high_resolution_clock::now();
	//ShowText(L"Working time", ssprintf("%.1g ms \n %d number of fields \n ",
	//	1.*(end1-begin)/1000/1000, flatten_result.size() ));

	// для считывания этой древовидной структуры
	if (flatten_result.size() == 0)  // если парсинг дал пустой результат
		return false;

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
			DicomFileIndex fileindex_from_json = fileindex_base;
			//if (from_json_type1_inner_block(fileindex_from_json, el)) // взять остальные поля
			if (from_json_get_file_index(fileindex_from_json, el, 1) && fileindex_from_json.is_dicom()) // взять остальные поля
				dcmDirectoryIndex.add_file_index(std::move(fileindex_from_json) );  // после функции move объект fileindex_from_json уже не хранит информации
		}

	}

	return true;
}

// загрузить json файл, содержащий информацию об файлах, в структуру DicomDirectoryIndex& dcmDirectoryIndex
bool	 load_parse_json(DicomDirectoryIndex& dcmDirectoryIndex, const wstring& json_fname)
{
	json json_from_file;
	if (!load_json(json_from_file, json_fname))
		return false;

	for (auto& map_v : map_header_json_type1)  // проверить наличие обязательных полей = { "ID", "version", "type" };
		if (json_from_file.find(map_v.first) == json_from_file.end())		 // если обязательное поле отсутствует, то выйти
			return false;

	// проверить тип json файла
	size_t json_type = json_from_file["type"] == "type 2" ? 2 : json_from_file["type"] == "type 1" ? 1 : 0;
	if (json_type == 0)    // если неподдерживаемый тип
		return false;

	// json файл тип type 1
	if (json_type == 1)
	{
		if (json_from_file.find("dicomlist") != json_from_file.end())		// если обязательное поле "dicom" отсутствует, то выйти
		{
			if (!json_from_file["dicomlist"].is_null())
			{
				json& json_dicom_files = json_from_file.at("dicomlist");				// хранение информации о файлах в древовидной структуре первых 5-тэгов
				if (!load_json_type1_tree(dcmDirectoryIndex, json_dicom_files))			// загрузить json объект с древовидной структурой первых 5-тэгов
					return false;
			}
		}
	}

	// json файл тип type 1 или 2
	const string filelist[2] = { "non Dicom", "filelist" };
	// if (json_type == 1 || json_type == 2)   // это условие уже гарантировано проверками выше в этой функции
	{
		string tag_name = filelist[json_type - 1];
		if (json_from_file.find(tag_name) == json_from_file.end())		 // если обязательное поле отсутствует, то выйти
			return false;
		const json& json_filelist = *json_from_file.find(tag_name);
		// из поле "filelist", считать всю инф-цию о dicom файлах
		for (size_t i = 0; i < json_filelist.size(); i++)
		{
			DicomFileIndex fileindex_from_json;
			const json& json_file_tag = json_filelist[i];
			//if (from_json_type2(fileindex_from_json, json_file_tag))
			if (from_json_get_file_index(fileindex_from_json, json_file_tag, 2))
				dcmDirectoryIndex.add_file_index(fileindex_from_json);
		}
	}
	dcmDirectoryIndex.set_path(RemoveTrailingPathSeparator(file_path(json_fname)));
	return true;
}


// записать json файл,
wstring save_to_jsons(const DicomDirectoryIndex& dcmDirectoryIndex, JsonType json_type)
{

	// для каждого уникального кластера (директории с dicom файлами) сформировать json объект
	// json объект формируется согласно примерам образцов json файлов: sample1.json и sample2.json
	wstring wstr_json_fname;
	if (!dcmDirectoryIndex.m_FilesIndex.size())
		return wstr_json_fname; // TODO: Сделать удаление файла, если нет индексируемого содержимого.

	json json_to_save;
	switch (json_type)
	{
		case JsonType::type_1:
			dir_info_to_json_type1(dcmDirectoryIndex, json_to_save);
			wstr_json_fname = dcmDirectoryIndex.get_path() + wpath_separator() +
					j_name() + L"1." + j_extension();
			break;
		case JsonType::type_2:
			dir_info_to_json_type2(dcmDirectoryIndex, json_to_save);
			wstr_json_fname = dcmDirectoryIndex.get_path() + wpath_separator() +
					j_name() + L"2." + j_extension();
			break;
		default:
			return wstr_json_fname;
	}
	// сохранить dicom файл  в текущей уникальной директории
	//ShowText(L"Информация о записи dicom файла", ssprintf("%d число тэгов \nимя файла: \n%s", json_to_save.size(),
	//	convert_to_string(wstr_json_fname).c_str(), false)  );

	save_json(json_to_save, wstr_json_fname);
	return wstr_json_fname;
}



// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
bool test_write_load_json(DicomDirectoryIndex& dcmDirectoryIndex)
{
	// если некуда писать и откуда читать
	if (dcmDirectoryIndex.get_path().size() == 0)
		return true;

	DicomDirectoryIndex dir_index_from_json1;
	wstring wstr_json_fname1 = save_to_jsons(dcmDirectoryIndex, JsonType::type_1);
	if (!load_parse_json(dir_index_from_json1, wstr_json_fname1)) // если проблемы чтения json файла
		return false;
	DicomDirectoryIndex dir_index_from_json2;
	wstring wstr_json_fname2 = save_to_jsons(dcmDirectoryIndex, JsonType::type_2);
	if (!load_parse_json(dir_index_from_json2, wstr_json_fname2)) // если проблемы чтения json файла
		return false;

	bool is_equal01 = dcmDirectoryIndex.is_equal(dir_index_from_json1);
	bool is_equal02 = dcmDirectoryIndex.is_equal(dir_index_from_json2);
	bool is_equal12 = dir_index_from_json1.is_equal(dir_index_from_json2);
	bool res = is_equal01 && is_equal02 && is_equal12;
	//if (!res)
	//	ShowText(L"1", ssprintf("writeload res = %d\n file %s", res, convert_to_string(wstr_json_fname1).c_str()));

	return res;
}


// проверить актуальность инф-ции быстрого сканирования в DicomDirectoryIndex и информации из json файла m_filename_json
// на совпадание имена, размер, время создания
void check_actuality_json(DicomDirectoryIndex& dcmDirectoryIndex)
{
	//dcmDirectoryIndex.set_need_indexing(true);	// изначально предполагается необходимость индексации, пока не доказано обратное
	const wstring& json_name = dcmDirectoryIndex.get_path_json_2();
	// если json файл пуст (отсутствует)
	if (json_name.empty())
		return;

	DicomDirectoryIndex loaded_index;
	if (!load_parse_json(loaded_index, json_name)) // если проблемы чтения json файла
		return;

	dcmDirectoryIndex.set_need_indexing(false);	// предполагается отсутствие необходимости индексации, пока не доказано обратное

	// проверить актуальность информации о файлах
	for (auto& inform_fromfilelist : dcmDirectoryIndex.m_FilesIndex) // для каждого файла  DicomCatalogIndex
	{
		bool found = false;
		// TODO: Переделать цикл на поиск в map по ключу.
		auto filename = inform_fromfilelist.get_file_name();
		for (const auto& loaded_file_data : loaded_index.m_FilesIndex)  // для каждого файла, загруженных из json файла
		{
			if (loaded_file_data.get_file_name() == filename)
			{
				found = true;
				if (loaded_file_data.equal_fast(inform_fromfilelist))
				{
					if (loaded_file_data.is_dicom()) // скопировать dicom тэги из json файла
						inform_fromfilelist = loaded_file_data;
					inform_fromfilelist.set_need_indexing(false);
				}
				else
				{
					dcmDirectoryIndex.set_need_indexing(true);
					inform_fromfilelist.set_need_indexing(true);
				}
				break;
			}
		}
		if (!found) // если не найдено совпадения для данного файла
		{
			dcmDirectoryIndex.set_need_indexing(true);
			inform_fromfilelist.set_need_indexing(true);
		}
	}
}

}  // end namespace Dicom

XRAD_END
