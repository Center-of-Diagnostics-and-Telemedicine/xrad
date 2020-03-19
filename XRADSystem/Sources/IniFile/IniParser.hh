#ifndef XRAD_INI_FILES_CPP
#error "These routines are needed in XRADIniFile.cpp only"
#endif

#include <XRADBasic/Sources/Containers/DataArray.h>
#include "XRADIniFile.h"

XRAD_BEGIN

namespace ns_ini_file_parser
{
typedef	DataArray<char> raw_container_t;
typedef	raw_container_t::const_iterator raw_iterator;


//	функции для однократной проверки содержимого массива по итератору

template<class ITERATOR_T>
bool is_eol_f(ITERATOR_T it, ITERATOR_T ie)
	{
	if(it>=ie) return false;
	return !(*it) || *it=='\n' || *it=='\r';
	}

template<class ITERATOR_T>
bool is_whitespace_f(ITERATOR_T it, ITERATOR_T ie)
	{
	if(it>=ie) return false;
	return *it>=0 && *it<=' ';
	}


template<class ITERATOR_T>
bool is_comment_f(ITERATOR_T it, ITERATOR_T ie)
	{
	if(it>=ie) return false;
	if(*it == ';' || *it == '#') return true;

	if(ie-it<2) return false;

	if(*it != '/') return false;
	if(*++it != '/') return false;

	return true;
	}

bool is_utf8_BOM_f(raw_iterator it, raw_iterator ie)
	{
	if(ie-it<3) return false;
	if (*it!= '\xEF') return false;
	if (*(++it)!= '\xBB') return false;
	if (*(++it)!= '\xBF') return false;
	return true;
	}

bool is_utf16_LE_BOM_f(raw_iterator it, raw_iterator ie)
	{
	if(ie-it<2) return false;
	if (*it!= '\xFF') return false;
	if (*(++it)!= '\xFE') return false;
	return true;
	}

bool is_utf16_BE_BOM_f(raw_iterator it, raw_iterator ie)
	{
	if(ie-it<2) return false;
	if (*it!= '\xFE') return false;
	if (*(++it)!= '\xFF') return false;
	return true;
	}

//--------------------------------------------------------------




//	функторы проверки наличия символа
struct	is_not_whitespace
	{
	template<class T>
		bool	operator() (const T& c){return !(c>=0 && c <= ' ');}
	};

template<class T>
struct is_character
	{
	const T	character;
	is_character(const T& in_char) : character(in_char){}
	bool	operator() (const T& c){return c==character;}
	};



struct is_eol
	{
	template<class T>
		bool	operator() (const T& c)
		{
		return !c || c=='\n' || c=='\r';
		}
	};

// побайтовый перевод потока данных в целые числа 16 бит

void ImportUTF16BE(DataArray<wchar_t>::iterator it2, DataArray<wchar_t>::iterator ie2, DataArray<char>::const_iterator it)
	{
	for(; it2<ie2; ++it2)
		{
		//little endian utf16
		*it2 += wchar_t(*it)<<8;
		++it;
		*it2 += *it;
		++it;
		}
	}
void ImportUTF16LE(DataArray<wchar_t>::iterator it2, DataArray<wchar_t>::iterator ie2, DataArray<char>::const_iterator it)
	{
	for(; it2<ie2; ++it2)
		{
		//big endian utf16
		*it2 += *it;
		++it;
		*it2 += wchar_t(*it)<<8;
		++it;
		}
	}


// перевод строковых типов во внутренний строковый тип контейнера ini-файла
template<class STRING_T> ini_internal_string_t convert_to_internal(const STRING_T &);

template<>
ini_internal_string_t convert_to_internal<ini_internal_string_t>(const ini_internal_string_t &data)
	{
	return data;
	}

template<>
ini_internal_string_t convert_to_internal<string>(const string &data)
	{
	return string_to_ustring(data, e_decode_literals);
	}

template<>
ini_internal_string_t convert_to_internal<wstring>(const wstring &data)
	{
	return wstring_to_ustring(data);
	}

// inline string	convert_to_string(const ini_internal_string_t &data)
// 	{
// 	return ustring_to_string(data, e_encode_literals);
// 	}

//
template<class ITERATOR_T, class DATA_STRING_T>
void	ReadParamLine(ITERATOR_T &it, ITERATOR_T &ie, section_data_t &current_section)
	{
	// return index pointing past the end of line
	ITERATOR_T paramNameBeg = it;
	for(; it<ie; ++it)
		{
		if(*it == '=')
			{
			ITERATOR_T paramNameEnd = it;
			if(paramNameEnd > paramNameBeg)
				{
				ITERATOR_T paramValueBeg = paramNameEnd+1;
				for(it=paramValueBeg; ; ++it)
					{
					if(is_eol_f(it,ie))
						{
						break;
						}
					}
				ITERATOR_T paramValueEnd = it;
				++it;
				// ok, add the parameter

				current_section.Params.push_back(param_data_t(convert_to_internal(DATA_STRING_T(paramNameBeg, paramNameEnd)), convert_to_internal(DATA_STRING_T(paramValueBeg, paramValueEnd))));
				}
			else
				{
				// error: empty parameter name
				it = find_if(it, ie, is_eol());
				++it;
				}
			break;
			}
		else if(is_eol_f(it,ie))
			{
			// error: not a parameter line
			++it;
			break;
			}
		}
	}

}// namespace ns_ini_file_parser

XRAD_END
