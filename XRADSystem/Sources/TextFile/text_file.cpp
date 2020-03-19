#include "pre.h"
#include "text_file.h"

XRAD_BEGIN

namespace
{

// TODO: При работе с UTF-16 заменить wchar_t на char16_t.

template<class STRING_T>
int vfscanf_util(FILE* const file, const STRING_T &format, text_encoding::file_type enc, va_list args)
{
	int	result;
	switch(enc)
	{
		case text_encoding::unknown://важно: раньше для неизвестной кодировки вылетало исключение, это неправильно.
		case text_encoding::char_8bit:
		{
			string	format8 = convert_to_string(format);
			result = vfscanf(file, format8.c_str(), args);
		}
		break;

		case text_encoding::utf16_native:
		{
			wstring	format16 = convert_to_wstring(format);
			result = vfwscanf(file, format16.c_str(), args);
			break;
		}

		case text_encoding::utf8:
		{
			ustring	formatu8 = convert_to_ustring(format);
			result = vfscanf(file, uchar_t::pointer_to_char(formatu8.c_str()), args);
		}
		break;

		default:
			throw invalid_argument("scanf_util(), unknown text file format");

		case text_encoding::utf16_reverse:
			throw invalid_argument("scanf_util(), can't parse reverse byte-order text");
	}
	return result;
}

template<class CHAR_T>
size_t vfprintf_template(FILE* const file, const CHAR_T *format, text_encoding::file_type enc, va_list args)
{
	auto	buffer = vssprintf(format, args);

	switch(enc)
	{
		case text_encoding::unknown:
		case text_encoding::char_8bit:
		{
			string	sbuffer = convert_to_string(buffer);
			return fwrite(sbuffer.c_str(), sizeof(char), sbuffer.size(), file);
		}
		break;

		case text_encoding::utf16_native:
		{
			wstring	wbuffer = convert_to_wstring(buffer);
			return fwrite(wbuffer.c_str(), sizeof(wchar_t), wbuffer.size(), file);
		}
		break;

		case text_encoding::utf16_reverse:
		{
			wstring	rbuffer = convert_to_wstring(buffer);
			auto reverse_bytes = [](wchar_t x)
			{
				return ((x >> 8) & 0xFF) | ((x << 8) & 0xFF00);
			};
			transform(rbuffer.begin(), rbuffer.end(), rbuffer.begin(), reverse_bytes);
			return fwrite(rbuffer.c_str(), sizeof(wchar_t), rbuffer.size(), file);
		}
		break;

		case text_encoding::utf8:
		{
			ustring	ubuffer = convert_to_ustring(buffer);
			return fwrite(ubuffer.c_str(), sizeof(char), ubuffer.size(), file);
		}
		break;

		default:
			throw invalid_argument("vprintf_util(), unknown text file format");
	}
}


}//namespace




size_t	text_file_reader::scanf_util(const wstring format, text_encoding::file_type tff, va_list args)
{
	return vfscanf_util(c_file(), format, tff, args);
}

size_t	text_file_reader::scanf_util(const string format, text_encoding::file_type tff, va_list args)
{
	return vfscanf_util(c_file(), format, tff, args);
}

size_t	text_file_reader::scanf_util(const ustring format, text_encoding::file_type tff, va_list args)
{
	return vfscanf_util(c_file(), format, tff, args);
}


void text_file_reader::open(const wstring &filename, text_encoding::recognize_method rem /*= xrad::recognize_encoding_bom*/)
{
	shared_cfile::open(filename, L"rb");

	switch(rem)
	{
		case text_encoding::recognize_encoding_none:
			m_encoding = text_encoding::unknown;
			bom_size = 0;
			break;

		case text_encoding::recognize_encoding_bom:
			m_encoding = recognize_encoding_by_bom();
			bom_size = standard_bom_size(encoding());
			break;

		case text_encoding::recognize_encoding_content:
			m_encoding = recognize_encoding_by_bom();// пытаемся определить BOM, и только в случае неудачи анализируем content
			if(m_encoding == text_encoding::unknown)
			{
				m_encoding = recognize_encoding_by_content();
				bom_size = 0;
			}
			else
			{
				bom_size = standard_bom_size(encoding());
			}
			break;
	};
}



void text_file_reader::read(wstring &s)
{
//	size_t	bom_size = standard_bom_size(encoding());
	size_t	file_size = size();

	if(file_size<=bom_size)
	{
		s.resize(0);
		return;
	}

	size_t	data_size = file_size-bom_size;
	fpos_t	stored_position = get_pos();
	seek(bom_size, SEEK_SET);
	DataArray<char>	buffer(data_size+2, 0);//2 добавлено, чтобы если данные 16-бит, гарантировать null-terminated строку
	size_t	read_count = parent::read(buffer.data(), 1, data_size);
	set_pos(stored_position);

	if(read_count != data_size) throw file_container_error(ssprintf("text_file_reader::read, %d bytes read of %d", int(read_count), int(data_size)));

	switch(encoding())
	{
		case text_encoding::unknown:
		case text_encoding::char_8bit:
			s = convert_to_wstring(string(buffer.data()));
			break;

		case text_encoding::utf8:
			{
			s = convert_to_wstring(ustring(buffer.begin(), buffer.end()));
			}
			break;

		case text_encoding::utf16_native:
			s = wstring(reinterpret_cast<const wchar_t *>(buffer.data()));
			break;

		case text_encoding::utf16_reverse:
			for(size_t i = 0; i < buffer.size(); i+=2) std::swap(buffer[i], buffer[i+1]);
			s = convert_to_wstring(wstring(reinterpret_cast<const wchar_t *>(buffer.data())));
			break;

		default:
			throw invalid_argument("scanf_util(), unknown text file format");
	}
}

text_encoding::file_type	text_file_reader::recognize_encoding_by_content()
{
	DataArray<char>	buffer(size() + 1, 0);

	fpos_t	stored_position = get_pos();
	seek(0, SEEK_SET);
	shared_cfile::read(buffer.data(), buffer.size(), 1);
	set_pos(stored_position);

	return text_encoding::recognize_by_content(buffer);
}


text_encoding::file_type text_file_reader::recognize_encoding_by_bom()
{
	DataArray<char>	buffer(size_t(4), char(0));

	fpos_t	stored_position = get_pos();
	seek(0, SEEK_SET);
	size_t	read_count = parent::read(buffer.data(), 1, 4);
	set_pos(stored_position);
	if(read_count<4) return text_encoding::unknown;

	return text_encoding::recognize_by_bom(buffer);
}


//--------------------------------------------------------------
//	writer

bool text_file_writer::write_bom(text_encoding::file_type tff)
{
	const string &bom = standard_bom(tff);
	if(!bom.size()) return false;
	else return write(bom.c_str(), bom.size(), 1)==1;
}
size_t	text_file_writer::printf_util(const char *format, text_encoding::file_type tff, va_list args)
{
	return vfprintf_template(c_file(), format, tff, args);
}

size_t	text_file_writer::printf_util(const wchar_t *format, text_encoding::file_type tff, va_list args)
{
	return vfprintf_template(c_file(), format, tff, args);
}

// size_t	text_file_writer::printf_util(const uchar_t *format, text_encoding::file_type tff, va_list args)
// {
// 	return vfprintf_template(c_file(), format, tff, args);
// }

XRAD_END
