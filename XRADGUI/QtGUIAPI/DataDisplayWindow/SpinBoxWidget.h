#ifndef SPIN_BOX_WIDGET_H
#define SPIN_BOX_WIDGET_H

#include "pre.h"

#include <QWidget>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
class SpinBoxWidget : public QWidget
{
	Q_OBJECT

public:
	SpinBoxWidget(QWidget *parent, const QString& title);
	SpinBoxWidget(QWidget *parent, const QString& title, int min, int max);
	
	~SpinBoxWidget();

	int value();
	void setRange(int min, int max);
	void setVal(int value);


private:

	QSpinBox* spin_box_;
	QHBoxLayout* main_layout_;

	QLabel* title_label_;
	

};

#endif // SPIN_BOX_WIDGET_H