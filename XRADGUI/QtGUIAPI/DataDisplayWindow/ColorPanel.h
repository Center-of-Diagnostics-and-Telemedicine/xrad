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
	QLabel* title_label_;
	QLabel* red_label_;
	QLabel* green_label_;
	QLabel* blue_label_;

	QSpinBox* red_spin_box_;
	QSpinBox* gren_spin_box_;
	QSpinBox* blue_spin_box_;

	QHBoxLayout* main_layout_;

	size_t changed_item_number_ = 0;
	

	std::vector<QRadioButton*> items_;
	std::vector<QColor> colors_;





};


#endif //COLOR_PANEL_H