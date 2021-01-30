﻿#include "pre.h"
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



PainterWindow::PainterWindow(const QString &in_title, size_t in_vsize, size_t in_hsize, GUIController &gc)
	:
	DataDisplayWindow(gc),
	m_vsize(in_vsize),
	m_hsize(in_hsize), 
	m_title(in_title)
{
	try
	{
		// задаем положение окна
		auto corner = GetCornerPosition();
		setGeometry(QRect(QPoint(corner.x(), corner.y()), QPoint(int(in_vsize), int(in_hsize))));

//		SetBackground(Qt::white, Qt::SolidPattern);

//TODO	Копировать сюда остальное



		if (objectName().isEmpty()) setObjectName(m_title);
		setWindowTitle(m_title);

		drawing_graphicsView = new QGraphicsView();
		drawing_scene = new PaintScene();
		return_result_button = new QPushButton();
		image_label = new QLabel();



		drawing_scene->SetColor(Qt::red);

		connect(return_result_button, SIGNAL(clicked()), this, SLOT(return_result_button_click()));



		resize(m_hsize + 1, m_vsize + 80);
		drawing_graphicsView->resize(m_hsize, m_vsize);

		return_result_button->setParent(this);
		return_result_button->setGeometry((m_hsize / 2) - 50, m_vsize, 100, 40);
		return_result_button->setText("Send image");

		drawing_scene->setParent(drawing_graphicsView);
		drawing_scene->setSceneRect(0, 0, m_hsize - 2, m_vsize - 2);
		drawing_scene->setBackgroundBrush(Qt::white);

		drawing_graphicsView->setScene(drawing_scene);
		drawing_graphicsView->setParent(this);



//		plot->installEventFilter(this);

//		QObject::connect(cbShowSymbol, SIGNAL(toggled(bool)), this, SLOT(slotSetSymbol(bool)));
//		QObject::connect(cbIgnoreScale, SIGNAL(toggled(bool)), this, SLOT(slotSetTransform(bool)));


		//добавляем объект в массив диалогов
		gui_controller.AddWidget(this);
		setAttribute(Qt::WA_DeleteOnClose);
		installEventFilter(this);
	}
	catch(...)
	{
	}
}

PainterWindow::~PainterWindow()
{
	//удаляем объект из массива диалогов
	gui_controller.RemoveWidget(this);
	delete drawing_graphicsView;
	delete drawing_scene;

}

QImage	PainterWindow::GetResult()
{
	return result;
}

void PainterWindow::return_result_button_click()
{
	result = QImage(drawing_scene->sceneRect().size().toSize(), QImage::Format_RGB888);

	QPainter painter(&result);
	drawing_scene->render(&painter);


	image_label->setGeometry(500, 500, result.width(), result.height());
	image_label->setPixmap(QPixmap::fromImage(result));
	image_label->show();
}


void PainterWindow::closeEvent(QCloseEvent *event)
{
	QDialog::closeEvent(event);
}

/*
void PainterWindow::SavePicture(QString filename)
{
	if(filename.isEmpty()) return;

	QwtPlotRenderer	renderer;
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, true);
	//	renderer.setDiscardFlag(QwtPlotRenderer::DiscardLegend, true);
	renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasFrame, true);

	QString	format = FormatByFileExtension(filename);
	bool	raster_format = IsRasterImageFormat(format);
	bool	vector_format = IsVectorImageFormat(format);
	if(raster_format||vector_format)
	{
		// приходится делать равностороннюю геометрию, чтобы сохранить толщину осей!
		const QRect	store_geometry = geometry();
		//		QRect	cg = plot->canvas()->geometry();
		QRect	cg = plot->geometry();
		// для этого меняем размер всего окна таким образом, чтобы размер плота был new_size x new_size

		int	new_size = max(cg.width(), cg.height());
		QRect	new_geometry(geometry().x(), geometry().y(),
							 new_size + geometry().width()-cg.width(),
							 new_size + geometry().height()-cg.height());
		setGeometry(new_geometry);
		UpdateAxesDrawer();

		if(vector_format)
			renderer.renderDocument(plot, filename, format, QSizeF(100, 100), 100);
		else
			// 			renderer.renderDocument(plot, file_name, format, QSizeF(300,300), 100);
			renderer.renderDocument(plot, filename, format, QSizeF(100, 100), 300);
		// для растрового формата увеличиваем разрешение.
		// для векторных этого не делаем, т.к. нарушается толщина
		// кривой на легенде

		setGeometry(store_geometry);
		UpdateAxesDrawer();
	}

}

*/


void PainterWindow::slotSavePicture()
{
	const char *prompt = "Save picture";
	const char *type = "png (*.png);;pdf (*.pdf);;svg (*.svg);;jpeg (*.jpg);;bmp (*.bmp)";
	QString filename = GetSaveFileName(QFileDialog::tr(prompt), QFileDialog::tr(type));

	SavePicture(filename);
}

// Обработчик всех событий
bool PainterWindow::eventFilter(QObject *target, QEvent *event)
{


	// если событие произошло для графика, то

	//if(target == plot)
	//{
	//	// если произошло одно из событий от мыши, то
	//	switch(event->type())
	//	{
	//		case QEvent::MouseButtonPress:
	//		case QEvent::MouseMove:
	//		case QEvent::MouseButtonRelease:
	//			procMouseEvent(event);
	//			break;

	//		case QEvent::Resize:
	//		case QEvent::Show:
	//			// 			case QEvent::UpdateRequest:
	//			// 			case QEvent::Paint:
	//			UpdateAxesDrawer();
	//		default:
	//			break;
	//	};
	//}
	// передаем управление стандартному обработчику событий
	return QObject::eventFilter(target, event);
}

void PainterWindow::keyPressEvent(QKeyEvent *event)
{
	//if(event->type()==QEvent::KeyPress)
	//{
	//	switch(event->key())
	//	{
	//		case Qt::Key_S:
	//			if(event->modifiers() == Qt::ControlModifier) slotSavePicture();
	//			break;

	//		case Qt::Key_Escape:
	//			emit signal_esc();
	//			break;
	//	};
	//}
	return QWidget::keyPressEvent(event);
}



//// Обработчик обычных событий от мыши
//void PainterWindow::procMouseEvent(QEvent *event)
//{
//	// создаем указатель на событие от мыши
////	QMouseEvent *mEvent = static_cast<QMouseEvent *>(event);
//
//	// в зависимости от типа события вызываем соответствующий обработчик
//	//switch(event->type())
//	//{
//	//	// нажата кнопка мыши
//	//	case QEvent::MouseButtonPress:
//	//		StartCurrentValueDraw(mEvent);
//	//		break;
//	//		// перемещение мыши
//	//	case QEvent::MouseMove:
//	//		DrawCurrentValue(mEvent);
//	//		break;
//	//		// отпущена кнопка мыши
//	//	case QEvent::MouseButtonRelease:
//	//		EndCurrentValueDraw(mEvent);
//	//		break;
//	//		// для прочих событий ничего не делаем
//	//	default:;
//	//}
//}


}//namespace XRAD_GUI
