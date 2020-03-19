#include "pre_GUI.h"

#include "VisualizationPlot.h"

namespace XRAD_GUI
{



VisualizationPlot::VisualizationPlot( QwtPlot *&in_plot ) :
		plot_ref(in_plot),
		grid(NULL),
		main_zoom(NULL),
		canvas(NULL)
	{
	}

VisualizationPlot::~VisualizationPlot()
	{
	XRAD_USING

	DestroyObject(canvas); //не нужно?
	DestroyObject(grid);
	DestroyObject(main_zoom);
	}

void VisualizationPlot::InitVisualizationPlot()
	{
	InitGrid();
	InitZoom();
	InitCanvas();

// 	plot_ref->setCursor(Qt::ArrowCursor);
	}

void VisualizationPlot::SetBackground( QColor color, Qt::BrushStyle pattern )
	{
	//		QBrush brush(QColor(255, 255, 255, 255));
	QBrush brush(color);
	brush.setStyle(pattern);
	plot_ref->setCanvasBackground(brush);
	}

void VisualizationPlot::SetAxesLabels(const QString &in_y_label, const QString&in_x_label)
	{
	x_label = in_x_label.toStdString();
	y_label = in_y_label.toStdString();
	// задаем название осей
	if(axes_visible)
		{
		plot_ref->setAxisTitle(QwtPlot::xBottom, QString::fromStdString(x_label));
		plot_ref->setAxisTitle(QwtPlot::yLeft, QString::fromStdString(y_label));
		}
	else
		{
		plot_ref->setAxisTitle(QwtPlot::xBottom, QString::fromStdString(""));
		plot_ref->setAxisTitle(QwtPlot::yLeft, QString::fromStdString(""));
		}
	}

void VisualizationPlot::InitCanvas()
	{
	// создаем канву
	canvas = new QwtPlotCanvas();
	canvas->setBorderRadius( 10 );
	//canvas->setBackgroundColor(Qt::white);
	plot_ref->setCanvas( canvas );
	// назначаем тип курсора для канвы
	canvas->setCursor(Qt::ArrowCursor);
	}

void VisualizationPlot::InitGrid()
	{
	// создаем сетку
	QPen	major_grid(Qt::black,0,Qt::DotLine);
	QPen	minor_grid(Qt::gray,0,Qt::DotLine);
	// 		QPen	major_grid(Qt::gray_ui8,0,Qt::DashLine);
	// 		QPen	minor_grid(Qt::gray_ui8,0,Qt::DotLine);

	grid = new QwtPlotGrid;

	//		включает минорные линии сетки
	if(0)
		{
		grid->enableXMin(true);
		grid->enableYMin(true);
		}

	//		включает мажорные линии сетки
	if(1)
		{
		grid->enableX(true);
		grid->enableY(true);
		}

	grid->setMajorPen(major_grid);
	grid->setMinorPen(minor_grid);
	grid->attach(plot_ref);
	}



void	VisualizationPlot::SetGridCount(int n_x_steps, int n_x_steps_min, int n_y_steps, int n_y_steps_min)

