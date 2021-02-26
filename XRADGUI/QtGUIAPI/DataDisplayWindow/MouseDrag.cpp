/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "MouseDrag.h"

#include "ChartZoom.h"

namespace XRAD_GUI
{

MouseDrag::MouseDrag( ChartZoom &in_zoom ) :zoom(in_zoom)
{
	zoom.plot()->installEventFilter(this);
}


bool MouseDrag::eventFilter(QObject *target,QEvent *event)
{
	// если событие произошло для графика, то
	if (target == zoom.plot())
	{
		// если произошло одно из событий от мыши, то
		// вызываем соответствующий обработчик
		switch(event->type())
		{
			case QEvent::MouseButtonPress:
			case QEvent::MouseMove:
			case QEvent::MouseButtonRelease:
				procMouseEvent(event);
				break;

			default:
				break;
		};
	}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target,event);
}

}//namespace XRAD_GUI
