#include "pre.h"
#include "StyleSheetUtils.h"


QString getStyleSheet(QColor color)
{

	QString reschecked =
		QString(" QRadioButton::indicator{ width:20px;	height: 20px;    border - radius:  7px;}") +
		QString("QRadioButton::indicator:checked{" +
			getBackColorStyleSheet(color)) +
		QString("border:2px solid black;}") +
		QString("QRadioButton::indicator:unchecked{" +
			getBackColorStyleSheet(color.red(), color.green(), color.blue(), color.alpha() / 1.5)) +
		QString("border:2px solid grey;}");


	return reschecked;
}

QString getBackColorStyleSheet(int r, int g, int b, int a)
{
	QString result;

	result.append("background-color: rgba(");
	result.append(QString::number(r));
	result.append(", ");
	result.append(QString::number(g));
	result.append(", ");
	result.append(QString::number(b));
	result.append(", ");
	result.append(QString::number(a));
	result.append(");");

	return result;
}

QString getBackColorStyleSheet(QColor color)
{
	QString result;

	result.append("background-color: rgba(");
	result.append(QString::number(color.red()));
	result.append(", ");
	result.append(QString::number(color.green()));
	result.append(", ");
	result.append(QString::number(color.blue()));
	result.append(", ");
	result.append(QString::number(color.alpha()));
	result.append(");");

	return result;
}

