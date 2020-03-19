#ifndef __mouse_drag_h
#define __mouse_drag_h

#include "MouseDrag.h"

//--------------------------------------------------------------
//
//	created:	2014/03/17
//	created:	17.3.2014   10:12
//	author:		KNS
//
//--------------------------------------------------------------

#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

//TODO: поместить все интерфейсные объекты в свой namespace (вчерне сделано). соответственно этому упростить имена классов

namespace XRAD_GUI
{



class	ChartZoom;

XRAD_USING

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

#endif //__mouse_drag_h
