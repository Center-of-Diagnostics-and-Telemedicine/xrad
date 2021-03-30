#include "pre.h"
#include "SpinBoxWidget.h"


SpinBoxWidget::SpinBoxWidget(QWidget* parent, const QString& title)
	: QWidget(parent)
{
	main_layout_ = new QHBoxLayout(this);
	title_label_ = new QLabel(title, this);
	spin_box_ = new QSpinBox(this);

	main_layout_->addWidget(title_label_);
	main_layout_->addWidget(spin_box_);
}
SpinBoxWidget::SpinBoxWidget(QWidget* parent, const QString& title, int min, int max)
	: QWidget(parent)
{
	main_layout_ = new QHBoxLayout(this);
	title_label_ = new QLabel(title, this);
	spin_box_ = new QSpinBox(this);
	spin_box_->setRange(min, max);

	

	spin_box_->installEventFilter(parent);
	main_layout_->addWidget(title_label_);
	main_layout_->addWidget(spin_box_);
}

SpinBoxWidget::~SpinBoxWidget()
{
}

int SpinBoxWidget::value()
{
	return this->spin_box_->value();
}

void SpinBoxWidget::setRange(int min, int max)
{
	this->spin_box_->setMinimum(min);
	this->spin_box_->setMaximum(max);

}

void SpinBoxWidget::setVal(int val)
{
	spin_box_->setEnabled(false);
	spin_box_->setValue(val);
	spin_box_->setEnabled(true);
	spin_box_->update();
}
