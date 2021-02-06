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



		PainterWindow::PainterWindow(const QString& in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc)
		:
		DataDisplayWindow(gc),
		m_vsize(in_vsize),
		m_hsize(in_hsize),
		m_title(in_title),
		result(in_result)
	{
		try
		{
			ui.setupUi(this);
			// задаем положение окна
			auto corner = GetCornerPosition();
			setGeometry(QRect(QPoint(corner.x(), corner.y()), QPoint(int(in_hsize), int(in_vsize))));

			//		SetBackground(Qt::white, Qt::SolidPattern);

			//TODO	Копировать сюда остальное



			if (objectName().isEmpty()) setObjectName(m_title);
			setWindowTitle(m_title);

			
			drawing_scene = new PaintScene();
			drawing_scene->setBackgroundBrush(Qt::white);

			


			setFixedSize(QSize(int(m_hsize) + 14, int(m_vsize) + 75));

			drawing_scene->setParent(ui.drawing_graphicsView);
			drawing_scene->setSceneRect(0, 0, m_hsize, m_vsize);
			


			ui.drawing_graphicsView->setFixedSize(m_hsize + 4, m_vsize + 4);
			ui.drawing_graphicsView->setSceneRect(0, 0, m_hsize, m_vsize);
			ui.drawing_graphicsView->setScene(drawing_scene);
			

			*result = QImage(drawing_scene->sceneRect().size().toSize(), QImage::Format_RGB888);



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
		//delete drawing_graphicsView;
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
		if (target == drawing_scene || target == ui.drawing_graphicsView)
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
				drawing_scene->SetBrushColor(QColor::fromRgb(ui.red_spinBox->value(), ui.green_spinBox->value(), ui.blue_spinBox->value()));
				drawing_scene->SetBrushSize(ui.size_spinBox->value());
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
