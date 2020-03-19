//	file ButtonDecisionDialog.cpp
//	Created by IRD on 06.2013
//	Version 3.0.2
#include "pre_GUI.h"

//--------------------------------------------------------------

#include "DecisionDialogs.h"
#include "GuiController.h"

//--------------------------------------------------------------
//
//	диалог с командными кнопками, возвращает единственное значение
//
namespace XRAD_GUI
{



void	ButtonDecisionDialog::CalculateButtonsGrid()
{
	size_t	max_rows_number = 12;
	if(n_buttons > 36) max_rows_number += sqrt(n_buttons-36);
	// эмпирическая подгонка высоты таблицы, чтобы большое количество кнопок не слишком
	// разрасталось в высоту или ширину

	if(n_buttons <= max_rows_number)
	{
		// мало кнопок, только один столбец
		n_rows = n_buttons;
		n_columns = 1;
	}
	else if(!(n_buttons%max_rows_number))
	{
		// число кнопок делится на число строк по умолчанию
		n_rows = max_rows_number;
		n_columns = n_buttons/max_rows_number;
	}
	else
	{
		// подбор высоты столбца так, чтобы самый правый минимально отличался от остальных
		n_rows = max_rows_number;
		int n_rows_hypothesis = int(n_rows)-1;//только знаковое, size_t мешает работать алгоритму

		int	criterium = int(n_rows) - int(n_buttons%n_rows);// разница высоты последнего стоблца и остальных, минимизируем ее
		while(n_rows_hypothesis >= int(max_rows_number)/2)
		{
			int new_criterium = n_buttons%n_rows_hypothesis ?
				int(n_rows_hypothesis) - int(n_buttons%n_rows_hypothesis) :
				0;

			// во избежание слишком широких форм не даем снижаться, если количество колонок превысило 7
			if(int(ceil(double(n_buttons)/n_rows_hypothesis)) > 7) new_criterium = max_int();

			if(new_criterium==0)
			{
				// делится нацело, прекращаем поиск
				n_rows = n_rows_hypothesis;
				n_rows_hypothesis = 0;
			}
			else if(new_criterium < criterium && new_criterium > 0)
			{
				criterium = new_criterium;
				n_rows = n_rows_hypothesis;
			}
			--n_rows_hypothesis;
		}
		n_columns = size_t(ceil(double(n_buttons)/n_rows));
	}
}

ButtonDecisionDialog::ButtonDecisionDialog(
	size_t &in_result,
	QString prompt,
	const vector<QString> &buttons,
	size_t in_default_button,
	size_t in_cancel_button) :

