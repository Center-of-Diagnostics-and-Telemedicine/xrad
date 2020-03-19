#ifndef XRAD__VisualizationPlot_h
#define XRAD__VisualizationPlot_h
//--------------------------------------------------------------
//
//	created:	2014/01/31
//	created:	31.1.2014   18:06
//	author:		KNS
//
//	purpose:	базовый класс для ImageWindow и GraphWindow, объединяющий
//				общие для них элементы QWT
//
//--------------------------------------------------------------

#include "ChartZoom.h"

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

namespace XRAD_GUI
{

XRAD_USING
using namespace std;



class	VisualizationPlot
{
	protected:
		QwtPlot	*&plot_ref;
		QwtPlotCanvas *canvas;
		QwtPlotGrid *grid;
		ChartZoom *main_zoom;

		string	x_label, y_label;// здесь не QString ради тех манипуляций, какие с ними делаются по старинке
		bool	axes_visible;

		VisualizationPlot(QwtPlot *&in_plot);
		~VisualizationPlot();

		void	InitVisualizationPlot();
		void	SetBackground(QColor color, Qt::BrushStyle pattern);
		//void	SetGridStep(double x_step, double x_step_min, double y_step, double y_step_min);
		void	SetGridCount(int n_x_steps, int n_x_steps_min, int n_y_steps, int n_y_steps_min);

		//point2_I32 RelativeCursorPosition(QMouseEvent *mEvent)
		//	{
		//	return point2_I32(mEvent->pos().x() - canvas->geometry().x(), mEvent->pos().y() - canvas->geometry().y());
		//	}

	public:
		void SetAxesLabels(const QString &in_y_label, const QString &in_x_label);

	private:
		void	InitCanvas();
		void	InitGrid();
		void	InitZoom();

	public:
		void	SetAxesVisibility(bool visible);
};



}//namespace XRAD_GUI

#endif // XRAD__VisualizationPlot_h
