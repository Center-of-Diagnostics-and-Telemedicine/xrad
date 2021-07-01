/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_hex_char_h__
#define XRAD__File_hex_char_h__

#include "String.h"

XRAD_BEGIN

enum
{
	max_hex_digits_for_unicode = 8,
	max_octal_digits_for_unicode = 11
};

template<class T>
bool is_hex_char(T c)
{
	switch(c)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'A':
		case 'b':
		case 'B':
		case 'c':
		case 'C':
		case 'd':
		case 'D':
		case 'e':
		case 'E':
		case 'f':
		case 'F':
			return true;
	}
	return false;
}

template<class T>
uint32_t hex_char_to_int(T c)
{
	switch(c)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a':
		case 'A': return 0xA;
		case 'b':
		case 'B': return 0xB;
		case 'c':
		case 'C': return 0xC;
		case 'd':
		case 'D': return 0xD;
		case 'e':
		case 'E': return 0xE;
		case 'f':
		case 'F': return 0xF;
	}
	return 0;
}

inline char int_to_hex_char(int n)
{
	switch(n)
	{
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		case 0xA: return 'A';
		case 0xB: return 'B';
		case 0xC: return 'C';
		case 0xD: return 'D';
		case 0xE: return 'E';
		case 0xF: return 'F';
	}
	return default_ascii_character();
}

//--------------------------------------------------------------

template<class T>
bool is_oct_char(T c)
{
	switch(c)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
			return true;
	}
	return false;
}

//--------------------------------------------------------------

template<class T>
uint32_t oct_char_to_int(T c)
{
	switch(c)
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
	}
	return 0;
}

template<class T>
uint32_t esc_sequence_to_char(T c)
{
	switch(c)
	{
		case 'a':
			return '\a';
			break;
		case 'b':
			return '\b';
			break;
		case 'f':
			return '\f';
			break;
		case 'n':
			return '\n';
			break;
		case 'r':
			return '\r';
			break;
		case 't':
			return '\t';
			break;
		case 'v':
			return '\v';
			break;
		case '\\':
			return '\\';
		case '"':
			return '"';
			break;
		default:
			return c;
			// любой символ \С, кроме вышеперечисленных переходит в С
	}
}

XRAD_END

#endif //XRAD__File_hex_char_h__
