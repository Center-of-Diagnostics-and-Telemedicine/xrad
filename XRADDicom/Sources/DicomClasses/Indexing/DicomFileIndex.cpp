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

vector<wstring> DicomFileIndex::m_FileNameSizeTimeDiscr = { L"filename", L"size_in_bytes", L"time_write" };
map<Dicom::tag_e, std::string>	DicomFileIndex::m_dicom_tags_discription;
map<std::string, Dicom::tag_e>	DicomFileIndex::m_dicom_discription_tags;		// map<discription -> ID>


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


/// конструктор: при первом же вызове заполняет статические переменные класса
/// так как они статические - это делается единственный раз при создании первого экзкмпляра класса
DicomFileIndex::DicomFileIndex()
{
	m_DicomSource = DicomSource::no_information;
	m_bNeedIndexing = true;

	if (m_dicom_tags_discription.size() != get_dicom_tags_length()) // если описание тэгов ещё не заполнено
	{
		for (const auto& el : m_dicom_tags)
		{
			//wstring wstring_tag_name = Dicom::MakeContainer()->get_wstring(el); // уже не работает
			wstring wstring_tag_name = Dicom::get_tagname(el);
			//wstring tt = Dicom::get_tag_as_string(el) + Dicom::get_tagname(el);
			m_dicom_tags_discription[el] = convert_to_string(wstring_tag_name);
			m_dicom_discription_tags[convert_to_string(wstring_tag_name)] = el;
		}
	}


	// заполнить map типов изображений false
	for (const auto& el : map_imagetype_fix_disc)
		m_dicom_image_type[el.first] = false;

	// резервируем память для полей m_FileNameSizeTimeDiscr
	m_FileNameSizeTime.resize(m_FileNameSizeTimeDiscr.size());
};


//  содержит ли Dicom тэги
bool DicomFileIndex::is_dicom() const
{
	return (m_DicomSource == DicomSource::yes_dicom_from_json || m_DicomSource == DicomSource::yes_dicom_from_file);
}


//  имеет какой-либо тип Dicom Image Type:
bool DicomFileIndex::has_image_type() const
{
	for (const auto& el : m_dicom_image_type)
		if (el.second)
			return true;
	return false;
}


// вернуть имя файла из  m_FileNameSizeTimeDiscr
string DicomFileIndex::get_file_name() const
{
	if (m_FileNameSizeTime.size() > 0)
		return m_FileNameSizeTime[0];
	else
		return string("");
};


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
			string str_tag_discr = get_dicom_tags_discr(i);
			str_dicom += get_dicom_tags_value(i);
		}
	}

	return str_dicom;
}


// сформировать vector 3-х строк
// 1) имя файла 2) размер файла в байтах файла 3) время создания файла
bool DicomFileIndex::fill_name_size_time(const wstring& fname, const wstring &name_part)
{
	string str_size, str_date;
	if (!GetFileSizeAndModifyTime(convert_to_string(fname), str_size, str_date))
		return false;
	m_FileNameSizeTime = { convert_to_string(name_part), str_size, str_date };
	return true;
};


// заполнить поля m_dicom_tags_value для файла fname
bool DicomFileIndex::fill_filetags_from_file(const wstring &path, const wstring &name)
{
	wstring full_filename = path + wpath_separator() + name;
	if (!fill_name_size_time(full_filename, name))
	{
		m_DicomSource = DicomSource::file_not_exist;
		return false;
	}
	m_DicomSource = DicomSource::not_dicom_from_file;

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
		for (auto& el : map_imagetype_disc)
			m_dicom_image_type[el.first] = (el.second == str_type_image);
	}
	catch (std::runtime_error &) // при неустойчивой работе сети не можем считать файлы
	{
		m_DicomSource = DicomSource::exeption_loading;
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

	m_DicomSource = DicomSource::yes_dicom_from_file;

	return true;
}

bool DicomFileIndex::fill_from_fileinfo(const FileInfo & fileinfo_val)
{
	string str_size, str_date;
	struct tm* clock;

	clock = gmtime(&fileinfo_val.time_write);		// Get the last modified time and put it into the time structure

	str_date = ssprintf("%d-%02d-%02dT", clock->tm_year + 1900, clock->tm_mon, clock->tm_mday);
	str_date += ssprintf("%02d:%02d:%02dZ", clock->tm_hour, clock->tm_min, clock->tm_sec);

	str_size = ssprintf("%llu", EnsureType<unsigned long long>(fileinfo_val.size));
	m_FileNameSizeTime = { convert_to_string(fileinfo_val.filename), str_size, str_date };
	return true;
}


