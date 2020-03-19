#include "pre.h"
#include "FileNameOperations_Win32.h"

#ifdef XRAD_USE_FILENAMES_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <XRADBasic/Sources/Containers/DataArray.h>

#include <windows.h>
#include <stdexcept>
#include <algorithm> //for transform
#include <cwctype> //for towupper

XRAD_BEGIN

//--------------------------------------------------------------

wstring CmpNormalizeFilename_MS(const wstring &filename)
{
	if (filename.length() >= 0x7FFFFFFF)
		throw length_error("CmpNormalizeFilename_MS: File name is too long.");
	wstring result(filename);
	CharUpperBuffW(&result[0], (DWORD)result.length());
	return result;
}

//note (Kovbas) Нормализация путей для корректной работы с длинными путями.
//todo (Kovbas) Возможно, нужно как-то объединить с CmpNormalizeFilename_MS
namespace
{
wstring	long_path_prefix = L"\\\\?\\";
wstring	long_path_prefix_gen = L"//?/";
wstring	long_unc_prefix = long_path_prefix + L"UNC\\";
wstring	long_unc_prefix_gen = long_path_prefix_gen + L"UNC/";
wstring	network_prefix = L"\\\\";
wstring	network_prefix_gen = L"//";
}

inline wstring path_expander(const wstring &path_in)
{
	DataArray<wchar_t> buf(path_in.size() + 1);

	size_t retLen = GetLongPathNameW(path_in.c_str(), buf.data(), static_cast<DWORD>(buf.size()));

	if (retLen == 0)
		return path_in;

	if (retLen > buf.size())
	{
		buf.realloc(retLen);
		retLen = GetLongPathNameW(path_in.c_str(), buf.data(), static_cast<DWORD>(buf.size()));
	}

	auto string_end = std::find(buf.begin(), buf.end(), '\0');

	return wstring(buf.begin(), string_end);
}

//TODO возможно, не на месте
wstring delete_all_unnecessary_backslashes(const wstring& in_path)
{
	if (!in_path.length())
		return wstring();
	wstring path(in_path);
	bool appended = false;
	if (path.back() != L'\\')
	{
		// Добавляем в конец '\\' для единообразного разбора последовательностей вида ".\\".
		path += L"\\";
		appended = true;
	}

	// TODO: По-хорошему, надо сначала выделить корень ("C:\", "\\server\share", "\\?\C:\"...),
	// потом оставшуюся часть упростить (убрать двойные '\\', пути ".\\", "folder\\.."),
	// в конце склеить корень и урощенную часть.

	size_t real_path_start_index = 0;
	if (path.front() == L'\\')
	{
		for (auto el : path)
		{
			if (el == '\\')
				++real_path_start_index;
			else
				break;
		}
	}

	vector<wchar_t> buffer;
	buffer.push_back(path[real_path_start_index]);
	for (size_t i = real_path_start_index + 1; i < path.size(); ++i)
	{
		if (buffer.back() == '\\')
		{
			if (path[i] == '\\')
				continue;
			if (!wcsncmp(path.c_str()+i, L".\\", 2))
			{
				++i;
				continue;
			}
			if (!wcsncmp(path.c_str()+i, L"..\\", 3))
			{
				auto rpos = std::find(std::next(buffer.rbegin()), buffer.rend(), L'\\') - buffer.rbegin();
				while (rpos--)
					buffer.pop_back();
				i += 2;
				continue;
			}
		}

		buffer.push_back(path[i]);
	}

	if (appended && buffer.back() == L'\\')
		buffer.pop_back();

	return wstring(buffer.data(), buffer.size());
}

