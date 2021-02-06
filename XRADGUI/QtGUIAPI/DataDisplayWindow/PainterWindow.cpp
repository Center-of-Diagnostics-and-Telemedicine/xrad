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
			ui.size_spinBox->installEventFilter(this);
			ui.red_spinBox->installEventFilter(this);
			ui.blue_spinBox->installEventFilter(this);
			ui.green_spinBox->installEventFilter(this);
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
		if (target == drawing_scene )
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
		if (target == ui.size_spinBox)
		{
			ui.drawing_graphicsView->setCursor(GetCursor(ui.size_spinBox->value() / 2));
			drawing_scene->SetBrushSize(ui.size_spinBox->value());
		}
		if (target == ui.red_spinBox || ui.blue_spinBox || ui.green_spinBox)
		{
			drawing_scene->SetBrushColor(QColor::fromRgb(ui.red_spinBox->value(), ui.green_spinBox->value(), ui.blue_spinBox->value()));
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


	QPixmap PainterWindow::GetCursor(size_t  radius)
	{
		//radius = 10;

		QPixmap result_pxmp;
		QImage result_img(radius * 2, radius * 2, QImage::Format_RGBA8888);

		float thickness = radius < 30 ? 1.7 : float(radius) / 20;
		float	circle_radius = radius - thickness;

		auto alpha = [](float d) -> unsigned int {return unsigned int(255. * sqrt(d)) << 24; };
		int	black = 0;
		int white = 0xFFFFFF;


		for (size_t i = 0; i < 2 * radius; i++)
		{
			for (size_t j = 0; j < 2 * radius; j++)
			{
				float delta = hypot(float(i) - radius, float(j) - radius) - circle_radius;
				auto pt = QPoint(i, j);
				float	d = fabs(delta) / thickness;

				if (d < 1)
				{
					int	color = delta < 0 ? black : white;

					result_img.setPixel(pt, color | alpha(1 - d));
				}
				else result_img.setPixel(pt, 0x00000000);
			}
		}
		result_pxmp = QPixmap::fromImage(result_img);

		return result_pxmp;
	}

}//namespace XRAD_GUI
