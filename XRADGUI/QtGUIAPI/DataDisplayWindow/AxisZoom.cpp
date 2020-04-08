#include "pre.h"
#include "AxisZoom.h"
#include <qwt_scale_widget.h>



namespace XRAD_GUI
{

AxisZoom::AxisZoom(ChartZoom &in_zoom):
	QObject(),
	zoom(in_zoom),
	zoom_mode(e_conversion_none),
	min_scalable_sizes(18, 36),
	max_scalefactor(4),
	min_scalefactor(0.125)
{

	for (int axis=0; axis < QwtPlot::axisCnt; ++axis)
	{
		// назначаем обработчик событий (фильтр событий)
		zoom.plot()->axisWidget(axis)->installEventFilter(this);
	}
}



// Обработчик всех событий
bool AxisZoom::eventFilter(QObject *target, QEvent *event)
{
	QwtPlot::Axis target_axis(QwtPlot::axisCnt);
	bool	found = false; // шкала пока не найдена
	// просматриваем список шкал
	for (int axis_no=0; axis_no < QwtPlot::axisCnt && !found; ++axis_no)
	{
		// если событие произошло для данной шкалы, то
		if (target == zoom.plot()->axisWidget(axis_no))
		{
			target_axis = static_cast<QwtPlot::Axis>(axis_no);     // запоминаем номер шкалы
			found = true;
		}
	}
	// если шкала была найдена, то
	if (found)
	{
		// если произошло одно из событий от мыши, то
		switch(event->type())
		{
			case QEvent::MouseButtonPress:
			case QEvent::MouseMove:
			case QEvent::MouseButtonRelease:
				AxisMouseEvent(event,target_axis);
				break;
			default:
				break;
		};
	}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target,event);
}



// Применение результатов перемещения границы шкалы
void AxisZoom::ProcAxisZoom(QMouseEvent *mouse_event)
{
	// определяем текущее положение курсора относительно канвы (за вычетом смещений графика)
	point2_I32 p = zoom.CursorPosition(mouse_event) - graph_to_widget;
	range2_F64 new_scale = scale_at_start;

	switch (zoom_mode)
	{
		// режим изменения левой границы
		case e_axis_left:
		{
			// ограничение на положение курсора справа
			p.x() = range(p.x(), -max_int(), scale_at_start_pix.width()-1);
			// вычисляем масштабный множитель
			double	scale_factor = range(
				double(scale_at_start_pix.width() - start_cursor_position.x()) / (scale_at_start_pix.width() - p.x()),
				min_scalefactor, max_scalefactor);
			// вычисляем новую левую границу
			new_scale.x1() = scale_at_start.x2() - scale_at_start.width()*scale_factor;
			break;
		}
		// режим изменения правой границы
		case e_axis_right:
		{
			// ограничение на положение курсора слева
			p.x() = range(p.x(), 1, max_int());
			double	scale_factor = range(
				double(start_cursor_position.x()) / p.x(),
				min_scalefactor, max_scalefactor);
			// вычисляем новую правую границу
			new_scale.x2() = scale_at_start.x1() + scale_at_start.width()*scale_factor;;
			break;
		}
		// режим изменения нижней границы
		case e_axis_bottom:
		{
			// ограничение на положение курсора сверху
			p.y() = range(p.y(), 1, max_int());
			double	scale_factor = range(
				double(start_cursor_position.y()) / p.y(),
				min_scalefactor, max_scalefactor);

			// вычисляем новую нижнюю границу
			new_scale.y1() = scale_at_start.y2() - scale_at_start.height()*scale_factor;
			// устанавливаем ее для вертикальной шкалы
			break;
		}
		// режим изменения верхней границы
		case e_axis_top:
		{
			// ограничение на положение курсора снизу
			p.y() = range(p.y(), -max_int(), scale_at_start_pix.height()-1);
			double	scale_factor = range(
				double(scale_at_start_pix.height() - start_cursor_position.y()) / (scale_at_start_pix.height() - p.y()),
				min_scalefactor, max_scalefactor);
			// вычисляем новую верхнюю границу
			new_scale.y2() = scale_at_start.y1() + scale_at_start.height()*scale_factor;
			break;
		}
		// для прочих режимов ничего не делаем
		default:
			break;
	}
	// если какя-либо граница изменилась, то перестраиваем график
	if (new_scale != scale_at_start)
		{
		zoom.SetBounds(new_scale, true);
		zoom.plot()->replot();
	}
}