// проверить соответветствие данных в структуре
bool DicomFileIndex::check_consistency() const
{
	if (is_dicom())
	{
		if (m_dicom_tags_discription.size() != get_dicom_tags_length())
			return false;
		if (m_dicom_tags_value.size() != get_dicom_tags_length())
			return false;
	}
	if (m_FileNameSizeTimeDiscr.size() != m_FileNameSizeTime.size())
		return false;

	return true;
}


bool DicomFileIndex::equal_fast(const DicomFileIndex& a) const
{
	if (!check_consistency() || !a.check_consistency())
		return false;
	return m_FileNameSizeTime == a.m_FileNameSizeTime;
}


// проверка равенства двух DicomFileIndex объектов
bool DicomFileIndex::operator==(const DicomFileIndex& a) const
{
	bool t0 = is_dicom() == a.is_dicom();
	if (!t0)
		return false;
	//if (!is_dicom())	// для не dicom файла проверить только совпадение имён
	//	return get_file_name() == a.get_file_name();

	// для dicom файла
	bool t1 = m_dicom_tags_value == a.m_dicom_tags_value;
	bool t2 = m_dicom_image_type == a.m_dicom_image_type;
	bool t3 = m_FileNameSizeTime == a.m_FileNameSizeTime;
	// bool t4 = m_FileNameSizeTimeDiscr == a.m_FileNameSizeTimeDiscr;

	return (t1 && t2 && t3);
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


// получить значение m_dicom_tags_discription.at(m_dicom_tags[i])
string DicomFileIndex::get_dicom_tags_discr(size_t i) const
{
	return m_dicom_tags_discription.at(m_dicom_tags[i]);
}


// установить значение m_dicom_tags_value[m_dicom_tags[i]
void DicomFileIndex::set_dicom_tags_value(size_t i, const wstring& wstr_value)
{
	m_dicom_tags_value[m_dicom_tags[i]] = wstr_value;
}

// установить значение m_dicom_tags_discription.at(m_dicom_tags[i])
void DicomFileIndex::set_dicom_tags_discr(size_t i, const string& str_value)
{
	m_dicom_tags_discription[m_dicom_tags[i]] = str_value;
}


// установить значение DicomSource
void DicomFileIndex::set_dicomsource_type(DicomSource dicome_type)
{
	m_DicomSource = dicome_type;
}


// установить значение m_dicom_image_type[image_type] = b_value
void DicomFileIndex::set_dicom_image_type(ImageType image_type, bool b_value)
{
	m_dicom_image_type[image_type] = b_value;
}


// получить вектор ненулевых map_imagetype_disc
vector<string> DicomFileIndex::get_image_type_vector() const
{
	vector<string> vec_image_types;
	for (auto& el : m_dicom_image_type)
	{
		if (!el.second || map_imagetype_disc.find(el.first) == map_imagetype_disc.end()) // если не нашли елемент по ключу
			continue;
		vec_image_types.push_back(map_imagetype_fix_disc.at(el.first));
	}
	return vec_image_types;
}


// получить значение m_FileNameSizeTime
string DicomFileIndex::get_dicom_namesizetime_value(size_t i) const
{
	return m_FileNameSizeTime[i];
}


// получить значение m_FileNameSizeTimeDiscr
wstring DicomFileIndex::get_dicom_namesizetime_discr(size_t i) const
{
	return m_FileNameSizeTimeDiscr[i];
}


// установить значение m_FileNameSizeTime
void DicomFileIndex::set_dicom_namesizetime_value(size_t i, const string& str_value)
{
	//if (m_FileNameSizeTime.size() < i + 1)
	//	m_FileNameSizeTime.resize(i+1);
	m_FileNameSizeTime[i] = str_value;
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


// получить размер файла в байтах и времени создания файла в байтах
// https://tools.ietf.org/html/rfc3339
// 1985-04-12T23:20:50.52Z
// This represents 20 minutes and 50.52 seconds after the 23rd hour of April 12th, 1985 in UTC.
// и/или https://ru.wikipedia.org/wiki/ISO_8601
bool GetFileSizeAndModifyTime(const std::string& filename, std::string& str_size, std::string& str_date)
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
	str_size = ssprintf("%ld", EnsureType<long>(stat_buf.st_size));
	return true;
}

} //namespace Dicom

XRAD_END
