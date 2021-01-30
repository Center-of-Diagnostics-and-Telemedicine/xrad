//	Created by IRD on 07.2013
//  Version 3.0.5
//--------------------------------------------------------------

#ifndef __XRAD_PainterWindow
#define __XRAD_PainterWindow

//--------------------------------------------------------------

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>

//#include "ui_PainterWindow.h"

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

#include "DataDisplayWindow.h"

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>

namespace XRAD_GUI
{

XRAD_USING




class PainterWindow: public DataDisplayWindow
{
		Q_OBJECT

	public:
		PainterWindow(const QString &in_title, size_t in_vsize, size_t in_hsize, GUIController &gc);
		~PainterWindow();

		void	SavePicture(QString file_name);
		QImage	GetResult();

	private:
//		double	brush_size;

		//	обработчики событий
		void closeEvent(QCloseEvent *event);
		void keyPressEvent ( QKeyEvent * event );

		bool eventFilter(QObject *target,QEvent *event);
		void procMouseEvent(QEvent *event);

		// режим отображения координат


		//	слоты и сигналы

	public slots:
		void slotSavePicture();
		void slotSaveRawData();

	private slots:
		void return_result_button_click();

	signals:
		void signal_esc();
};



}//namespace XRAD_GUI

#endif // __XRAD_PainterWindow
