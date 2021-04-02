#ifndef TOOLS_MENU_WIDGET_H
#define TOOLS_MENU_WIDGET_H

#include "pre.h"
#include <QWidget.h>
#include <QPushButton>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <vector>
#include "PaintWidget.h"
#include <QRadioButton>


class ToolsMenuWidget : public QWidget
{

public:
	ToolsMenuWidget(QWidget* parent);
	ToolsMenuWidget(int x, int y, QWidget* parent);
	~ToolsMenuWidget();

	void addItem(size_t w, size_t h, int drawer, const QIcon& icon);

	int getCurrentDrawer();

private: //methods

	QRadioButton* getNewButton(int x, int y, size_t w, size_t h, const QIcon& icon, QWidget* parent);

private slots:
	void click();


private: //fields

	std::vector<QRadioButton*> items_;
	std::vector<int> values_;

	int chosen_item_value_;

	QVBoxLayout* main_layout_;



	size_t buttons_counter_ = 0;
	size_t button_width = 40;
	size_t button_height = 40;

};

#endif // TOOLS_MENU_WIDGET_H