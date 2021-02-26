/*!
	\file
	\date 2018/03/9 13:29
	\author nicholas
*/
#ifndef XRAD__File_file_info_h__
#define XRAD__File_file_info_h__

#include <XRADBasic/Core.h>

XRAD_BEGIN

namespace filesystem
{
	// утилиты анализа и обработки имен файлов

	//! Проверяет, есть ли на конце строки path_separator. Если нет, добавляет его в результат.
	wstring normalize_file_path(const wstring &val);

	//! Поиск имени общей папки, содержащей два заданных каталога
	wstring	detect_common_root_folder(const wstring &path_1, const wstring &path_2);

	//! Формирует имя из восьми символов: первые до 3х символов строки и остальное число с промежутком спереди, заполненным нулями
	//todo (Kovbas) Возможно, нужно сделать функцию с заданием параметров для формирования имени файла
	wstring generate_filename(const wstring &val, const size_t i);

	//! Находит строку old_root_path_name в начале m_path и подставляет вместо ее new_root_path_name
	wstring replace_root_foldername_util(wstring file_path, wstring old_root_path_name, wstring new_root_path_name);

	/*!
		\brief Класс реализует работу с полным путём к файлу (путь+имя). Хранит обе части раздельно.
		\details Этот класс на рассмотрении для того, чтобы быть удалённым, т.к. в C++17 есть std::filesystem::path, выполняющий все описанные в этом классе манипуляции.
		// Помимо этого, в VS2015 есть заголовочный файл <filesystem>, предтеча принятого в C++17. Можно попробовать использовать его.
	*/
	class file_info
	{
	public:
		file_info() = delete;
		file_info(const wstring &path_in, const wstring &filename_in)
			: m_file_name(filename_in), m_path(path_in)
		{
			normalize_file_path(path_in);
		}
		file_info (const wstring &full_filepath)
		{
			set_full_file_path(full_filepath);
		}

		wstring full_file_path() const { return (m_path + m_file_name); };
		wstring folder_path() const { return m_path; };
		wstring file_name() const { return m_file_name; };

		void set_full_file_path(const wstring &val);
		void set_file_name(const wstring &val);
		void set_folder_path(const wstring &val);

		//! Находит строку old_root_path_name в начале m_path и подставляет вместо нее new_root_path_name
		void replace_root_foldername(wstring old_root_path_name, wstring new_root_path_name);

		bool operator==(const file_info &file_info_in)
		{
			return (m_file_name == file_info_in.m_file_name && m_file_name == file_info_in.m_path);
		}

	private:
		wstring m_file_name;
		wstring m_path;
	};


}//namespace filesystem

XRAD_END

#endif // XRAD__File_file_info_h__
