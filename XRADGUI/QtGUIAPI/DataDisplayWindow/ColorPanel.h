#ifndef COLOR_PANEL_H
#define  COLOR_PANEL_H

#include "pre.h"

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QRadioButton>
#include "StyleSheetUtils.h"
#include <QSpinBox>
#include <QEvent>
#include <QMouseEvent>

class ColorPanel : public QWidget
{
	Q_OBJECT

public:
	ColorPanel(QWidget* parent);
	~ColorPanel();

	QColor currentColor();
	

private slots:
	void click();


private: //methods

	bool eventFilter(QObject* target, QEvent* event);
	QSpinBox* getNewSpinBox(int min, int max);

	void setColorPanelValue(const QColor&);
	void setColorPickerValue(const QColor&);

private:  //fields
	QLabel* m_title_label;
	QLabel* m_red_label;
	QLabel* m_green_label;
	QLabel* m_blue_label;

	QSpinBox* m_red_spin_box;
	QSpinBox* m_gren_spin_box;
	QSpinBox* m_blue_spin_box;

	QHBoxLayout* m_main_layout;

	size_t m_changed_item_number = 0;
	

	std::vector<QRadioButton*> m_items;
	std::vector<QColor> m_colors;





};


#endif //COLOR_PANEL_H