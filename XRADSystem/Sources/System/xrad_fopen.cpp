/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"

#include "xrad_fopen.h"
#include "FileNameOperations.h"
#include "SystemConfig.h"

XRAD_BEGIN

//--------------------------------------------------------------

std::FILE* xrad_fopen(const char* filename, const char* mode)
{
#if defined(XRAD_USE_CFILE_WIN32_VERSION)
	return _wfopen(GetPathSystemRawFromAutodetect(convert_to_wstring(filename)).c_str(),
			convert_to_wstring(mode).c_str());
#elif defined(XRAD_USE_CFILE_UNIX_VERSION)
	return std::fopen(
			convert_to_string(GetPathSystemRawFromAutodetect(convert_to_wstring(filename))).c_str(),
			mode);
#else
	#error Unknown platform.
#endif
}

//--------------------------------------------------------------

XRAD_END
