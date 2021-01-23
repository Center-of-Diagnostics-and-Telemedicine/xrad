#include "pre.h"
#include "FileSystem_Win32.h"

#ifdef XRAD_USE_FILESYSTEM_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include "FileNameOperations_Win32.h"
#include <XRADSystem/Sources/System/FileNameOperations.h>
#include <XRADSystem/Sources/System/FileNamePatternMatch.h>
#include <XRADBasic/Sources/Containers/DataArray.h>

#include <windows.h>
#include <memory>
#include <stdexcept>
#include <corecrt_io.h>

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

void	split_foldername(std::wstring path, std::wstring &parent_dir, std::wstring &subdir)
{
	auto slash = std::find_if(path.rbegin(), path.rend(), xrad::is_path_separator<wchar_t>);
	if (slash == path.rend())
	{
		parent_dir = std::wstring();
		subdir = std::wstring();
		return;
	}
	ptrdiff_t	subdir_len = slash - path.rbegin();
	subdir = path.substr(path.length() - subdir_len);
	parent_dir = path.substr(0, path.length() - subdir_len - 1);
}

} // namespace

//--------------------------------------------------------------

void GetDirectoryContent_MS(const wstring &dir_path,
		vector<wstring> *files,
		vector<wstring> *folders,
		const wstring &filter)
{
	/*
	_wfinddata_t.attrib values
	#define _A_NORMAL 0x00 // Normal file - No read/write restrictions
	#define _A_RDONLY 0x01 // Read only file
	#define _A_HIDDEN 0x02 // Hidden file
	#define _A_SYSTEM 0x04 // System file
	#define _A_SUBDIR 0x10 // Subdirectory
	#define _A_ARCH   0x20 // Archive file
	*/
	FileNamePatternMatch filter_match(filter);

	wstring search_string = GetPathSystemRawFromGeneric(dir_path);
	if (dir_path.length() && dir_path.back() != L'/' && dir_path.back() != L'\\')
		search_string +=  L'\\';
	search_string += L"*.*";

	_wfinddatai64_t data;
	errno = 0;
	intptr_t ff = _wfindfirsti64(search_string.c_str(), &data);
	if (ff == -1)
	{
		auto err = errno;
		if (err == ENOENT)// нет таких файлов или директорий, это не ошибка, возвращаем пустой результат.
			return;
		throw runtime_error(ssprintf(
				"Failed obtaining directory content for \"%s\" (1):\n%s",
				EnsureType<const char*>(convert_to_string(search_string).c_str()),
				EnsureType<char*>(strerror(err))));
	}
	try
	{
		for (;;)
		{
			if (data.attrib & _A_SUBDIR)
			{
				if (folders &&
						wcscmp(data.name, L".") && wcscmp(data.name, L".."))
				{
					folders->push_back(data.name);
				}
			}
			else
			{
				if (files &&
						(filter_match.empty() || filter_match(data.name)))
				{
					files->push_back(data.name);
				}
			}
			errno = 0;
			if (_wfindnexti64(ff, &data))
			{
				auto err = errno;
				if (err == ENOENT)
					break;
				throw runtime_error(ssprintf(
						"Failed obtaining directory content for \"%s\" (2):\n%s",
						EnsureType<const char*>(convert_to_string(search_string).c_str()),
						EnsureType<char*>(strerror(err))));
			}
		}
	}
	catch (...)
	{
		_findclose(ff);
		throw;
	}
	_findclose(ff);
}



namespace
{

DirectoryInfo findfiledata_to_directory_info(const _wfinddatai64_t& data)
{
	DirectoryInfo result;
	result.filename = data.name;
	#ifdef XRAD_FSObjectInfo_HAS_C_A_TIMES
	result.time_access = data.time_access;
	result.time_create = data.time_create;
	#endif
	result.time_write = data.time_write;
	return result;
}

FileInfo findfiledata_to_file_info(const _wfinddatai64_t& data)
{
	FileInfo result;
	result.filename = data.name;
	#ifdef XRAD_FSObjectInfo_HAS_C_A_TIMES
	result.time_access = data.time_access;
	result.time_create = data.time_create;
	#endif
	result.time_write = data.time_write;
	result.size = data.size;
	return result;
}

} // namespace

