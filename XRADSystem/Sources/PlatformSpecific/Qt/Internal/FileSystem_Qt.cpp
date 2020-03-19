#include "pre.h"
#include "FileSystem_Qt.h"

#ifdef XRAD_USE_FILESYSTEM_QT_VERSION

#include <XRAD/PlatformSpecific/Qt/Internal/StringConverters_Qt.h>
#include <XRAD/PlatformSpecific/MSVC/PC_XRADQtCoreLink.h>

#include <QApplication.h>

XRAD_BEGIN

//--------------------------------------------------------------

//	filesystem

bool FileExists_Qt(const wstring &filename)
{
	QFileInfo file_info(filename);
	return file_info.exists() && file_info.isFile();
}

bool DirectoryExists_Qt(wstring directory_path)
{
	QDir directory(wstring_to_qstring(directory_path));
	return directory.exists(wstring_to_qstring(directory_path));
}



void GetApplicationPathName_Qt(wstring &directory_path_name)
{
	QString buffer(QCoreApplication::applicationFilePath());
	QString buffer_from_native = QDir::toNativeSeparators(buffer);
	directory_path_name = qstring_to_wstring(buffer_from_native);//.toStdWString();
}

void GetCurrentDirectory_Qt(wstring &directory_path)
{
	QString buffer(QDir::currentPath());
	QString buffer_from_native = QDir::toNativeSeparators(buffer);
	directory_path = qstring_to_wstring(buffer_from_native);//.toStdWString();
}

bool	SetCurrentDirectory_Qt(wstring directory_path)
{
	return QDir::setCurrent(wstring_to_qstring(directory_path));
}

bool CreateFolder_Qt(wstring directory_path, wstring subdirectory_name)
{
	QDir directory(wstring_to_qstring(directory_path));
	if(!directory.exists()) return false;
	return directory.mkdir(wstring_to_qstring(subdirectory_name));
}

bool CreatePath_Qt(wstring directory_path)
{
	QDir root_dir = QDir::root();
	return root_dir.mkpath(wstring_to_qstring(directory_path));
}

vector<wstring>	GetDirectoryContent_Qt(const wstring &directory_path, bool files, bool folders, const wstring &filter)
{
#error See comment.
	/*
		Функция вызывается только в вариантах {files, folders} = {true, false} и {false, true}
		(проверить).
		Поведение функции должно повторять поведение GetDirectoryFileContent_Qt и
		GetDirectoryFolderContent_Qt.
	*/
	QDir directory(wstring_to_qstring(directory_path), wstring_to_qstring(filter));
	QDir::Filters	filters = QDir::Hidden;

	if(files) filters |= QDir::Files;
	if(folders) filters |= (QDir::AllDirs | QDir::NoSymLinks);

	directory.setFilter(filters);
	directory.setSorting(QDir::Name);

	std::vector<wstring>	result;
	QStringList	file_name_list_buf = directory.entryList();
	result.resize(file_name_list_buf.size());

	for(int i = 0; i < file_name_list_buf.size(); ++i)
	{
		result[i] = qstring_to_wstring(file_name_list_buf.at(i));//.toStdWString();
	}
	return result;
}

void GetDirectoryFileContent_Qt(std::vector<wstring> &return_file_name_list, const wstring &directory_path, const wstring &type)
{
#error Sync with GetDirectoryContent_Qt and delete.
	QDir directory(wstring_to_qstring(directory_path), wstring_to_qstring(type));
	directory.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	directory.setSorting(QDir::Name);
	QStringList	file_name_list_buf = directory.entryList();
	return_file_name_list.resize(file_name_list_buf.size());

	for(int i = 0; i < file_name_list_buf.size(); ++i)
	{
		return_file_name_list[i] = qstring_to_wstring(file_name_list_buf.at(i));//.toStdWString();
	}
}

void GetDirectoryFolderContent_Qt(std::vector<wstring> &return_folder_name_list, const wstring &directory_path)
{
#error Sync with GetDirectoryContent_Qt and delete.
	QDir directory(wstring_to_qstring(directory_path));
	directory.setFilter(QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
	directory.setSorting(QDir::Name);
	QStringList	folder_name_list_buf = directory.entryList();
	return_folder_name_list.resize(folder_name_list_buf.size());
	for(int i = 0; i < folder_name_list_buf.size(); ++i)
	{
		return_folder_name_list[i] = qstring_to_wstring(folder_name_list_buf.at(i));//.toStdWString();
	}
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_FILESYSTEM_QT_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_SystemUtils_Qt() {}
XRAD_END

#endif // XRAD_USE_FILESYSTEM_QT_VERSION
