#include "pre_GUI.h"
#include "TextDisplayer.h"
#include <XRADGUIAPI.h>


/*!
	\file
	\date 2019/01/16 17:52
	\author kulberg

	\brief 
*/

XRAD_BEGIN




TextDisplayer::TextDisplayer(const string &title, bool fixed_width, bool editable)
{
	window.reset(new TextWindowContainer(api_CreateTextDisplayer(convert_to_wstring(title), fixed_width, editable)));
}


TextDisplayer::TextDisplayer(const wstring &title, bool fixed_width /*= false*/, bool editable /*= false*/)
{
	window.reset(new TextWindowContainer(api_CreateTextDisplayer(title, fixed_width, editable)));
}


bool TextDisplayer::SetText(const wstring &in_text)
{
	return api_SetText(text_container(), in_text);
}


bool TextDisplayer::SetText(const string &in_text)
{
	return api_SetText(text_container(), convert_to_wstring(in_text));
}


bool TextDisplayer::SetFontSize(double size)
{
	return api_SetFontSize(text_container(), size);
}

bool TextDisplayer::SetFixedWidth(bool fw)
{
	return api_SetFixedWidth(text_container(), fw);
}

bool TextDisplayer::SetEditable(bool ed)
{
	return api_SetEditable(text_container(), ed);
}

wstring TextDisplayer::WGetText() const
{
	return api_GetText(text_container());
}
string TextDisplayer::GetText() const
{
	return convert_to_string(api_GetText(text_container()));
}

XRAD_END

