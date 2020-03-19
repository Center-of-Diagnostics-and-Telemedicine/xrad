#include "pre.h"
#include "FileNameOperations.h"
#include "SystemConfig.h"

#if defined(XRAD_USE_FILENAMES_WIN32_VERSION)

#include <XRADSystem/Sources/PlatformSpecific/MSVC/Internal/FileNameOperations_Win32.h>

#elif defined(XRAD_USE_FILENAMES_UNIX_VERSION)

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)
#else
#error No std implementation for FileNameOperations.
#endif

XRAD_BEGIN

//--------------------------------------------------------------

#if defined(XRAD_USE_FILENAMES_WIN32_VERSION)

namespace
{
auto api_CmpNormalizeFilename = CmpNormalizeFilename_MS;
auto api_GetPathGenericFromAutodetect = GetPathGenericFromAutodetect_MS;
auto api_GetPathNativeFromGeneric = GetPathNativeFromGeneric_MS;
auto api_GetPathSystemRawFromGeneric = GetPathSystemRawFromGeneric_MS;
} // namespace

#elif defined(XRAD_USE_FILENAMES_UNIX_VERSION)

namespace
{

wstring api_CmpNormalizeFilename(const wstring &filename)
{
	return filename;
}

wstring api_GetPathGenericFromAutodetect(const wstring &original_path)
{
	return original_path;
}

wstring api_GetPathNativeFromGeneric(const wstring &original_path)
{
	return original_path;
}

wstring api_GetPathSystemRawFromGeneric(const wstring &original_path)
{
	return original_path;
}

} // namespace

#elif defined(XRAD_USE_DUMMY_STD_FALLBACK_VERSION)

namespace
{

wstring api_CmpNormalizeFilename(const wstring &filename)
{
	return filename;
}

wstring api_GetPathGenericFromAutodetect(const wstring &original_path)
{
	return original_path;
}

wstring api_GetPathNativeFromGeneric(const wstring &original_path)
{
	return original_path;
}

wstring api_GetPathSystemRawFromGeneric(const wstring &original_path)
{
	return original_path;
}

} // namespace

#else
#error No std implementation for FileNameOperations.
#endif

//--------------------------------------------------------------

void SplitFilename(const string &filename, string *path, string *name_with_extension,
		string *name_without_extension, string *extension)
{
	auto name_pos = filename.find_last_of(path_separator());
	if (name_pos == filename.npos)
		name_pos = 0;
	else
		++name_pos;
	if (path)
		*path = filename.substr(0, name_pos);
	if (name_with_extension)
		*name_with_extension = filename.substr(name_pos);
	if (name_without_extension || extension)
	{
		auto ext_pos = filename.find_last_of(extension_separator());
		if (ext_pos == filename.npos || ext_pos < name_pos)
			ext_pos = filename.length();
		if (name_without_extension)
			*name_without_extension = filename.substr(name_pos, ext_pos - name_pos);
		if (extension)
			*extension = filename.substr(ext_pos);
	}
}

void SplitFilename(const wstring &filename, wstring *path, wstring *name_with_extension,
		wstring *name_without_extension, wstring *extension)
{
	string path_buf;
	string name_with_extension_buf;
	string name_without_extension_buf;
	string extension_buf;
	SplitFilename(convert_to_string(filename),
			path? &path_buf: nullptr,
			name_with_extension? &name_with_extension_buf: nullptr,
			name_without_extension? &name_without_extension_buf: nullptr,
			extension? &extension_buf: nullptr);
	if (path)
		*path = convert_to_wstring(path_buf);
	if (name_with_extension)
		*name_with_extension = convert_to_wstring(name_with_extension_buf);
	if (name_without_extension)
		*name_without_extension = convert_to_wstring(name_without_extension_buf);
	if (extension)
		*extension = convert_to_wstring(extension_buf);
}

//--------------------------------------------------------------

wstring RemoveTrailingPathSeparator(const wstring &path)
{
	if (path.empty())
		return path;
	if (path.back() != wpath_separator())
		return path;
	// TODO: Эта функция в некоторых случаях работает неверно.
	// Например: пути "C:/", "//Server/Share/" должны оставаться без изменений, а она их изменяет.
	// Ввести зависящую от платформы функцию api_RemoveTrailingPathSeparator.
	size_t len = path.length()-1;
	while (len && path[len-1] == wpath_separator())
		--len;
	return path.substr(0, len);
}

//--------------------------------------------------------------

string CmpNormalizeFilename(const string &filename)
{
	return convert_to_string(CmpNormalizeFilename(convert_to_wstring(filename)));
}

wstring CmpNormalizeFilename(const wstring &filename)
{
	return api_CmpNormalizeFilename(filename);
}

//--------------------------------------------------------------

string GetPathMachineReadable(const string &path_original)
{
	return convert_to_string(GetPathSystemRawFromAutodetect(convert_to_wstring(path_original)));
}

wstring GetPathMachineReadable(const wstring &path_original)
{
	return GetPathSystemRawFromAutodetect(path_original);
}

string GetPathHumanReadable(const string &path_original)
{
	return convert_to_string(GetPathGenericFromAutodetect(convert_to_wstring(path_original)));
}

wstring GetPathHumanReadable(const wstring &path_original)
{
	return GetPathGenericFromAutodetect(path_original);
}

//--------------------------------------------------------------

wstring GetPathGenericFromAutodetect(const wstring &path)
{
	return api_GetPathGenericFromAutodetect(path);
}

wstring GetPathNativeFromGeneric(const wstring &path)
{
	return api_GetPathNativeFromGeneric(path);
}

wstring GetPathNativeFromAutodetect(const wstring &path)
{
	return GetPathNativeFromGeneric(GetPathGenericFromAutodetect(path));
}

wstring GetPathSystemRawFromGeneric(const wstring &path)
{
	return api_GetPathSystemRawFromGeneric(path);
}

wstring GetPathSystemRawFromAutodetect(const wstring &path)
{
	return GetPathSystemRawFromGeneric(GetPathGenericFromAutodetect(path));
}

//--------------------------------------------------------------

XRAD_END