	{
	double	x_min = grid->xScaleDiv().lowerBound();
	double	x_max = grid->xScaleDiv().upperBound();

	double	y_min = grid->yScaleDiv().lowerBound();
	double	y_max = grid->yScaleDiv().upperBound();

	double x_step = (x_max-x_min)/n_x_steps;
	double x_step_min = (x_max-x_min)/n_x_steps_min;
	double y_step = (y_max-y_min)/n_y_steps;
	double y_step_min = (y_max-y_min)/n_y_steps_min;


	QList<double> x_ticks_major, x_ticks_minor;
	QList<double> y_ticks_major, y_ticks_minor;

	for(double	x = x_min; x < x_max; x += x_step) x_ticks_major.append(x);
	for(double	x = x_min; x < x_max; x += x_step_min) x_ticks_minor.append(x);
	for(double	y = x_min; y < y_max; y += y_step) y_ticks_major.append(y);
	for(double	y = x_min; y < y_max; y += y_step_min) y_ticks_minor.append(y);

	QwtScaleDiv	x_div(x_min, x_max);
	QwtScaleDiv	y_div(y_min, y_max);

	x_div.setTicks(QwtScaleDiv::MajorTick, x_ticks_major);
	x_div.setTicks(QwtScaleDiv::MinorTick, x_ticks_minor);
	y_div.setTicks(QwtScaleDiv::MajorTick, y_ticks_major);
	y_div.setTicks(QwtScaleDiv::MinorTick, y_ticks_minor);
#if 1

	//QwtScaleEngine *x_scale = plot_ref->axisScaleDraw(Qt::XAxis);//->enableComponent(QwtAbstractScaleDraw::Labels, visible);
	//QwtScaleEngine *y_scale = plot_ref->axisScaleDraw(Qt::YAxis);//->enableComponent(QwtAbstractScaleDraw::Labels, visible);

	plot_ref->axisScaleDraw(Qt::XAxis)->setScaleDiv(x_div);
	plot_ref->axisScaleDraw(Qt::YAxis)->setScaleDiv(y_div);

	grid->setXDiv(x_div);
	grid->setYDiv(y_div);


#else

	//QwtScaleDiv	x_div(double low, double high, const QList< double > &minorTicks, const QList< double > &mediumTicks, const QList< double > &majorTicks)
	//grid->setXDiv(x_div);
	//grid->setYDiv(x_div);



	grid->attach(plot_ref);
#endif
	}

void VisualizationPlot::SetAxesVisibility( bool visible )
	{
	/*
	//! Backbone = the line where the ticks are located
	Backbone = 0x01,

	//! Ticks
	Ticks = 0x02,

	//! Labels
	Labels = 0x04
	*/

	axes_visible = visible;

	plot_ref->axisScaleDraw(Qt::XAxis)->enableComponent(QwtAbstractScaleDraw::Labels, visible);
	plot_ref->axisScaleDraw(Qt::YAxis)->enableComponent(QwtAbstractScaleDraw::Labels, visible);
	plot_ref->axisScaleDraw(Qt::ZAxis)->enableComponent(QwtAbstractScaleDraw::Labels, visible);

	plot_ref->axisScaleDraw(Qt::XAxis)->enableComponent(QwtAbstractScaleDraw::Backbone, visible);
	plot_ref->axisScaleDraw(Qt::YAxis)->enableComponent(QwtAbstractScaleDraw::Backbone, visible);
	plot_ref->axisScaleDraw(Qt::ZAxis)->enableComponent(QwtAbstractScaleDraw::Backbone, visible);

	plot_ref->axisScaleDraw(Qt::XAxis)->enableComponent(QwtAbstractScaleDraw::Ticks, visible);
	plot_ref->axisScaleDraw(Qt::YAxis)->enableComponent(QwtAbstractScaleDraw::Ticks, visible);
	plot_ref->axisScaleDraw(Qt::ZAxis)->enableComponent(QwtAbstractScaleDraw::Ticks, visible);

	grid->enableXMin(visible);
	grid->enableYMin(visible);
	grid->enableX(visible);
	grid->enableY(visible);

//#error here continue
	SetGridCount(4,16,4,16);

	if(visible)
		{
		plot_ref->setAxisTitle(QwtPlot::xBottom, QString::fromStdString(x_label));
		plot_ref->setAxisTitle(QwtPlot::yLeft, QString::fromStdString(y_label));
		}
	else
		{
		plot_ref->setAxisTitle(QwtPlot::xBottom, QString::fromStdString(""));
		plot_ref->setAxisTitle(QwtPlot::yLeft, QString::fromStdString(""));
		}
	plot_ref->replot();
	}

void VisualizationPlot::InitZoom()
	{
	//main_zoom = new QwtPlotZoomer(plot->canvas());//был еще такой вариант, про запас оставлю, понять, что значило

	main_zoom = new ChartZoom(plot_ref);
	// это цвет рамки выделения. еще бы ему стиль штриховки придать
	main_zoom->setRubberBandColor(Qt::darkGray);
	}



}//namespace XRAD_GUI
