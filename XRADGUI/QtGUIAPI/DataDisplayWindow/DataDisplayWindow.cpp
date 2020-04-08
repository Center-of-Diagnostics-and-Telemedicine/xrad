//	Created by KNS on 11.2013
//  Version 3.0.5
//--------------------------------------------------------------
#include "pre.h"
#include "DataDisplayWindow.h"

#include "ConsoleWindow.h"
#include "GUIController.h"

namespace XRAD_GUI
{

using namespace std;

list<int>	DataDisplayWindow::object_ids(0);
int	DataDisplayWindow::object_count = 0;

void	DataDisplayWindow::SetWindowPosition()
{
	// начальное положение окна
	auto	corner = GetCornerPosition();
	auto	window_size = WindowGeometry::default_graph_window_size();
	setGeometry(QRect(QPoint(corner.x(), corner.y()), window_size));//TODO размер взят из окна графика, должно быть другим
}


void DataDisplayWindow::init()
{

	cached_allow_stay_on_top = gui_controller.GetStayOnTopAllowed();

	object_id = GenerateId();
	Register(object_id);
	++object_count;

	UpdateWindowFlags();

	gui_controller.AddGUISettingsChangedTarget(this);
	//добавляем объект в массив диалогов
	gui_controller.AddWidget(this);
	setAttribute(Qt::WA_DeleteOnClose);
// 	installEventFilter(this); //здесь это вызывать нельзя, приводит к ошибке

	SetWindowPosition();
}

DataDisplayWindow::~DataDisplayWindow()
{
	//удаляем объект из массива диалогов
	gui_controller.RemoveWidget(this);

	gui_controller.RemoveGUISettingsChangedTarget(this);

	b_window_persistent = false;
	Unregister(object_id);
	--object_count;
}

void	DataDisplayWindow::Register(int id)
{
	try
	{
		object_ids.push_back(id);
	}
	catch(...)
	{
	}
}

void	DataDisplayWindow::Unregister(int id)
{
	try
	{
		list<int>::iterator ie = object_ids.end();
		list<int>::iterator found = std::find(object_ids.begin(), ie, id);

		if(found != ie)
		{
			object_ids.erase(found);
		}
	}
	catch(...)
	{
	}
}

int	DataDisplayWindow::GenerateId()
{
	try
	{
		size_t	n = 1;

		// ищет в списке первый незанятый номер и возвращает его
		while(n <= object_ids.size())
		{
			if(object_ids.end() == std::find(object_ids.begin(), object_ids.end(), n)) return int(n);
			++n;
		}
		return int(n);
	}

	catch(...)
	{
		return 0;
	}
}

point2_PDT	DataDisplayWindow::GetCornerPosition()
{
	int result = (WindowGeometry::top_margin() + object_id*WindowGeometry::window_stair_step()) % WindowGeometry::lowest_window_position();
	return point2_PDT(result, result);
}

void DataDisplayWindow::GUISettingsChanged()
{
	bool allow = gui_controller.GetStayOnTopAllowed();
	if(allow == cached_allow_stay_on_top)
		return;
	cached_allow_stay_on_top = allow;
	UpdateWindowFlags();
}



void DataDisplayWindow::SetPersistent(bool persistent)
{
	if(b_window_persistent == persistent)
		return;
	b_window_persistent = persistent;
	UpdateWindowFlags();
}

void DataDisplayWindow::SetStayOnTop(bool stay_on_top)
{
	if(stay_on_top == stay_on_top_flag)
		return;
	stay_on_top_flag = stay_on_top;
	UpdateWindowFlags();
}

//--------------------------------------------------------------


void DataDisplayWindow::UpdateWindowFlags()
{
	bool visible = isVisible();
	WindowFlags wf = windowFlags();
	wf |= CustomizeWindowHint | WindowTitleHint | WindowMinimizeButtonHint | WindowCloseButtonHint | Qt::WindowSystemMenuHint;
	if(stay_on_top_flag && cached_allow_stay_on_top)
		wf |= WindowStaysOnTopHint;
	else
		wf &= ~WindowStaysOnTopHint;

	if(b_window_persistent)
		wf &= ~(WindowCloseButtonHint | WindowMinimizeButtonHint);
	else
		wf |= (WindowCloseButtonHint | WindowMaximizeButtonHint);
	setWindowFlags(wf);
	if(visible)
		show(); // Qt может сделать окно невидимым при вызове setWindowFlags().
}

void DataDisplayWindow::keyPressEvent(QKeyEvent *event)
{
	if(event->type()==QEvent::KeyPress)
	{
		switch(event->key())
		{
			break;

			case Qt::Key_F4:
				if(b_window_persistent) return;
				break;

			case Qt::Key_Escape:
				if(b_window_persistent) return;
				emit signal_esc();
				break;
		};
	}
	//TODO если не вызвать выше signal_esc, то закрытие окна по умолчанию происходит неправильно. Надо бы понять, почему
	return parent::keyPressEvent(event);
}


}//namespace XRAD_GUI
