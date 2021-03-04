/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file XRADIniFile.h
//	Created by ACS on 10.10.01
//--------------------------------------------------------------
#ifndef XRAD__File_XRADIniFile_h
#define XRAD__File_XRADIniFile_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADBasic/Sources/Containers/DataArray.h>
#include <XRADSystem/CFile.h>
#include <XRADSystem/TextFile.h>
#include <string>
#include <vector>

//--------------------------------------------------------------

XRAD_BEGIN

template<class STRING_T>
struct IniFileParam
{
	IniFileParam(const STRING_T &in_name, const STRING_T &in_value) : parameter_name(in_name), parameter_value(in_value){}
	STRING_T parameter_name;
	STRING_T parameter_value;
};

template<class STRING_T>
struct IniFileSection
{
	STRING_T section_name;
	vector<IniFileParam<STRING_T> > Params;
};

typedef ustring ini_internal_string_t;
typedef	ini_internal_string_t::const_iterator ini_internal_string_iterator;

typedef IniFileParam<ini_internal_string_t> param_data_t;
typedef IniFileSection<ini_internal_string_t> section_data_t;


class IniFile
{
protected:
	// перевод служебных символов в esc-последовательности
	static string encode_section_name(const string &section);
	static string encode_key(const string &key_name);

	static string encode_string_value(const string &value);
	static wstring encode_string_value(const wstring &value);
};

class IniFileReader : private IniFile
{
	shared_cfile read_file;
public:
	IniFileReader();
	virtual ~IniFileReader();
private:
	IniFileReader(const IniFileReader &);
	IniFileReader &operator=(const IniFileReader &);
	//
public:
	void open(const string &filename);
	void open(const wstring &filename);
	void close();

	//
	void set_section(const string &section);
	//
	string read_string(const string &id);
	wstring read_wstring(const string &id);
	wstring read_wstring(const string &id, const wstring &default_value);
	string read_string(const string &id, const string &default_value);

	int read_int(const string &id);
	int read_int(const string &id, int default_value);

	bool try_read_size_t(const string &id, size_t *out_result);
	size_t read_size_t(const string &id);
	size_t read_size_t(const string &id, size_t default_value);

	double read_double(const string &id);
	double read_double(const string &id, double default_value);

private:
	vector<section_data_t> Sections;
	section_data_t &CurrentSection() const;
	//
	//
private:
	section_data_t *m_current_section;
	section_data_t GlobalSection;

	template<class ITERATOR_T, class DATA_STRING_T>
	void ReadSectionLine(ITERATOR_T &it, ITERATOR_T &ie, section_data_t **retCurrentSection);

	template<class ITERATOR_T, class DATA_STRING_T>
	void AnalyzeIniRawData(ITERATOR_T it, ITERATOR_T ie);
// 		void AnalyzeIniRawData(raw_iterator it, raw_iterator ie);
	void AnalyzeIniData();
	const param_data_t &FindParam(const string &id);
	bool FindParam(const string &id, const param_data_t **out_param);
	//
//	protected:
	wstring ReadWStringKernel(const string &id);

};

//--------------------------------------------------------------

class IniFileWriter : private IniFile
{
	text_file_writer write_file;
public:
	IniFileWriter();
	virtual ~IniFileWriter();
private:
	IniFileWriter(const IniFileWriter &);
	IniFileWriter &operator=(const IniFileWriter &);
	//
public:
	void open(const string &filename);
	void open(const wstring &filename);
	void close();
	//
public:
	void write_section(const string &section);
	//
	void write_string(const string &id, const string &value);
	void write_wstring(const string &id, const wstring &value);
	void write_int(const string &id, int value);
	void write_long(const string &id, long value);
	void write_size_t(const string &id, size_t value);
	void write_double(const string &id, double value);
//
//
//
	//
private:
	//временно передача string& nonconst, для выявления мест, где передается конструктор от char*
// 		static string encode_string_value(const char *value);
};



class ini_file_error : public logic_error
{
public:
	explicit ini_file_error(const char* what_arg) : logic_error(what_arg) {}
	explicit ini_file_error(const string& what_arg) : logic_error(what_arg) {}
};



//--------------------------------------------------------------
//
// Comments
//
//--------------------------------------------------------------

/*

	File format:
		[<global_section_params>]
		[<section1_name>
		<section1_params>
		[<section2_name>
		<section2_params>
		[...]]]

	section_name:
		"[name]"

	parameters:
		[<parameter1>
		[<parameter2>
		[...]]]

	parameter:
		"name=" [ "value1" [ ",value2" [...]]]
	без пробелов между "name" и "="!!!

*/

XRAD_END


//--------------------------------------------------------------
#endif // XRAD__File_XRADIniFile_h
