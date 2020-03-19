//--------------------------------------------------------------
#ifndef XRAD__File_FileSystem_Win32_h
#define XRAD__File_FileSystem_Win32_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADSystem/Sources/System/SystemConfig.h>

#ifdef XRAD_USE_FILESYSTEM_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <XRADSystem/Sources/System/FileSystem.h>
#include <string>
#include <vector>

XRAD_BEGIN

//--------------------------------------------------------------

using namespace std;

/*!
	\brief Сбор содержимого директории с получением детальной информации о файлах и/или директориях.
	Возможна фильтрация результата по расширениям файлов в формате \ref FileNamePatternMatch

	\param dir_path [in] -- путь к анализируемой директории.
	\param files [out] -- список файлов, может быть NULL.
	\param folders [out] -- список директорий, может быть NULL.
	\param filter [in] -- фильтр имен файлов, только для файлов.
	\return Вектор с информацией о файлах и директориях в анализируемой директории.

	\note
	Для фильтрации используется \ref FileNamePatternMatch, а не механизм Win32.
*/
void GetDirectoryContent_MS(const wstring &dir_path,
		vector<wstring> *files,
		vector<wstring> *folders,
		const wstring &filter);

/*!
	\brief Сбор содержимого директории с получением детальной информации о файлах и/или директориях.
	Возможна фильтрация результата по расширениям файлов в формате "*.ext1"

	\param dir_path [in] -- путь к анализируемой директории.
	\param files [out] -- список найденных файлов.
	\param folders [out] -- список найденных директорий.
	\param filter [in] -- фильтр имен файлов, только для файлов.

	\note
	Для фильтрации используется \ref FileNamePatternMatch, а не механизм Win32.
*/
void GetDirectoryContentDetailed_MS(const wstring &dir_path,
		vector<FileInfo> *files,
		vector<DirectoryInfo> *folders,
		const wstring &filter);

bool FileExists_MS(const wstring &filename);
bool DirectoryExists_MS(const wstring &directory_path);
bool CreateFolder_MS(const wstring &directory_path, const wstring &subdirectory_name);
bool CreatePath_MS(const wstring &directory_path);
bool DeleteFile_MS(const wstring &filename);
wstring GetApplicationPathName_MS();
wstring GetTempDirectory_MS();
wstring GetCurrentDirectory_MS();
bool SetCurrentDirectory_MS(const wstring &directory_path);



//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_FILESYSTEM_WIN32_VERSION

//--------------------------------------------------------------
#endif // XRAD__File_FileSystem_Win32_h
