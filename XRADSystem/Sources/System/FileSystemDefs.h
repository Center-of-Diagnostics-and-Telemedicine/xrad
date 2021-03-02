/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file FileSystemDefs.h
//--------------------------------------------------------------
#ifndef XRAD__File_FileSystemDefs_h
#define XRAD__File_FileSystemDefs_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <cstdint>

XRAD_BEGIN

//--------------------------------------------------------------

/*!
	\brief Тип для хранения смещения в файле. Всегда знаковый, 64-битный

	Тип С++, лежащий в основе, различается у разных компиляторов:
	- MSVC: long long;
	- GCC 9.2.1 / Ubuntu 18.04 x64: long.
*/
using file_offset_t = int64_t;

/*!
	\brief Тип для хранения размера файла: беззнаковый, 64-битный

	Тип С++, лежащий в основе, различается у разных компиляторов:
	- MSVC: unsigned long long;
	- GCC 9.2.1 / Ubuntu 18.04 x64: unsigned long.
*/
using file_size_t = uint64_t;

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_FileSystemDefs_h
