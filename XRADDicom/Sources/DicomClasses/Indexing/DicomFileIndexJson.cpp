﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2019/10/17 13:00
	\author novik

	\brief Реализация функций для создания-записи-считывания json объектов с информацией о файле
*/
#include "pre.h"
#include "DicomFileIndexJson.h"

#include "DicomClustering.h"

#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/xray_image.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice_siemens.h>

#include <XRADDicom/XRADDicom.h>

XRAD_BEGIN

namespace Dicom
{



namespace
{

const map<ImageType, string> imagetype_description_class =
{
	{ ImageType::image, typeid(Dicom::image).name() },
	{ ImageType::tomogram_slice, typeid(Dicom::tomogram_slice).name() },
	{ ImageType::ct_slice, typeid(Dicom::ct_slice).name() },
	{ ImageType::xray_image, typeid(Dicom::xray_image).name() },
	{ ImageType::mr_slice, typeid(Dicom::mr_slice).name() },
	{ ImageType::mr_slice_siemens, typeid(Dicom::mr_slice_siemens).name() }
};

const map<ImageType, string> imagetype_description_fixed =
{
	{ ImageType::image, "image" },
	{ ImageType::tomogram_slice, "tomogram_slice" },
	{ ImageType::ct_slice, "ct_slice" },
	{ ImageType::xray_image, "xray_image" },
	{ ImageType::mr_slice, "mr_slice" },
	{ ImageType::mr_slice_siemens, "mr_slice_siemens" }
};

} // namespace



// из json файла заданного типа сформировать DicomFileIndex объект
DicomFileIndex  from_json_get_file_index(const json& json_file_tag, index_file_type json_type)
{
	// проверить допустимые значения json_type
	XRAD_ASSERT_THROW(json_type == index_file_type::hierarchical ||
			json_type == index_file_type::plain)

	DicomFileIndex result;
	result.set_dicomsource_type(file_info_source::no_information);
	size_t n_fields = 3;  // 2 дополнительных поля "tags" и "image_type"

	XRAD_ASSERT_THROW(json_file_tag.size() >= n_fields)   //  число полей меньше минимального (3) - выйти

	// считать обязятельные поля FileNameSizeTimeDiscr
	result.set_filename(json_get_tag_string(json_file_tag, u8"filename"));
	auto fi_file_size = json_get_tag_uint(json_file_tag, u8"size");
	static_assert(std::is_same<decltype(fi_file_size), file_size_t>::value, "Invalid types.");
	result.set_file_size(fi_file_size);
	result.set_file_mtime(convert_to_string(json_get_tag_string(json_file_tag, u8"time_write")));

	result.set_dicomsource_type(file_info_source::non_dicom_from_json);	// на данном этапе заполнена информация об общих тэгах

	auto find_tags = json_file_tag.find("tags");
	if (find_tags == json_file_tag.end())   // если поле "tags" не найдено
	{
		return result;		//  если dicom тэгов нет - больше ничего не заполняем. Видимо, это для индексов типа 1
	}

	const json& json_tag_info = json_file_tag.at("tags");

	// считаваем Dicom тэги
	size_t start_pos = NFIELDS_TYPE_1 * (json_type == index_file_type::hierarchical);		// для type1 начинаем заполнять с позиции NFIELDS_TYPE_1
	for (size_t i = start_pos; i < result.get_dicom_tags_length(); i++)			// получаем все dicom тэги (кроме полученных ранее из древовидной структуры для type1 )
	{
		auto	label = result.get_dicom_tags_description(i);
		auto	tag = json_get_tag_string(json_tag_info, label);
		result.set_dicom_tags_value(i, tag);
	}

	// считываем с тэга "image_type" поля, описывающий типы изображений
	if (json_file_tag.find("image_type") != json_file_tag.end())						// если тэг "image_type" не найден - не заполняем
	{
		const json   &json_image_type = json_file_tag.at("image_type");

		// пытаемся считать значения image type
		if (json_image_type.is_array())													// проверяем тип array
		{
			for (size_t i = 0; i < json_image_type.size(); i++)					// для все тэгов
			{
				if (json_image_type[i].is_string()) 	// если тип элемента array не "string", ничего не делать
				{
					const auto &str_image_type = json_image_type[i];
					for(const auto &el : imagetype_description_fixed)
					{
						if(el.second == str_image_type)
						{
							result.set_dicom_image_type(el.first, true);
							break;
						}
					}
				}
			}
		}
	}
	result.set_dicomsource_type(file_info_source::dicom_from_json);

	return result;
}

// из списка тэгов файла сгенерировать json типа type1  для записи во внутренний блок
bool to_json_type1_inner_block(const DicomFileIndex& dcmFileIndex, json& json_file_tag)
{
	if (!dcmFileIndex.check_consistency())   //  отсутствуют заполненные поля
		return false;

	if (dcmFileIndex.is_dicom())		// если Dicom файл, то записать Dicom тэги
	{
		json   &json_tag_info = json_file_tag["tags"];
		// записать индексы с [NFIELDS_TYPE_1 и до конца]
		for (size_t i = NFIELDS_TYPE_1; i < dcmFileIndex.get_dicom_tags_length(); i++)
		{
			string str_tag_discr = dcmFileIndex.get_dicom_tags_description(i);
			json_tag_info[str_tag_discr] = convert_to_string8(dcmFileIndex.get_dicom_tags_value(i));
		}
		// записать dicom image типы
		vector<string> vec_image_types = dcmFileIndex.get_image_type_vector();

		if (vec_image_types.size() > 0)
			json_file_tag["image_type"] = vec_image_types;

	}

	// записать информацию о имени файла, размере и времени создания
	json_file_tag[u8"filename"] = convert_to_string8(dcmFileIndex.get_file_name());
	static_assert(numeric_limits<json::number_unsigned_t>::max() >=
			numeric_limits<decltype(dcmFileIndex.get_file_size())>::max(),
			"JSON unsigned number type is too narrow.");
	json_file_tag[u8"size"] = json::number_unsigned_t(dcmFileIndex.get_file_size());
	json_file_tag[u8"time_write"] = convert_to_string8(dcmFileIndex.get_file_mtime());

	return true;
}



// из списка тэгов файла сгенерировать json объект типа "type2"
// в tags_disciption содержаться vector описания тэгов
bool to_json_type2(const DicomFileIndex& dcmFileIndex, json& json_file_tag)
{
	// const size_t TYPE2_N = 11;
	if (!dcmFileIndex.check_consistency())   //  отсутствуют заполненные поля
		return false;


	json_file_tag[u8"filename"] = convert_to_string8(dcmFileIndex.get_file_name());
	static_assert(numeric_limits<json::number_unsigned_t>::max() >=
			numeric_limits<decltype(dcmFileIndex.get_file_size())>::max(),
			"JSON unsigned number type is too narrow.");
	json_file_tag[u8"size"] = json::number_unsigned_t(dcmFileIndex.get_file_size());
	json_file_tag[u8"time_write"] = convert_to_string8(dcmFileIndex.get_file_mtime());

	if (dcmFileIndex.is_dicom())
	{
		json json_tag_info;
		// записать dicom тэги
		for (size_t i = 0; i < dcmFileIndex.get_dicom_tags_length(); i++)
		{
			string str_tag_discr = dcmFileIndex.get_dicom_tags_description(i);
			json_tag_info[str_tag_discr] = convert_to_string8(dcmFileIndex.get_dicom_tags_value(i));
		}
		// записать dicom image типы
		vector<string> vec_image_types = dcmFileIndex.get_image_type_vector();

		if (vec_image_types.size() > 0)
			json_file_tag["image_type"] = vec_image_types;

		json_file_tag["tags"] = json_tag_info;
	}

	return true;
}



} // end namespace Dicom

XRAD_END
