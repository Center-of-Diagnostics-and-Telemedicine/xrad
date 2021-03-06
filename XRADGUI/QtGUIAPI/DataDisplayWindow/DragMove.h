﻿/*
	Copyright (c) 2021, Moscow Center for Diagnostics & Telemedicine
	All rights reserved.
	This file is licensed under BSD-3-Clause license. See LICENSE file for details.
*/
#ifndef XRAD__File_mouse_drag_h
#define XRAD__File_mouse_drag_h

//--------------------------------------------------------------
//
//	created:	2014/03/17
//	created:	17.3.2014   10:12
//	author:		KNS
//
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include "MouseDrag.h"

namespace XRAD_GUI
{

class	ChartZoom;

class DragMove : public MouseDrag
{
		Q_OBJECT

	public:
		// конструктор
		explicit DragMove(ChartZoom &in_zoom);


	private:
		bool	is_dragging;

		// прорисовка изображения индикатора перемещения
		void applyDrag(QMouseEvent *);

		// обработчик событий от мыши
		void procMouseEvent(QEvent *);

		void startDrag(QMouseEvent *);
		void procDrag(QMouseEvent *);
		void endDrag(QMouseEvent *);
};



}//namespace XRAD_GUI

#include "ChartZoom.h"

#endif //XRAD__File_mouse_drag_h
