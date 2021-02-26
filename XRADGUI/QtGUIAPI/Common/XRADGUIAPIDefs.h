/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//--------------------------------------------------------------
#ifndef XRAD__File_XRADGUIAPIDefs_h
#define XRAD__File_XRADGUIAPIDefs_h
//--------------------------------------------------------------

namespace XRAD_GUI
{

enum file_dialog_mode
{
	file_open_dialog, file_save_dialog
};

enum display_text_dialog_status
{
	single_use_window, //окно с кнопкой ОК, закрывается при нажатии
	multiple_use_window_paused,
	multiple_use_window_silent
};

}// namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_XRADGUIAPIDefs_h
