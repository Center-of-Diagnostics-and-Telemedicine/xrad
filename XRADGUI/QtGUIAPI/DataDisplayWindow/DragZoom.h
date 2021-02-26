/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_drag_zoom
#define XRAD__File_drag_zoom

//--------------------------------------------------------------
//
//	created:	2014/03/17
//	created:	17.3.2014   13:13
//	author:		KNS
//
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "MouseDrag.h"

namespace XRAD_GUI
{



class DragZoom : public MouseDrag
{
		Q_OBJECT

	public:
		explicit DragZoom(ChartZoom &);
		// установка цвета рамки, задающей новый размер графика
		void setRubberBandColor(QColor);


	private:
		QWidget *scale_frame;
			// Виджет для отображения индикатора перемещения графика
		QColor scale_frame_color;
			// Цвет рамки, задающей новый размер графика
		bool	zoom_in_process;

		// прорисовка виджета выделенной области
		void showZoomWidget(QRect);

		virtual void procMouseEvent(QEvent *);

		void startZoom(QMouseEvent *);
		void selectZoomRect(QMouseEvent *);
		void procZoom(QMouseEvent *);
};



}//namespace XRAD_GUI

#endif //XRAD__File_drag_zoom