	result(in_result),
	n_buttons(buttons.size()),
	cancel_button(in_cancel_button),
	cancel_button_assigned(false),
	default_button_assigned(false)
{
	CalculateButtonsGrid();

	size_t default_button = range(in_default_button, 0, n_buttons-1);

	result = default_button;

	layout = new QGridLayout;

	setWindowTitle(prompt);

	// делаю размер окна неизменяемым
	layout->setSizeConstraint(QLayout::SetFixedSize);
	setSizeGripEnabled(false);

	for(size_t i = 0; i < n_buttons; ++i)
	{
		QPushButton *button=new QPushButton();
		button->setParent(this);
		button->setText(buttons[i]);
		button->installEventFilter(this); //переопределяю обработчик событий

		if(i == default_button)
		{
			default_button_assigned = true;
			button->setDefault(true);
			button->setFocus();
		}
		if(i==cancel_button)
		{
			cancel_button_assigned = true;
			button->setText(buttons[i] + " (esc)");
			connect(this, SIGNAL(rejected()), button, SLOT(click()));
		}

		connect(button, SIGNAL(clicked()), this, SLOT(ButtonClicked()));
		connect(button, SIGNAL(clicked()), this, SLOT(accept()));

		layout->addWidget(button, int(i%n_rows), int(i/n_rows));
		button_objects.append(button);
	}

	for(size_t i = 0; i < n_columns; ++i)
	{
		layout->setColumnMinimumWidth(int(i), 150);
	}

	setLayout(layout);
}

bool ButtonDecisionDialog::eventFilter(QObject *target, QEvent *event)
{
	if(event->type() == QEvent::KeyPress) // если нажата клавиша клавиатуры
	{
		QKeyEvent *keyPressEvent = (QKeyEvent *)event;
		//определяем индекс текущей ячейки
		int index = layout->indexOf(qobject_cast<QWidget*>(target));
		result = index;

		switch(keyPressEvent->key())
		{
			case Qt::Key_Escape:
				// если кнопка отмены не назначена, игнорировать нажатие клавише Esc
				if(!cancel_button_assigned) return true;
				else	result = cancel_button;
				break;

			case Qt::Key_Enter:
			case Qt::Key_Return:
				// если кнопка по умолчанию не назначена, игнорировать нажатие Enter и Return
				if(!default_button_assigned) return true;
				break;

			case Qt::Key_Left:
			case Qt::Key_Right:
				// если стрелки влево/вправо и колонок больше, чем одна
				if(n_columns > 1)
				{
					//определяем индекс позицию ячейки
					int row, column, rowspan, colspan;
					layout->getItemPosition(index, &row, &column, &rowspan, &colspan);

					//меняем номер колонки
					if(keyPressEvent->key()== Qt::Key_Right) (++column)%=n_columns;
					else (column+=int(n_columns)-1)%=n_columns;

					//получаем ячейку с новыми координатами
					QLayoutItem* next= layout->itemAtPosition(row, column);
					if (next)
					{
						// и кнопку в ней и устанавливаем фокус
						QPushButton* button = qobject_cast<QPushButton*>(next->widget());
						button->setFocus();
						default_button_assigned = true;
					}

					return true;
				}
		}
	}
	return QDialog::eventFilter(target, event);
}

void ButtonDecisionDialog::ButtonClicked(void)
{
	result = button_objects.indexOf(sender());
}

//
//--------------------------------------------------------------



DecisionDialog::DecisionDialog()
{
	dialog_layout = new QVBoxLayout;
	buttons_layout = new QHBoxLayout;

	button_ok = new QPushButton(this);
	button_ok->setText("OK");
	button_ok->setDefault(true);
	button_ok->setAutoDefault(false);

	button_cancel = new QPushButton(this);
	button_cancel->setText("Cancel");
	button_cancel->setDefault(false);
	button_cancel->setAutoDefault(false);

	buttons_layout->addWidget(button_ok);
	buttons_layout->addWidget(button_cancel);

	connect(button_ok, SIGNAL(clicked()), this, SLOT(OKClicked()));
	connect(button_cancel, SIGNAL(clicked()), this, SLOT(CancelClicked()));

	dialog_layout->addLayout(buttons_layout);
	setLayout(dialog_layout);
	installEventFilter(this);
}


void DecisionDialog::OKClicked()
{
	UpdateResult();
	emit finished(0);
	accept();
}

void DecisionDialog::CancelClicked(void)
{
	ScheduleCurrentOperationCancel();
	reject();
}

bool DecisionDialog::eventFilter(QObject *target, QEvent *event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *keyEvent = (QKeyEvent *)event;
		switch(keyEvent->key())
		{
			case Qt::Key_Escape:
				CancelClicked();
				return true;
				break;

			case Qt::Key_Enter:
			case Qt::Key_Return:
				OKClicked();
				return true;
				break;
		}
	}

	return QDialog::eventFilter(target, event);
}



//--------------------------------------------------------------
//
//	диалог с флажками (результат во множественных значениях)
//
CheckboxDecisionDialog::CheckboxDecisionDialog(const QString &prompt, const vector<pair<QString, bool*> > &boxes) :
	checkboxes(boxes.size(), NULL),
	values(boxes.size())
{
	setWindowTitle(prompt);

	for(size_t i = 0; i < boxes.size(); ++i)
	{
		checkboxes[i] = new QCheckBox(this);
		checkboxes[i]->setText(boxes[i].first);
		values[i] = boxes[i].second;

		if(*values[i] == 0)
		{
			checkboxes[i]->setCheckState(Qt::Unchecked);
		}
		else
		{
			checkboxes[i]->setCheckState(Qt::Checked);
		}
		dialog_layout->insertWidget(int(i), checkboxes[i]);
		connect(checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(UpdateResult()));
	}

//	connect(this, SIGNAL(rejected()), this, SLOT(CancelClicked()));
}

void CheckboxDecisionDialog::UpdateResult(void)
{
	for(size_t i=0; i<checkboxes.size(); i++)
	{
		*values[i] = (checkboxes[i]->checkState() == Qt::Checked);
	}
}



//--------------------------------------------------------------

//TODO URGENT see comment
//	3. наладить связь между control-G и завершением активного диалога в Dialogs

