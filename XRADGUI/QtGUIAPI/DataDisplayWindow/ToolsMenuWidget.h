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

	std::vector<QRadioButton*> m_items;
	std::vector<int> m_values;

	int m_chosen_item_value;

	QVBoxLayout* m_main_layout;



	size_t m_buttons_counter = 0;
	size_t m_button_width = 40;
	size_t m_button_height = 40;

};

#endif // TOOLS_MENU_WIDGET_H