/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	Created by IRD on 05.2013
//  Version 3.0.2
//--------------------------------------------------------------
#ifndef XRAD__GetValueDialogH
#define XRAD__GetValueDialogH

#include <XRADGUI/Sources/Internal/std.h>
#include "ui_GetValueDialog.h"
#include "StayOnTopDialog.h"

//--------------------------------------------------------------

// Классы построения окна для ввода текстового значения, вещественного числа и целого числа соответственно
namespace XRAD_GUI
{



class GetValueDialog : public StayOnTopDialog, public Ui::getValue
{
		Q_OBJECT
	private:
		bool eventFilter(QObject *target, QEvent *event);


	protected:
		GetValueDialog(QString prompt);
		protected slots:
		virtual void DefaultClicked() = 0;
		virtual void OKClicked();
		virtual void CancelClicked();

		virtual void MaxClicked() = 0;
		virtual void MinClicked() = 0;

		virtual void Increase() = 0;
		virtual void Decrease() = 0;
		virtual	void ValueTextChanged() = 0;
};



class GetStringDialog : public GetValueDialog
{
		Q_OBJECT

	private:
		const QString default_string;
		QString	&result;
	//		bool eventFilter(QObject *target,QEvent *event);

	public:
		GetStringDialog(QString &in_result, QString prompt, QString in_default);

		private slots:
		virtual void DefaultClicked();
		virtual void MaxClicked(){};
		virtual void MinClicked(){};
		virtual void Increase(){}
		virtual void Decrease(){}
		virtual	void ValueTextChanged();
};

//--------------------------------------------------------------

class GetFloatingDialog : public GetValueDialog
{
		Q_OBJECT

	private:
		bool allow_out_of_range;
		double default_value;
		double min_value;
		double max_value;
		double &result;

	public:
		GetFloatingDialog(double &in_result, QString prompt, double in_min, double in_max, double in_default, bool in_allow_out_of_range);

		private slots:

		virtual void DefaultClicked();
		virtual void MaxClicked();
		virtual void MinClicked();

		virtual void Increase(){}
		virtual void Decrease(){}

		virtual void ValueTextChanged();
};



class GetIntegralDialog : public GetValueDialog
{
		Q_OBJECT
	private:
		bool allow_out_of_range;
		ptrdiff_t default_value;
		ptrdiff_t min_value;
		ptrdiff_t max_value;
		ptrdiff_t &result;
		size_t	width;

	public:
		GetIntegralDialog(ptrdiff_t &in_result, QString prompt, ptrdiff_t in_min, ptrdiff_t in_max, ptrdiff_t in_default, size_t in_width, bool in_allow_out_of_range);

		private slots:
		virtual void DefaultClicked();
		virtual void MaxClicked();
		virtual void MinClicked();

		virtual void Increase();
		virtual void Decrease();

		virtual void ValueTextChanged();
};



}//namespace XRAD_GUI

#endif // XRAD__GetValueDialogH
