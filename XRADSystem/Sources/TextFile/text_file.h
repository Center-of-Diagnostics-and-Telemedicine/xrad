/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_text_file_h
#define XRAD__File_text_file_h
/*!
	\file
	\date 2017/12/29 11:09
	\author kulberg

	\brief  текстовый файл, учитывающий кодировку текста
*/

#include <XRADSystem/CFile.h>
#include "text_encoding.h"
#include <cstdarg>

XRAD_BEGIN



class text_file : protected shared_cfile
{
	PARENT(shared_cfile);

public:
private:

protected:
	text_encoding::file_type	m_encoding;


	text_file() : m_encoding(text_encoding::unknown){}

public:

	text_encoding::file_type	encoding() const{ return m_encoding; }

	using parent::size;
	using parent::tell;
	using parent::seek;
	using parent::get_pos;
	using parent::set_pos;
	using parent::flush;
	using parent::close;
	using parent::error;
	using parent::is_open;
};

class text_file_reader : public text_file
{
	PARENT(text_file);
private:
	size_t	scanf_util(const string format, text_encoding::file_type tff, va_list args);
	size_t	scanf_util(const wstring format, text_encoding::file_type tff, va_list args);
	size_t	scanf_util(const ustring format, text_encoding::file_type tff, va_list args);

	text_encoding::file_type	recognize_encoding_by_bom();
	text_encoding::file_type	recognize_encoding_by_content();//тестировать
	size_t	bom_size;

public:
	text_file_reader(){}
	text_file_reader(const wstring &filename, text_encoding::recognize_method rem = text_encoding::recognize_encoding_bom){ open(filename, rem); }
	text_file_reader(const string &filename, text_encoding::recognize_method rem = text_encoding::recognize_encoding_bom){ open(filename, rem); }

	void	open(const wstring &filename, text_encoding::recognize_method rem = text_encoding::recognize_encoding_bom);

	void	open(const string &filename, text_encoding::recognize_method rem = text_encoding::recognize_encoding_bom){ open(convert_to_wstring(filename), rem); }


	void	set_encoding(text_encoding::file_type enc){ m_encoding = enc; }

	void	read(wstring &s);
	void	read(string &s){ wstring buffer; read(buffer); s = convert_to_string(buffer); }
	void	read(ustring &s){wstring buffer; read(buffer); s = convert_to_ustring(buffer);}

	template<class STRING_T>
	size_t	scanf_(const STRING_T format, ...)
	{
		va_list args;
		va_start(args, format);
		size_t result = scanf_util(format, encoding(), args);
		va_end(args);
		return result;
	}
};



class text_file_writer : public text_file
{
	PARENT(text_file);
private:
	bool	write_bom(text_encoding::file_type tff);

	size_t	printf_util(const char *format, text_encoding::file_type tff, va_list args);
	size_t	printf_util(const wchar_t *format, text_encoding::file_type tff, va_list args);
//	size_t	printf_util(const uchar_t *format, text_encoding::file_type tff, va_list args);
//	printf в utf-8 строках не работает, хотя, кажется, должен бы.

public:
	text_file_writer(){}
	text_file_writer(const string &filename, text_encoding::file_type enc){ open_create(filename, enc); };
	text_file_writer(const wstring &filename, text_encoding::file_type enc){ open_create(filename, enc); };

	void	open_append(const string &filename)
	{
		open_append(convert_to_wstring(filename));
	}

	void	open_append(const wstring &filename)
	{
		try
		{
			text_file_reader	detector(filename);
			m_encoding = detector.encoding();
			detector.close();
			shared_cfile::open(filename, L"ab");
			seek(0, SEEK_END);
		}
		catch(...)
		{
			throw;
		}
	}


	void	open_create(const wstring &filename, text_encoding::file_type enc)
	{
		m_encoding = enc;
		shared_cfile::open(filename, L"wb");
		write_bom(encoding());
	}

	void	open_create(const string &filename, text_encoding::file_type enc)
	{
		open_create(convert_to_wstring(filename), enc);
	}

public:
template <class STRING_T, typename ... Args>
	size_t	printf_(const STRING_T &format, Args const & ... args)
	{
		return printf_core(ssprintf_arg(format), ssprintf_arg(args) ...);
	}

private:
	template<class CHAR_T>
	size_t	printf_core(const CHAR_T *format, ...)
	{
		va_list args;
		va_start(args, format);
		size_t result = printf_util(format, encoding(), args);
		va_end(args);
		return result;
	}
};



XRAD_END

#endif // XRAD__File_text_file_h
