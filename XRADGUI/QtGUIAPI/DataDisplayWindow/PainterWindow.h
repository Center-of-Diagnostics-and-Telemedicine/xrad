//	Created by IRD on 07.2013
//  Version 3.0.5
//--------------------------------------------------------------

#ifndef __XRAD_PainterWindow
#define __XRAD_PainterWindow

//--------------------------------------------------------------

#include <QGraphicsView>

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>
#include <ui_PainterWindow.h>
#include <XRADBasic/ContainersAlgebra.h>
#include <XRADGUI/QTGUIAPI/DataDisplayWindow/DataDisplayWindow.h>

#include "PaintWidget.h"

namespace XRAD_GUI
{

	XRAD_USING




		class PainterWindow : public DataDisplayWindow
	{
		Q_OBJECT

	public:
		PainterWindow(const QString& in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc);
		~PainterWindow();



	private:



		void closeEvent(QCloseEvent* event);
		void keyPressEvent(QKeyEvent* event);

		bool eventFilter(QObject* target, QEvent* event);


		QPixmap GetCursor(size_t);
		QString GetStringStyleSheet(int, int, int);
		QString m_sTitle;

		size_t m_nVSize, m_nHSize;
		
		shared_ptr<QImage> m_pResult;
		PaintWidget* pw;

		Ui::Dialog ui;


	public slots:
		//void slotSavePicture();
		//void slotSaveRawData();

	signals:
		void signal_esc();
	};



}//namespace XRAD_GUI

#endif // __XRAD_PainterWindow
