#ifndef XRAD__File_FileSystem_h
#define XRAD__File_FileSystem_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "FileSystemDefs.h"
#include <vector>
#include <string>
#include <set>
#include <ctime>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Структура для хранения информации о файле/директории
struct FSObjectInfo
{
	wstring filename; //!< Имя файла/директории (относительное)
	time_t time_write = 0; //!< Время последней записи
	#if 0
	// Следующие данные нельзя получить через функции std::filesystem (можно через функции Win32).
	// Кроме того, не все файловые системы поддерживают эти данные.
	// Сейчас они не используются, поэтому не разрешаем их использовать.
	#define XRAD_FSObjectInfo_HAS_C_A_TIMES
	time_t time_create; //!< Время создания
	time_t time_access; //!< Время последнего доступа
	#endif
};

//! \brief Структура для хранения информации о файле
struct FileInfo: FSObjectInfo
{
	file_size_t size = 0; //!< Размер файла в байтах
};

using DirectoryInfo = FSObjectInfo;

//--------------------------------------------------------------

bool FileExists(const string &filename);
bool FileExists(const wstring &filename);

bool DirectoryExists(const string &directory_path);
bool DirectoryExists(const wstring &directory_path);

/*!
	\brief Получить информацию о файле

	\param file_info [out] Должен быть не NULL. Структура для записи информации о файле.

	\return
		- true Информация о файле получена.
		- false Информация не получена (файл не существует; такое имя имеет директория, а не файл;
			нет доступа к информации о файле и т.п.).
*/
bool GetFileInfo(const string &filename, FileInfo *file_info);

//--------------------------------------------------------------

//!	\brief Опрос содержимого конкретной директории (без подпапок).
//! Возможна фильтрация результата по расширениям файлов в формате "*.ext1;*.ext2"
void GetDirectoryContent(vector<string> &return_file_names_list,
		vector<string> &return_folder_names_list,
		const string &directory_path,
		const string &filter= {});

//!	\brief Опрос содержимого конкретной директории (без подпапок).
//! Возможна фильтрация результата по расширениям файлов в формате "*.ext1;*.ext2"
void GetDirectoryContent(vector<wstring> &return_file_names_list,
		vector<wstring> &return_folder_names_list,
		const wstring &directory_path,
		const wstring &filter = {});

//! \brief Получение списка имен файлов в каталоге, включающих в себя полные пути.
//! Возможна фильтрация результата по расширениям файлов в формате "*.ext1;*.ext2"
//! Допускается рекурсивный анализ подкаталогов
//!
//! \todo TODO вместо wstring для результата использовать system path и аналогичные механизмы stl
vector<wstring>	GetDirectoryFiles(const wstring &root_folder_name, const wstring &filter,
		bool analyze_subfolders, ProgressProxy pp = VoidProgressProxy());

//! \brief Получение списка имен файлов в каталоге, включающих в себя полные пути.
//! Возможна фильтрация результата по расширениям файлов в формате "*.ext1;*.ext2"
//! Допускается рекурсивный анализ подкаталогов
vector<string>	GetDirectoryFiles(const string &root_folder_name, const string &filter,
		bool analyze_subfolders, ProgressProxy pp = VoidProgressProxy());

struct DirectoryContentInfo
{
	vector<FileInfo> files;
	struct Directory;
	vector<Directory> directories;
};

struct DirectoryContentInfo::Directory: DirectoryInfo
{
	DirectoryContentInfo content;

	Directory(const DirectoryInfo &di, DirectoryContentInfo &&ci):
			DirectoryInfo(di),
			content(std::move(ci))
	{}
};

/*!
	\brief Сбор содержимого директории с получением детальной инф-ции о файлах.
	Возможна фильтрация результата по расширениям файлов в формате "*.ext1;*.ext2"

	\param root_folder_name [in] -- путь к анализируемой директории.
	\param filter [in] -- используемый фильтр
	\param analyze_subfolders [in] -- контроль поиска по поддиректориям.
	\param pp [in] -- progress bar.
	\return Информация о директориях и файлах в анализируемой директории.
*/
DirectoryContentInfo GetDirectoryFilesDetailed(const wstring &root_folder_name,
		const wstring &filter,
		bool analyze_subfolders,
		ProgressProxy pp = VoidProgressProxy());

//--------------------------------------------------------------

//! \brief Установить данные о приложении из параметра main(). Должно вызываться из main()
//! \note
//! При использовании startup-кода библиотеки XRAD вызывается автоматически, из xrad::xrad_main()
//! вызывать эту функцию не нужно
void SetApplicationArg0(const wstring &arg0);

string GetApplicationName();
wstring	WGetApplicationName();

string GetApplicationPath();
wstring	WGetApplicationPath();

string GetApplicationDirectory();
wstring	WGetApplicationDirectory();

string GetTempDirectory();
wstring WGetTempDirectory();

string GetCurrentDirectory();
wstring	WGetCurrentDirectory();

#ifdef SetCurrentDirectory
	#undef SetCurrentDirectory
	// в Windows.h это же имя может быть определено через #define SetCurrentDirectory SetCurrentDirectoryW
#endif

bool SetCurrentDirectory(const string &directory_path);
bool SetCurrentDirectory(const wstring &directory_path);

//--------------------------------------------------------------

bool CreateFolder(const string &directory_path, const string &subdirectory_name);
bool CreateFolder(const wstring &directory_path, const wstring &subdirectory_name);

bool CreatePath(const string &directory_path);
bool CreatePath(const wstring &directory_path);

#ifdef DeleteFile
	#undef DeleteFile
	// Win32: это имя может быть определено через #define.
#endif

//! Удаляет файл по указанному пути
bool DeleteFile(const string &path);
//! Удаляет файл по указанному пути
bool DeleteFile(const wstring &path);

//--------------------------------------------------------------

XRAD_END

#endif // XRAD__File_FileSystem_h
