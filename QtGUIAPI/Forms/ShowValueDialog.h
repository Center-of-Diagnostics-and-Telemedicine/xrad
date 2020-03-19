#ifndef __show_value_h
#define __show_value_h
/********************************************************************
	created:	2015/01/13
	created:	13:1:2015   14:49
	author:		kns
*********************************************************************/

#include "ui_ShowValueDialog.h"
#include "StayOnTopDialog.h"
#include <DataDisplayWindow.h>
#include <XRADGUIApi.h>//TODO не на месте

namespace XRAD_GUI
{
XRAD_USING

void	SetTableSizes(QLabel *label, QString &text);


class ShowValueDialog : public StayOnTopDialog, public Ui::showValue
{
		Q_OBJECT

	protected:
		ShowValueDialog(QString prompt);
		protected slots:
		virtual void OKClicked();
};

class ShowStringDialog : public ShowValueDialog/*, public DataDisplayWindow*/
{
		Q_OBJECT

	public:

		ShowStringDialog(QString prompt, QString display_string, GUIController &gc/*, display_text_dialog_status status*/);

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

#endif // __show_value_h
