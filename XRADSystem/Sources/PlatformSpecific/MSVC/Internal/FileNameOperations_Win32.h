/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_FileNameOperations_Win32_h
#define XRAD__File_FileNameOperations_Win32_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADSystem/Sources/System/SystemConfig.h>

#ifdef XRAD_USE_FILENAMES_WIN32_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <string>

XRAD_BEGIN

//--------------------------------------------------------------

using namespace std;

wstring CmpNormalizeFilename_MS(const wstring &filename);
wstring GetPathGenericFromAutodetect_MS(const wstring &original_path);
wstring GetPathNativeFromGeneric_MS(const wstring &original_path);
wstring GetPathSystemRawFromGeneric_MS(const wstring &original_path);

//--------------------------------------------------------------

XRAD_END

#endif // XRAD_USE_FILENAMES_WIN32_VERSION

//--------------------------------------------------------------
#endif // XRAD__File_FileNameOperations_Win32_h
