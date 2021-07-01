/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_System_h
#define XRAD__File_System_h
//--------------------------------------------------------------

#include "Sources/System/FileNameOperations.h"
#include "Sources/System/FileNamePatternMatch.h"
#include "Sources/System/FileSystem.h"
#include "Sources/System/xrad_fopen.h"

//--------------------------------------------------------------

#ifndef XRAD_NO_LIBRARIES_LINKS
#ifdef XRAD_COMPILER_MSC
#include "Sources/PlatformSpecific/MSVC/MSVC_XRADSystemLink.h"
#endif // XRAD_COMPILER_MSC
#endif // XRAD_NO_LIBRARIES_LINKS

//--------------------------------------------------------------
#endif // XRAD__File_System_h