wstring GetPathGenericFromAutodetect_MS(const wstring &original_path)
{
#if 1
	wstring path(original_path);
	std::replace(path.begin(), path.end(), L'\\', L'/');
	if (path.length() > long_path_prefix_gen.length() &&
			path.substr(0, long_path_prefix_gen.length()) == long_path_prefix_gen)
	{
		// "\\\\?\\"
		// "\\\\?\\UNC\\"
		if (path.length() > long_unc_prefix_gen.length() &&
				get_upper(path.substr(0, long_unc_prefix_gen.length())) == long_unc_prefix_gen)
		{
			return network_prefix_gen + path.substr(long_unc_prefix_gen.length());
		}
		return path.substr(long_path_prefix_gen.length());
	}
	return path;
#else
	wstring path(GetPathSystemRawFromGeneric_MS(original_path));

	wstring prefix(path.substr(0, long_unc_prefix.size()));
	transform(prefix.begin(), prefix.end(), prefix.begin(), toupper);
	if (prefix == long_unc_prefix)
		path = network_prefix + path.substr(long_unc_prefix.size());

	if (path.substr(0, long_path_prefix.size()) == long_path_prefix)
		path = path.substr(long_path_prefix.size());

	wstring buffer(path.size(), L'\0');
	std::replace_copy(path.begin(), path.end(), buffer.begin(), L'\\', L'/');
	return buffer;
#endif
}

wstring GetPathNativeFromGeneric_MS(const wstring &original_path)
{
	// Проверка формата: для отладки.
	if (original_path.find(L'\\') != original_path.npos)
	{
		fprintf(stderr, "GetPathSystemRawFromGeneric_MS: Path format is not generic: \"%s\".\n",
				EnsureType<const char*>(convert_to_string(original_path).c_str()));
	}

	wstring path(original_path);
	std::replace(path.begin(), path.end(), L'/', L'\\');
	return path;
}

wstring GetPathSystemRawFromGeneric_MS(const wstring &original_path)
{
	// "\\?\D:\very long path" or "\\?\UNC\server\share" according to https://docs.microsoft.com/en-us/windows/desktop/fileio/naming-a-file
	//https://arsenmk.blogspot.com/2015/12/handling-long-paths-on-windows.html
	wstring buffer(original_path.size(), L'\0');

	// Проверка формата: для отладки.
	if (original_path.find(L'\\') != original_path.npos)
	{
		fprintf(stderr, "GetPathSystemRawFromGeneric_MS: Path format is not generic: \"%s\".\n",
				EnsureType<const char*>(convert_to_string(original_path).c_str()));
	}

	std::replace_copy(original_path.begin(), original_path.end(), buffer.begin(), L'/', L'\\');

	// Уже готовый длинный путь, нечего менять
	if (buffer.substr(0, long_path_prefix.size()) == long_path_prefix)
		return path_expander(long_path_prefix + delete_all_unnecessary_backslashes(buffer.substr(long_path_prefix.size())));

	// "Короткий" путь с буквой диска, добавить префикс
	if (buffer.substr(1, 2) == L":\\")
		return  path_expander(long_path_prefix + delete_all_unnecessary_backslashes(buffer));

	// "Короткий" путь к сетевому диску, добавить префикс, убрав начальные '\\'
	if (buffer.substr(0, network_prefix.size()) == network_prefix)
		return  path_expander(long_unc_prefix + delete_all_unnecessary_backslashes(buffer.substr(network_prefix.size())));

	// "Длинный" путь к сетевому ресурсу, возвращаемый Qt. Требует корректировки
	wstring prefix(buffer.substr(0, 5));
	toupper(prefix);
	if (prefix == L"\\UNC\\")
		return  path_expander(long_unc_prefix + delete_all_unnecessary_backslashes(buffer.substr(5)));

	prefix = buffer.substr(0, 4);
	toupper(prefix);
	if (prefix == L"UNC\\")
	{
		ForceDebugBreak(); //note (Kovbas) для проверки возможности такого варианта
		return  path_expander(long_unc_prefix + delete_all_unnecessary_backslashes(buffer.substr(4)));
	}

	return buffer; //note Kovbas всё, что не удовлетворяет условиям, либо относительные пути, либо неправильные пути. Пусть пользователь с ними сам разбирается.
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_FILENAMES_WIN32_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_FileNameOperations_Win32() {}
XRAD_END

#endif // XRAD_USE_FILENAMES_WIN32_VERSION
