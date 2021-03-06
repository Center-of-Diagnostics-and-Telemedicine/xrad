﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file XRADGUIUtils.h
//--------------------------------------------------------------
#ifndef XRAD__File_XRADGUIUtils_h
#define XRAD__File_XRADGUIUtils_h
//--------------------------------------------------------------

#include <string>

namespace XRAD_GUI
{

using namespace std;

//! \brief Получить строку сообщения исключения в блоке catch
//!
//! Допускается вызывать только изнутри блока catch.
//! В противном случае программа аварийно завершится.
string GetGUIExceptionString();

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_XRADGUIUtils_h