void GetDirectoryContentDetailed_MS(const wstring &dir_path,
		vector<FileInfo> *files,
		vector<DirectoryInfo> *folders,
		const wstring &filter)
{
	FileNamePatternMatch filter_match(filter);

	wstring search_string = GetPathSystemRawFromGeneric(dir_path);
	if (dir_path.length() && dir_path.back() != L'/' && dir_path.back() != L'\\')
		search_string += L'\\';
	search_string += L"*.*";

	_wfinddatai64_t data;
	errno = 0;
	intptr_t ff = _wfindfirsti64(search_string.c_str(), &data);
	if (ff == -1)
	{
		auto err = errno;
		if (err == ENOENT)
			return;
		throw runtime_error(ssprintf(
				"Failed obtaining directory content for \"%s\" (1):\n%s",
				EnsureType<const char*>(convert_to_string(search_string).c_str()),
				EnsureType<char*>(strerror(err))));
	}
	try
	{
		for (;;)
		{
			if (data.attrib & _A_SUBDIR)
			{
				if (folders &&
						wcscmp(data.name, L".") && wcscmp(data.name, L".."))
				{
					folders->push_back(findfiledata_to_directory_info(data));
				}
			}
			else
			{
				if (files &&
						(filter_match.empty() || filter_match(data.name)))
				{
					files->push_back(findfiledata_to_file_info(data));
				}
			}
			errno = 0;
			if (_wfindnexti64(ff, &data))
			{
				auto err = errno;
				if (err == ENOENT)
					break;
				throw runtime_error(ssprintf(
						"Failed obtaining directory content for \"%s\" (2):\n%s",
						EnsureType<const char*>(convert_to_string(search_string).c_str()),
						EnsureType<char*>(strerror(err))));
			}
		}
	}
	catch (...)
	{
		_findclose(ff);
		throw;
	}
	_findclose(ff);
}