RadioButtonDecisionDialog::RadioButtonDecisionDialog(
	size_t &in_result,
	const QString &prompt,
	const vector<QString> &buttons,
	size_t in_default_button)
	:
	result(in_result),
	n_buttons(buttons.size()),
	double_click_delay(chrono::milliseconds(QApplication::doubleClickInterval())),
	previous_click_time(performance_time_t(0)),
	cancel_button_assigned(false),
	default_button_assigned(false)
{
//	dialog_layout = new QVBoxLayout;
	group_box = new QGroupBox(prompt);
	group_box_layout = new QVBoxLayout;

	size_t default_button = range(in_default_button, 0, n_buttons-1);


	for(size_t i = 0; i < n_buttons; ++i)
	{
		QRadioButton *radio_button=new QRadioButton(this);
		radio_button->setText(buttons[i]);

		if(i == default_button)
		{
			radio_button->setChecked(true);
			radio_button->setFocus(Qt::MouseFocusReason);
			default_button_assigned = true;
		}
		else
		{
			radio_button->setChecked(false);
		}

		connect(radio_button, SIGNAL(clicked()), this, SLOT(UpdateResult()));
		connect(radio_button, SIGNAL(clicked()), this, SLOT(CheckForDoubleClick()));

		group_box_layout->addWidget(radio_button);
		radio_buttons.push_back(radio_button);
	}
	group_box->setLayout(group_box_layout);
	dialog_layout->insertWidget(0, group_box);

	UpdateResult();

	installEventFilter(this); //переопределяю обработчик событий
}

bool RadioButtonDecisionDialog::eventFilter(QObject *target, QEvent *event)
{
	if(event->type() == QEvent::KeyPress) // если нажата клавиша клавиатуры
	{
		QKeyEvent *keyPressEvent = (QKeyEvent *)event;
		switch(keyPressEvent->key())
		{
// 			case Qt::Key_Escape:
// 				// если кнопка отмены не назначена, игнорировать нажатие клавише Esc
// 				if(!cancel_button_assigned) return true;
//
// 				radio_buttons[cancel_button]->setFocus(Qt::MouseFocusReason);
// 				radio_buttons[cancel_button]->setChecked(true);
// 				result = cancel_button;
// 				break;

			case Qt::Key_Enter:
			case Qt::Key_Return:
				// если кнопка по умолчанию не назначена, игнорировать нажатие Enter и Return
				if(!default_button_assigned) return true;
				break;

			case Qt::Key_Up:
			case Qt::Key_Down:
				// Обеспечиваем циклическое перемещение стрелками вверх-вниз по списку, за последним вновь первый и наоборот. Обработчик по умолчанию упирается в конец
				if(keyPressEvent->key()==Qt::Key_Down) (++result)%=radio_buttons.size();
				else  (result += radio_buttons.size()-1)%=radio_buttons.size();
				radio_buttons[result]->setFocus(Qt::MouseFocusReason);
				radio_buttons[result]->setChecked(true);

				break;
		}
	}
	return DecisionDialog::eventFilter(target, event);
}

void RadioButtonDecisionDialog::OKClicked()
{
	UpdateResult();
	if(in_range(result, 0u, n_buttons-1))
	{
		emit finished(0);
		accept();
	}
}

void RadioButtonDecisionDialog::UpdateResult()
{
	result = 0;
	for(size_t i = 0; i < n_buttons; ++i)
	{
		if(radio_buttons[i]->isChecked())
		{
			result = i;
			// устанавливает фокус без пунктирной рамочки вокруг названия, для единообразия
			radio_buttons[i]->setFocus(Qt::MouseFocusReason);
			default_button_assigned = true;
		}
	}
// 	if(!in_range(result, 0u, radio_buttons.size()-1))
// 	{
// 		button_ok->setEnabled(false);
// 	}
// 	else
// 	{
// 		button_ok->setEnabled(true);
// 	}
}

void RadioButtonDecisionDialog::showEvent(QShowEvent *event)
{
// при показе ставит фокус ввода на блок радиокнопок,
// чтобы можно было сразу же управлять стрелками
	QDialog::showEvent(event);
	group_box->setFocus();
	UpdateResult();
}

void RadioButtonDecisionDialog::CheckForDoubleClick()
{
// этот слот вызывается на каждом нажатии мыши и выясняет, не было ли это вторым щелчком double-click
// если да, завершает диалог
	auto current_time = GetPerformanceCounterStd();

	if(result == previous_result)
	{
		if((current_time - previous_click_time) <= double_click_delay)
		{
			OKClicked();
		}
	}

	previous_click_time = current_time;
	previous_result = result;
}



}//namespace XRAD_GUI
