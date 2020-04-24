//	Created by ACS on 20.06.03
//--------------------------------------------------------------
#include "pre.h"
#include "StringConverters_MS.h"

#ifdef XRAD_USE_MS_VERSION

#ifndef _MSC_VER
	#error "This file is for MS Visual Studio only"
#endif

#include <XRADBasic/Sources/Core/HexChar.h>
#include <XRADBasic/Sources/Containers/DataArray.h>

#include <windows.h>
#include <cstdint>

XRAD_BEGIN

//--------------------------------------------------------------

namespace
{

CodePage global_code_page = CodePage::cp_system;

} // namespace

CodePage get_code_page_MS()
{
	return global_code_page;
}

void set_code_page_MS(CodePage code_page)
{
	global_code_page = code_page;
}

//--------------------------------------------------------------

namespace
{

int CodePageToMS(CodePage code_page)
{
	switch (code_page)
	{
		case CodePage::cp_system:
			return CP_ACP;
		case CodePage::cp_1251:
			return 1251;
		case CodePage::cp_54936:
			return 54936;
		default:
			throw invalid_argument(ssprintf("int CodePageToMS(CodePage code_page), unknown codepage = %d", int(code_page)));
	}
}

wstring DecodeCPLiterals(const wstring &str, bool *error)
{
	const wchar_t *src_data = str.c_str();
	size_t src_length = str.length();
	vector<wchar_t> result_buffer;
	for (size_t i = 0; i < src_length;)
	{
		bool decoded = false;
		if (src_length - i > 2 &&
				src_data[i] == L'{' &&
				src_data[i + 1] == L'\\' &&
				src_data[i + 2] == L'u')
		{
			// 0...0x10FFFF
			uint32_t code = 0;
			unsigned int j = 0;
			for (; j < 6; ++j)
			{
				unsigned int c = src_data[i + 3 + j];
				if (is_hex_char(c))
				{
					code = (code << 4) | hex_char_to_int(c);
				}
				else
				{
					break;
				}
			}
			if (j && src_data[i + j + 3] == L'}')
			{
				// Допускаем управляющие последовательности только для не ASCII-кодов и только в допустимом диапазоне Unicode.
				// (*1520042069)
				if (code > 0x7Fu && code <= 0x10FFFF)
				{
					if (code < 0x10000u)
					{
						if (code < 0xD800 || code > 0xDFFF)
						{
							result_buffer.push_back(static_cast<wchar_t>(code));
							decoded = true;
							i += j + 4;
						}
					}
					else
					{
						uint32_t code_biased = code - 0x10000u;
						result_buffer.push_back(0xD800 +
								static_cast<wchar_t>(code_biased >> 10));
						result_buffer.push_back(0xDC00 +
								static_cast<wchar_t>(code_biased & 0x3FFu));
						decoded = true;
						i += j + 4;
					}
				}
			}
			if (!decoded && error)
				*error = true;
		}
		if (!decoded)
		{
			result_buffer.push_back(src_data[i]);
			++i;
		}
	}
	return wstring(result_buffer.data(), result_buffer.size());
}

} // namespace

/*!
	\note Замечание про замену недопустимых символов.
	В соответствии с MSDN функция MultiByteToWideChar должна в качестве заменителя недопустимых символов
	использовать U+FFFD. Реально она подставляет '?', и это поведение, похоже, нельзя изменить.
	Поэтому заменой недопустимых символов на заданный символ здесь не занимаемся.
*/
wstring string_to_wstring_MS(const string &str, CodePage code_page, bool decode_literals, bool *error)
{
	if (str.empty())
		return wstring();
	if (str.length() >= 0x7FFFFFFFu) // Актуально и в 64-битной, и, теоретически, в 32-битной версии
		throw length_error("String to wstring conversion error: string is too long.");
	for (;;)
	{
		UINT code_page_ms = CodePageToMS(code_page);

		DWORD flags_ic = MB_ERR_INVALID_CHARS; // Заставляет функцию вернуть 0 в случае ошибки преобразования и установить last error.
		SetLastError(0);
		int size_ic = MultiByteToWideChar(code_page_ms, flags_ic, str.c_str(), int(str.length()), NULL, 0);
		int error_ic = GetLastError();

		DWORD flags = 0;
		int size_needed = MultiByteToWideChar(code_page_ms, flags, str.c_str(), int(str.length()), NULL, 0);
		if (!size_needed)
			break;

		wstring result(size_needed, 0);
		SetLastError(0);
		int size_written = MultiByteToWideChar(code_page_ms, flags, str.c_str(), int(str.length()), &result[0], size_needed);
		if (!size_written)
			break;
		int error_code = GetLastError();
		if (error && (error_code == ERROR_NO_UNICODE_TRANSLATION || error_ic || !size_ic))
		{
			// Замечание. В дествительности проверка на ERROR_NO_UNICODE_TRANSLATION не срабатывает (Win10.1607, CP54936).
			// Поэтому для анализа используется результат вызова с flags_ic = MB_ERR_INVALID_CHARS.
			*error = true;
		}
		if (size_written != size_needed)
		{
			result.resize(size_written);
		}
		if (!decode_literals)
			return result;
		return DecodeCPLiterals(result, error);
	}
	if (error)
	{
		*error = true;
	}
	return wstring();
}

//--------------------------------------------------------------

