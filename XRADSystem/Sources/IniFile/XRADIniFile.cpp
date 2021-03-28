/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	file XRADIniFile.cpp
//	Created by ACS on 10.10.01
//--------------------------------------------------------------
#include "pre.h"

#include "XRADIniFile.h"

#include <XRADBasic/Sources/Core/EscapeSequences.h>

#define	XRAD_INI_FILES_CPP
#include "IniParser.hh"

XRAD_BEGIN

using namespace std;

//--------------------------------------------------------------

// TODO: ини-файлы распознавать кодировку (8бит или утф)
// структуру с raw данными и ссылками на нее упразднить. найденные параметры хранить в копиях вида ustring

using namespace ns_ini_file_parser;

string IniFile::encode_section_name(const string &section_name)
{
	return encode_escape_sequences(section_name, "[]\"");
}

//--------------------------------------------------------------

string IniFile::encode_key(const string &key_name)
{
	return encode_escape_sequences(key_name, "[]=\"");
}

//--------------------------------------------------------------

string IniFile::encode_string_value(const string &value)
{
	return encode_escape_sequences(value, "\"");
}

wstring IniFile::encode_string_value(const wstring &value)
{
	return encode_escape_sequences(value, L"\"");
}

//
//		template<class STRING_T> internal_string_t convert_to_internal(const STRING_T &);




//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------


IniFileReader::IniFileReader(void) :
//		raw_data(0),
m_current_section(0)
{
}

//--------------------------------------------------------------

IniFileReader::~IniFileReader()
{
}

//--------------------------------------------------------------



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------







//--------------------------------------------------------------
template<class ITERATOR_T, class DATA_STRING_T>
void IniFileReader::ReadSectionLine(ITERATOR_T &it, ITERATOR_T &ie, section_data_t **retCurrentSection)
{
	if(*it != '[') return;
	ITERATOR_T section_name_start = ++it;
//#error
	for(;it<ie;++it)
	{
		if(*it == ']')
		{
		// section name end reached
			ITERATOR_T section_name_end = it;
			if(section_name_end>section_name_start)
			{
			// move to the eol
				++it;
				for(; it<ie; ++it)
				{
					if(is_eol_f(it, ie))
					{
					// ok
						++it;
						Sections.push_back(section_data_t());
						section_data_t *currentSection = &Sections.back();
						currentSection->section_name = convert_to_internal(DATA_STRING_T(section_name_start, section_name_end));
						*retCurrentSection = currentSection;
						break;
					}
					else if(!is_whitespace_f(it, ie))
					{
					// error: no other characters allowed to follow the name
						it = find_if(it, ie, is_eol());
						++it;
						break;
					}
				}
			}
			else
			{
			// error: empty section name
				it = find_if(it, ie, is_eol());
				++it;
			}
			break;
		}
		else if(is_eol_f(it, ie))
		{
		// error: section name must end with ']'
			++it;
			break;
		}
	}
}



//--------------------------------------------------------------

template<class ITERATOR_T, class DATA_STRING_T>
void IniFileReader::AnalyzeIniRawData(ITERATOR_T it, ITERATOR_T ie)
{
	section_data_t *currentSection = &GlobalSection;

	for(; it<ie;)
	{
		it = std::find_if(it, ie, is_not_whitespace()); // ищем первый непустой символ
		if(it >= ie)
			break;
		if(*it == '[')
		{
		// new section name encountered
			currentSection = 0;
			ReadSectionLine<ITERATOR_T, DATA_STRING_T>(it, ie, &currentSection);
		}
		else if(is_comment_f(it, ie))
		{
		// comment line, skip
			it = find_if(it, ie, is_eol());
			++it;
		}
		else
		{
		// may be parameter
			if(!currentSection)
			{
			// no section to add the parameter to, so skip the line
				it = find_if(it, ie, is_eol());
				++it;
			}
			else
			{
			// may be parameter
				ReadParamLine<ITERATOR_T, DATA_STRING_T>(it, ie, *currentSection);
			}
		}
	}
}