// Обработчик событий от мыши для шкалы
void AxisZoom::AxisMouseEvent(QEvent *event,QwtPlot::Axis axis)
{
	// создаем указатель на событие от мыши
	QMouseEvent *mouse_event = static_cast<QMouseEvent *>(event);
	// в зависимости от типа события вызываем соответствующий обработчик
	switch (event->type())
	{
		// нажата кнопка мыши
		case QEvent::MouseButtonPress:
			StartAxisZoom(mouse_event,axis);
			break;
		// перемещение мыши
		case QEvent::MouseMove:
			ProcAxisZoom(mouse_event);
			break;
		// отпущена кнопка мыши
		case QEvent::MouseButtonRelease:
			EndAxisZoom(mouse_event);
			break;
		// для прочих событий ничего не делаем
		default:
			break;
	}
}

point2_I32 AxisZoom::AxisWidgetOffset(QwtScaleWidget *scale_widget)
{
	// получаем геометрию
	QRect gc = zoom.plot()->canvas()->geometry();
	// канвы графика
	QRect gw = scale_widget->geometry();
	// и виджета шкалы

	return point2_I32(gw.y(), gw.x()) - point2_I32(gc.y(), gc.x()) + point2_I32(gc.height(), 0);
}



// Обработчик нажатия на кнопку мыши над шкалой
// (включение изменения масштаба шкалы)
void AxisZoom::StartAxisZoom(QMouseEvent *mouse_event,QwtPlot::Axis axis)
{
	// если в данный момент уже включен один из режимов, выход
	if (zoom_mode != e_conversion_none) return;

	// масштабирование только левой кнопкой мыши, иначе выход
	if (mouse_event->button() != Qt::LeftButton) return;

	QwtScaleWidget *axis_widget = zoom.plot()->axisWidget(axis);

	scale_at_start = zoom.GetScale();
		// текущий масштаб графика
	scale_at_start_pix = range2_I32(zoom.TransformCoordinates(scale_at_start.p1()), zoom.TransformCoordinates(scale_at_start.p2()));
		// текущее положение графика в пикселах относительно канвы
	graph_to_widget = scale_at_start_pix.p1() - AxisWidgetOffset(axis_widget);
		// текущее смещение графика (в пикселах относительно виджета шкалы)
	start_cursor_position = zoom.CursorPosition(mouse_event) - graph_to_widget;
		// текущее положение курсора относительно канвы (за вычетом смещений графика)

	// если масштабируется горизонтальная шкала
	if (axis == QwtPlot::xBottom || axis == QwtPlot::xTop)
	{
		// если ширина канвы больше минимума,
		if (scale_at_start_pix.width() > min_scalable_sizes.x())
		{
			// выбор режима в зависимости от положения курсора (правее или левее середины шкалы)
			if (start_cursor_position.x() >= scale_at_start_pix.width()/2)
			{
				zoom_mode = e_axis_right;
			}
			else
			{
				zoom_mode = e_axis_left;
			}
		}
	}
	else    // иначе (масштабируется вертикальная шкала)
	{
		// если высота канвы больше минимума,
		if (scale_at_start_pix.height() > min_scalable_sizes.y())
		{
			// в зависимости от положения курсора (ниже или выше середины шкалы)
			if (start_cursor_position.y() >= scale_at_start_pix.height()/2)
			{
				zoom_mode = e_axis_bottom;
			}
			else
			{
				zoom_mode = e_axis_top;
			}
		}
	}
	if (zoom_mode != e_conversion_none)
	{
		// если началось масштабирование, устанавливаем курсор PointingHand
		zoom.SetCursor(Qt::PointingHandCursor);
	}
}



// Обработчик отпускания кнопки мыши
// восстановление "нулевого" режима
void AxisZoom::EndAxisZoom(QMouseEvent *mouse_event)
{
	// если отпущена левая кнопка мыши, то
	if (mouse_event->button() == Qt::LeftButton)
	{
		switch(zoom_mode)
		{
			case e_axis_left:
			case e_axis_right:
			case e_axis_bottom:
			case e_axis_top:
				// воостанавливаем курсор
				zoom.ResetCursor();
				// выключаем режим масштабирования
				zoom_mode = e_conversion_none;
				break;
			default:
				break;
		};
	}
}

}//namespace XRAD_GUI
