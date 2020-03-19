#ifndef EscapeSequences_h__
#define EscapeSequences_h__

#include "Config.h"
#include "BasicMacros.h"

#include <string>

//	Внутренний файл библиотеки.

XRAD_BEGIN

// перекодирование служебных символов ASCII в escape-последовательности в строке
// например, символ перевода строки '\n' --> '\\n'.
// дополнительные символы, если они заданы, преобразуются '\s' --> '\\s'
string encode_escape_sequences(const string &str, const string &aux_symbols);
wstring encode_escape_sequences(const wstring &str, const wstring &aux_symbols);

string decode_escape_sequences(const string &str);
wstring decode_escape_sequences(const wstring &str);

XRAD_END

#endif // EscapeSequences_h__