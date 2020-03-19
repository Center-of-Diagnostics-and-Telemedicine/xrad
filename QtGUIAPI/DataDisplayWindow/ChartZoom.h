#ifndef __chart_zoom_h
#define __chart_zoom_h

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

XRAD_USING

namespace XRAD_GUI
{



class DragZoom; // интерфейс масштабирования графика
class DragMove; // интерфейс перемещения графика
class WheelZoom;
class AxisZoom;

class ChartZoom : public QObject
	{
		Q_OBJECT

	public:
		// конструктор
		explicit ChartZoom(QwtPlot *);
		// деструктор
		~ChartZoom();



		// Контейнеры границ шкалы
		// (вертикальной и горизонтальной)
//		QScaleBounds *scale_bounds_x,*scale_bounds_y;

		// текущий режим масштабирования
//		chart_conversion_t ZoomMode() const;
		// переключение режима масштабирования
//		void SetZoomMode(chart_conversion_t);

		// указатель на опекаемый компонент QwtPlot
		QwtPlot *plot();

		const QwtPlot::Axis &masterV() const;
		const QwtPlot::Axis &masterH() const;

		range2_F64 GetScale();

		// установка цвета рамки, задающей новый размер графика
		void setRubberBandColor(QColor);


		void	RestoreBounds();
		void	SetBounds(const range2_F64 &gs, bool set_custom);
		void	SetAutoBounds(const range2_F64 &gs);
		range2_F64 GetCurrentBounds();


		point2_F64 InvertTransformCoordinates(const point2_I32 &p)
			{
			point2_F64 result;
			result.x() = plot()->invTransform(x_axis,p.x());
			result.y() = plot()->invTransform(y_axis,p.y());
			return result;
			}
		point2_I32 TransformCoordinates(const point2_F64 &p)
			{
			point2_I32 result;
			using rv_t = point2_I32::value_type;
			result.x() = iround_n<rv_t>(plot()->transform(x_axis,p.x()));
			result.y() = iround_n<rv_t>(plot()->transform(y_axis,p.y()));
			return result;
			}
	public:
		void SetCursor(const QCursor new_cursor);
		void ResetCursor();

	protected:
		// обработчик всех событий
		bool eventFilter(QObject *,QEvent *);

	private:
		QObject *main_window;          // Главное окно приложения
		QwtPlot *plot_component;          // Компонент QwtPlot, который отображает график

		QCursor stored_cursor;        // Буфер для временного хранения курсора

		range2_F64	full_data_scale;

		// Текущее значение максимального количества основных делений
			// на горизонтальной шкале графика
		int n_divisions_h;  // основной
			// на вертикальной шкале графика
		int n_divisions_v;  // основной

		// горизонтальная шкала
		QwtPlot::Axis x_axis;
		// вертикальная шкала
		QwtPlot::Axis y_axis;

		// Интерфейс масштабирования графика
		DragZoom *drag_zoom;
		// Интерфейс перемещения графика
		DragMove *drag_move;
		WheelZoom *wheel_zoom;
		AxisZoom *axis_zoom;


// 		chart_conversion_t convType;     // Тип текущего преобразования графика

		// определение главного родителя
		QObject *generalParent(QObject *);
		// назначение основной и дополнительной шкалы
		void allocAxis(int,int,QwtPlot::Axis *);


		// создание списка ширины меток горизонтальной шкалы
		QList<int> *getLabelWidths(int);
		// определение средней ширины меток горизонтальной шкалы
		int meanLabelWidth(int);
		// обновление горизонтальной шкалы графика
		bool updateHorAxis(int,int *);
		// обновление вертикальной шкалы графика
		bool updateVerAxis(int,int *);

	public:
		point2_I32 RelativeCursorPosition(const QMouseEvent *mEvent)
			{
			// определяем текущее положение курсора (относительно канвы графика)
			QRect cg = plot()->canvas()->geometry();
			return point2_I32(mEvent->pos().y() - cg.y(), mEvent->pos().x() - cg.x());
			}
		point2_I32 CursorPosition(const QMouseEvent *mEvent)
			{
			// определяем текущее положение курсора
			return point2_I32(mEvent->pos().y(), mEvent->pos().x());
			}
		bool	EventBelongsToCanvas(QMouseEvent *mEvent)
			{
			QRect cg = plot()->canvas()->geometry();
			point2_I32 cursor_position = RelativeCursorPosition(mEvent);
			return	in_range(cursor_position.x(), 0, cg.width()) &&
					in_range(cursor_position.y(), 0, cg.height());
			}
		bool	custom_scale;

		void updatePlot();  // обновление графика

	signals:
		void ScaleChanged();
	};

inline double AbsoluteScaleLimitation(){return max_double()/4;}



}//namespace XRAD_GUI

#include "DragMove.h"
#include "AxisZoom.h"
#include "WheelZoom.h"
#include "DragZoom.h"

#endif // __chart_zoom_h
