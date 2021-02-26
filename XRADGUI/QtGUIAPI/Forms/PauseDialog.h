/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef PauseDialog_h__
#define PauseDialog_h__
/********************************************************************
	created:	2015/01/01
	created:	1:1:2015   17:34
	author:		kns
*********************************************************************/

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_PauseDialog.h"
#include "StayOnTopDialog.h"

namespace XRAD_GUI
{

XRAD_USING

class PauseDialog : public StayOnTopDialog, public Ui::Pause
{
		Q_OBJECT
	public:
		PauseDialog ();

	public slots:
		void	QuitClicked();
		void	ContinueClicked();
};

}//namespace XRAD_GUI

#endif // PauseDialog_h__
