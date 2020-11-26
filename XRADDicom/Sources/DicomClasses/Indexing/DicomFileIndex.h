#ifndef DicomFileIndex_h__
#define DicomFileIndex_h__
/*!
	\file
	\date 2019/10/17 13:00
	\author novik
	\brief Объявление функций и полей класса DicomFileIndex

	Класс DicomFileIndex  предназначен для обработки одного файла.
*/

#include <XRADDicom/Sources/DicomClasses/tags_enum.h>
#include <XRADSystem/Sources/System/FileSystem.h>

XRAD_BEGIN

namespace Dicom
{

//!	типы индексных файлов, связанные с полем "type" в json индексе
//!	значения определяются словарем в cpp file.

enum class index_file_type
{
	unknown = 0,
	hierarchical = 1,
	raw = 2
};

//! связь между словарем типов индекса и значениями index_file_type
string	index_file_label(index_file_type ift);
index_file_type interpret_index_file_type(string s);



/// число полей, включаемых в древовидную структуру json файла index_file_type::hierarchical
const size_t NFIELDS_TYPE_1 = 5;

//! An enum: признак принадлежности информации к Dicom файлу и определяет источник информации.
/*! More detailed enum description. */
//	класс назывался DicomSource. Переименован, т.к. путается с Dicom::source
enum class file_info_source
{
	//! файл не найден, проверяется функцией, возвращающей размер файла (-1)
	file_not_exist = 0,
	//! получено исключение, когда информация считывалась с диска, одна из возможных причин - нестабильность локальной сети
	exeption_loading,
	//! неизвестно, Dicom это файл или нет
	no_information,
	//! не Dicom файл, информация считывалась с json файла
	non_dicom_from_json,
	//! Dicom файл, информация считывалась с json файла
	dicom_from_json,
	//! не Dicom файл, информация считывалась с диска
	non_dicom_from_file,
	//! Dicom файл, информация считывалась с диска
	dicom_from_file
};

//! An enum: тип Dicom файла (CreateInstance), в данном случае тип "Image" несёт расширенный смысл, характеризующий Dicom файлы
enum class ImageType
{
	image = 0,			///< базовый тип
	tomogram_slice,		///< базовый тип для томограммы
	ct_slice,			///< файл содержит изображение среза томограммы
	xray_image,			///< файл содержит xray изображение
	mr_slice,			///< файл содержит изображение среза МРТ
	mr_slice_siemens,	///< файл содержит изображение среза МРТ с устройства Siemens
};



/*!
	\brief Класс для обработки и хранения информации об одном файле
*/
class DicomFileIndex
{
	/// вектор ID тэгов, которые нужно получить из dicom файла
	/// переменная является статической, так как одинакова для всех экземпляров класса
	static const vector<Dicom::tag_e>	m_dicom_tags;

	/// map ID -> discription, краткое описание каждого ID тэга
	/// переменная является статической, так как одинакова для всех экземпляров класса
	static map<Dicom::tag_e, std::string>	m_dicom_tags_description;

	/// map discription -> ID, ID для каждого краткого описания
	/// переменная является статической, так как одинакова для всех экземпляров класса
	static map<std::string, Dicom::tag_e>	m_dicom_description_tags;

	/// map ID -> value, значение по ID
	map<size_t, wstring>				m_dicom_tags_value;

	/// map ImageType -> bool
	map<ImageType, bool>				m_dicom_image_type;

	wstring m_filename;
	uint64_t m_file_size;
	string m_file_mtime;

	/// признак file_info_source
	file_info_source							m_DicomSource;

public:
	DicomFileIndex();

	void	append(const DicomFileIndex &other)
	{
		for(auto &tag : other.m_dicom_tags_value) m_dicom_tags_value[tag.first] = tag.second;
		for(auto &image_type : other.m_dicom_image_type) m_dicom_image_type[image_type.first] |= image_type.second;
		if(m_filename.empty()) m_filename = other.m_filename;
		if(m_file_mtime.empty()) m_file_mtime = other.m_file_mtime;
		if(!m_file_size) m_file_size = other.m_file_size;
	}

	/// содержит ли класс Dicom тэги
	bool is_dicom() const;

	/// имеет какой-либо тип Dicom Image Type
	bool has_image_type() const;

	/// получить строку имя файла
	wstring get_file_name() const { return m_filename; }

	/// получить размер файла (m_file_size)
	uint64_t get_file_size() const { return m_file_size; }

	/// получить время модификации файла (m_file_mtime)
	string get_file_mtime() const { return m_file_mtime; }

	static string FormatTime(const time_t *t);

	/// заполнить поля DicomFileTagsValue и DicomFileTags для заданного файла
	/// todo: собирать диагностическую информацию о процессе заполнения для обработки её в классе DicomDirectoryIndex
	bool fill_filetags_from_file(const wstring &path, const wstring &name);

	/// получить размер файла в байтах, получить строку времени создания файла в байтах
	/// \param fname [in] полное имя файла
	/// \return успех или нет
	bool fill_name_size_time(const wstring& fname, const wstring &name_part);

	/// проверить соответветствие данных друг другу
	bool check_consistency() const;

	/// проверка равенства двух DicomFileIndex объектов
	bool operator== (const DicomFileIndex& a) const;

	void set_filename(const wstring &filename) { m_filename = filename; }
	void set_file_size(uint64_t file_size) { m_file_size = file_size; }
	void set_file_mtime(const string &file_mtime) { m_file_mtime = file_mtime; }



	//!	\name функции для работы с характеристикой ImageType
	//! @{
	Dicom::tag_e get_dicom_tags_ID(size_t i) const;					///< получить значение m_dicom_tags[i]
	wstring get_dicom_tags_value(size_t i) const;						///< получить значение m_dicom_tags_value.at(m_dicom_tags[i])
	string get_dicom_tags_description(size_t i) const;						///< получить значение m_dicom_tags_disciption(m_dicom_tags[i])
	size_t get_dicom_tags_length() const							///< длина вектора m_dicom_tags
	{
		return m_dicom_tags.size();
	};
	void set_dicom_tags_value(size_t i, const wstring& wstr_value);	///< установить значение m_dicom_tags_value.at(m_dicom_tags[i])
	void set_dicom_tags_description(size_t i, const string& str_value);		///< установить значение m_dicom_tags_disciption(m_dicom_tags[i])
	void set_dicomsource_type(file_info_source dicome_type);					///< установить значение DicomSource
	void set_dicom_image_type(ImageType image_type, bool b_value);	///< установить значение m_dicom_image_type[image_type] = b_value
	vector<string> get_image_type_vector() const;					///< получить вектор ненулевых map_imagetype_disc
	//! @}

	//! \name функции для работы с характеристикой ImageType
	//! @{
	bool get_ImageType_tomogram_slice() const;						///< получить значение ImageType::tomogram_slice
	bool get_ImageType_ct_slice() const;							///< получить значение ImageType::ct_slice
	bool get_ImageType_mr_slice() const;							///< получить значение ImageType::mr_slice
	bool get_ImageType_mr_slice_siemens() const;					///< получить значение ImageType::mr_slice_siemens
	bool get_ImageType_xray_image() const;							///< получить значение ImageType::xray_image
	bool get_ImageType_image() const;								///< получить значение ImageType::image
	//! @}

};


} //namespace Dicom

XRAD_END



#endif // DicomFileIndex_h__
