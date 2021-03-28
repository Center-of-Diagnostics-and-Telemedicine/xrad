/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "WheelZoom.h"
#include "ChartZoom.h"

namespace XRAD_GUI
{



WheelZoom::WheelZoom(ChartZoom &main_zoom) :
	QObject(),
	zoom(main_zoom)
{
	// назначаем коэффициент, определяющий изменение масштаба графика
	// при вращении колеса мыши
	wheel_zoom_factor = 1.2;
}

// Задание коэффициента масштабирования графика
// при вращении колеса мыши (по умолчанию он равен 1.2)
void WheelZoom::setWheelFactor(double in_zoom_factor)
{
	wheel_zoom_factor = in_zoom_factor;
}

enum wheel_axis_mode
{
	wheel_none=0,
	wheel_drag =	0x0001,
	wheel_zoom =	0x0002,

	preserve_center =	0x0100,
	preserve_min =		0x0200,
	preserve_max =		0x0300,
	preserve_zero =		0x0400,

	zoom_center = wheel_zoom | preserve_center,
	zoom_min = wheel_zoom | preserve_min,
	zoom_max = wheel_zoom | preserve_max,
	zoom_zero = wheel_zoom | preserve_zero
};

void	RecalculateBounds(double &min, double &max, double factor, int mode)
{
	switch(mode)
	{
		case zoom_center:
			{
				double	centre = (max+min)/2;
				double	delta = (max-min)/2;
				delta *= factor;
				max = centre+delta;
				min = centre-delta;
			}
			break;

		case zoom_min:
			{
				double	delta = factor*(max-min);
				max = min+delta;
			}
			break;

		case zoom_max:
			{
				double	delta = factor*(max-min);
				min = max-delta;
			}
			break;

		case zoom_zero:
			{
				min *= factor;
				max *= factor;
			}
			break;

		case wheel_drag:
			{
				double	delta = max-min;
				double increment = factor>1?
								-delta*(factor - 1):
								delta*(1./factor - 1);
				max+=increment;
				min+=increment;
			}

		default:
		case wheel_none:
			return;
	};
}


// Применение изменений по вращении колеса мыши
void WheelZoom::applyWheel(QEvent *event)
{
	// TODO: operator| for Qt::KeyboardModifier is not constexpr in Qt for MSVC 2015.
	// So we need to convert the values to int for constexpr bitwise or. Waiting for compiler + Qt update...
	enum
	{
		alt_shift = int(Qt::ShiftModifier)|int(Qt::AltModifier),
		ctrl_shift = int(Qt::ShiftModifier)|int(Qt::ControlModifier),
		ctrl_alt = int(Qt::AltModifier)|int(Qt::ControlModifier),
		ctrl_alt_shift = int(Qt::ShiftModifier)|int(Qt::ControlModifier)|int(Qt::AltModifier)
	};


	// приводим тип QEvent к QWheelEvent
	QWheelEvent *wheel_event= static_cast<QWheelEvent *>(event);

	// определяем положение курсора относительно геометрического центра графика
	bool cursor_right = (wheel_event->position().x() - (zoom.plot()->canvas()->geometry().x() + zoom.plot()->canvas()->geometry().width()/2))>0;
	bool cursor_up = (wheel_event->position().y() - (zoom.plot()->canvas()->geometry().y() + zoom.plot()->canvas()->geometry().height()/2))<0;

	int mode_x;
	int mode_y;

	Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();

	switch(modifiers)
	{
		case Qt::NoModifier:
			mode_x=wheel_none;
			mode_y=wheel_drag;
			break;

		case Qt::ControlModifier:
			mode_x=wheel_drag;
			mode_y=wheel_none;
			break;

		case Qt::ShiftModifier:
			mode_x=wheel_none;
			mode_y=zoom_center;
			break;

		case Qt::AltModifier:
			mode_x=zoom_center;
			mode_y=wheel_none;
			break;

		case alt_shift:
			mode_x=zoom_center;
			mode_y=zoom_center;
			break;

		case ctrl_shift:
			mode_x=wheel_none;
			if(cursor_up) mode_y=zoom_max;
			else mode_y=zoom_min;
			break;

		case ctrl_alt:
			if(cursor_right) mode_x=zoom_max;
			else mode_x=zoom_min;
			mode_y=wheel_none;
			break;

		case ctrl_alt_shift:
			if(cursor_up) mode_y=zoom_max;
			else mode_y=zoom_min;
			if(cursor_right) mode_x=zoom_max;
			else mode_x=zoom_min;
			break;

		default:
			mode_x=mode_y=wheel_none;
			break;
	};

	// нажатый Alt "меняет местами" горизонтальную и вертикальную прокрутку.
	// а нам это не нужно, т.к. мы используем Alt в качестве самостоятельного
	// действующего фактора. поэтому вводим соответствующую поправку

	if (wheel_event->orientation() == Qt::Vertical)
	{
		if((modifiers&Qt::AltModifier))
			std::swap(mode_x,mode_y);
	}
	else
	{
		if(!(modifiers&Qt::AltModifier))
			std::swap(mode_x,mode_y);
	}

	// определяем угол поворота колеса мыши
	// (значение 120 соответствует углу поворота 15°)
	double wheel_increment = double(wheel_event->angleDelta().y())/120;
	// вычисляем масштабирующий множитель
	// (во сколько раз будет увеличен/уменьшен график)
	if (wheel_increment != 0)    // если колесо вращалось, то
	{
		double	wheel_factor = wheel_zoom_factor*wheel_increment;
		double	resize_factor = wheel_increment>0 ? 1/wheel_factor : -wheel_factor;

		range2_F64 bounds = zoom.GetCurrentBounds();

		RecalculateBounds(bounds.x1(), bounds.x2(), resize_factor, mode_x);
		RecalculateBounds(bounds.y1(), bounds.y2(), resize_factor, mode_y);

		zoom.SetBounds(bounds, true);
	}
}



}//namespace XRAD_GUI
