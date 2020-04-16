#ifndef __wheel_zoom_h
#define __wheel_zoom_h

#include <XRADGUI/Sources/Internal/std.h>

namespace XRAD_GUI
{

class ChartZoom;

class WheelZoom : public QObject
{
		Q_OBJECT

	public:
		// конструктор
		explicit WheelZoom(ChartZoom &);

		// задание коэффициента масштабирования графика
		// при вращении колеса мыши
		void setWheelFactor(double);
		// применение изменений по вращении колеса мыши
		void applyWheel(QEvent *);

	private:
		ChartZoom &zoom;     // Опекаемый менеджер масштабирования
		double wheel_zoom_factor;           // Коэффициент, определяющий изменение масштаба графика
								// при вращении колеса мыши (по умолчанию равен 1.2)
};

}//namespace XRAD_GUI

#endif // __wheel_zoom_h
