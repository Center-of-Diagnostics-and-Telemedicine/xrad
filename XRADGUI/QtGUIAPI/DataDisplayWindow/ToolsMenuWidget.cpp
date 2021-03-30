#include "pre.h"
#include "ToolsMenuWidget.h"


XRAD_USING

ToolsMenuWidget::ToolsMenuWidget(QWidget* parent)
	: QWidget(parent)
{
	main_layout_ = new QVBoxLayout(this);
	main_layout_->setSpacing(5);
	main_layout_->setMargin(0);

	installEventFilter(main_layout_);
	installEventFilter(this);

	chosen_item_value_ = Drawers::Hand;
}

ToolsMenuWidget::ToolsMenuWidget(int x, int y, QWidget* parent) :
	QWidget(parent)
{
	setGeometry(x, y, 0, 0);

	main_layout_ = new QVBoxLayout(this);
	main_layout_->setSpacing(5);
	main_layout_->setMargin(0);
	 


	chosen_item_value_ = Drawers::Hand;
	installEventFilter(this);
}

ToolsMenuWidget::~ToolsMenuWidget()
{
	delete main_layout_;
	for (size_t i = 0; i < items_.size(); i++)
	{
		delete items_[i];
	}
}

void ToolsMenuWidget::addItem(size_t w, size_t h, int drawer, const QIcon& icon)
{
	items_.push_back(getNewButton(0, 0, w, h,  icon, this));
	values_.push_back(drawer);

	items_[0]->setChecked(true);
	chosen_item_value_ = values_[0];
}



int ToolsMenuWidget::getCurrentDrawer()
{
	return chosen_item_value_;
}

QRadioButton* ToolsMenuWidget::getNewButton(int x, int y, size_t w, size_t h, const QIcon& icon, QWidget* parent)
{
	QRadioButton* result = new QRadioButton(parent);
	buttons_counter_++;

	result->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	result->setGeometry(x, y, int(w), int(h));
	result->setIcon(icon);
	result->setStyleSheet("");
	result->setIconSize(QSize(int(w), int(h)));
	result->setFocusPolicy(Qt::TabFocus);
	
	connect(result, &QRadioButton::clicked, this, &ToolsMenuWidget::click);

	main_layout_->addWidget(result);

	setGeometry(this->x(), this->y(), int(w) + 2, height() + int(h) + main_layout_->spacing());

	

	return result;
}

void ToolsMenuWidget::click()
{

	for (size_t i = 0; i < items_.size(); i++)
	{
		if (items_[i]->isChecked())
		{
			chosen_item_value_ = values_[i];
		}
	}
}