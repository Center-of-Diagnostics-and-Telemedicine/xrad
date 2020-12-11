#include "pre.h"
#include "FileSystem.h"
#include "FileNamePatternMatch.h"
#include "FileNameOperations.h"

#include "SystemConfig.h"

#if defined(XRAD_USE_FILESYSTEM_WIN32_VERSION)

#include <XRADSystem/Sources/PlatformSpecific/MSVC/Internal/FileSystem_Win32.h>

#elif defined(XRAD_USE_FILESYSTEM_QT_VERSION)

#include <XRADSystem/Sources/PlatformSpecific/Qt/Internal/FileSystem_Qt.h>

#elif defined(XRAD_USE_FILESYSTEM_STD_VERSION)

#include <filesystem>

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)

#else
#error No std implementation for FileSystem.
#endif

XRAD_BEGIN

//--------------------------------------------------------------

#if defined(XRAD_USE_FILESYSTEM_WIN32_VERSION)

namespace
{

auto api_GetDirectoryContent = GetDirectoryContent_MS;
auto api_GetDirectoryContentDetailed = GetDirectoryContentDetailed_MS;
auto api_FileExists = FileExists_MS;
auto api_DirectoryExists = DirectoryExists_MS;
auto api_GetFileInfo = GetFileInfo_MS;
auto api_CreateFolder = CreateFolder_MS;
auto api_CreatePath = CreatePath_MS;
auto api_DeleteFile = DeleteFile_MS;
void api_SetApplicationArg0(const wstring &arg0)
{
}
auto api_GetApplicationPathName = GetApplicationPathName_MS;
auto api_GetTempDirectory = GetTempDirectory_MS;
auto api_GetCurrentDirectory = GetCurrentDirectory_MS;
auto api_SetCurrentDirectory = SetCurrentDirectory_MS;

} // namespace

#elif defined(XRAD_USE_FILESYSTEM_QT_VERSION)

namespace
{

auto api_GetDirectoryContent = GetDirectoryContent_Qt;
#error TODO: api_GetDirectoryContentDetailed not implemented.
auto api_FileExists = FileExists_Qt;
auto api_DirectoryExists = DirectoryExists_Qt;
#error TODO: api_GetFileInfo not implemented.
auto api_CreateFolder = CreateFolder_Qt;
auto api_CreatePath = CreatePath_Qt;
#error TODO: api_DeleteFile not implemented.
void api_SetApplicationArg0(const wstring &arg0)
{
}
auto api_GetApplicationPathName = GetApplicationPathName_Qt;
#error TODO: api_GetTempDirectory not implemented.
auto api_GetCurrentDirectory = GetCurrentDirectory_Qt;
auto api_SetCurrentDirectory = SetCurrentDirectory_Qt;

} // namespace

#elif defined(XRAD_USE_FILESYSTEM_STD_VERSION)