namespace
{

const size_t wchar_to_char_MS_out_buffer_size = 8;

/*!
	wsymbol должен содержать хотя бы один элемент.

	Замечание. Эта функция может работать с кодировками, отличными от CP_ACP. В том числе с теми,
	которые несовместимы с ASCII (например, EBCDIC).

	\return
		- количество записанных в result_buffer символов
		- 0 в случае ошибки преобразования
*/
size_t wchar_to_char_MS(char result_buffer[wchar_to_char_MS_out_buffer_size],
		wchar_t *wsymbol, size_t wsymbol_size, int code_page_ms)
{
	BOOL inconvertible = FALSE;
	SetLastError(0);
	long res = WideCharToMultiByte(code_page_ms, WC_NO_BEST_FIT_CHARS,
		wsymbol, int(wsymbol_size),
		result_buffer, int(wchar_to_char_MS_out_buffer_size),
		nullptr,
		&inconvertible);
	if (!res && (GetLastError() == ERROR_INVALID_FLAGS || GetLastError() == ERROR_INVALID_PARAMETER))
	{
		res = WideCharToMultiByte(code_page_ms, WC_ERR_INVALID_CHARS,
			wsymbol, int(wsymbol_size),
			result_buffer, int(wchar_to_char_MS_out_buffer_size),
			nullptr,
			nullptr);
	}

	if (res > 0 && res <= wchar_to_char_MS_out_buffer_size &&
			!inconvertible)
	{
		return res;
	}
	return 0;
}

} // namespace

string wstring_to_string_MS(const wstring &str, CodePage code_page, bool encode_literals, int default_char, bool *error)
{
#ifdef XRAD_DEBUG
	if (default_char < 0 || default_char > 0x7F)
	{
		throw invalid_argument(ssprintf("wstring_to_string_MS(): invalid default_char (code = %i).", default_char));
	}
#endif
	int code_page_ms = CodePageToMS(code_page);
	// Эта функция предполагает, что кодовая страница совместима с ASCII.
	const wchar_t *str_data = str.c_str();
	size_t str_length = str.length();
	vector<char> result_buffer;
	for (size_t i = 0; i < str_length;)
	{
		wchar_t src_wchar_buffer[2];
		wchar_t current_wchar = str_data[i];
		src_wchar_buffer[0] = current_wchar;
		size_t src_wchar_length = 1;
		bool invalid_wchar = false;
		static_assert(wchar_t(0xFFFF) > 0, "The wchar_t is signed but it is expected to be unsigned.");
		if (current_wchar >= 0xD800u && current_wchar <= 0xDFFFu)
		{
			if (current_wchar >= 0xDC00u)
			{
				invalid_wchar = true;
			}
			else
			{
				wchar_t second_wchar = str_data[i+1];
				if (second_wchar < 0xDC00u || second_wchar > 0xDFFFu)
				{
					invalid_wchar = true;
				}
				else
				{
					src_wchar_buffer[1] = second_wchar;
					src_wchar_length = 2;
				}
			}
		}
		if (invalid_wchar)
		{
			// Ошибка: Исходная строка содержит недопустимый для UTF-16 код. Заменяем на default_char.
			// default_char - ASCII-символ, преобразование не требуется.
			result_buffer.push_back(static_cast<char>(default_char));
			if (error)
				*error = true;
		}
		else if (current_wchar <= 0x7Fu)
		{
			// Оптимизация: используемая кодовая страница (должна быть) совместима с ASCII.
			// Также исключаем использование управляющих последовательностей для ASCII. (*1520042069)
			result_buffer.push_back(static_cast<char>(current_wchar));
		}
		else
		{
			char char_buffer[wchar_to_char_MS_out_buffer_size];
			size_t char_length = wchar_to_char_MS(char_buffer, src_wchar_buffer, src_wchar_length, code_page_ms);
			if (char_length != 0)
			{
				result_buffer.insert(result_buffer.end(), char_buffer, char_buffer + char_length);
			}
			else
			{
				// Символ отсутствует в выходной кодовой странице.
				if (!encode_literals)
				{
					// default_char - ASCII-символ, преобразование не требуется.
					result_buffer.push_back(static_cast<char>(default_char));
					if (error)
						*error = true;
				}
				else
				{
					uint32_t code;
					if (src_wchar_length == 2)
					{
						code = 0x10000u + (uint32_t(current_wchar - 0xD800u) << 10) + (src_wchar_buffer[1] - 0xDC00u);
					}
					else
					{
						code = current_wchar;
					}
					char buffer[40];
					int length = sprintf(buffer, "{\\u%X}", EnsureType<unsigned int>(code));
					result_buffer.insert(result_buffer.end(), buffer, buffer + length);
				}
			}
		}
		i += src_wchar_length;
	}
	return string(result_buffer.data(), result_buffer.size());
}

//--------------------------------------------------------------

XRAD_END

#else // XRAD_USE_MS_VERSION

#include <XRADBasic/Core.h>
XRAD_BEGIN
// To avoid  warning LNK4221: This object file does not define any previously undefined public
// symbols, so it will not be used by any link operation that consumes this library.
// (MSVC 2015)
void xrad__dummy_StringConverters_MS() {}
XRAD_END

#endif // XRAD_USE_MS_VERSION

//--------------------------------------------------------------
