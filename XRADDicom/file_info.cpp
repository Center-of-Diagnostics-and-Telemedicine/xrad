/*!
	\file

	\date 2018/03/9 13:27
	\author nicholas

	\brief
*/
#include "pre.h"
#include "file_info.h"


#include <XRADSystem/System.h>

XRAD_BEGIN

namespace filesystem
{

	wstring replace_root_foldername_util(wstring file_path, wstring old_root_path_name, wstring new_root_path_name)
	{
		if(!is_path_separator(old_root_path_name.back())) old_root_path_name += path_separator();
		if(!is_path_separator(new_root_path_name.back())) new_root_path_name += path_separator();

		if(file_path.size()<old_root_path_name.size()) throw invalid_argument("generic_dicom_params::replace_root_foldername, 'full_path_and_filename' doesn't contain 'old_root_path_name'");

		size_t i = 0;
		for(auto &c : old_root_path_name)
		{
			if(file_path[i++] != c) throw invalid_argument("generic_dicom_params::replace_root_foldername, 'full_path_and_filename' doesn't contain 'old_root_path_name'");
		}
		size_t	replace_size = old_root_path_name.size();

		//исключаем возможную потерю слэша, если пути заданы по-разному
		if(is_path_separator(old_root_path_name.back()) && !is_path_separator(new_root_path_name.back())) --replace_size;

		file_path.replace(file_path.begin(), file_path.begin() + replace_size, new_root_path_name);

		return file_path;
	}

	void file_info::replace_root_foldername(wstring old_root_path_name, wstring new_root_path_name)
	{
		m_path = replace_root_foldername_util(m_path, old_root_path_name, new_root_path_name);
	}

	void file_info::set_full_file_path(const wstring &new_value)
	{
		// пустая входная строка это команда на сброс, единственный случай дозволенного некорректного результата.
		if(new_value.empty())
		{
			m_file_name = m_path = L"";
			return;
		}
		// ищется последний path separator
		auto pos = new_value.rfind(L'\\');
		if(pos == string::npos) pos = new_value.rfind(L'/');

		// то, что справа от него, записывается в m_file_name, остальное (включая сам сепаратор) в m_path
		m_path = new_value.substr(0, pos + 1);
		m_file_name = new_value.substr(pos + 1, new_value.size() - pos);

		// любой некорректный результат не пропускается
		if(m_file_name.empty())
		{
			throw invalid_argument("source::set_full_file_path, invalid argument = " + convert_to_string(new_value));
		}
		if(m_path.empty())
		{
			throw invalid_argument("source::set_full_file_path, invalid argument = " + convert_to_string(new_value));
		}
	}

	void file_info::set_file_name(const wstring &new_value)
	{
		if(new_value.empty()) throw invalid_argument("source::set_file_name, invalid argument = " + convert_to_string(new_value));
		m_file_name = new_value;
		while(is_path_separator(m_file_name.front()) && m_file_name.size()) m_file_name.erase(0, 1);//сначала вычисляется правый аргумент, если контейнер пустой, front() не вызовется
	}

	void file_info::set_folder_path(const wstring &new_value)
	{
		if(new_value.empty()) throw invalid_argument("source::set_folder_path, invalid argument = " + convert_to_string(new_value));
		m_path = new_value;
		if(!is_path_separator(m_path.back())) m_path += path_separator();
	}

	// разрозненные утилиты анализа и обработки имен файлов
	/*!
		brief\
	*/
	wstring normalize_file_path(const wstring &val)
	{
		if(is_path_separator(val.back()))
		{
			return val;
		}
		else
		{
			if(val.find(L"\\") != string::npos)
			{
				return val + L"\\";
			}
			else
			{
				return val + L"/";
			}
		}
	}

	/*!
		brief\
	*/
	wstring	detect_common_root_folder(const wstring &path_1, const wstring &path_2)
	{
		size_t	min_len = min(path_1.length(), path_2.length());
		size_t	common_len = 0;
		auto it1 = path_1.begin();
		auto it2 = path_2.begin();

		while(common_len<min_len && *it1==*it2) ++common_len, ++it1, ++it2;
	//	path_1.erase(path_1.begin()+common_len, path_1.end());

		wstring	result(path_1.begin(), path_1.begin()+common_len);

		// Может оказаться, что совпадают части имен подпапок, возможно ошибочное создание папки с таким именем.
		// По соглашению имя каталога должно кончаться слэшом.
		// Находим последний с конца слэш. Если после него есть символы, удаляем их.

		auto	slash_position_from_beginning = result.rend() - find_if(result.rbegin(), result.rend(), is_path_separator<wchar_t>);

		if(slash_position_from_beginning)
		{
			result.erase(result.begin()+slash_position_from_beginning, result.end());
		}

		return result;
	}

	/*!
		brief\ генерирует имя файла из заданных символов и числа
	*/
	wstring generate_filename(const wstring &val, const size_t i)
	{
		const size_t max_filename_len = 8;

		wstring buffer(val);

		if (buffer.size() == 0) buffer = L"NM";
		if (buffer.size() > 3) buffer = buffer.substr(0, 2);

		buffer += ssprintf(L"%0" + std::to_wstring(max_filename_len - buffer.size()) + L"d", i);

		return buffer;
	};

}//namespace filesystem

XRAD_END