//--------------------------------------------------------------

void IniFileReader::open(const string &filename)
{
	close();
	read_file.open(filename.c_str(), "rb");
	AnalyzeIniData();
}

void IniFileReader::open(const wstring &filename)
{
	close();
	read_file.open(filename, L"rb");
	AnalyzeIniData();
}



void IniFileReader::AnalyzeIniData()
{
	size_t	file_length = read_file.size();

	DataArray<char> raw_data(file_length+1);

	size_t rsize = read_file.read(raw_data.data(), 1, file_length);
	if(rsize != file_length)
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("""IniFileReader: there were errors while reading the file"));
	}
	raw_data[file_length] = 0;

	raw_iterator it = raw_data.cbegin();
	raw_iterator ie = raw_data.cend();

	if(is_utf8_BOM_f(it, ie))
	{
		it += 3;
		DataArray<ustring::value_type> buffer(it, ie);
		AnalyzeIniRawData<DataArray<ustring::value_type>::const_iterator, ustring>(buffer.cbegin(), buffer.cend());
	}
	else if(is_utf16_BE_BOM_f(it, ie))
	{
		it += 2;
		DataArray<wchar_t> buffer(file_length/2 - 1, 0);
		ImportUTF16BE(buffer.begin(), buffer.end(), it);
		AnalyzeIniRawData<DataArray<wchar_t>::const_iterator, wstring>(buffer.cbegin(), buffer.cend());
	}

	else if(is_utf16_LE_BOM_f(it, ie))
	{
		it += 2;
		DataArray<wchar_t> buffer(file_length/2 - 1, 0);
		ImportUTF16LE(buffer.begin(), buffer.end(), it);
		AnalyzeIniRawData<DataArray<wchar_t>::const_iterator, wstring>(buffer.cbegin(), buffer.cend());
	}
	else
	{
		AnalyzeIniRawData<DataArray<string::value_type>::const_iterator, string>(it, ie);
	}
}

//--------------------------------------------------------------

void IniFileReader::close()
{
	read_file.close();
}

//--------------------------------------------------------------



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



void	IniFileReader::set_section(const string &section)
{
	m_current_section = 0;
	if(section.empty())
	{
		m_current_section = &GlobalSection;
		return;
	}
	for(size_t i=0; i<Sections.size(); ++i)
	{
		string ss(Sections[i].section_name.begin(), Sections[i].section_name.end());
		if(ss == section)
		{
			m_current_section = &Sections[i];
			return;
		}
	}
	ForceDebugBreak();
	throw ini_file_error(ssprintf("IniFileReader::set_section(const string &), section '%s' not found", section.c_str()));
}

//--------------------------------------------------------------

const param_data_t &IniFileReader::FindParam(const string &id)
{
	for(size_t i=0; i<CurrentSection().Params.size(); ++i)
	{
		param_data_t &param = CurrentSection().Params[i];
		ini_internal_string_t	internal = convert_to_internal(id);
		if(param.parameter_name == internal)
		{
			return param;
		}
	}

	ForceDebugBreak();
	throw ini_file_error(ssprintf("IniFileReader::FindParam(string), id = '%s' -- parameter not found", id.c_str()));
}

bool IniFileReader::FindParam(const string &id, const param_data_t **out_param)
{
	for(size_t i=0; i<CurrentSection().Params.size(); ++i)
	{
		const param_data_t &param = CurrentSection().Params[i];
		ini_internal_string_t	internal = convert_to_internal(id);
		if(param.parameter_name == internal)
		{
			*out_param = &param;
			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------


wstring IniFileReader::ReadWStringKernel(const string &id)
{
//	загрузка строкового параметра всегда в wstring.
//	последующее упрощение, если надо, делается по результатам этой функции
	const param_data_t &currentParam = FindParam(id);

	ini_internal_string_iterator it = currentParam.parameter_value.begin(), ie = currentParam.parameter_value.end();
	ini_internal_string_iterator	param_start = std::find_if(it, ie, is_not_whitespace()); // ищем первый непустой символ
	ini_internal_string_iterator	quote_start = std::find_if(param_start, ie, is_character<string::value_type>('\"')); // после него ищем первую кавычку в строке

	if(quote_start>=ie)	// если не нашли, ошибка

	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("IniFileReader::ReadWStringKernel(string), id = '%s' -- string begin not found", id));
	}
	++quote_start; // пропускаем найденную первую кавычку и начинаем искать последнюю
	ini_internal_string_iterator	quote_end = quote_start;

	while(*quote_end != '\"')
	{
		quote_end = std::find_if(quote_end, ie, is_character<string::value_type>('\"')); // находим следующую кавычку
		if(*(quote_end-1)=='\\') ++quote_end; // проверяем, не литерал ли это, и если так, пропускаем
		if(quote_end>=ie) // если не нашли, ошибка
		{
			ForceDebugBreak();
			throw ini_file_error(ssprintf("IniFileReader::ReadWStringKernel(string), id = '%s' -- string end not found", id));
		}
	}

	ustring	utf_buffer(quote_start, quote_end);
	return decode_escape_sequences(convert_to_wstring(utf_buffer, e_encode_literals));
}



//--------------------------------------------------------------

string IniFileReader::read_string(const string &id)
{
	wstring	wresult = ReadWStringKernel(id);
	return wstring_to_string(wresult, e_encode_literals);
}

wstring IniFileReader::read_wstring(const string &id)
{
	return ReadWStringKernel(id);
}

wstring IniFileReader::read_wstring(const string &id, const wstring &default_value)
{
	try
	{
		return ReadWStringKernel(id);
	}
	catch(ini_file_error &)
	{
		return default_value;
	}
}


string IniFileReader::read_string(const string &id, const string &default_value)
{
	try
	{
		return read_string(id);
	}
	catch(ini_file_error &)
	{
		return default_value;
	}
}


//--------------------------------------------------------------

int IniFileReader::read_int(const string &id)
{
	const param_data_t &currentParam = FindParam(id);
	int result;
	if(sscanf(convert_to_string(currentParam.parameter_value).c_str(), " %i", &result)==1)
	{
		return result;
	}
	else
	{
		ForceDebugBreak();
		throw ini_file_error(string("IniFileReader::read_int, couldn't read param ") + id);
	}
}

int IniFileReader::read_int(const string &id, int default_value)
{
	try
	{
		return read_int(id);
	}
	catch(ini_file_error &)
	{
		return default_value;
	}
}

//--------------------------------------------------------------

bool IniFileReader::try_read_size_t(const string &id, size_t *out_result)
{
	const param_data_t *currentParam;
	if(!FindParam(id, &currentParam))
		return false;
	size_t result;
	if(sscanf(convert_to_string(currentParam->parameter_value).c_str(), " %zu", &result)!=1)
		return false;
	*out_result = result;
	return true;
}

size_t IniFileReader::read_size_t(const string &id)
{
	const param_data_t &currentParam = FindParam(id);
	size_t result;
	if(sscanf(convert_to_string(currentParam.parameter_value).c_str(), " %zu", &result)==1)
	{
		return result;
	}
	else
	{
		ForceDebugBreak();
		throw ini_file_error(string("IniFileReader::read_size_t, couldn't read param ") + id);
	}
}

size_t IniFileReader::read_size_t(const string &id, size_t default_value)
{
	size_t result = 0;
	if(!try_read_size_t(id, &result))
		return default_value;
	return result;
}

//--------------------------------------------------------------


double IniFileReader::read_double(const string &id)
{
	const param_data_t &currentParam = FindParam(id);
	double result;
	if(sscanf(convert_to_string(currentParam.parameter_value).c_str(), " %lf", &result) == 1)
	{
		return result;
	}
	else
	{
		ForceDebugBreak();
		throw ini_file_error(string("IniFileReader::read_double, couldn't read param ") + id);
	}
}

double IniFileReader::read_double(const string &id, double default_value)
{
	try
	{
		return read_double(id);
	}
	catch(ini_file_error &)
	{
		return default_value;
	}
}


section_data_t &IniFileReader::CurrentSection() const
{
	if(!m_current_section)
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("IniFileReader::CurrentSection(), section undefined"));
	}
	return *m_current_section;
}
//--------------------------------------------------------------




//--------------------------------------------------------------


//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------

IniFileWriter::IniFileWriter()
{
}

//--------------------------------------------------------------

IniFileWriter::~IniFileWriter()
{
}

//--------------------------------------------------------------

void IniFileWriter::open(const string &filename)
{
	open(convert_to_wstring(filename));
}

void IniFileWriter::open(const wstring &filename)
{
	write_file.open_create(filename, text_encoding::utf8);
//	write_file.write("\xEF\xBB\xBF", 3, 1); // utf-8 BOM
	write_file.printf_("# utf-8\n");
}

//--------------------------------------------------------------

void IniFileWriter::close()
{
	write_file.printf_("\n");
	write_file.close();
}



//--------------------------------------------------------------
//
//
//
//--------------------------------------------------------------



//--------------------------------------------------------------

//--------------------------------------------------------------

void IniFileWriter::write_section(const string &section_name)
{
	if(!section_name.length())
	{
		ForceDebugBreak();
		throw ini_file_error("IniFileWriter::write_section(), section_name is empty");
	}
	string str = encode_section_name(section_name);
	write_file.printf_("\n[%s]\n", str.c_str());
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_section(\"%s\")", section_name.c_str()));
	}
}


//--------------------------------------------------------------
void IniFileWriter::write_wstring(const string &id, const wstring &value)
{
	string sid = encode_key(id);

	wstring	escape_value = encode_string_value(value);

	ustring svalue = wstring_to_ustring(escape_value);
	write_file.printf_("%s=\"%s\"\n", sid.c_str(), &svalue[0]);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_string(%s,%s)", id.c_str(), value.c_str()));
	}
}

void IniFileWriter::write_string(const string &id, const string &value)
{
// 	write_wstring(id, string_to_wstring(encode_string_value(value), decode_literals));
//	write_wstring(id, string_to_wstring(value, decode_literals));
	string sid = encode_key(id);

	string	escape_value = encode_string_value(value);

	ustring svalue = string_to_ustring(escape_value, e_decode_literals);
	write_file.printf_("%s=\"%s\"\n", sid.c_str(), &svalue[0]);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_string(%s,%s)", id.c_str(), value.c_str()));
	}
}

//--------------------------------------------------------------

void IniFileWriter::write_int(const string &id, int value)
{
	string sid = encode_key(id);
	write_file.printf_("%s=%i\n", sid.c_str(), value);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_int(%s,%i)", id.c_str(), value));
	}
}

//--------------------------------------------------------------

void IniFileWriter::write_long(const string &id, long value)
{
	string sid = encode_key(id);
	write_file.printf_("%s=%li\n", sid.c_str(), value);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_long(%s,%li)", id.c_str(), value));
	}
}

//--------------------------------------------------------------

void IniFileWriter::write_size_t(const string &id, size_t value)
{
	string sid = encode_key(id);
	write_file.printf_("%s=%zu\n", sid.c_str(), value);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_size_t(%s,%zu)", id.c_str(), value));
	}
}

//--------------------------------------------------------------

void IniFileWriter::write_double(const string &id, double value)
{
	string sid = encode_key(id);
	write_file.printf_("%s=%e\n", sid.c_str(), value);
	if(write_file.error())
	{
		ForceDebugBreak();
		throw ini_file_error(ssprintf("Error in IniFileWriter::write_double(%s,%g)", id.c_str(), value));
	}
}


XRAD_END


//--------------------------------------------------------------
