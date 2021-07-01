#include "pre.h"
#include "SpinBoxWidget.h"


SpinBoxWidget::SpinBoxWidget(QWidget* parent, const QString& title)
	: QWidget(parent)
{
	m_main_layout = new QHBoxLayout(this);
	m_title_label = new QLabel(title, this);
	m_spin_box = new QSpinBox(this);

	m_main_layout->addWidget(m_title_label);
	m_main_layout->addWidget(m_spin_box);
}
SpinBoxWidget::SpinBoxWidget(QWidget* parent, const QString& title, int min, int max)
	: QWidget(parent)
{
	m_main_layout = new QHBoxLayout(this);
	m_title_label = new QLabel(title, this);
	m_spin_box = new QSpinBox(this);
	m_spin_box->setRange(min, max);

	

	m_spin_box->installEventFilter(parent);
	m_main_layout->addWidget(m_title_label);
	m_main_layout->addWidget(m_spin_box);
}

SpinBoxWidget::~SpinBoxWidget()
{
}

int SpinBoxWidget::value()
{
	return this->m_spin_box->value();
}

void SpinBoxWidget::setRange(int min, int max)
{
	this->m_spin_box->setMinimum(min);
	this->m_spin_box->setMaximum(max);

}

void SpinBoxWidget::setVal(int val)
{
	m_spin_box->setEnabled(false);
	m_spin_box->setValue(val);
	m_spin_box->setEnabled(true);
	m_spin_box->update();
}
