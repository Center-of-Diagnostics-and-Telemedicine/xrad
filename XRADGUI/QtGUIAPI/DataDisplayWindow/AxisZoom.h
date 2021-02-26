#ifndef XRAD__File_axis_zoom_h
#define XRAD__File_axis_zoom_h

#include <QtCore/QEvent>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_draw.h>

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>



namespace XRAD_GUI
{

XRAD_USING

class ChartZoom;

class AxisZoom : public QObject
{
// 	Q_OBJECT
	public:
		// конструктор
		explicit AxisZoom(ChartZoom &);


	protected:
		// обработчик всех событий
		bool eventFilter(QObject *,QEvent *);

	private:
		ChartZoom &zoom;     // Опекаемый менеджер масштабирования

		enum
			{
			e_conversion_none,
			e_axis_left,	// изменение левой границы
			e_axis_right,	// изменение правой границы
			e_axis_bottom,	// изменение нижней границы
			e_axis_top		// изменение верхней границы
			} zoom_mode;


		range2_F64	scale_at_start;
		range2_I32	scale_at_start_pix;

		const point2_I32	min_scalable_sizes;	// минимальные размеры графика, который еще можно масштабировать, перетягивая шкалу
		const double	max_scalefactor;
		const double	min_scalefactor;

		point2_I32 start_cursor_position;
			// Положение курсора в момент начала преобразования
			// (в пикселах относительно канвы графика за вычетом смещений графика)
		point2_I32 graph_to_widget;
		point2_I32 AxisWidgetOffset(QwtScaleWidget *sc);
			// Текущие левое и верхнее смещение графика в момент начала преобразования
			// (в пикселах относительно виджета шкалы)

		void AxisMouseEvent(QEvent *,QwtPlot::Axis);
			// обработчик событий от мыши для шкалы

		void StartAxisZoom(QMouseEvent *,QwtPlot::Axis);
		void ProcAxisZoom(QMouseEvent *);
		void EndAxisZoom(QMouseEvent *);
};

}//namespace XRAD_GUI

#include "ChartZoom.h"

#endif // XRAD__File_axis_zoom_h