namespace
{

template <class Char = filesystem::path::value_type>
filesystem::path PrepareSystemPathHelper(const wstring &path) = delete;

template <>
filesystem::path PrepareSystemPathHelper<char>(const wstring &path)
{
	return filesystem::path(convert_to_string(path),
			filesystem::path::format::native_format);
}

template <>
filesystem::path PrepareSystemPathHelper<wchar_t>(const wstring &path)
{
	return filesystem::path(convert_to_wstring(path),
			filesystem::path::format::native_format);
}

filesystem::path PrepareSystemPath(const wstring &path)
{
	// Создание path из char не поддерживает Unicode в Windows (MSVC 2019, Windows 10.1909).
	// Создание path из wchar_t не работает должным образом в Linux (GCC 9.2.1, Ubuntu 18.04).
	// Функция u8path работает верно, но в ней нельзя задать filesystem::path::format::native_format.
	return PrepareSystemPathHelper(GetPathSystemRawFromGeneric(path));
}

wstring PathToWStringHelper(const filesystem::path &path)
{
	// Функция path.wstring() не работает должным образом в Linux (GCC 9.2.1, Ubuntu 18.04).
	// Преобразуем через UTF-8. Это работает и в Windows (MSVC 2019, Windows 10.1909), и в Linux.
	return string8_to_wstring(path.generic_u8string());
}

wstring PathToWString(const filesystem::path &path)
{
	return GetPathGenericFromAutodetect(PathToWStringHelper(path));
}

wstring PathFilenameToWString(const filesystem::path &path)
{
	return PathToWStringHelper(path.filename());
}

time_t FSTimeToTime(filesystem::file_time_type ft)
{
#if defined(XRAD_COMPILER_MSC) && (_MSC_VER == 1924)
	// Здесь file_time_type::clock не содержит метод to_time_t. Преобразуем вручную, используя
	// информацию об устройстве runtime-библиотеки MSVC.

	// Разница между 1970-01-01T00:00:00Z (time_t) и 1601-01-01T00:00:00Z (file_time_type) в секундах.
	constexpr long long c_to_fs_time_disp_sec = 11644473600ll;
	using file_clock = filesystem::file_time_type::clock;
	static_assert(chrono::system_clock::period::num == 1 &&
			file_clock::period::num == 1 &&
			chrono::system_clock::period::den == file_clock::period::den,
			"Invalid system_clock.");
	return chrono::system_clock::to_time_t(chrono::system_clock::time_point(
			chrono::system_clock::duration(ft.time_since_epoch().count() -
					c_to_fs_time_disp_sec * file_clock::period::den)));
#elif defined(XRAD_COMPILER_GNUC)
#if defined(__cpp_lib_filesystem) && (__cpp_lib_filesystem == 201703)
	// From <bits/fs_fwd.h>: struct __file_clock
	// This clock's (unspecified) epoch is 2174-01-01 00:00:00 UTC.
	// A signed 64-bit duration with nanosecond resolution gives roughly
	// +/- 292 years, which covers the 1901-2446 date range for ext4.
	// static constexpr chrono::seconds _S_epoch_diff{6437664000};
	// Замечание. При такой конвертации (добавлении константы в file_clock::period) мы отрезаем
	// часть диапазона (после 2240 года). Для практических целей считаем это допустимым.
	constexpr long long c_to_fs_time_disp_sec = 6437664000ll;
	using file_clock = filesystem::file_time_type::clock;
	static_assert(chrono::system_clock::period::num == 1 &&
			file_clock::period::num == 1 &&
			chrono::system_clock::period::den == file_clock::period::den,
			"Invalid system_clock.");
	return chrono::system_clock::to_time_t(chrono::system_clock::time_point(
			chrono::duration_cast<chrono::system_clock::duration>(
			file_clock::duration(ft.time_since_epoch().count() +
					c_to_fs_time_disp_sec * file_clock::period::den))));
#else
	#error Unknown C++ library version.
#endif
#else
	return filesystem::file_time_type::clock::to_time_t(ft);
#endif
}

#ifdef XRAD_FSObjectInfo_HAS_C_A_TIMES
	#error XRAD_FSObjectInfo_HAS_C_A_TIMES: Extended times are not supported.
#endif

//! \param filter [in] Фильтр имен файлов, используется только для файлов.
void api_GetDirectoryContentDetailed(const wstring &dir_path,
		vector<FileInfo> *files,
		vector<DirectoryInfo> *folders,
		const wstring &filter)
{
	FileNamePatternMatch filter_match(filter);
	using namespace filesystem;
	directory_iterator dir_it(PrepareSystemPath(dir_path));
	for (auto &entry: dir_it)
	{
		auto status = entry.status();
		switch (status.type())
		{
			case file_type::regular:
				if (files)
				{
					FileInfo fi;
					fi.filename = PathFilenameToWString(entry.path());
					if (filter_match(fi.filename))
					{
						fi.time_write = FSTimeToTime(entry.last_write_time());
						fi.size = entry.file_size();
						files->push_back(fi);
					}
				}
				break;
			case file_type::directory:
				if (folders)
				{
					DirectoryInfo di;
					di.filename = PathFilenameToWString(entry.path());
					di.time_write = FSTimeToTime(entry.last_write_time());
					folders->push_back(di);
				}
				break;
		}
	}
}

//! \param filter [in] Фильтр имен файлов, используется только для файлов.
void api_GetDirectoryContent(const wstring &dir_path,
		vector<wstring> *files,
		vector<wstring> *folders,
		const wstring &filter)
{
	vector<FileInfo> found_files;
	vector<DirectoryInfo> found_dirs;
	api_GetDirectoryContentDetailed(dir_path, files? &found_files: nullptr,
			folders? &found_dirs: nullptr, filter);
	if (files)
	{
		for (auto &f: found_files)
			files->push_back(f.filename);
	}
	if (folders)
	{
		for (auto &d: found_dirs)
			folders->push_back(d.filename);
	}
}

bool api_FileExists(const wstring &filename)
{
	error_code ec;
	auto status = filesystem::status(PrepareSystemPath(filename), ec);
	if (ec)
		return false;
	return is_regular_file(status);
}

bool api_DirectoryExists(const wstring &directory_path)
{
	error_code ec;
	auto status = filesystem::status(PrepareSystemPath(directory_path), ec);
	if (ec)
		return false;
	return is_directory(status);
}

#ifdef XRAD_FSObjectInfo_HAS_C_A_TIMES
	#error XRAD_FSObjectInfo_HAS_C_A_TIMES: Extended times are not supported.
#endif

bool api_GetFileInfo(const wstring &filename, FileInfo *file_info)
{
	if (!file_info)
		throw invalid_argument("GetFileInfo: file_info == NULL.");
	error_code ec;
	filesystem::directory_entry entry(PrepareSystemPath(filename), ec);
	if (ec)
		return false;
	entry.refresh(ec);
	if (ec)
		return false;
	SplitFilename(filename, nullptr, &file_info->filename);
	file_info->size = entry.file_size(ec);
	file_info->time_write = FSTimeToTime(entry.last_write_time(ec));
	return true;
}

bool api_CreateFolder(const wstring &directory_path, const wstring &subdirectory_name)
{
	if (!api_DirectoryExists(directory_path))
		return false;
	error_code ec;
	filesystem::create_directory(PrepareSystemPath(directory_path), ec);
	return !ec;
}

bool api_CreatePath(const wstring &directory_path)
{
	error_code ec;
	filesystem::create_directories(PrepareSystemPath(directory_path), ec);
	return !ec;
}

bool api_DeleteFile(const wstring &filename)
{
	auto path = PrepareSystemPath(filename);
	error_code ec;
	auto status = filesystem::status(PrepareSystemPath(filename), ec);
	if (ec)
		return false;
	if (!is_regular_file(status))
		return false;
	return filesystem::remove(path, ec);
}

wstring application_arg0;

void api_SetApplicationArg0(const wstring &arg0)
{
	application_arg0 = GetPathGenericFromAutodetect(arg0);
}

wstring api_GetApplicationPathName()
{
	return application_arg0;
}

wstring api_GetTempDirectory()
{
	return RemoveTrailingPathSeparator(PathToWString(filesystem::temp_directory_path()));
}

wstring api_GetCurrentDirectory()
{
	return RemoveTrailingPathSeparator(PathToWString(filesystem::current_path()));
}

bool api_SetCurrentDirectory(const wstring &directory_path)
{
	error_code ec;
	filesystem::current_path(PrepareSystemPath(directory_path), ec);
	return !ec;
}

} // namespace

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)

