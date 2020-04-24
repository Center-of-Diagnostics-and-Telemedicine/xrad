// file i18n.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "I18nSupport.h"

#include "XRADGUI.h"

//--------------------------------------------------------------

XRAD_BEGIN

//--------------------------------------------------------------

string LoadLanguageId()
{
	return GetSavedParameter(L"GUI", L"Language", string());
}

//--------------------------------------------------------------

void SaveLanguageId(const string &id)
{
	SaveParameter(L"GUI", L"Language", id);
	SetLanguageId(id);
}

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
