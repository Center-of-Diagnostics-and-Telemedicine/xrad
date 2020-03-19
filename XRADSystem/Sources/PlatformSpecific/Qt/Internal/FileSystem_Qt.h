#ifndef XRAD__File_FileSystem_Qt_h
#define XRAD__File_FileSystem_Qt_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADSystem/Sources/System/SystemConfig.h>

#ifdef XRAD_USE_FILESYSTEM_QT_VERSION

#include <string>
#include <vector>

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

vector<wstring>	GetDirectoryContent_Qt(const wstring &dir_path, bool files, bool folders, const wstring &filter = {});
void GetApplicationPathName_Qt(wstring &result);
void GetCurrentDirectory_Qt(wstring &directory_path);
bool SetCurrentDirectory_Qt(wstring directory_path);

bool FileExists_Qt(const wstring &filename);
bool DirectoryExists_Qt(wstring directory_path);
bool CreateFolder_Qt(wstring directory_path, wstring subdirectory_name);
bool CreatePath_Qt(wstring directory_path);

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_FILESYSTEM_QT_VERSION

//--------------------------------------------------------------
#endif // XRAD__File_FileSystem_Qt_h
