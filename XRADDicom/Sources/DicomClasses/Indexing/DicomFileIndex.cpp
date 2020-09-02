/*!
	\file
	\date 2019/10/17 13:00
	\author novik

	\brief Имплементация функционала класса DicomFileIndex - структуры для работы с тэгами одного файла

	Функции работы с json файлом вынесены в файл DicomFileIndexJson.cpp
*/
#include "pre.h"
#include "DicomFileIndex.h"

#include <XRADDicom/XRADDicom.h>

#include <XRADDicom/Sources/DicomClasses/Instances/ct_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/xray_image.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice.h>
#include <XRADDicom/Sources/DicomClasses/Instances/mr_slice_siemens.h>

#include <typeinfo>

XRAD_BEGIN

namespace Dicom
{


//!	словарь значений поля "type" в json индексе
//!	значение "type 1" соответствует иерархической записи полей по исследованиям/сериям/сборкам
//! значение "type 2" соответствует неструктурированной записи информации обо всех файлах

static const string	hierarchical_v0 = "type 1";
static const string	raw_v0 = "type 2";

string index_file_label(index_file_type ift)
{
	if(ift==index_file_type::hierarchical) return hierarchical_v0;
	if(ift==index_file_type::raw) return raw_v0;
	return "unknown";//кидать исключение при нынешней ситуации нельзя, т.к. инициализируются статические члены класса
}

index_file_type interpret_index_file_type(string s)
{
	if(s == raw_v0) return index_file_type::raw;
	if(s == hierarchical_v0) return index_file_type::hierarchical;
	return index_file_type::unknown;
}


namespace
{

// получить размер файла в байтах и времени создания файла в байтах
// https://tools.ietf.org/html/rfc3339
// 1985-04-12T23:20:50.52Z
// This represents 20 minutes and 50.52 seconds after the 23rd hour of April 12th, 1985 in UTC.
// и/или https://ru.wikipedia.org/wiki/ISO_8601
bool GetFileSizeAndModifyTime(const std::string& filename, uint64_t &size, std::string& str_date)
{
	struct stat stat_buf;
	struct tm* clock;								// create a time structure
	int rc = stat(filename.c_str(), &stat_buf);		// get the attributes of afile.txt
	//xrad::fileinfo stat_fast = xrad::GetFileInfoStruct(convert_to_wstring(filename));
	if (rc != 0)
		return false;
	clock = gmtime(&(stat_buf.st_mtime));			// Get the last modified time and put it into the time structure

													// clock->tm_year returns the year (since 1900)
													// clock->tm_mon returns the month (January = 0)
													// clock->tm_mday returns the day of the month
	str_date = ssprintf("%d-%02d-%02dT", clock->tm_year + 1900, clock->tm_mon, clock->tm_mday) +
			ssprintf("%02d:%02d:%02dZ", clock->tm_hour, clock->tm_min, clock->tm_sec);

	// TODO: Необходимо получить 64-битный размер файла.
	size = stat_buf.st_size;
	return true;
}

} // namespace

// сформировать vector элементов tag : 12 элементов, описывающих Dicom instance
vector<Dicom::tag_e>	DicomFileIndex::m_dicom_tags =
{
	//IDs
	Dicom::e_acquisition_number,			// 0
	Dicom::e_stack_id,					// 1
	Dicom::e_series_instance_uid,		// 2
	Dicom::e_study_instance_uid,		// 3
	Dicom::e_patient_id,				// 4

	Dicom::e_patient_name,				// 11

										//study IDs
	Dicom::e_study_id,					// 5
										//series number
	Dicom::e_series_number,				// 6
	Dicom::e_modality,					// 7
										//instance number
	Dicom::e_instance_number,			// 8
	Dicom::e_sop_instance_uid,			// 9
	Dicom::e_accession_number,			// 10
};  // заполнить вектор тэгов, которые будут получены из dicom файла

map<Dicom::tag_e, std::string>	DicomFileIndex::m_dicom_tags_description;
map<std::string, Dicom::tag_e>	DicomFileIndex::m_dicom_description_tags;		// map<discription -> ID>


static const map<ImageType, string> imagetype_description_class =
{
	{ ImageType::image, typeid(Dicom::image).name() },
	{ ImageType::tomogram_slice, typeid(Dicom::tomogram_slice).name() },
	{ ImageType::ct_slice, typeid(Dicom::ct_slice).name() },
	{ ImageType::xray_image, typeid(Dicom::xray_image).name() },
	{ ImageType::mr_slice, typeid(Dicom::mr_slice).name() },
	{ ImageType::mr_slice_siemens, typeid(Dicom::mr_slice_siemens).name() }
};


static const map<ImageType, string> imagetype_description_fixed =
{
	{ ImageType::image, "image" },
	{ ImageType::tomogram_slice, "tomogram_slice" },
	{ ImageType::ct_slice, "ct_slice" },
	{ ImageType::xray_image, "xray_image" },
	{ ImageType::mr_slice, "mr_slice" },
	{ ImageType::mr_slice_siemens, "mr_slice_siemens" }
};


/// конструктор: при первом же вызове заполняет статические переменные класса
/// так как они статические - это делается единственный раз при создании первого экзкмпляра класса
DicomFileIndex::DicomFileIndex()
{
	m_DicomSource = file_info_source::no_information;
	m_b_indexing_needed = true;

	if (m_dicom_tags_description.size() != get_dicom_tags_length()) // если описание тэгов ещё не заполнено
	{
		for (const auto& el : m_dicom_tags)
		{
			//wstring wstring_tag_name = Dicom::MakeContainer()->get_wstring(el); // уже не работает
			wstring wstring_tag_name = Dicom::get_tagname(el);
			//wstring tt = Dicom::get_tag_as_string(el) + Dicom::get_tagname(el);
			m_dicom_tags_description[el] = convert_to_string(wstring_tag_name);
			m_dicom_description_tags[convert_to_string(wstring_tag_name)] = el;
		}
	}


	// заполнить map типов изображений false
	for (const auto& el : imagetype_description_fixed)
	{
		m_dicom_image_type[el.first] = false;
	}
}


//  содержит ли Dicom тэги
bool DicomFileIndex::is_dicom() const
{
	return (m_DicomSource == file_info_source::dicom_from_json || m_DicomSource == file_info_source::dicom_from_file);
}


//  имеет какой-либо тип Dicom Image Type:
bool DicomFileIndex::has_image_type() const
{
	for (const auto& el : m_dicom_image_type)
	{
		if(el.second) return true;
	}
	return false;
}


// из списка тэгов файла сгенерировать строку
wstring DicomFileIndex::get_summary_info_string() const
{
	wstring str_dicom;
	if (!check_consistency())   //  отсутствуют заполненные поля
		return str_dicom;

	if (is_dicom())		// если Dicom файл, то записать Dicom тэги
	{
		// во внутренний блок пишем только индексы с [NFIELDS_TYPE_1 и до конца]
		for (size_t i = 0; i < NFIELDS_TYPE_1; i++)
		{
			string str_tag_discr = get_dicom_tags_description(i);
			str_dicom += get_dicom_tags_value(i);
		}
	}

	return str_dicom;
}


// сформировать vector 3-х строк
// 1) имя файла 2) размер файла в байтах файла 3) время создания файла
bool DicomFileIndex::fill_name_size_time(const wstring& fname, const wstring &name_part)
{
	uint64_t file_size;
	string date;
	if (!GetFileSizeAndModifyTime(convert_to_string(fname), file_size, date))
		return false;
	m_filename = name_part;
	m_file_size = file_size;
	m_file_mtime = convert_to_wstring(date);
	return true;
};


// заполнить поля m_dicom_tags_value для файла fname
bool DicomFileIndex::fill_filetags_from_file(const wstring &path, const wstring &name)
{
	wstring full_filename = path + wpath_separator() + name;
	if (!fill_name_size_time(full_filename, name))
	{
		m_DicomSource = file_info_source::file_not_exist;
		return false;
	}
	m_DicomSource = file_info_source::non_dicom_from_file;

	try
	{
		// the first way open/load data in Dicom file: Dicom::Container created from Dicom::instancestorage_file
		const Dicom::instancestorage_ptr instancestoreage_p = new Dicom::instancestorage_file(full_filename);
		Dicom::filter_t filter;

		Dicom::instance_ptr dicom_instance = Dicom::CreateInstance(instancestoreage_p,
				Dicom::dicom_instance_predicate::true_predicate());
		// создает полиморфный контейнер, опираясь на информацию о модальности в источнике Dicom

		if (!dicom_instance)
			return true;    // это не файл dicom - выйти
		if (dicom_instance->is_dicomdir())
			return true;    // это файл dicomdir - выйти

		// получить значения всех тэгов для dicom файла
		for (auto el : m_dicom_tags)
			m_dicom_tags_value[el] = dicom_instance->get_wstring(Dicom::tag_e(el));

		// заполнить map типов изображения
		string str_type_image = typeid(*dicom_instance).name();
		for (auto& el : imagetype_description_class)
			m_dicom_image_type[el.first] = (el.second == str_type_image);
	}
	catch (std::runtime_error &) // при неустойчивой работе сети не можем считать файлы
	{
		m_DicomSource = file_info_source::exeption_loading;
		return false;
	}
	//catch (std::bad_alloc) {
		// memory allocation error handling
	//}

	// заполнить иерархию наследования Image Type: если CT slice, то tomogram_slice т.д.
	{
		if (m_dicom_image_type[ImageType::mr_slice_siemens])
			m_dicom_image_type[ImageType::mr_slice] = true;

		if (m_dicom_image_type[ImageType::mr_slice] || m_dicom_image_type[ImageType::ct_slice])
			m_dicom_image_type[ImageType::tomogram_slice] = true;

		if (m_dicom_image_type[ImageType::tomogram_slice] || m_dicom_image_type[ImageType::xray_image])
			m_dicom_image_type[ImageType::image] = true;
	}

	m_DicomSource = file_info_source::dicom_from_file;

	return true;
}

void DicomFileIndex::fill_from_fileinfo(const FileInfo &fileinfo_val)
{
	m_filename = fileinfo_val.filename;
	m_file_size = fileinfo_val.size;

	struct tm* clock = gmtime(&fileinfo_val.time_write);		// Get the last modified time and put it into the time structure

	m_file_mtime = ssprintf(L"%d-%02d-%02dT", clock->tm_year + 1900, clock->tm_mon, clock->tm_mday);
	m_file_mtime += ssprintf(L"%02d:%02d:%02dZ", clock->tm_hour, clock->tm_min, clock->tm_sec);
}


// проверить соответветствие данных в структуре
bool DicomFileIndex::check_consistency() const
{
	if (is_dicom())
	{
		if (m_dicom_tags_description.size() != get_dicom_tags_length())
			return false;
		if (m_dicom_tags_value.size() != get_dicom_tags_length())
			return false;
	}

	return true;
}


bool DicomFileIndex::equal_fast(const DicomFileIndex& a) const
{
	if (!check_consistency() || !a.check_consistency())
		return false;
	if (m_file_size != a.m_file_size)
		return false;
	if (m_filename != a.m_filename)
		return false;
	if (m_file_mtime != a.m_file_mtime)
		return false;
	return true;
}


// проверка равенства двух DicomFileIndex объектов
bool DicomFileIndex::operator==(const DicomFileIndex& a) const
{
	bool t0 = is_dicom() == a.is_dicom();
	if (!t0)
		return false;
	//if (!is_dicom())	// для не dicom файла проверить только совпадение имён
	//	return get_file_name() == a.get_file_name();

	if (m_file_size != a.m_file_size)
		return false;
	if (m_filename != a.m_filename)
		return false;
	if (m_file_mtime != a.m_file_mtime)
		return false;

	// для dicom файла
	if (m_dicom_tags_value != a.m_dicom_tags_value)
		return false;
	if (m_dicom_image_type != a.m_dicom_image_type)
		return false;

	return true;
}


// получить значение m_dicom_tags[i]
Dicom::tag_e DicomFileIndex::get_dicom_tags_ID(size_t i) const
{
	return m_dicom_tags[i];
}


// получить значение m_dicom_tags_value.at(m_dicom_tags[i])
wstring DicomFileIndex::get_dicom_tags_value(size_t i) const
{
	// вставить проверку на вхождение в диапазон  и наличие поля?
	return m_dicom_tags_value.at(m_dicom_tags[i]);
}


// получить значение m_dicom_tags_description.at(m_dicom_tags[i])
string DicomFileIndex::get_dicom_tags_description(size_t i) const
{
	return m_dicom_tags_description.at(m_dicom_tags[i]);
}


// установить значение m_dicom_tags_value[m_dicom_tags[i]
void DicomFileIndex::set_dicom_tags_value(size_t i, const wstring& wstr_value)
{
	m_dicom_tags_value[m_dicom_tags[i]] = wstr_value;
}

// установить значение m_dicom_tags_description.at(m_dicom_tags[i])
void DicomFileIndex::set_dicom_tags_description(size_t i, const string& str_value)
{
	m_dicom_tags_description[m_dicom_tags[i]] = str_value;
}


// установить значение file_info_source
void DicomFileIndex::set_dicomsource_type(file_info_source dicome_type)
{
	m_DicomSource = dicome_type;
}


// установить значение m_dicom_image_type[image_type] = b_value
void DicomFileIndex::set_dicom_image_type(ImageType image_type, bool b_value)
{
	m_dicom_image_type[image_type] = b_value;
}


// получить вектор ненулевых imagetype_description
vector<string> DicomFileIndex::get_image_type_vector() const
{
	vector<string> vec_image_types;
	for (auto& el : m_dicom_image_type)
	{
		if(el.second && imagetype_description_class.find(el.first) != imagetype_description_class.end()) // если нашли элемент по ключу
		{
			vec_image_types.push_back(imagetype_description_fixed.at(el.first));
		}
	}
	return vec_image_types;
}



// работа с ImageType - определение типа
// получить значение ImageType::mr_slice
bool DicomFileIndex::get_ImageType_mr_slice() const
{
	if (m_dicom_image_type.find(ImageType::mr_slice) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::mr_slice);
}


// получить значение ImageType::mr_slice_siemens
bool DicomFileIndex::get_ImageType_mr_slice_siemens() const
{
	if (m_dicom_image_type.find(ImageType::mr_slice_siemens) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::mr_slice_siemens);
}


// получить значение ImageType::tomogram_slice
bool DicomFileIndex::get_ImageType_tomogram_slice() const
{
	if (m_dicom_image_type.find(ImageType::tomogram_slice) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::tomogram_slice);
}


// получить значение ImageType::ct_slice
bool DicomFileIndex::get_ImageType_ct_slice() const
{
	if (m_dicom_image_type.find(ImageType::ct_slice) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::ct_slice);
}


// получить значение ImageType::xray_image
bool DicomFileIndex::get_ImageType_xray_image() const
{
	if (m_dicom_image_type.find(ImageType::xray_image) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::xray_image);
}


// получить значение ImageType::image
bool DicomFileIndex::get_ImageType_image() const
{
	if (m_dicom_image_type.find(ImageType::image) == m_dicom_image_type.end())
		return false;
	return m_dicom_image_type.at(ImageType::image);
}

} //namespace Dicom

XRAD_END
