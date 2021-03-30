#include "pre.h"
#include "ColorPanel.h"

ColorPanel::ColorPanel(QWidget* parent)
	: QWidget(parent)
{
	main_layout_ = new QHBoxLayout(this);
	main_layout_->setSpacing(5);
	main_layout_->setMargin(0);

	title_label_ = new QLabel("Color:");
	red_label_ = new QLabel("R:");
	green_label_ = new QLabel("G:");
	blue_label_ = new QLabel("B:");


	red_spin_box_ = getNewSpinBox(0, 255);
	gren_spin_box_ = getNewSpinBox(0, 255);
	blue_spin_box_ = getNewSpinBox(0, 255);



	main_layout_->addWidget(title_label_);
	main_layout_->addWidget(red_label_);
	main_layout_->addWidget(red_spin_box_);
	main_layout_->addWidget(green_label_);
	main_layout_->addWidget(gren_spin_box_);
	main_layout_->addWidget(blue_label_);
	main_layout_->addWidget(blue_spin_box_);


	for (size_t i = 0; i < 5; i++)
	{
		items_.push_back(new QRadioButton);
		colors_.push_back(QColor(QColor::fromRgb(i % 2 == 0 && i != 4 ? 255 : 0, i % 3 == 0 ? 255 : 0, i % 4 == 0 ? 255 : 0)));
	}
	for (size_t i = 0; i < items_.size(); i++)
	{

		items_[i]->setStyleSheet(getStyleSheet(colors_[i]));
		items_[i]->setObjectName((QString)int(i));
		items_[i]->setFocusPolicy(Qt::TabFocus);
		main_layout_->addWidget(items_[i]);
		connect(items_[i], &QRadioButton::clicked, this, &ColorPanel::click);
	}



	items_[0]->setChecked(true);
	setColorPickerValue(colors_[0]);

	installEventFilter(this);


}
ColorPanel::~ColorPanel()
{

}

bool ColorPanel::eventFilter(QObject* target, QEvent* event)
{
	if (target == blue_spin_box_ || target == red_spin_box_ || target == gren_spin_box_)
	{
   		setColorPanelValue(QColor::fromRgb(red_spin_box_->value(), gren_spin_box_->value(), blue_spin_box_->value()));
		
	}

	return parent()->eventFilter(target, event);
	
}

QSpinBox* ColorPanel::getNewSpinBox(int min, int max)
{
	QSpinBox* result = new QSpinBox(this);
	result->setRange(min, max);
	result->setFocusPolicy(Qt::FocusPolicy::ClickFocus);

	result->installEventFilter(this);

	return result;
}
QColor ColorPanel::currentColor()
{
	return colors_[changed_item_number_];
}

void ColorPanel::setColorPanelValue(const QColor& in_color)
{
	colors_[changed_item_number_] = in_color;
	items_[changed_item_number_]->setStyleSheet(getStyleSheet(in_color));
}

void ColorPanel::setColorPickerValue(const QColor& color)
{
	red_spin_box_->setValue(color.red());
	gren_spin_box_->setValue(color.green());
	blue_spin_box_->setValue(color.blue());
}

void ColorPanel::click()
{
	QObject* obj = sender();

	for (size_t i = 0; i < items_.size(); i++)
	{
		if (obj->objectName() == items_[i]->objectName())
		{
			changed_item_number_ = i;
			setColorPickerValue(colors_[i]);
		}
	}

}

