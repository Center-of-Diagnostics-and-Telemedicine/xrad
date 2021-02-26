/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_xrad_fstream_h
#define XRAD__File_xrad_fstream_h
/*!
	\addtogroup gr_FileSystem
	@{

	\file

	@}
*/
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include "SystemConfig.h"
#include <fstream>

XRAD_BEGIN

/*!
	\addtogroup gr_FileSystem
	@{
*/

//--------------------------------------------------------------

#if defined(XRAD_USE_FILENAMES_WIN32_VERSION)
wstring xrad_fstream_ConvertFilename(const string &filename);
#elif defined(XRAD_USE_FILENAMES_UNIX_VERSION)
string xrad_fstream_ConvertFilename(const string &filename);
#else
#error Unknown XRAD filename configuration.
#endif

//--------------------------------------------------------------
/*!
	\addtogroup gr_FileSystem
	@{
	# Классы потоков #

	Во всех классах xrad_basic_XXstream не пишем using basic_XXstream::NNN, где NNN — имя конструктора
	или open. Использование этих директив приводит в MSVC2015 к ошибкам при вызовах соответствующих
	методов. Сигнатуры методов в рантайм-библиотеке MSVC отличаются от стандартных наличием
	дополнительных параметров (со значениями по умолчанию). При определении такого метода
	со стандартной сигнатурой в xrad_basic_XXstream происходит не перекрытие родительского метода,
	а добавление нового, из-за этого при попытке вызова метода возникает неоднозначность,
	т.к. в области видимости оказываются два подходящих метода: из класса-потомка и неперекрытый
	метод из класса-родителя, введенный в область видимости директивой using.

	Для использования других конструкторов (C++17 std::filesystem::path) следует использовать классы
	потоков std.
	@}
*/
//--------------------------------------------------------------

template <class CharT, class Traits = std::char_traits<CharT>>
class xrad_basic_ifstream: public std::basic_ifstream<CharT, Traits>
{

	private:
		PARENT(std::basic_ifstream<CharT, Traits>);
	public:
		xrad_basic_ifstream() = default;
		explicit xrad_basic_ifstream(const char *filename,
				std::ios_base::openmode mode = ios_base::in):
			xrad_basic_ifstream(string(filename), mode)
		{
		}
		explicit xrad_basic_ifstream(const std::string &filename,
				std::ios_base::openmode mode = ios_base::in):
			parent(xrad_fstream_ConvertFilename(filename), mode)
		{
		}

		void open(const char *filename, ios_base::openmode mode = ios_base::in)
		{
			open(string(filename), mode);
		}
		void open(const std::string &filename, ios_base::openmode mode = ios_base::in)
		{
			parent::open(xrad_fstream_ConvertFilename(filename), mode);
		}
};

using xrad_ifstream = xrad_basic_ifstream<char>;

using ifstream = xrad_ifstream;

//--------------------------------------------------------------

template <class CharT, class Traits = std::char_traits<CharT>>
class xrad_basic_ofstream: public std::basic_ofstream<CharT, Traits>
{
	private:
		PARENT(std::basic_ofstream<CharT, Traits>);
	public:
		xrad_basic_ofstream() = default;
		explicit xrad_basic_ofstream(const char *filename,
				std::ios_base::openmode mode = ios_base::out):
			xrad_basic_ofstream(string(filename), mode)
		{
		}
		explicit xrad_basic_ofstream(const std::string &filename,
				std::ios_base::openmode mode = ios_base::out):
			parent(xrad_fstream_ConvertFilename(filename), mode)
		{
		}

		void open(const char *filename, ios_base::openmode mode = ios_base::out)
		{
			open(string(filename), mode);
		}
		void open(const std::string &filename, ios_base::openmode mode = ios_base::out)
		{
			parent::open(xrad_fstream_ConvertFilename(filename), mode);
		}
};

using xrad_ofstream = xrad_basic_ofstream<char>;

using ofstream = xrad_ofstream;

//--------------------------------------------------------------

template<class CharT, class Traits = std::char_traits<CharT>>
class xrad_basic_fstream: public std::basic_fstream<CharT, Traits>
{
	private:
		PARENT(std::basic_fstream<CharT, Traits>);
	public:
		xrad_basic_fstream() = default;
		explicit xrad_basic_fstream(const char *filename,
				std::ios_base::openmode mode = ios_base::in|ios_base::out):
			xrad_basic_fstream(string(filename), mode)
		{
		}
		explicit xrad_basic_fstream(const std::string &filename,
				std::ios_base::openmode mode = ios_base::in|ios_base::out):
			parent(xrad_fstream_ConvertFilename(filename), mode)
		{
		}

		void open(const char *filename,
				ios_base::openmode mode = ios_base::in|ios_base::out)
		{
			open(string(filename), mode);
		}
		void open(const std::string &filename,
				ios_base::openmode mode = ios_base::in|ios_base::out)
		{
			parent::open(xrad_fstream_ConvertFilename(filename), mode);
		}
};

using xrad_fstream = xrad_basic_fstream<char>;

using fstream = xrad_fstream;

//--------------------------------------------------------------

//! @} <!-- ^group gr_FileSystem -->

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_xrad_fstream_h
