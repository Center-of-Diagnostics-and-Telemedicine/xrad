/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
//	Created by IRD on 07.2013
//  Version 3.0.5
//--------------------------------------------------------------

#ifndef XRAD__ShowGraphH
#define XRAD__ShowGraphH

//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>

#include "ui_GraphWindow.h"

#ifdef XRAD_COMPILER_MSC
#pragma warning (push)
#pragma warning(disable:4083)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4800)
#pragma warning(disable:4250)
#pragma warning(disable:4505)
#endif // XRAD_COMPILER_MSC

// вот эту штуку не получилось как следует включить в pre.h
// в ней куча предупреждений не по делу
#include <qwt_plot_curve.h>

#ifdef XRAD_COMPILER_MSC
#pragma warning (pop)
#endif // XRAD_COMPILER_MSC

#include "VisualizationPlot.h"
#include "ChartZoom.h"
#include "DataDisplayWindow.h"
#include "GraphStyleSet.h"

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

namespace XRAD_GUI
{

XRAD_USING



// вспомогательный класс для рисования перпендикулярных линий, пересекающихся с графиком
class GraphValueDrawer : public QWidget, public range2_I32
{
		Q_OBJECT

	private:
		const int	n_graphs;
		const graph_line_style line_style;
		const double	line_width;
	public:
		GraphValueDrawer(QWidget *parent, int in_n_graphs, graph_line_style in_gss, double in_line_weight);
		range2_I32 bounds;
		// координаты прямых, пересекающихся с графиком (в пикселах)
		int	x_current_pix;
		std::vector<int> y_current_pix;

	protected:
		void paintEvent(QPaintEvent *event);
};


// Форма графика
class GraphWindow: public DataDisplayWindow, public Ui::ShowGraph, public VisualizationPlot
{
		Q_OBJECT

	public:
		GraphWindow(const QString &in_title, const QString &in_y_label, const QString &in_x_label, GUIController &gc);
		~GraphWindow();

		void	SetupCurve(int in_curve_no, const DataArray<double> &in_data_y, const DataArray<double> &in_data_x, const QString &graph_name);
		void	SetGraphLabels(const QString &title, const QString &y_label, const QString &x_label);
		void	SetWindowTitle(const QString &title);

		void	SetScale(const range2_F64 &gs);
		void	ResetScale();
		range2_F64	GetScale() const;

		void	SetLineStyle(graph_line_style gss, double in_line_width = -1);
		bool	AllCurvesCompleted(){return !n_incomplete_curves;}

		void	SavePicture(QString file_name);

	private:
		void	SetCurveValues(size_t curve_no);
		void	UpdateTransformedCurve(size_t curve_no);
		void	UpdateTransformedCurves();
		QString	yLabelTransformed(const QString &in_label);

		//	исходные данные для отображения: набор массивов double,
		//	определяющий параметрические кривые.
		//	приватные методы доступа к этим данным

		int	n_curves;
		int	transform;
		std::vector<xrad::RealFunctionF64>  data_x, data_y, data_y_transformed;
		std::vector<QwtPlotCurve*> curves;

		QwtPlotShapeItem* axes_drawer;

		std::vector<QString>	graph_labels;
		QString x_label, y_label;
		double	line_width;
		graph_line_style line_style;

		int		n_incomplete_curves;
			// когда создается новая кривая, этот счетчик увеличивается на 1. когда она полностью создана, уменьшается на 1.
			// нужно, чтобы отрисовка не начиналась, пока все данные не сформированы.
		//range2_F64 scale;

		void	EraseCurve(int curve_no);
		int		AddCurve();
		void	SetLegend(bool);

		void	UpdateScale();

	private:

		//	обработчики событий

		void closeEvent(QCloseEvent *event);
		void keyPressEvent ( QKeyEvent * event );

		bool eventFilter(QObject *target,QEvent *event);
		//void resizeEvent ( QResizeEvent  * event );
		void procMouseEvent(QEvent *event);

		// режим отображения координат

		void StartCurrentValueDraw(QMouseEvent *mEvent); // включение режима
		void DrawCurrentValue(QMouseEvent *mEvent);  // рисование
		void EndCurrentValueDraw(QMouseEvent *mEvent);   // выключение режима



	private:

		//	легенда (расшифровка стилей кривых)

		QwtLegend *legend;
		GraphValueDrawer *graph_value_drawer;

		//	слоты и сигналы

	public slots:
		void SetAxesVisibility(bool visible){VisualizationPlot::SetAxesVisibility(visible);}
		void slotSetSymbol(bool checked); //вкл/выкл маркера
		void slotSetTransform(int t);
		void slotSavePicture();
		void slotSaveRawData();

		void slotGraphStyle();
		void UpdateAxesDrawer();
		void SetGridVisibility(bool visible);

	signals:
		void signal_esc();
};



}//namespace XRAD_GUI

#endif // XRAD__ShowGraphH