bool FileExists_MS(const wstring &filename)
{
	int result = GetFileAttributesW(GetPathSystemRawFromGeneric_MS(filename).c_str());
	if (result == INVALID_FILE_ATTRIBUTES || (result & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}

bool DirectoryExists_MS(const wstring &directory_path)
{
	int result = GetFileAttributesW(GetPathSystemRawFromGeneric_MS(directory_path).c_str());
	if (result == INVALID_FILE_ATTRIBUTES || !(result & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}

namespace
{

// FILETIME отсчитывается от 1601-01-01T00:00:00Z.
// time_t (UNIX time в MSVC2015 и др.) отсчитывается от 1970-01-01T00:00:00Z.
// Разница в секундах:
constexpr int64_t unix_to_ft_unix_disp = 11644473600ll;

time_t FILETIMEToTime(FILETIME ft)
{
	// Проверяем time_t == int64_t (MSVC2015).
	static_assert(std::is_same<time_t, int64_t>::value,
			"Invalid C++ runtime library version (time_t type mismatch).");
	auto ft_i = ((int64_t)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
	return ft_i/10000000 - unix_to_ft_unix_disp;
}

} // namespace

bool GetFileInfo_MS(const wstring &filename, FileInfo *file_info)
{
	if (!file_info)
		throw invalid_argument("GetFileInfo: file_info == NULL.");
	// Функция _wstat64 не поддерживает длинные имена файлов ("\\?\C:\Temp\filename").
	// Поэтому используем GetFileAttributesExW.
	WIN32_FILE_ATTRIBUTE_DATA fa;
	if (!GetFileAttributesExW(GetPathSystemRawFromGeneric_MS(filename).c_str(),
			GetFileExInfoStandard, &fa))
	{
		return false;
	}
	if (fa.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;
	SplitFilename(filename, nullptr, &file_info->filename);
	file_info->size = ((file_size_t)fa.nFileSizeHigh << 32) + fa.nFileSizeLow;
	file_info->time_write = FILETIMEToTime(fa.ftLastWriteTime);
#ifdef XRAD_FSObjectInfo_HAS_C_A_TIMES
	#error XRAD_FSObjectInfo_HAS_C_A_TIMES: Extended times are not supported.
#endif
	return true;
}



bool CreateFolder_MS(const wstring &in_directory_path, const wstring &subdirectory_name)
{
	if(!DirectoryExists_MS(in_directory_path))
		return false;
	wstring directory_path(GetPathSystemRawFromGeneric(in_directory_path));
	size_t	len = directory_path.size();
	if(directory_path[len-1]!=L'/' && directory_path[len-1]!=L'\\')
		directory_path += L"\\";
	directory_path += subdirectory_name;

	// Функция _wmkdir не поддерживает длинные имена файлов ("\\?\C:\Temp\folder").
	// Поэтому используем CreateDirectoryW.
	SetLastError(0);
	BOOL	result = CreateDirectoryW(directory_path.c_str(), NULL);
	if(result)
	{
		// Здесь был вызов SetCurrentDirectoryW. Вызов убран по ряду причин:
		// 1. SetCurrentDirectoryW не поддерживает длинные пути без специальных настроек Windows:
		// https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation
		// 2. Функцией нельзя безопасно пользоваться в многопоточных приложениях: текущая директория
		// общая для всех потоков.
		// 3. В реализации функции для других платформ этого вызова нет.

		return true;//created
	}
	else
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return true;
		return false;
	}
}



bool CreatePath_MS(const wstring &directory_path)
{
	wstring	parent_dir, subdir;
	split_foldername(directory_path, parent_dir, subdir);
	if (parent_dir.empty())
	{
		return false;
	}
	if(DirectoryExists_MS(parent_dir))
	{
		return CreateFolder_MS(parent_dir, subdir);
	}
	if(CreatePath_MS(parent_dir))
	{
		return CreateFolder_MS(parent_dir, subdir);
	}
	return false;
}



bool DeleteFile_MS(const wstring &filename)
{
	return DeleteFileW(GetPathSystemRawFromGeneric_MS(filename).c_str());
}



wstring GetApplicationPathName_MS()
{
	DWORD   nSize = 4096;
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[1024]);
	DWORD success = GetModuleFileNameW(NULL, buffer.get(), nSize);
	if(success)
	{
		return GetPathGenericFromAutodetect_MS(wstring(buffer.get()));
	}
	else
	{
		return wstring();
	}
}



wstring GetTempDirectory_MS()
{
	DWORD buffer_size = MAX_PATH+1;
	DataArray<wchar_t> buffer(buffer_size);
	DWORD str_length = GetTempPathW(buffer_size, buffer.data());
	if (str_length > buffer_size)
	{
		buffer_size = str_length;
		buffer.realloc(buffer_size);
		str_length = GetTempPathW(buffer_size, buffer.data());
	}
	if (!str_length || str_length > buffer_size)
		throw runtime_error("GetTempDirectory() failed.");

	// Удалить конечный '\\', если это не корень диска ("C:\\").
	while (str_length > 3 && buffer[str_length-1] == L'\\')
		--str_length;
	if (str_length >= 2 && buffer[str_length-1] == L'\\' &&
			(str_length != 3 || buffer[str_length-2] != L':'))
		--str_length;
	return GetPathGenericFromAutodetect_MS(wstring(buffer.data(), str_length));
}



wstring GetCurrentDirectory_MS()
{
	size_t	buffer_size = 4096;//FILENAME_MAX

	unique_ptr<wchar_t[]> filename_buffer(new wchar_t[buffer_size]);
	std::fill(filename_buffer.get(), filename_buffer.get()+ buffer_size, L'\0');

	if(!_wgetcwd(filename_buffer.get(), int(buffer_size)))
	{
		throw runtime_error("Can't get application directory");
	}

	return GetPathGenericFromAutodetect_MS(wstring(filename_buffer.get()));
}

bool SetCurrentDirectory_MS(const wstring &directory_path)
{
//	return SetCurrentDirectory(directory_path.c_str());
	return _wchdir(GetPathSystemRawFromGeneric(directory_path).c_str())==0;
}



//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_FILESYSTEM_WIN32_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_FileSystem_Win32() {}
XRAD_END

#endif // XRAD_USE_FILESYSTEM_WIN32_VERSION
