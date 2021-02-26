/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file XRADGUIUtils.cpp
//--------------------------------------------------------------
#include "pre.h"
#include "XRADGUIUtils.h"

//--------------------------------------------------------------

namespace XRAD_GUI
{

//--------------------------------------------------------------

string GetGUIExceptionString()
{
	string result;
	try
	{
		throw;
	}
#if QT_VERSION >= 0x050000
	catch( QException &e)
	{
		return xrad::ssprintf("%s\n[%s]", e.what(), typeid(e).name());
	}
#endif
	catch( exception &e)
	{
		return xrad::ssprintf("%s\n[%s]", e.what(), typeid(e).name());
	}
	catch(...)
	{
		return """Unknown exception";
	}
}

//--------------------------------------------------------------

} // namespace XRAD_GUI

//--------------------------------------------------------------
