/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file IGUISettingsChanged.h
//--------------------------------------------------------------
#ifndef XRAD__File_IGUISettingsChanged_h
#define XRAD__File_IGUISettingsChanged_h
//--------------------------------------------------------------

namespace XRAD_GUI
{

class IGUISettingsChanged
{
	public:
		virtual void GUISettingsChanged() = 0;
};

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_IGUISettingsChanged_h
