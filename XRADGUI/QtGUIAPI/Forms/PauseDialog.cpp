﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "PauseDialog.h"

#include "WorkflowControl.h"

namespace XRAD_GUI
{

void PauseDialog::QuitClicked()
{
	ScheduleApplicationQuit();
	accept();
}

void PauseDialog::ContinueClicked()
{
	accept();
}

PauseDialog::PauseDialog()
{
	setupUi(this);
	connect(buttonOK, &QPushButton::clicked, this, &PauseDialog::ContinueClicked);
	connect(buttonQuit, &QPushButton::clicked, this, &PauseDialog::QuitClicked);
}

}//namespace XRAD_GUI
