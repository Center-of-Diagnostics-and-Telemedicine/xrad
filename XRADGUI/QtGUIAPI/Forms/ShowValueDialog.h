/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_show_value_h
#define XRAD__File_show_value_h
/********************************************************************
	created:	2015/01/13
	created:	13:1:2015   14:49
	author:		kns
*********************************************************************/

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_ShowValueDialog.h"
#include "StayOnTopDialog.h"

namespace XRAD_GUI
{

void	SetTableSizes(QLabel *label, QString &text);

class ShowValueDialog : public StayOnTopDialog, public Ui::showValue
{
		Q_OBJECT

	protected:
		ShowValueDialog(QString prompt);
	protected slots:
		virtual void OKClicked();
};

class ShowStringDialog : public ShowValueDialog
{
		Q_OBJECT

	public:
		ShowStringDialog(QString prompt, QString display_string);

};

class ShowIntegralDialog : public ShowValueDialog
{
		Q_OBJECT

	public:
		ShowIntegralDialog(QString prompt, int64_t value);
};

class ShowFloatingDialog : public ShowValueDialog
{
		Q_OBJECT

	public:
		ShowFloatingDialog(QString prompt, double value);
};

QString	ExtendedInformation(double value);
QString	ExtendedInformation(int64_t value);



}//namespace XRAD_GUI

#endif // XRAD__File_show_value_h
