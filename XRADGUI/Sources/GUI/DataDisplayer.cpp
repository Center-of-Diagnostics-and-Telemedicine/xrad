/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "DataDisplayer.h"
#include <XRADGUIAPI.h>

/*!
	\file
	\date 2019/02/19 14:57
	\author kulberg

	\brief
*/

XRAD_BEGIN

using namespace XRAD_GUI;



bool DataDisplayer::Display(bool is_stopped /*= true*/)
{
	return api_ShowDataWindow(*window, is_stopped);
}

bool DataDisplayer::SetStayOnTop(bool stay_on_top)
{
	return api_SetStayOnTop(*window, stay_on_top);
}

bool DataDisplayer::SetPersistent(bool persistent)
{
	return api_SetPersistent(*window, persistent);
}


bool	DataDisplayer::SetWindowTitle(const wstring &new_title)
{
	return api_SetDataWindowTitle(*window, new_title);
}


bool DataDisplayer::SetWindowTitle(const string &new_title)
{
	return SetWindowTitle(convert_to_wstring(new_title));
}


bool DataDisplayer::Close()
{
	return api_CloseDataWindow(*window);
}

bool DataDisplayer::Hide()
{
	return api_HideDataWindow(*window);
}


XRAD_END
