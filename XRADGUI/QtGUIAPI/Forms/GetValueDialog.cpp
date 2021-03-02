/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	Created by IRD on 05.2013
//  Version 3.0.2
#include "pre.h"
#include "GetValueDialog.h"

#include "ShowValueDialog.h"
#include "WorkflowControl.h"

//--------------------------------------------------------------

namespace XRAD_GUI
{
XRAD_USING



GetValueDialog::GetValueDialog(QString prompt)
{
	setupUi(this);

	setWindowTitle(prompt);
	value->installEventFilter(this);
	installEventFilter(this);

	setFixedHeight(geometry().height());
	setFixedWidth(geometry().width());

	min_button->setToolTip("Enter min value (Page Down)");
	max_button->setToolTip("Enter max value (Page Up)");
	default_button->setToolTip("Enter default value (Home)");
	ok_button->setToolTip("Returns current value");
	cancel_button->setToolTip("Cancel current operation");

	cancel_button->setEnabled(true);

	connect(default_button, SIGNAL(pressed()), this, SLOT(DefaultClicked()));
	connect(ok_button, SIGNAL(pressed()), this, SLOT(OKClicked()));
	connect(cancel_button, SIGNAL(pressed()), this, SLOT(CancelClicked()));

	connect(max_button, SIGNAL(pressed()), this, SLOT(MaxClicked()));
	connect(min_button, SIGNAL(pressed()), this, SLOT(MinClicked()));
	connect(value, SIGNAL(textChanged(const QString&)), this, SLOT(ValueTextChanged()));

	//	этот сигнал связывается для всех диалогов перед вызовом, см. Dialogs::ExecuteActiveDialog();
	// 	connect(this, SIGNAL(finished(int)), SLOT(deleteLater()));
}

bool GetValueDialog::eventFilter(QObject *target, QEvent *event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = (QKeyEvent *)event;
		// если стрелки влево/вправо и колонок больше, чем одна
		switch(keyEvent->key())
		{
			// TODO: Для ввода специальных значений использовать комбинации Ctrl+буква.
			// Клавиши Home, End переназначать нельзя, т.к. они используются
			// для редактирования строки.
			//case Qt::Key_Home:
			//	DefaultClicked();
			//	return true;

			case Qt::Key_Escape:
				DefaultClicked();
				// отмена действия по esc рисковано, можно второпях отменить чего не надо
				//CancelClicked();
				return true;

			case Qt::Key_Enter:
			case Qt::Key_Return:
				OKClicked();
				return true;

			// Клавишу Tab переназначать нельзя, т.к. она используется для навигации по диалогу.
			//case Qt::Key_Tab:
			//	return true;

			case Qt::Key_PageUp:
				MaxClicked();
				return true;

			case Qt::Key_PageDown:
				MinClicked();
				return true;

			case Qt::Key_Up:
				Increase();
				return true;

			case Qt::Key_Down:
				Decrease();
				return true;
		}

		auto modifiers = keyEvent->modifiers();
		if ((modifiers & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier)) ==
				Qt::ControlModifier)
		{
			switch (keyEvent->key())
			{
				case Qt::Key_D:
					DefaultClicked();
					return true;
			}
		}
	}

	return QObject::eventFilter(target, event);
}

//--------------------------------------------------------------

void GetValueDialog::OKClicked(void)
{
	if(ok_button->isEnabled())
	{
		accept();
	}
}

void GetValueDialog::CancelClicked(void)
{
	ScheduleCurrentOperationCancel();
//	DefaultClicked();
//	value->setText("");
	reject();
}

//--------------------------------------------------------------



GetStringDialog::GetStringDialog(QString &in_result, QString prompt, QString in_default) :
	result(in_result),
	GetValueDialog(prompt),
	default_string(in_default)
{
	numerical_params->setVisible(false); // cкрываем кнопки ввода чисел
	setFixedHeight(geometry().height() - numerical_params->geometry().height());
	DefaultClicked();
}

void GetStringDialog::DefaultClicked(void)
{
	value->setText(default_string);
	value->setFocus();
	value->selectAll();
}

void GetStringDialog::ValueTextChanged()
{
	result = value->text();
// 	cancel_button->setEnabled(default_string != result);
}

GetFloatingDialog::GetFloatingDialog(double &in_result, QString prompt, double in_min, double in_max, double in_default, bool in_allow_out_of_range) :
	result(in_result),
	GetValueDialog(prompt),
	allow_out_of_range(in_allow_out_of_range)
{

	min_button->setText(min_button->text()+": "+QString::number(in_min, 'g', 12));
	max_button->setText(max_button->text()+": "+QString::number(in_max, 'g', 12));

	default_value = in_default;
	min_value = in_min;
	max_value = in_max;

	warning->setStyleSheet("color: red");
	ok_button->setEnabled(false);

	QDoubleValidator	*validator = new QDoubleValidator(in_min, in_max, 32, this);
	validator->setLocale(QLocale::C);// если этого не сделать, может отказаться принимать точку в качестве разделителя
	value->setValidator(validator);

	DefaultClicked();
}

