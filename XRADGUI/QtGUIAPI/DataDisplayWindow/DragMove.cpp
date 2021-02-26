/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#include "pre.h"
#include "DragMove.h"

namespace XRAD_GUI
{



// Конструктор
DragMove::DragMove(ChartZoom &in_zoom):
	MouseDrag(in_zoom),
	is_dragging(false)
	{
	}


// Применение результатов перемещения графика
void DragMove::applyDrag(QMouseEvent *mEvent)
	{
	point2_F64	offsetd = zoom.InvertTransformCoordinates(start_cursor_position) - zoom.InvertTransformCoordinates(zoom.RelativeCursorPosition(mEvent));
	range2_F64	gs = scale_at_start + range2_F64(offsetd,offsetd);

	zoom.SetBounds(gs, true);
	zoom.plot()->replot();
	}

// Обработчик событий от мыши
void DragMove::procMouseEvent(QEvent *event)
{
	// создаем указатель на событие от мыши
	QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
	// в зависимости от типа события вызываем соответствующий обработчик
	switch (event->type())
	{
		// нажата кнопка мыши
	case QEvent::MouseButtonPress:
		startDrag(mEvent);
		break;
		// перемещение мыши
	case QEvent::MouseMove:
		procDrag(mEvent);
		break;
		// отпущена кнопка мыши
	case QEvent::MouseButtonRelease:
		endDrag(mEvent);
		break;
		// для прочих событий ничего не делаем
	default: ;
	}
}

// Обработчик нажатия на кнопку мыши
// (включение перемещения графика)
void DragMove::startDrag(QMouseEvent *mEvent)
	{
	// если в данный момент еще не включен ни один из режимов
//	if (zoom.ZoomMode() == ChartZoom::e_conversion_none)
		{
		// получаем указатели на
		QwtPlotCanvas *cv = (QwtPlotCanvas*)zoom.plot()->canvas();  // и канву
		// получаем геометрию канвы графика
		QRect cg = cv->geometry();
		// определяем текущее положение курсора (относительно канвы графика)
		start_cursor_position = zoom.RelativeCursorPosition(mEvent);
		// если курсор находится над канвой графика
		if (range2_I32(0,0,cg.height(), cg.width()).contains(start_cursor_position))
			// если нажата правая кнопка мыши, то
			{
			if (mEvent->button() == Qt::RightButton)
				{
				// прописываем соответствующий признак режима
				//zoom.SetZoomMode(ChartZoom::e_conversion_drag);
				is_dragging = true;
				// запоминаем текущий курсор и устанавливаем курсор OpenHand
// 				zoom.SetCursor(Qt::OpenHandCursor);
				zoom.plot()->canvas()->setCursor(Qt::OpenHandCursor);

				scale_at_start = zoom.GetScale();

				applyDrag(mEvent);
				}
			}
		}
	}

// Обработчик перемещения мыши
// (выполнение перемещения или выбор нового положения графика)
void DragMove::procDrag(QMouseEvent *mEvent)
	{
	// если включен режим перемещения графика, то
// 	if (zoom.ZoomMode() == ChartZoom::e_conversion_drag)
	if(is_dragging)
		{
		// устанавливаем курсор ClosedHand
// 		zoom.ResetCursor();
// 		zoom.SetCursor(Qt::ClosedHandCursor);
		zoom.plot()->canvas()->setCursor(Qt::ClosedHandCursor);
		// если включена индикация, то
		applyDrag(mEvent);
		}
	}

// Обработчик отпускания кнопки мыши
// (выключение перемещения графика)
void DragMove::endDrag(QMouseEvent *mEvent)
	{
	// если включен режим изменения масштаба или режим перемещения графика
// 	if (zoom.ZoomMode() == ChartZoom::e_conversion_drag)
	if(is_dragging)
		{
		is_dragging = false;
		// если отпущена правая кнопка мыши, то
		if (mEvent->button() == Qt::RightButton)
			{
			// применяем результаты перемещения графика
			applyDrag(mEvent);
			// восстанавливаем курсор
//			zoom.ResetCursor();
// 			zoom.SetCursor(Qt::ArrowCursor);
			zoom.plot()->canvas()->setCursor(Qt::ArrowCursor);
//			zoom.SetZoomMode(ChartZoom::e_conversion_none);  // и очищаем признак режима
			}
		}
	}



}//namespace XRAD_GUI
