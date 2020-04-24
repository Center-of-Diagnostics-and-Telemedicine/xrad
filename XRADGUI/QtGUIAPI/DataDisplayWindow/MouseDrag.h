#ifndef XRAD__MouseDrag_h
#define XRAD__MouseDrag_h
//--------------------------------------------------------------
//
//	created:	2014/03/17
//	created:	17.3.2014   10:35
//	author:		KNS
//
//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

namespace XRAD_GUI
{
XRAD_USING



class	ChartZoom;

class MouseDrag : public QObject
{
		Q_OBJECT
	private:
		virtual bool eventFilter(QObject *,QEvent *);
		// обработчик событий от мыши
		virtual void procMouseEvent(QEvent *){}

	protected:
		MouseDrag(ChartZoom &in_zoom);
		ChartZoom &zoom;     // Опекаемый менеджер масштабирования

		point2_I32	start_cursor_position;
					// Положение курсора в момент начала преобразования
					// (в пикселах относительно канвы графика)

		range2_F64	scale_at_start;

};


}//namespace XRAD_GUI

#endif // XRAD__MouseDrag_h
