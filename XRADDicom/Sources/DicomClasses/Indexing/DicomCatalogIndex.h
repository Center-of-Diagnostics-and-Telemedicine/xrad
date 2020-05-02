#ifndef DicomCatalogIndex_h__
#define DicomCatalogIndex_h__
/*!
	\file
	\date 2019/10/21 10:00
	\author novik
	\brief Объявление функций и полей класса DicomCatalogIndex, предназначенных для работы с файлами
	и директориями каталога

	Класс DicomCatalogIndex  предназначен для обработки всех поддиректорий некоторого каталога.
*/
#include <XRADBasic/Sources/Utils/TimeProfiler.h>
#include "DicomDirectoryIndex.h"
#include "DicomDirectoryIndexJson.h"

XRAD_BEGIN

namespace Dicom
{

//! \brief Класс для работы и хранения информации о каталоге с файлами
class DicomCatalogIndex
{
	private:
		/// требуется ли вывод вспомогательной информации в stdout
		const bool	m_b_show_info;
		const bool	m_check_consistency;
		const bool	m_update;
	public:

		DicomCatalogIndex(bool show_info, bool check_consistency, bool update) : m_b_show_info(show_info), m_check_consistency(check_consistency), m_update(update)
		{
		}

	private:
		/// вектор информации о директориях
		vector<DicomDirectoryIndex>		m_data;

	private:
		/*!
			\brief обработать список структур с описаниями файлов
			для получения списка vector<fileinfo>& fileinfo_raw использовать функцию GetDirectoryFilesDetailed

			1) рассортировать файлы в списке fileinfo_raw по уникальным директориям
			2) заполнить значения тэгов для каждого файла

			\param fileinfo_raw [in] список структур с описаниями файлов
		*/
		void fill_from_fileinfo(const wstring &path, const DirectoryContentInfo& fileinfo_raw,
				ProgressProxy pp = VoidProgressProxy());

		/// проверить актуальность инф-ции из json файлов и сохранить json файлы только обновлённых директорий
		/// по сути выполняет последовательность функций check_actuality() и  update() для каждой директории каталога,
		/// но запись обновлённого json файла выполняется сразу
		void check_actuality_and_update(ProgressProxy pp = VoidProgressProxy());

	public:
		/// проверить адекватность записи/чтения инф-ции в/из json файлов в двух форматах
		bool test_json_write_load();

	public:
		/// стереть все данные, освободить память
		void clear();

		/// проверка равенства двух DicomFileIndex объектов
		bool operator== (const DicomCatalogIndex& a)  const;

		/// индексировать все файлы в каталоге root_path: несколько шагов
		/// 1) составить список всех файлов и разбить их на директории
		/// 2) проверить адекватность информации в json файлах с проверяемыми файлами:
		/// 3) если информация из json файла неактуальна - её нужно индексировать: "update"
		/// 4) сгенерировать json файлы для оставшихся файлов
		/// \param root_path [in] путь к анализируемому каталогу
		/// \param show_info [in] выводить вспомогательную информацию
		void PerformCatalogIndexing(const wstring& root_path, ProgressProxy pp = VoidProgressProxy());

		vector<DicomDirectoryIndex> &data() { return m_data; }
};



#if 0
/// \brief Класс для записи сообщений в текстовой файл
/// базируется на коде из файла XRADImmediateTest TestThreads.cpp
class Log
{
	public:
		Log()
		{
			counter = 0;
			srand(time(0));
			log_ID = rand();
			stream = fopen("log_fileindex.txt", "ab");
			if (!stream)
				stream = stdout;
			Write("start logging");
		}
		~Log()
		{
			Write("end logging");
			if (stream != stdout)
			{
				fclose(stream);
			}
		}
		/// записать сообщение типа string
		void Write(const string &str)
		{
			unique_lock<mutex> lock(mx);

			if (stream)
			{
				if (counter>0)
					fprintf(stream, "%zd :%04zd :%s\n", log_ID, counter, str.c_str());
				else
					fprintf(stream, "%zd :---- :%s\n", log_ID, str.c_str());
				fflush(stream);
				counter++;
			}
		}
		/// записать сообщение типа wstring
		void Write(const wstring &str)
		{
			Write(convert_to_string(str));
		}
	private:
		size_t counter;		// счётчик сообщений
		size_t log_ID;		// уникальный номер данного лога
		FILE *stream;
		mutex mx;
	};
#endif

} //namespace Dicom

XRAD_END

#endif // DicomCatalog_h__
