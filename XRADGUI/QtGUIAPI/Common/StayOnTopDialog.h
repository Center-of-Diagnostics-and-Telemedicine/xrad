/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
// file StayOnTopDialog.h
//--------------------------------------------------------------
#ifndef XRAD__File_StayOnTopDialog_h
#define XRAD__File_StayOnTopDialog_h
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "IGUISettingsChanged.h"
#include <QtWidgets/QDialog>

//--------------------------------------------------------------

namespace XRAD_GUI
{

class StayOnTopDialog: public QDialog, public IGUISettingsChanged
{
		Q_OBJECT
	public:
		StayOnTopDialog();
		~StayOnTopDialog();

		void SetStayOnTop(bool stay_on_top);

		//! \name IGUISettingsChanged
		//! @{
	public:
		virtual void GUISettingsChanged() override;
		//! @}

	private:
		virtual void UpdateWindowFlags();
	private:
		bool stay_on_top_flag = false;
		bool cached_allow_stay_on_top = false;
};

} // namespace XRAD_GUI

//--------------------------------------------------------------
#endif // XRAD__File_StayOnTopDialog_h
