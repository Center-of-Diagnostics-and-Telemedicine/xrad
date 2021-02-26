/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
/*!
	\file
	\date 2019/09/24 10:30
	\author novik

	\brief Реализация функции для обработки json файлов и кластеризации имён

	Функции работы с json файлом вынесены с файл DicomFileIndexJson.cpp
*/
#include "pre.h"
#include "DicomClustering.h"
#include <XRADSystem/Sources/System/SystemConfig.h>
#include <fstream>
#include <map>

XRAD_BEGIN



namespace Dicom
{

	/*!
	\brief flatten json to result string

	Преобразовать древовидные структуры в "плоский" вид, в котором
	все вершины  до конечной преобразуются в список string значений.
	Используется рекурсия.

	\param reference_string [in] - список строк, для хранения предыдущих результатов в рекурсии
	\param value [in] - json объект для обработки
	\param result [out] - итоговое "плоское" представление в виде списка строк
	*/
	void json_flatten_string(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result)
	{
		switch (value.type())
		{
		case nlohmann::detail::value_t::array:
		{
			if (value.empty())
			{
				// flatten empty array as null
				//result[reference_string] = nullptr;
			}
			else
			{
				// iterate array and use index as reference string
				for (std::size_t i = 0; i < value.size(); ++i)
				{
					vector<string> reference_string1(reference_string);
					reference_string1.push_back(to_string(i));
					json_flatten_string(reference_string1, value[i], result);
				}
			}
			break;
		}

		case nlohmann::detail::value_t::object:
		{
			if (value.empty())
			{
				// flatten empty object as null
				//result[reference_string] = nullptr;
			}
			else
			{
				// iterate object and use keys as reference string
				for (const auto& element : value.items())
				{
					vector<string> reference_string1(reference_string);
					reference_string1.push_back(element.key());
					json_flatten_string(reference_string1, element.value(), result);
				}
			}
			break;
		}

		default:
		{
			// add primitive value with its reference string
			vector<string> reference_string1(reference_string);
			reference_string1.push_back(value);
			result.push_back(reference_string1);
			break;
		}

		}
	}



	/*!
	\brief parse json to list of dicom reference

	Преобразовать древовидные структуры в "плоский" вид, в котором
	все вершины  до начала массива тэгов преобразуются в список string значений.

	Используется рекурсия.

	\param reference_string [in] - список строк, для хранения предыдущих результатов в рекурсии
	\param value [in] - json объект для обработки
	\param result [out] - итоговое "плоское" представление в виде списка строк
	\param vec_json_dicom [out] - список json объектов, являющихся конечной вершиной с массивом тэгов
	*/
	void json_parse_type1(const vector<string>& reference_string,
		const json& value,
		vector<vector<string>>& result,
		vector<json>& vec_json_dicom)
	{
		switch (value.type())
		{
		case nlohmann::detail::value_t::array:
			if (!value.empty())
			{
				result.push_back(reference_string);
				vec_json_dicom.push_back(value);
			}
			break;

		case nlohmann::detail::value_t::object:
			if (!value.empty())
			{
				// iterate object and use keys as reference string
				for (const auto& element : value.items())
				{
					vector<string> reference_string1(reference_string);
					reference_string1.push_back(element.key());
					json_parse_type1(reference_string1, element.value(), result, vec_json_dicom);
				}
			}
			break;

		default: // сюда мы не должны попасть, так как 5 уникальных меток перед массивом заканчиваются раньше
			break;
		}
	}


	/*!
	\brief получить из json объекта значение тэга

	Получить из json объекта значение тэга label, скопировать его в str_tag_value.
	Проверить, что это значение имеет тип string

	\param json_obj [in] - json объект для обработки
	\param label [in] - имя тэга
	\return значение тэга
	*/
	wstring json_get_tag_string(const json & json_obj, const string & label)
	{
		auto find_tag = json_obj.find(label);
		XRAD_ASSERT_THROW(find_tag != json_obj.end());
		XRAD_ASSERT_THROW(find_tag->is_string());
		return string8_to_wstring(*find_tag);
	}

	uint64_t json_get_tag_uint(const json & json_obj, const string & label)
	{
		auto find_tag = json_obj.find(label);
		XRAD_ASSERT_THROW(find_tag != json_obj.end());
		XRAD_ASSERT_THROW(find_tag->is_number_unsigned());

		static_assert(numeric_limits<json::number_unsigned_t>::max() <= numeric_limits<uint64_t>::max(), "JSON unsigned number type is too wide.");
		return *find_tag;
	}



	/*!
	\brief получить список уникальных директорий

	\param vector_paths [in] -- список файлов.
	\return список уникальных директорий
	*/
	vector<wstring> get_unique_paths(const vector<wstring> &vector_paths)
	{
		// int n_json_type = 1;
		wstring wstr_json_fname;
		vector<wstring> vector_fname_path_unique;
		if (vector_paths.size() == 0)
			return vector_fname_path_unique;


		// вычисление уникальных директорий с помощью функции unique
		vector_fname_path_unique = vector_paths;
		// получить уникальные значения short_fname
		vector_fname_path_unique.erase(
			unique(vector_fname_path_unique.begin(), vector_fname_path_unique.end()),
			vector_fname_path_unique.end());

		return vector_fname_path_unique;
	}


	/*!
	\brief сохранить json объект в файл

	\param json_to_save [in] - json объект.
	\param report_dst [in] - имя файла для записи.

	\return false в случае неуспешного завершения
	*/
	void save_json(const json &json_to_save, const wstring &filename)
	{
		wstring native_filename = GetPathSystemRawFromGeneric(filename);
#if defined(XRAD_USE_CFILE_WIN32_VERSION)
		auto	open_filename = native_filename;
#elif defined(XRAD_USE_CFILE_UNIX_VERSION)
		auto	open_filename = convert_to_string(native_filename);
#else
		#error Unknown OS platform.
#endif
		// Использование utf8_stream здесь может привести к недоразумениям:
		// он может определять операторы <<, которые будут делать лишние преобразования,
		// но они всё равно не будут использованы, т.к. в json используется только
		// статический срез ostream&, а не фактический тип передаваемого потока.
		// Сам json выводит данные в UTF-8.
		ofstream	out_file(open_filename, ios_base::out | ios_base::binary);
		XRAD_ASSERT_THROW(out_file.is_open());

		out_file << "\xEF\xBB\xBF"; //utf-8 BOM
		out_file.width(1); // параметр потока должен быть установлен, чтобы вывод форматировался с отступами
		out_file.fill('\t'); // отступы табуляцией
		out_file << json_to_save << "\n";
		out_file.close();
	}

	/*!
	\brief загрузить json объект из файла

	\param json_loaded [out] - json объект.
	\param json_fname [in] - имя файла для загрузки.

	\return false в случае неуспешного завершения
	*/
	json load_json(const wstring &filename)
	{
		wstring native_filename = GetPathSystemRawFromGeneric(filename);
#if defined(XRAD_USE_CFILE_WIN32_VERSION)
		auto	open_filename = native_filename;
#elif defined(XRAD_USE_CFILE_UNIX_VERSION)
		auto	open_filename = convert_to_string(native_filename);
#else
		#error Unknown OS platform.
#endif
		ifstream in_file(open_filename, ios_base::in | ios_base::binary);
		XRAD_ASSERT_THROW(in_file.is_open());

		json	json_from_file;
		in_file >> json_from_file;
		return json_from_file;
	}



}  // end namespace Dicom

XRAD_END
