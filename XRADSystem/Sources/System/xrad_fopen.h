/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_xrad_fopen_h
#define XRAD__File_xrad_fopen_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <cstdio>

#if defined(XRAD_COMPILER_MSC) && (_MSC_VER == 1900) // MSVC 2015
extern "C" {
[[deprecated("std::fopen doesn't support Unicode under Windows. Use xrad::xrad_fopen instead.")]]
_ACRTIMP FILE* __cdecl fopen(
    char const* _FileName,
    char const* _Mode
    );
}
#endif

XRAD_BEGIN

//--------------------------------------------------------------

std::FILE* xrad_fopen(const char* filename, const char* mode);

inline std::FILE* fopen(const char* filename, const char* mode)
{
	return xrad_fopen(filename, mode);
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_xrad_fopen_h
