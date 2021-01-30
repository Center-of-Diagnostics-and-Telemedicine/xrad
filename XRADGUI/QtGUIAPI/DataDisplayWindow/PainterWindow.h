//	Created by IRD on 07.2013
//  Version 3.0.5
//--------------------------------------------------------------

#ifndef __XRAD_PainterWindow
#define __XRAD_PainterWindow

//--------------------------------------------------------------

#include <QGraphicsView>

#include <XRADGUI/Sources/Internal/std.h>
#include <XRADBasic/Core.h>

#include "DataDisplayWindow.h"

#include <XRADBasic/MathFunctionTypes.h>
#include <XRADBasic/Sources/Containers/SpaceCoordinates.h>
#include "paintScene.h"

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

		const size_t	m_vsize, m_hsize;
		QString	m_title;

		QGraphicsView* drawing_graphicsView;
		PaintScene* drawing_scene;
		QPushButton* return_result_button;
		QImage result;
		QLabel* image_label;

		//TODO следующие функции пересмотреть
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
