#include "pre.h"
#include "DragZoom.h"

#include "ChartZoom.h"

namespace XRAD_GUI
{



DragZoom::DragZoom(ChartZoom &in_zoom) :
	scale_frame(NULL),
	scale_frame_color(Qt::black),
	zoom_in_process(false),
	MouseDrag(in_zoom)
	{
	}


// Установка цвета рамки, задающей новый размер графика
void DragZoom::setRubberBandColor(QColor clr)
	{
	scale_frame_color = clr;
	}

// Обработчик всех событий
// bool DragZoom::eventFilter(QObject *target,QEvent *event)
// 	{
// 	// если событие произошло для графика, то
// 	if (target == zoom.plot())
// 		// если произошло одно из событий от мыши, то
// 		if (event->type() == QEvent::MouseButtonPress ||
// 			event->type() == QEvent::MouseMove ||
// 			event->type() == QEvent::MouseButtonRelease)
// 			procMouseEvent(event);  // вызываем соответствующий обработчик
// 	// передаем управление стандартному обработчику событий
// 	return QObject::eventFilter(target,event);
// 	}

// Прорисовка виджета выделенной области
void DragZoom::showZoomWidget(QRect draw_rect)
	{
	if(scale_frame)
		{
		// устанавливаем положение и размеры виджета, отображающего выделенную область
		scale_frame->setGeometry(draw_rect);
		// запоминаем для удобства
		int dw = draw_rect.width();    // ширину области
		int dh = draw_rect.height();   // и высоту
		// формируем маску для виджета, отображающего выделенную область
		QRegion opaque_region(0,0,dw,dh);      // непрозрачная область
		QRegion transparent_region(1,1,dw-2,dh-2);  // прозрачная область
		// устанавливаем маску путем вычитания из непрозрачной области прозрачной
		scale_frame->setMask(opaque_region.subtracted(transparent_region));
		// делаем виджет, отображающий выделенную область, видимым
		scale_frame->setVisible(true);
		// перерисовываем виджет
		scale_frame->repaint();
		}
	}

// Обработчик обычных событий от мыши
void DragZoom::procMouseEvent(QEvent *event)
{
	//TODO посмотреть внимательнее, как оно делается. kns
	// МАСШТАБИРОВАНИЕ ВКЛЮЧАЕМ ПРИ НАЖАТОМ Shift'е
	// ДОПОЛНЕНИЕ ВНЕСЕНО IRD ДЛЯ ИСПОЛЬЗОВАНИЯ ДВИЖЕНИЙ МЫШИ БЕЗ SHIFT В КЛАССЕ ShowGraph, ИЮЛЬ-2013

	// создаем указатель на событие от мыши
	QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
	// в зависимости от типа события вызываем соответствующий обработчик
	switch (event->type())
		{
			// нажата кнопка мыши
		case QEvent::MouseButtonPress:
			if ((mEvent->modifiers() == Qt::ShiftModifier))
				{
				startZoom(mEvent);
				}
				break;

			// перемещение мыши
			// ПРИ перемещении и отпускании КНОПКИ МЫШИ АНАЛИЗ НА SHIFT НЕ ИМЕЕТ ЗНАЧЕНИЕ
			//(ДАЖЕ МЕШАЕТ, Т.К. ИНОГДА SHIFT ОТПУСКАЕТСЯ РАНЬШЕ, ЧЕМ КНОПКА МЫШИ)
		case QEvent::MouseMove:
				selectZoomRect(mEvent);
				break;

			// отпущена кнопка мыши

		case QEvent::MouseButtonRelease:
			procZoom(mEvent);
			break;

			// для прочих событий ничего не делаем
		default:
			break;
		}
}

// Обработчик нажатия на кнопку мыши
// (включение изменения масштаба)
void DragZoom::startZoom(QMouseEvent *mEvent)
	{
	// фиксируем исходные границы графика (если этого еще не было сделано)
// 	zoom.LockBounds();
	// если в данный момент еще не включен ни один из режимов
//	if (zoom.ZoomMode() == ChartZoom::e_conversion_none)
	if(!zoom_in_process)
		{
		// получаем указатели на
		QwtPlot *plt = zoom.plot();        // график
// 		QwtPlotCanvas *cv = (QwtPlotCanvas *)plt->canvas();  // и канву
		// получаем геометрию канвы графика
		QRect cg = zoom.plot()->canvas()->geometry();
		// определяем текущее положение курсора (относительно канвы графика)
		start_cursor_position = zoom.RelativeCursorPosition(mEvent);
		// если курсор находится над канвой графика
		if (range2_I32(0,0,cg.height(), cg.width()).contains(start_cursor_position))
			// если нажата левая кнопка мыши, то
			{
			if (mEvent->button() == Qt::LeftButton)
				{
				// прописываем соответствующий признак режима
//				zoom.SetZoomMode(ChartZoom::e_conversion_zoom);
				zoom_in_process = true;
				// запоминаем текущий курсор и устанавливаем курсор Cross
				zoom.SetCursor(Qt::CrossCursor);
				// создаем виджет, который будет отображать выделенную область
				// (он будет прорисовываться на том же виджете, что и график)
				scale_frame = new QWidget(plt->parentWidget());
				// и назначаем ему цвет
				scale_frame->setStyleSheet(QString(
					"background-color:rgb(%1,%2,%3);").arg(
					scale_frame_color.red()).arg(scale_frame_color.green()).arg(scale_frame_color.blue()));
				}
			}
		}
	}

// Обработчик перемещения мыши
// (выделение новых границ графика)
void DragZoom::selectZoomRect(QMouseEvent *mEvent) {
	// если включен режим изменения масштаба, то
	//if (zoom.ZoomMode() == ChartZoom::e_conversion_zoom)
	if(zoom_in_process)
		{
		point2_I32 plot_corner(zoom.plot()->geometry().y(), zoom.plot()->geometry().x());
		point2_I32 canvas_corner(zoom.plot()->canvas()->geometry().y(), zoom.plot()->canvas()->geometry().x());
		point2_I32 frame_corner = plot_corner+canvas_corner+start_cursor_position;
		point2_I32 delta = zoom.RelativeCursorPosition(mEvent) - start_cursor_position;

		// если ширина выделенной области отрицательна, то текущая точка находится левее начальной,
		// и тогда именно ее мы используем в качестве опоры для отображения выделенной области
		if (delta.x() < 0)
			{
			delta.x() = -delta.x();
			frame_corner.x() -= delta.x();
			}
		// иначе если ширина равна нулю, то для того чтобы выделенная область все-таки отбражалась,
		// принудительно сделаем ее равной единице
		else if (delta.x() == 0) delta.x() = 1;

		if (delta.y() < 0)
			{
			delta.y() = -delta.y();
			frame_corner.y() -= delta.y();
			}
		else if (delta.y() == 0) delta.y() = 1;
		// отображаем выделенную область
		showZoomWidget(QRect(frame_corner.x(),frame_corner.y(),delta.x(),delta.y()));
		}
	}

// Обработчик отпускания кнопки мыши
// (выполнение изменения масштаба)
void DragZoom::procZoom(QMouseEvent *mEvent)
	{
	// если включен режим изменения масштаба или режим перемещения графика
//	if (zoom.ZoomMode() == ChartZoom::e_conversion_zoom)
	if(zoom_in_process)
		{
		zoom_in_process = false;
		// если отпущена левая кнопка мыши, то
		if (mEvent->button() == Qt::LeftButton)
			{
			// восстанавливаем курсор
			zoom.ResetCursor();
			// удаляем виджет, отображающий выделенную область
			DestroyObject(scale_frame);

			point2_I32 event_position = zoom.RelativeCursorPosition(mEvent);

			if(start_cursor_position.x()>event_position.x()) swap(start_cursor_position.x(),event_position.x());
			if(start_cursor_position.y()<event_position.y()) swap(start_cursor_position.y(),event_position.y());// потому что y вверх тормашками

			// по клику без смещения сброс масштаба
			if (event_position == start_cursor_position)
				{
				zoom.RestoreBounds();
				}
			// иначе изменяем масштаб
			else
				{
				range2_F64	gs;
				gs.set_p1(zoom.InvertTransformCoordinates(start_cursor_position));
				gs.set_p2(zoom.InvertTransformCoordinates(event_position));
				zoom.SetBounds(gs, true);

				// перестраиваем график (синхронно с остальными)
				zoom.plot()->replot();
				}
			// очищаем признак режима
//			zoom.SetZoomMode(ChartZoom::e_conversion_none);
			}
		}
	}



}//namespace XRAD_GUI
