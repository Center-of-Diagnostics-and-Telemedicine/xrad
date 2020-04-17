//--------------------------------------------------------------
#ifndef __XRADGUIAPIDefs_h
#define __XRADGUIAPIDefs_h
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
#endif // __XRADGUIAPIDefs_h
