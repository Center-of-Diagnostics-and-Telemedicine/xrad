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

#include <typeinfo>

XRAD_BEGIN

namespace Dicom
{

	static const map<ImageType, string> map_imagetype_disc =
	{
		{ ImageType::image, typeid(Dicom::image).name() },
		{ ImageType::tomogram_slice, typeid(Dicom::tomogram_slice).name() },
		{ ImageType::ct_slice, typeid(Dicom::ct_slice).name() },
		{ ImageType::xray_image, typeid(Dicom::xray_image).name() },
		{ ImageType::mr_slice, typeid(Dicom::mr_slice).name() },
		{ ImageType::mr_slice_siemens, typeid(Dicom::mr_slice_siemens).name() }
	};


	static const map<ImageType, string> map_imagetype_fix_disc =
	{
		{ ImageType::image, "image" },
		{ ImageType::tomogram_slice, "tomogram_slice" },
		{ ImageType::ct_slice, "ct_slice" },
		{ ImageType::xray_image, "xray_image" },
		{ ImageType::mr_slice, "mr_slice" },
		{ ImageType::mr_slice_siemens, "mr_slice_siemens" }
	};



	// из json файла заданного типа сформировать DicomFileIndex объект
	bool from_json_get_file_index(DicomFileIndex& dcmFileIndex, const json& json_file_tag, const int json_type)
	{
		if (json_type != 1 && json_type != 2)		// проверить допустимые значения json_type
			return false;

		dcmFileIndex.set_dicomsource_type(DicomSource::no_information);
		size_t n_fields = dcmFileIndex.get_dicom_namesizetime_length();  // 2 дополнительных поля "tags" и "image_type"

		if (json_file_tag.size() < n_fields )   //  число полей меньше минимального (3) - выйти
			return false;

		// считать обязятельные поля FileNameSizeTimeDiscr
		for (size_t j = 0; j < dcmFileIndex.get_dicom_namesizetime_length(); j++) // получить информацию о имени файла, размере и времени создания
		{
			string str_tag_discr = convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j));
			string str_tag_value;
			if (!json_get_tag_value(json_file_tag, str_tag_discr, str_tag_value))
				return false;
			dcmFileIndex.set_dicom_namesizetime_value(j, string8_to_string(str_tag_value));
		}

		dcmFileIndex.set_dicomsource_type(DicomSource::not_dicom_from_json);	// на данном этапе заполнена информация об общих тэгах

		auto find_tags = json_file_tag.find("tags");
		if (find_tags == json_file_tag.end())   // если поле "tags" не найдено
		{
			return true;		//  если dicom эгов нет - больше ничего не заполняем
		}


		const json& json_tag_info = json_file_tag.at("tags");

		// считаваем Dicom тэги
		size_t start_pos = NFIELDS_TYPE_1 * (json_type == 1);								// для type1 начинаем заполнять с позиции NFIELDS_TYPE_1
		for (size_t i = start_pos; i < dcmFileIndex.get_dicom_tags_length(); i++)			// получаем все dicom тэги (кроме полученных ранее из древовидной структуры для type1 )
		{
			string str_tag_discr = dcmFileIndex.get_dicom_tags_discr(i);
			string str_tag_value;
			if (!json_get_tag_value(json_tag_info, str_tag_discr, str_tag_value))
				return false;																// если обязательный dicom тэг отсутствует, выйти
			dcmFileIndex.set_dicom_tags_value(i, convert_to_wstring(string8_to_string(str_tag_value)));
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
					if (!json_image_type[i].is_string())
						continue;															// если тип элемента array не "string", ничего не делать
					const auto &str_image_type = json_image_type[i];
					for (const auto &el : map_imagetype_fix_disc)
					{
						if (el.second == str_image_type)
						{
							dcmFileIndex.set_dicom_image_type(el.first, true);
							break;
						}
					}
				}
			}
		}
		dcmFileIndex.set_dicomsource_type(DicomSource::yes_dicom_from_json);

		return true;
	}


	// из json файла type1 сформировать DicomFileIndex объект
	bool from_json_type1_inner_block(DicomFileIndex& dcmFileIndex, const json& json_file_tag)
	{
		dcmFileIndex.set_dicomsource_type(DicomSource::no_information);
		size_t n_fields = dcmFileIndex.get_dicom_namesizetime_length() + 1 + 1;  // 2 дополнительных поля "tags" и "image_type"
																   // const size_t TYPE2_N = 11;
		if (json_file_tag.size() != n_fields && json_file_tag.size() != n_fields - 1)   //  число полей не равно ожидаемому: image_type может отсутствовать
			return false;

		// считать поля FileNameSizeTimeDiscr
		//dcmFileIndex.m_FileNameSizeTime.resize(dcmFileIndex.get_dicom_namesizetime_length());
		for (size_t j = 0; j < dcmFileIndex.get_dicom_namesizetime_length(); j++) // получить информацию о имени файла, размере и времени создания
		{
			string str_tag_discr = convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j));
			string str_tag_value;
			if (!json_get_tag_value(json_file_tag, str_tag_discr, str_tag_value))
				return false;
			dcmFileIndex.set_dicom_namesizetime_value(j, string8_to_string(str_tag_value));
		}

		if (json_file_tag.find("tags") == json_file_tag.end())  // если поле "tags" не найдено - выйти, в type 1 json этот тэг обязателен
			return false;
		const json& json_tag_info = json_file_tag.at("tags");

		// считаваем Dicom тэги
		for (size_t i = NFIELDS_TYPE_1; i < dcmFileIndex.get_dicom_tags_length(); i++)				// для всеx тэгов кроме полученных ранее из древовидной структуры
		{
			string str_tag_discr = dcmFileIndex.get_dicom_tags_discr(i);
			string str_tag_value;
			if (!json_get_tag_value(json_tag_info, str_tag_discr, str_tag_value))
				return false;
			dcmFileIndex.set_dicom_tags_value(i, convert_to_wstring(str_tag_value));
		}


		// считываем с тэга "image_type" поля, описывающий типы изображений
		if (json_file_tag.find("image_type") != json_file_tag.end())  // если тэг "image_type" не найден - не заполняем
		{
			const json   &json_image_type = json_file_tag.at("image_type");

			// пытаемся считать значения image type
			if (json_image_type.size() > 0 && json_image_type.is_array())					// проверяем наличие поля и тип array
			{
				for (size_t i = 0; i < json_image_type.size(); i++)				// для все тэгов
				{
					if (!json_image_type[i].is_string())
						continue;														// если тип элемента array не string, ничего не делать
					auto &str_image_type = json_image_type[i];
					for (auto &el : map_imagetype_fix_disc)
					{
						if (el.second == str_image_type)
						{
							dcmFileIndex.set_dicom_image_type(el.first, true);
							break;
						}
					}
				}
			}
		}
		dcmFileIndex.set_dicomsource_type(DicomSource::yes_dicom_from_json);

		return true;
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
				string str_tag_discr = dcmFileIndex.get_dicom_tags_discr(i);
				json_tag_info[str_tag_discr] = convert_to_string8(dcmFileIndex.get_dicom_tags_value(i));
			}
			// записать dicom image типы
			vector<string> vec_image_types = dcmFileIndex.get_image_type_vector();

			if (vec_image_types.size() > 0)
				json_file_tag["image_type"] = vec_image_types;

		}

		// записать информацию о имени файла, размере и времени создания
		for (size_t j = 0; j < dcmFileIndex.get_dicom_namesizetime_length(); j++)
		{
			json_file_tag[convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j))] =
					convert_to_string8(dcmFileIndex.get_dicom_namesizetime_value(j));
		}

		return true;
	}


	// сгенерировать json объект типа "type1"
	bool to_json_type1(const DicomFileIndex& dcmFileIndex, json& json_unique_type1)
	{
		if (!dcmFileIndex.check_consistency())   //  отсутствуют заполненные поля
			return false;
		json json_filelist, json_filelist_dicom, json_tmp;

		if (dcmFileIndex.is_dicom())	// если Dicom файл
		{
			json json_file_tag;

			// записать все тэги, характеризующие дайком файл во внутренний блок
			if (to_json_type1_inner_block(dcmFileIndex, json_file_tag))
				json_filelist_dicom[json_filelist_dicom.size()] = json_file_tag;

			for (size_t i = 0; i < NFIELDS_TYPE_1; i++)			// в json файл заполнить дерево полей об первых 5 тэгах, характеризующих исследование
			{
				auto str_tag_value = dcmFileIndex.get_dicom_tags_value(i);
				json_tmp[convert_to_string8(str_tag_value.c_str())] = json_filelist_dicom;
				json_filelist_dicom = json_tmp;
				json_tmp.clear();
			}

		}

		json_unique_type1 = json_filelist_dicom;
		return true;
	}



	// из списка тэгов файла сгенерировать json объект типа "type2"
	// в tags_disciption содержаться vector описания тэгов
	bool to_json_type2(const DicomFileIndex& dcmFileIndex, json& json_file_tag)
	{
		// const size_t TYPE2_N = 11;
		if (!dcmFileIndex.check_consistency())   //  отсутствуют заполненные поля
			return false;


		for (size_t j = 0; j < dcmFileIndex.get_dicom_namesizetime_length(); j++) // записать информацию о имени файла, размере и времени создания
		{
			json_file_tag[convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j))] =
					convert_to_string8(dcmFileIndex.get_dicom_namesizetime_value(j));
		}

		if (dcmFileIndex.is_dicom())
		{
			json json_tag_info;
			// записать dicom тэги
			for (size_t i = 0; i < dcmFileIndex.get_dicom_tags_length(); i++)
			{
				string str_tag_discr = dcmFileIndex.get_dicom_tags_discr(i);
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


	// из json файла сформировать DicomFileIndex объект
	bool from_json_type2(DicomFileIndex& dcmFileIndex, const json& json_file_tag)
	{
		dcmFileIndex.set_dicomsource_type(DicomSource::no_information);
		size_t n_fields = dcmFileIndex.get_dicom_namesizetime_length();
		// const size_t TYPE2_N = 11;
		if (json_file_tag.size() != n_fields && json_file_tag.size() != n_fields + 1 && json_file_tag.size() != n_fields + 2)   //  число полей не равно ожидаемому: 2 дополнительных поля
			return false;

		// считать поля FileNameSizeTimeDiscr
		//dcmFileIndex.m_FileNameSizeTime.resize(dcmFileIndex.get_dicom_namesizetime_length());
		for (size_t j = 0; j < dcmFileIndex.get_dicom_namesizetime_length(); j++) // получить информацию о имени файла, размере и времени создания
		{
			string str_tag_discr = convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j));
			if (json_file_tag.find(str_tag_discr) == json_file_tag.end())
			{
				//json_file_tag.clear();
				return false;
			}
			string str_tag_value = json_file_tag[convert_to_string(dcmFileIndex.get_dicom_namesizetime_discr(j))];
			dcmFileIndex.set_dicom_namesizetime_value(j, string8_to_string(str_tag_value));
		}

		if (json_file_tag.size() == n_fields)
		{
			dcmFileIndex.set_dicomsource_type(DicomSource::not_dicom_from_json);
			return true;
		}

		// считываем с поля "tags" поля, описывающий dicom файл
		if (json_file_tag.find("tags") == json_file_tag.end())  // если поле "tags" не найдено - выйти
			return false;
		const json   &json_tag_info = json_file_tag["tags"];

		// пытаемся считать Dicom тэги
		if (json_tag_info.size() == dcmFileIndex.get_dicom_tags_length())					// проверяем число полей
		{
			for (size_t i = 0; i < dcmFileIndex.get_dicom_tags_length(); i++)				// для все тэгов
			{
				string str_tag_discr = dcmFileIndex.get_dicom_tags_discr(i);
				if (json_tag_info.find(str_tag_discr) == json_tag_info.end())
					return false;
				string str_tag_value = json_tag_info[str_tag_discr];
				dcmFileIndex.set_dicom_tags_value(i, convert_to_wstring(string8_to_string(str_tag_value)));
			}
		}

		// считываем с тэга "image_type" поля, описывающий типы изображений
		if (json_file_tag.find("image_type") != json_file_tag.end())				// если тэг "image_type" не найден - не заполняем
		{
			const json   &json_image_type = json_file_tag["image_type"];

			// пытаемся считать Dicom тэги
			if (json_image_type.size() > 0 && json_image_type.is_array())					// проверяем наличие поля и тип array
			{

				for (size_t i = 0; i < json_image_type.size(); i++)				// для всех тэгов
				{
					auto &str_image_type = json_image_type[i];
					for (auto &el : map_imagetype_fix_disc)
					{
						if (el.second == str_image_type)
							dcmFileIndex.set_dicom_image_type(el.first, true);
					}
				}
			}
		}

		dcmFileIndex.set_dicomsource_type(DicomSource::yes_dicom_from_json);
		return true;
	}

} // end namespace Dicom

XRAD_END
