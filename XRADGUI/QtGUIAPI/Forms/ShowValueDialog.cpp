/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "ShowValueDialog.h"

/********************************************************************
	created:	2015/01/13
	created:	13:1:2015   14:49
	author:		kns
*********************************************************************/

//--------------------------------------------------------------

namespace XRAD_GUI
{
XRAD_USING



QString	ExtendedInformation(double value)
{
	return QString(ssprintf("hex = %.17A\nfixed = %.17f\nexp = %.17e", value, value, value).c_str());
}

QString	ExtendedInformation(int64_t value)
{
	if(!(value & 0xFFFFFFFF00000000))
	{
		return QString(ssprintf("hex = 0x%lX; ", value).c_str()) +
			QString(ssprintf("oct = 0%lo;", value).c_str()) +
			QString("\nbin = ") + (QString("%1b").arg(value, 0, 2)).right(32);
	}
	else
	{
		return QString(ssprintf("hex = 0x%lX", uint32_t(value>>32)).c_str()) + QString(ssprintf(" %.8lX; ", uint32_t(value)).c_str()) +
			QString("\nbin = ") + (QString("%1b").arg(value, 0, 2)).right(64);
	}
}

//--------------------------------------------------------------

ShowValueDialog::ShowValueDialog(QString prompt)
{
	setupUi(this);

	setWindowTitle(prompt);
	comment->setStyleSheet("color: green");
	connect(ok_button, SIGNAL(pressed()), this, SLOT(OKClicked()));
}

void ShowValueDialog::OKClicked()
{
	accept();
}

//TODO отображение текста в окошке сделано, но очень сырое.
// Qt 5 и 4 версии по-разному понимают спецсимволы вроде QChar::Tabulation.
// для 4-й версии была сделана заглушка, сейчас удалена
void	Detabify(QString &text)
{
	// ширина табуляции в пикселях считается неоднозначно.
	// приходится перераспределять самостоятельно.

	QString	result;
	QString::iterator it = text.begin(), ie = text.end();

	int	char_in_string(0);
	int	tab_distance = 10;

	for(;it<ie;++it)
	{
		if(*it==QChar::LineFeed) char_in_string = 0;
		else ++char_in_string;
		if(*it!=QChar::Tabulation) result += *it;
		else
		{
			if(!(char_in_string%tab_distance)) ++char_in_string, result += QChar::Space;
			while(char_in_string%tab_distance)  ++char_in_string, result += QChar::Space;
		}
	}
	text = result;
}

void	SetTableSizes(QLabel *label, QString &text)
{
	// 	QFont	font("Times New Roman", 12);
	// 	label->setFont(font);
	// 	QFontMetrics fm(font);
	Detabify(text);

	QFontMetrics fm(label->font());
//	int	s = text.size();

	for(QString::iterator it = text.begin(); it<text.end(); ++it)
	{
		if(*it== "\t")
		{
			*it=QChar::Space;
		}
	}

	int	width(0), height(0);
	QString::iterator	it = text.begin(), ie=text.end(), pos;
	while(it<text.end())
	{
		pos = std::find(it, ie, QChar::LineFeed);
		int	len = pos-it;
		if(len)
		{
			QString	line;
			line.resize(len);
			std::copy(it, pos, line.begin());
			width = max(width, fm.width(line));
			height += fm.height();
			it=pos+1;
		}
	}
	label->setMinimumSize(width, height);
}

ShowFloatingDialog::ShowFloatingDialog(QString prompt, double value) : ShowValueDialog(prompt)
{
	display_value->setText(QString("%1").arg(value));
	comment->setText(ExtendedInformation(value));
}

ShowIntegralDialog::ShowIntegralDialog(QString prompt, int64_t value) : ShowValueDialog(prompt)
{
	//TODO Следующая строка неправильно обрабатывает числа, превышающие 0xFFFFFFFF. Заменить на ssprintf
	display_value->setText(QString("%1").arg(value));
	comment->setText(ExtendedInformation(value));
}



ShowStringDialog::ShowStringDialog(QString prompt, QString display_string) : ShowValueDialog(prompt)
{
		display_value->setText(display_string);
		comment->setText(QString("string length = ") + QString("%1").arg(display_string.length()));
}

}//namespace XRAD_GUI
