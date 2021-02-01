#include "pre.h"
#include "PainterWindow.h"

#include "ThreadGUI.h"
#include "GUIController.h"
#include "FileSaveUtils.h"
#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/TextFile.h>

namespace XRAD_GUI
{

	XRAD_USING


		//
		//--------------------------------------------------------------



		PainterWindow::PainterWindow(const QString &in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc)
		:
		DataDisplayWindow(gc),
		m_vsize(in_vsize),
		m_hsize(in_hsize),
		m_title(in_title),
		result(in_result)
	{
		try
		{
			// задаем положение окна
			auto corner = GetCornerPosition();
			setGeometry(QRect(QPoint(corner.x(), corner.y()), QPoint(int(in_hsize), int(in_vsize))));

			//		SetBackground(Qt::white, Qt::SolidPattern);

			//TODO	Копировать сюда остальное



			if (objectName().isEmpty()) setObjectName(m_title);
			setWindowTitle(m_title);

			drawing_graphicsView = new QGraphicsView();
			drawing_scene = new PaintScene();


			drawing_scene->SetColor(Qt::black);



			resize(int(m_hsize), int(m_vsize));
			drawing_graphicsView->resize(int(m_hsize) + 5, int(m_vsize) + 5);


			drawing_scene->setParent(drawing_graphicsView);
			drawing_scene->setSceneRect(0, 0, m_hsize, m_vsize);
			drawing_scene->setBackgroundBrush(Qt::white);

			drawing_graphicsView->setScene(drawing_scene);
			drawing_graphicsView->setParent(this);

			*result = QImage(drawing_scene->sceneRect().size().toSize(), QImage::Format_RGB888);


			//		plot->installEventFilter(this);

			//		QObject::connect(cbShowSymbol, SIGNAL(toggled(bool)), this, SLOT(slotSetSymbol(bool)));
			//		QObject::connect(cbIgnoreScale, SIGNAL(toggled(bool)), this, SLOT(slotSetTransform(bool)));


			setAttribute(Qt::WA_DeleteOnClose, true);
			drawing_scene->installEventFilter(this);
			installEventFilter(this);
			//добавляем объект в массив диалогов
			gui_controller.AddWidget(this);
		}
		catch (...)
		{
		}
	}

	PainterWindow::~PainterWindow()
	{
	//	QPainter painter(result.get());
	//	drawing_scene->render(&painter);

		//удаляем объект из массива диалогов
		gui_controller.RemoveWidget(this);
		delete drawing_scene;
		delete drawing_graphicsView;
	}

	//QImage	PainterWindow::GetResult()
	//{
	////	QPainter painter(&*result);
	//	QPainter painter(result.get());
	//	drawing_scene->render(&painter);
	//
	//	return result;
	//}



	void PainterWindow::closeEvent(QCloseEvent* event)
	{

		QDialog::closeEvent(event);
	}

	// Обработчик всех событий
	bool PainterWindow::eventFilter(QObject* target, QEvent* event)
	{
		if (target == drawing_scene || target == drawing_graphicsView)
		{
			// если произошло одно из событий от мыши, то
			switch (event->type())
			{
			//case QMouseEvent::GraphicsSceneMouseMove:
			case QMouseEvent::GraphicsSceneMousePress:
			case QMouseEvent::GraphicsSceneMouseRelease:
			{
				QPainter painter(result.get());
				drawing_scene->render(&painter);
			}
			break;
			default:
				break;
			};
		}
		//передаем управление стандартному обработчику событий
		return QObject::eventFilter(target, event);
	}

	void PainterWindow::keyPressEvent(QKeyEvent* event)
	{
		if (event->type() == QEvent::KeyPress)
		{
			switch (event->key())
			{
				//case Qt::Key_S:
				//	if(event->modifiers() == Qt::ControlModifier) slotSavePicture();
				//	break;

			case Qt::Key_Escape:
				emit signal_esc();
				break;
			};
		}
		return QWidget::keyPressEvent(event);
	}




}//namespace XRAD_GUI