namespace
{

void api_GetDirectoryContent(const wstring &dir_path,
		vector<wstring> *files,
		vector<wstring> *folders,
		const wstring &filter)
{
}

void api_GetDirectoryContentDetailed(const wstring &dir_path,
		vector<FileInfo> *files,
		vector<DirectoryInfo> *folders,
		const wstring &filter)
{
}

bool api_FileExists(const wstring &filename)
{
	return false;
}

bool api_DirectoryExists(const wstring &directory_path)
{
	return false;
}

bool api_GetFileInfo(const wstring &filename, FileInfo *file_info)
{
	return false;
}

bool api_CreateFolder(const wstring &directory_path, const wstring &subdirectory_name)
{
	return false;
}

bool api_CreatePath(const wstring &directory_path)
{
	return false;
}

bool api_DeleteFile(const wstring &filename)
{
	return false;
}

wstring application_arg0;

void api_SetApplicationArg0(const wstring &arg0)
{
	application_arg0 = GetPathGenericFromAutodetect(arg0);
}

wstring api_GetApplicationPathName()
{
	return application_arg0;
}

wstring api_GetTempDirectory()
{
	throw runtime_error("GetTempDirectory() failed.");
}

wstring api_GetCurrentDirectory()
{
	throw runtime_error("Can't get current directory");
}

bool api_SetCurrentDirectory(const wstring &directory_path)
{
	return false;
}

} // namespace

#else
#error No std implementation for FileSystem.
#endif

//--------------------------------------------------------------

bool FileExists(const string &filename)
{
	return api_FileExists(convert_to_wstring(filename));
}

bool FileExists(const wstring &filename)
{
	return api_FileExists(filename);
}

bool DirectoryExists(const string &directory_path)
{
	return api_DirectoryExists(convert_to_wstring(directory_path));
}

