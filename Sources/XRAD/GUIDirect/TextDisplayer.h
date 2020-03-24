﻿#ifndef __text_displayer_h
#define __text_displayer_h

#include <DataDisplayer.h>

XRAD_BEGIN


class	TextDisplayer : public DataDisplayer
{
	TextWindowContainer	&text_container(){ return static_cast<TextWindowContainer&>(*window); }
	const TextWindowContainer	&text_container() const { return static_cast<TextWindowContainer&>(*window); }

public:

	TextDisplayer(const wstring &title, bool fixed_width = false, bool editable = false);
	TextDisplayer(const string &title, bool fixed_width = false, bool editable = false);

	~TextDisplayer() = default;

	bool	SetText(const wstring &in_text);
	bool	SetText(const string &in_text);
	string	GetText() const;
	wstring	WGetText() const;

	bool	SetEditable(bool ed);
	bool	SetFixedWidth(bool fw);
	bool	SetFontSize(double size);


};




XRAD_END

#endif // __text_displayer_h