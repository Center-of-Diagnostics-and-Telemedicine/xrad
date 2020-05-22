#ifndef utf8_fstream_h__
#define utf8_fstream_h__

/*!
	\file
	\date 2019/09/18 14:14
	\author kulberg

	\brief  
*/

#include <fstream>
#include <XRADBasic/Sources/Core/StringEncode.h>

XRAD_BEGIN


//! \brief Заготовка класса ofstream, который корректно сохраняет юникод строки разного формата в utf-8
struct utf8_ofstream : public ofstream
{
	PARENT(ofstream);

	utf8_ofstream(const wstring &fn, ios_base::openmode mode = ios_base::out) : ofstream(fn, mode)
	{
		*this << ("\xEF\xBB\xBF");
		// Отступы табуляцией включаются прямо здесь, т.к. класс создавался прежде всего для вывода json. Возможно, надо сделать отдельно
		width(1);
		fill('\t');
	}

	utf8_ofstream &operator<<(const wchar_t *wstr)
	{
		*this << (convert_to_string8(wstr));
		return *this;
	}

	utf8_ofstream &operator<<(const wstring &wstr)
	{
		*this << (convert_to_string8(wstr));
		return *this;
	}

	template<class T>
	utf8_ofstream &operator << (const T x)
	{
		*(static_cast<ofstream*>(this)) << x;
		return *this;
	}
};


XRAD_END

#endif // utf8_fstream_h__