bool DirectoryExists(const wstring &directory_path)
{
	return api_DirectoryExists(directory_path);
}

bool GetFileInfo(const string &filename, FileInfo *file_info)
{
	return api_GetFileInfo(convert_to_wstring(filename), file_info);
}

//--------------------------------------------------------------

void GetDirectoryContent(vector<string> &return_file_names_list,
		vector<string> &return_folder_names_list,
		const string& directory_path, const string &filter)
{
	vector<wstring> wfilenames, wfoldernames;
	GetDirectoryContent(wfilenames, wfoldernames, convert_to_wstring(directory_path),
			convert_to_wstring(filter));

	return_file_names_list.clear();
	return_folder_names_list.clear();
	for(auto &fn: wfilenames)
		return_file_names_list.push_back(convert_to_string(fn));
	for(auto &fn: wfoldernames)
		return_folder_names_list.push_back(convert_to_string(fn));
}

void GetDirectoryContent(vector<wstring> &return_file_names_list,
		vector<wstring> &return_folder_names_list,
		const wstring &directory_path, const wstring &filter)
{
	return_file_names_list.clear();
	return_folder_names_list.clear();
	api_GetDirectoryContent(directory_path,
			&return_file_names_list, &return_folder_names_list,
			filter);
}

//--------------------------------------------------------------

namespace
{

// Вспомогательная функция получения списка файлов с путями.
// Возможна рекурсивная обработка подкаталогов.
// Возможно использование индикатора прогресса для больших списков
void	GetFolderFilesRecursive(vector<wstring> &names, const wstring &root_folder,
		const wstring &filter,
		bool analyze_subfolders,
		ProgressProxy pp,
		size_t recursion_level)
{
	RandomProgressBar	progress(pp);
	progress.start("Analyzing folder contents", 1);

	vector<wstring> filenames;
	vector<wstring> subfolders;
	GetDirectoryContent(filenames, subfolders, root_folder, filter);

	// Считаем, что при наличии подкаталогов сортировка имен файлов займет меньше времени,
	// однако совсем игнорировать ее не будем никогда, кроме первого уровня рекурсии
	// (в этом случае индикатор всегда "прыгал" бы до 10% вначале). Такие же скачки
	// внутри шагов не повредят.
	double	name_sort_complexity =
		subfolders.size() ?
			recursion_level > 0 ? 0.1 : 0 : 1;

	if(filenames.size())
	{
		ProgressBar	progress1(progress.subprogress(0, name_sort_complexity));
		progress1.start("", filenames.size());
		for(auto &name : filenames)
		{
			names.push_back(root_folder + wpath_separator() + name);
			++progress1;
		}
	}

	if(analyze_subfolders && subfolders.size())
	{
		ProgressBar	progress2(progress.subprogress(name_sort_complexity, 1));
		progress2.start("", subfolders.size());
		for(auto &subfolder : subfolders)
		{
			GetFolderFilesRecursive(names, root_folder + L"/" + subfolder, filter, analyze_subfolders,
					progress2.substep(), recursion_level + 1);
			++progress2;
		}
	}
}

} // namespace

vector<wstring>	GetDirectoryFiles(const wstring &root_folder, const wstring &filter,
		bool analyze_subfolders, ProgressProxy pp)
{
	vector<wstring> result;
	GetFolderFilesRecursive(result, root_folder, filter, analyze_subfolders, pp, 0);
	return result;
}

vector<string>	GetDirectoryFiles(const string &root_folder, const string &filter,
		bool analyze_subfolders, ProgressProxy pp)
{
	vector<wstring> wresult;
	GetFolderFilesRecursive(wresult, convert_to_wstring(root_folder), convert_to_wstring(filter),
			analyze_subfolders, pp, 0);

	vector<string> result;
	for(auto &fn: wresult)
		result.push_back(convert_to_string(fn));

	return result;
}

//--------------------------------------------------------------

