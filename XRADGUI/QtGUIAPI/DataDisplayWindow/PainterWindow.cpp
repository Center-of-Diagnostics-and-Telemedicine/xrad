#include "PainterWindow.h"
#include "pre.h"

#include "FileSaveUtils.h"
#include "GUIController.h"
#include "ThreadGUI.h"
#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/TextFile.h>
#include <QSpinBox>

namespace XRAD_GUI {

XRAD_USING

//
//--------------------------------------------------------------

PainterWindow::PainterWindow(const QString& in_title, shared_ptr<QImage> in_result, GUIController& gc)
	: DataDisplayWindow(gc),
	m_height(in_result->height()),
	m_width(in_result->width()),
	m_title(in_title),
	m_presult(in_result)
{
	try {
		ui.setupUi(this);

		// QHBoxLayout* hbox = new QHBoxLayout();

		m_sb_brush_size = new SpinBoxWidget(this, "Brush size:", 1, 255);
		m_color_panel = new ColorPanel(this);
		m_tools_menu = new ToolsMenuWidget(0, ui.verticalLayoutWidget->height(), this);
		m_painter_frame = new QFrame(this);
		m_paint_widget = new PaintWidget(m_painter_frame, m_width, m_height, m_presult);

		m_paint_widget->setImage(*m_presult);
		
		m_paint_widget->init(1, 1, m_width, m_height, Drawers::Hand, Qt::black, 10);
		setWindowTitle(m_title);
	

//		*presult_ = QImage(QSize(int(width_), int(height_)), QImage::Format_RGBA8888);

		addToolsMenuItem(50, 50, Drawers::Hand, "c:/temp/hand.png");
		addToolsMenuItem(50, 50, Drawers::Line, "c:/temp/line.png");
		addToolsMenuItem(50, 50, Drawers::Rect, "c:/temp/rect.png");
		addToolsMenuItem(50, 50, Drawers::Ellipse, "c:/temp/ellipse.png");
		addToolsMenuItem(50, 50, Drawers::Eraser, "c:/temp/eraser.png");
		addToolsMenuItem(50, 50, Drawers::Filler, "c:/temp/fill.png");

		m_painter_frame->setGeometry(m_tools_menu->width() + 1, ui.verticalLayoutWidget->height() + 1, m_paint_widget->width() + 2, m_paint_widget->height() + 2);
		m_painter_frame->setFrameShape(QFrame::Shape::Box);

		setMinimumSize(720, m_paint_widget->height() < 420 ? 420 : m_paint_widget->height() + ui.verticalLayoutWidget->height() + 50);

		setGeometry(0, 0, m_tools_menu->width() + m_paint_widget->width() + 10, m_paint_widget->height() + ui.verticalLayoutWidget->height() + 50);

		
		ui.horizontalLayout->addWidget(m_sb_brush_size);
		ui.horizontalLayout->addWidget(m_color_panel);

		m_paint_widget->installEventFilter(this);
		m_color_panel->installEventFilter(this);
//		m_color_panel->_ief(this);
		m_sb_brush_size->installEventFilter(this);
		m_paint_widget->installEventFilter(this);
		installEventFilter(this);

		//добавляем объект в массив добавляем
		gui_controller.AddWidget(this);
	}
	catch (...) {
	}
}


// PainterWindow::PainterWindow(const QString& in_title, const QImage& image, shared_ptr<QImage> in_result, GUIController& gc)
// 	: DataDisplayWindow(gc),
// 	height_(image.height()),
// 	width_(image.width()),
// 	title_(in_title),
// 	presult_(in_result)
// {
// 
// 	ui.setupUi(this);
// 
// 	setWindowTitle(title_);
// 	setMinimumSize(720, paint_widget_->height() < 420 ? 420 : paint_widget_->height() + ui.verticalLayoutWidget->height() + 50);
// 	setGeometry(0, 0, tools_menu_->width() + paint_widget_->width() + 10, paint_widget_->height() + ui.verticalLayoutWidget->height() + 50);
// 
// 
// 	*presult_ = image;
// 
// 	spin_box_ = new SpinBoxWidget(this, "Brush size:", 1, 255);
// 	color_panel_ = new ColorPanel(this);
// 	tools_menu_ = new ToolsMenuWidget(0, ui.verticalLayoutWidget->height(), this);
// 	painter_frame_ = new QFrame(this);
// 	paint_widget_ = new PaintWidget(painter_frame_, width_, height_, presult_);
// 
// 
// 	paint_widget_->init(1, 1, width_, height_, Drawers::Hand, Qt::black, 10);
// 	painter_frame_->setGeometry(tools_menu_->width() + 1, ui.verticalLayoutWidget->height() + 1, paint_widget_->width() + 2, paint_widget_->height() + 2);
// 	painter_frame_->setFrameShape(QFrame::Shape::Box);
// 
// 	ui.horizontalLayout->addWidget(spin_box_);
// 	ui.horizontalLayout->addWidget(color_panel_);
// 
// 	addToolsMenuItem(50, 50, Drawers::Hand, "c:/temp/hand.png");
// 	addToolsMenuItem(50, 50, Drawers::Line, "c:/temp/line.png");
// 	addToolsMenuItem(50, 50, Drawers::Rect, "c:/temp/rect.png");
// 	addToolsMenuItem(50, 50, Drawers::Ellipse, "c:/temp/ellipse.png");
// 	addToolsMenuItem(50, 50, Drawers::Eraser, "c:/temp/eraser.png");
// 	addToolsMenuItem(50, 50, Drawers::Filler, "c:/temp/fill.png");
// 
// 
// 
// 	paint_widget_->installEventFilter(this);
// 	color_panel_->installEventFilter(this);
// 	spin_box_->installEventFilter(this);
// 	paint_widget_->installEventFilter(this);
// 	installEventFilter(this);
// 
// 	//добавляем объект в массив добавляем
// 	gui_controller.AddWidget(this);
// 
// 
// }

PainterWindow::~PainterWindow()
{
	//удаляем объект из массива диалогов
	gui_controller.RemoveWidget(this);
}

void PainterWindow::closeEvent(QCloseEvent* event)
{
	QDialog::closeEvent(event);
}



// Обработчик всех событий
bool PainterWindow::eventFilter(QObject* target, QEvent* event)
{
	
	if (target == m_paint_widget && event->type() == QMouseEvent::MouseButtonPress)
	{
		m_paint_widget->setColor(m_color_panel->currentColor());
		m_paint_widget->setDrawer(m_tools_menu->getCurrentDrawer());
	}
	if (target == m_sb_brush_size)
	{
		m_paint_widget->setBrushSize(m_sb_brush_size->value());
		m_paint_widget->setCursor(m_paint_widget->getCursor(m_sb_brush_size->value() / 2));
	}
	
	if (event->type() == QEvent::KeyPress)
	{
		
		switch (static_cast<QKeyEvent*>(event)->key())
		{
			
			case Qt::Key_Z:
				m_paint_widget->undo();
				break;
			case Qt::Key_Y:
				m_paint_widget->redo();
				break;
			case Qt::Key_Delete:
				m_paint_widget->clear();
				break;
			case Qt::Key_BracketLeft:
				m_sb_brush_size->setVal(m_paint_widget->brushSize() - 1);
				break;
			case Qt::Key_BracketRight:
				m_sb_brush_size->setVal(m_paint_widget->brushSize() + 1);
				break;
			default:
				break;
		}
	}
	if (event->type() == QEvent::KeyRelease)
	{
		if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Shift)
		{
		//	paint_widget_->setShiftPressed(false);
		};
	}



	return DataDisplayWindow::eventFilter(target, event);
}

void PainterWindow::keyPressEvent(QKeyEvent* event)
{
	if (event->type() == QEvent::KeyPress) {
		switch (event->key()) {
			case Qt::Key_Escape:
				emit signal_esc();
				break;
			
		};
	}
	return DataDisplayWindow::keyPressEvent(event);
}

void PainterWindow::addToolsMenuItem(size_t w, size_t h, int drawer, const QString& path)
{
	QIcon icon = QIcon(QPixmap(path));
	m_tools_menu->addItem(w, h, drawer, icon);

}

} // namespace XRAD_GUI
