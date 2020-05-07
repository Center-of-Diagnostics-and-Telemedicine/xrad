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

/// число полей, включаемых в древовидную структуру json файла type 1
const size_t NFIELDS_TYPE_1 = 5;

//! An enum: признак принадлежности информации к Dicom файлу и определяет источник информации.
/*! More detailed enum description. */
enum class DicomSource
{
	/*!<файл не найден, проверяется функцией, возвращающей размер файла (-1)  */
	file_not_exist = 0,
	/*!<получено исключение, когда информация считывалась с диска, одна из возможных причин - нестабильность локальной сети */
	exeption_loading,
	/*!<не известно Dicom это файл или нет  */
	no_information,
	/*!<не Dicom файл, информация считывалась с json файла  */
	not_dicom_from_json,
	/*!<Dicom файл, информация считывалась с json файла  */
	yes_dicom_from_json,
	/*!<не Dicom файл, информация считывалась с диска  */
	not_dicom_from_file,
	/*!<Dicom файл, информация считывалась с диска  */
	yes_dicom_from_file,
};

	/// An enum: тип Dicom файла (CreateInstance), в данном случае тип "Image" несёт расширенный смысл, характеризующий Dicom файлы
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
	\brief класс для обработки и хранения инф-ции об одном файле

	*/
	class DicomFileIndex
	{
		/// вектор ID тэгов, которые нужно получить из dicom файла
		/// переменная является статической, так как одинакова для всех экземпляров класса
		static vector<Dicom::tag_e>	m_dicom_tags;

		/// map ID -> discription, краткое описание каждого ID тэга
		/// переменная является статической, так как одинакова для всех экземпляров класса
		static map<Dicom::tag_e, std::string>	m_dicom_tags_discription;

		/// map discription -> ID, ID для каждого краткого описания
		/// переменная является статической, так как одинакова для всех экземпляров класса
		static map<std::string, Dicom::tag_e>	m_dicom_discription_tags;

		/// map ID -> value, значение по ID
		map<size_t, wstring>				m_dicom_tags_value;

		/// map ImageType -> bool
		map<ImageType, bool>				m_dicom_image_type;

		string m_filename;
		uint64_t m_file_size;
		string m_file_mtime;

		/// требуется ли индексация, если true - то индексация требуется
		bool								m_bNeedIndexing;

		/// признак DicomSource
		DicomSource							m_DicomSource;

	public:
		DicomFileIndex();

		/// содержит ли класс Dicom тэги
		bool is_dicom() const;

		/// имеет какой-либо тип Dicom Image Type
		bool has_image_type() const;

		/// получить строку имя файла
		string get_file_name() const { return m_filename; }

		/// получить размер файла (m_file_size)
		uint64_t get_file_size() const { return m_file_size; }

		/// получить время модификации файла (m_file_mtime)
		string get_file_mtime() const { return m_file_mtime; }

		/// заполнить поля DicomFileTagsValue и DicomFileTags для заданного файла
		/// todo: собирать диагностическую информацию о процессе заполнения для обработки её в классе DicomDirectoryIndex
		bool fill_filetags_from_file(const wstring &path, const wstring &name);

		/// заполнить поля DicomFileTags (имя файла, время создания и размер файла ) из структуры fileinfo
		bool fill_from_fileinfo(const FileInfo& fileinfo_val);


		/// получить размер файла в байтах, получить строку времени создания файла в байтах
		/// \param fname [in] полное имя файла
		/// \return успех или нет
		bool fill_name_size_time(const wstring& fname, const wstring &name_part);

		/// проверить соответветствие данных друг другу
		bool check_consistency() const;

		/// быстро проверить равенство тэгов
		bool equal_fast(const DicomFileIndex& a) const;

		/// проверка равенства двух DicomFileIndex объектов
		bool operator== (const DicomFileIndex& a) const;

		/// из списка тэгов файла сгенерировать строку с краткой информацией
		wstring get_summary_info_string() const;

		void set_dicom_filename(const string &filename) { m_filename = filename; }
		void set_dicom_file_size(uint64_t file_size) { m_file_size = file_size; }
		void set_dicom_file_mtime(const string &file_mtime) { m_file_mtime = file_mtime; }

		/// получить значение m_bNeedIndexing, необходимость в индексации
		/// \return требуется ли индексация
		bool	get_isneed_indexing() const
		{
			return m_bNeedIndexing;
		}

		/// установить значение m_bNeedIndexing
		void	set_need_indexing(bool b_value)
		{
			m_bNeedIndexing = b_value;
		}



		//!	\name функции для работы с характеристикой ImageType
		//! @{
		Dicom::tag_e get_dicom_tags_ID(size_t i) const;					///< получить значение m_dicom_tags[i]
		wstring get_dicom_tags_value(size_t i) const;						///< получить значение m_dicom_tags_value.at(m_dicom_tags[i])
		string get_dicom_tags_discr(size_t i) const;						///< получить значение m_dicom_tags_disciption(m_dicom_tags[i])
		size_t get_dicom_tags_length() const							///< длина вектора m_dicom_tags
		{
			return m_dicom_tags.size();
		};
		void set_dicom_tags_value(size_t i, const wstring& wstr_value);	///< установить значение m_dicom_tags_value.at(m_dicom_tags[i])
		void set_dicom_tags_discr(size_t i, const string& str_value);		///< установить значение m_dicom_tags_disciption(m_dicom_tags[i])
		void set_dicomsource_type(DicomSource dicome_type);					///< установить значение DicomSource
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