namespace
{

// Вспомогательная функция получения вектора с информацией о файлах. Возможна рекурсивная обработка подкаталогов. Возможно использование индикатора прогресса для больших списков
DirectoryContentInfo GetFolderFilesRecursiveDetailed(const wstring &root_folder,
		const wstring &filter,
		bool analyze_subfolders,
		ProgressProxy pp,
		bool recursion_level)
{
	RandomProgressBar	progress(pp);
	progress.start("Analyzing folder contents");

	// Считаем, что при наличии подкаталогов получение списка имен файлов и каталогов
	// займет меньше времени,
	// однако совсем игнорировать ее не будем никогда, кроме первого уровня рекурсии
	// (в этом случае индикатор всегда "прыгал" бы до 10% вначале). Такие же скачки
	// внутри шагов не повредят.
	double	name_sort_complexity = recursion_level ? 0.1 : 0;

	DirectoryContentInfo dir_content;
	vector<DirectoryInfo> subfolders;
	api_GetDirectoryContentDetailed(root_folder, &dir_content.files, &subfolders, filter);
	progress.set_position(name_sort_complexity);

	if (analyze_subfolders && subfolders.size())
	{
		ProgressBar	progress2(progress.subprogress(name_sort_complexity, 1));
		progress2.start("", subfolders.size());
		for (auto &subfolder: subfolders)
		{
			dir_content.directories.push_back(
					DirectoryContentInfo::Directory(
							subfolder,
							GetFolderFilesRecursiveDetailed(
									root_folder + wpath_separator() + subfolder.filename,
									filter,
									true,
									progress2.substep(),
									true)));
			++progress2;
		}
	}

	return dir_content;
}

} // namespace

DirectoryContentInfo GetDirectoryFilesDetailed(const wstring &root_folder,
		const wstring &filter,
		bool analyze_subfolders,
		ProgressProxy pp)
{
	return GetFolderFilesRecursiveDetailed(root_folder, filter, analyze_subfolders, pp, false);
}

//--------------------------------------------------------------

void SetApplicationArg0(const wstring &arg0)
{
	api_SetApplicationArg0(arg0);
}

string GetApplicationName()
{
	return	convert_to_string(WGetApplicationName());
}

wstring WGetApplicationName()
{
	wstring path = WGetApplicationPath();
	// TODO: Использовать функции группы SplitFilename
	auto slash = find_if(path.rbegin(), path.rend(), [](wchar_t x){return x==L'/' || x==L'\\';});
	ptrdiff_t	slash_position = path.rend() - slash;
	if(slash_position < ptrdiff_t(path.length()))
	{
		path.erase(0, slash_position);
	}
	return path;
}

string GetApplicationPath()
{
	return	convert_to_string(WGetApplicationPath());
}

wstring WGetApplicationPath()
{
	return api_GetApplicationPathName();
}

string GetApplicationDirectory()
{
	return	convert_to_string(WGetApplicationDirectory());
}

wstring WGetApplicationDirectory()
{
	wstring path = WGetApplicationPath();
	// TODO: Использовать функции группы SplitFilename
	auto slash = find_if(path.rbegin(), path.rend(), [](wchar_t x){return x==L'/' || x==L'\\';});
	ptrdiff_t	slash_position = path.rend() - slash;
	if(slash_position < ptrdiff_t(path.length()) && slash_position > 0)
	{
		path.erase(slash_position-1, path.length());
	}
	return path;
}

string GetTempDirectory()
{
	return convert_to_string(WGetTempDirectory());
}

wstring WGetTempDirectory()
{
	return api_GetTempDirectory();
}

string GetCurrentDirectory()
{
	return	convert_to_string(WGetCurrentDirectory());
}

wstring WGetCurrentDirectory()
{
	return api_GetCurrentDirectory();
}

bool SetCurrentDirectory(const wstring &directory_path)
{
	return api_SetCurrentDirectory(directory_path);
}

bool SetCurrentDirectory(const string &directory_path)
{
	return api_SetCurrentDirectory(convert_to_wstring(directory_path));
}

//--------------------------------------------------------------

bool CreateFolder(const string &directory_path, const string &subdirectory_name)
{
	return api_CreateFolder(convert_to_wstring(directory_path), convert_to_wstring(subdirectory_name));
}

bool CreateFolder(const wstring &directory_path, const wstring &subdirectory_name)
{
	return api_CreateFolder(directory_path, subdirectory_name);
}

bool CreatePath(const string &directory_path)
{
	return api_CreatePath(convert_to_wstring(directory_path));
}

bool CreatePath(const wstring &directory_path)
{
	return api_CreatePath(directory_path);
}

//--------------------------------------------------------------

bool DeleteFile(const string &path)
{
	return DeleteFile(convert_to_wstring(path));
}

bool DeleteFile(const wstring &path)
{
	return api_DeleteFile(path);
}

//--------------------------------------------------------------

XRAD_END
