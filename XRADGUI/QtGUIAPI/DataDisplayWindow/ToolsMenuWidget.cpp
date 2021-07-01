#include "pre.h"
#include "ToolsMenuWidget.h"


XRAD_USING

ToolsMenuWidget::ToolsMenuWidget(QWidget* parent)
	: QWidget(parent)
{
	m_main_layout = new QVBoxLayout(this);
	m_main_layout->setSpacing(5);
	m_main_layout->setMargin(0);

	installEventFilter(m_main_layout);
	installEventFilter(this);

	m_chosen_item_value = Drawers::Hand;
}

ToolsMenuWidget::ToolsMenuWidget(int x, int y, QWidget* parent) :
	QWidget(parent)
{
	setGeometry(x, y, 0, 0);

	m_main_layout = new QVBoxLayout(this);
	m_main_layout->setSpacing(5);
	m_main_layout->setMargin(0);
	 


	m_chosen_item_value = Drawers::Hand;
	installEventFilter(this);
}

ToolsMenuWidget::~ToolsMenuWidget()
{
	delete m_main_layout;
	for (size_t i = 0; i < m_items.size(); i++)
	{
		delete m_items[i];
	}
}

void ToolsMenuWidget::addItem(size_t w, size_t h, int drawer, const QIcon& icon)
{
	m_items.push_back(getNewButton(0, 0, w, h,  icon, this));
	m_values.push_back(drawer);

	m_items[0]->setChecked(true);
	m_chosen_item_value = m_values[0];
}



int ToolsMenuWidget::getCurrentDrawer()
{
	return m_chosen_item_value;
}

QRadioButton* ToolsMenuWidget::getNewButton(int x, int y, size_t w, size_t h, const QIcon& icon, QWidget* parent)
{
	QRadioButton* result = new QRadioButton(parent);
	m_buttons_counter++;

	result->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	result->setGeometry(x, y, int(w), int(h));
	result->setIcon(icon);
	result->setStyleSheet("");
	result->setIconSize(QSize(int(w), int(h)));
	result->setFocusPolicy(Qt::TabFocus);
	
	connect(result, &QRadioButton::clicked, this, &ToolsMenuWidget::click);

	m_main_layout->addWidget(result);

	setGeometry(this->x(), this->y(), int(w) + 2, height() + int(h) + m_main_layout->spacing());

	

	return result;
}

void ToolsMenuWidget::click()
{

	for (size_t i = 0; i < m_items.size(); i++)
	{
		if (m_items[i]->isChecked())
		{
			m_chosen_item_value = m_values[i];
		}
	}
}