//TODO плохо отрабатывает десятичную точку, надо разбираться

void GetFloatingDialog::ValueTextChanged(void)
{
	QString	result_string = value->text().simplified();

	// toDouble не принимает запятую в качестве разделителя, а при вводе с цифровой клавиатуры
	// она часто приходит. ниже вручную это исправляется
	for(QString::iterator it = result_string.begin(); it<result_string.end(); ++it) if(*it==',') *it='.';

	double	raw_result = result_string.toDouble();

	warning->setText(ExtendedInformation(raw_result));
	warning->setVisible(true);

	if(value->hasAcceptableInput())
	{
		result = raw_result;

		ok_button->setEnabled(true);
// 		cancel_button->setEnabled(default_value != result);
		warning->setStyleSheet("color: green");
	}
	else
	{
		if(!allow_out_of_range)
		{
			result = default_value;
			ok_button->setEnabled(false);
		}
		else
		{
			result = raw_result;
			ok_button->setEnabled(true);
		}
// 		cancel_button->setEnabled(true);
		warning->setStyleSheet("color: red");
	}
}

void GetFloatingDialog::DefaultClicked(void)
{
	value->setText(QString::number(default_value, 'g', 12));
	value->setFocus();
	value->selectAll();
}

void GetFloatingDialog::MinClicked(void)
{
	value->setText(QString::number(min_value, 'g', 12));
	value->setFocus();
	value->selectAll();
}

void GetFloatingDialog::MaxClicked(void)
{
	value->setText(QString::number(max_value, 'g', 12));
	value->setFocus();
	value->selectAll();
}



//--------------------------------------------------------------



GetIntegralDialog::GetIntegralDialog(ptrdiff_t &in_result, QString prompt, ptrdiff_t in_min, ptrdiff_t in_max, ptrdiff_t in_default, size_t in_width, bool in_allow_out_of_range) :
	result(in_result),
	GetValueDialog(prompt),
	width(in_width),
	allow_out_of_range(in_allow_out_of_range)
{
	min_button->setText(min_button->text()+": "+QString::number(in_min));
	max_button->setText(max_button->text()+": "+QString::number(in_max));

	default_value = in_default;
	min_value = in_min;
	max_value = in_max;

	warning->setStyleSheet("color: red");
	ok_button->setEnabled(false);

	QDoubleValidator	*validator = new QDoubleValidator(in_min, in_max, 32, this);
	validator->setLocale(QLocale::C);// если этого не сделать, может отказаться принимать точку в качестве разделителя
	value->setValidator(validator);
//	value->setValidator(new QIntValidator(in_min, in_max, this));

	DefaultClicked();
}

void GetIntegralDialog::ValueTextChanged(void)
{
	QString	result_string = value->text().simplified();

	// toDouble не принимает запятую в качестве разделителя, а при вводе с цифровой клавиатуры
	// она часто приходит. ниже вручную это исправляется
	for(QString::iterator it = result_string.begin(); it<result_string.end(); ++it) if(*it==',') *it='.';

	int64_t	raw_result = range(result_string.toDouble(), -max_ptrdiff_t(), max_ptrdiff_t());
	warning->setVisible(true);
	warning->setText(ExtendedInformation(raw_result));

	if(value->hasAcceptableInput())
	{
		result = raw_result;

		ok_button->setEnabled(true);
// 		cancel_button->setEnabled(default_value != result);
		warning->setStyleSheet("color: green");
	}
	else
	{
		if(!allow_out_of_range)
		{
			result = default_value;
			ok_button->setEnabled(false);
		}
		else
		{
			result = raw_result;
			ok_button->setEnabled(true);
		}

		if(!allow_out_of_range)ok_button->setEnabled(false);
// 		cancel_button->setEnabled(true);
		warning->setStyleSheet("color: red");
	}
}

void GetIntegralDialog::DefaultClicked(void)
{
	value->setText(QString::number(default_value));
	value->setFocus();
	value->selectAll();
}

void GetIntegralDialog::MinClicked(void)
{
	value->setText(QString::number(min_value));
	value->setFocus();
	value->selectAll();
}

void GetIntegralDialog::MaxClicked(void)
{
	value->setText(QString::number(max_value));
	value->setFocus();
	value->selectAll();
}

void GetIntegralDialog::Increase(void)
{
	QString	result_string = value->text().simplified();

	if(allow_out_of_range)
	{
		result = result_string.toDouble() + 1;
	}
	else
	{
		result = range(result_string.toDouble() + 1, -max_long(), max_long());
	}

	value->setText(QString::number(result));
}

void GetIntegralDialog::Decrease(void)
{
	QString	result_string = value->text().simplified();

	if(allow_out_of_range)
	{
		result = result_string.toDouble() - 1;
	}
	else
	{
		result = range(result_string.toDouble() - 1, -max_long(), max_long());
	}

	value->setText(QString::number(result));
}



}//namespace XRAD_GUI
