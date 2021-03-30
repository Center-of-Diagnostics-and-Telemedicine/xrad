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

PainterWindow::PainterWindow(const QString& in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc)
	: DataDisplayWindow(gc),
	height_(in_vsize),
	width_(in_hsize),
	title_(in_title),
	presult_(in_result)
{
	try {
		ui.setupUi(this);

		// QHBoxLayout* hbox = new QHBoxLayout();

		spin_box_ = new SpinBoxWidget(this, "Brush size:", 1, 255);
		color_panel_ = new ColorPanel(this);
		tools_menu_ = new ToolsMenuWidget(0, ui.verticalLayoutWidget->height(), this);
		painter_frame_ = new QFrame(this);
		paint_widget_ = new PaintWidget(painter_frame_, width_, height_, presult_);


		paint_widget_->init(1, 1, width_, height_, Drawers::Hand, Qt::black, 10);
		setWindowTitle(title_);
	

		*presult_ = QImage(QSize(int(width_), int(height_)), QImage::Format_RGBA8888);

		addToolsMenuItem(50, 50, Drawers::Hand, "c:/temp/hand.png");
		addToolsMenuItem(50, 50, Drawers::Line, "c:/temp/line.png");
		addToolsMenuItem(50, 50, Drawers::Rect, "c:/temp/rect.png");
		addToolsMenuItem(50, 50, Drawers::Ellipse, "c:/temp/ellipse.png");
		addToolsMenuItem(50, 50, Drawers::Eraser, "c:/temp/eraser.png");
		addToolsMenuItem(50, 50, Drawers::Filler, "c:/temp/fill.png");

		painter_frame_->setGeometry(tools_menu_->width() + 1, ui.verticalLayoutWidget->height() + 1, paint_widget_->width() + 2, paint_widget_->height() + 2);
		painter_frame_->setFrameShape(QFrame::Shape::Box);

		setMinimumSize(720, paint_widget_->height() < 420 ? 420 : paint_widget_->height() + ui.verticalLayoutWidget->height() + 50);

		setGeometry(0, 0, tools_menu_->width() + paint_widget_->width() + 10, paint_widget_->height() + ui.verticalLayoutWidget->height() + 50);

		
		ui.horizontalLayout->addWidget(spin_box_);
		ui.horizontalLayout->addWidget(color_panel_);

		paint_widget_->installEventFilter(this);
		color_panel_->installEventFilter(this);
		spin_box_->installEventFilter(this);
		paint_widget_->installEventFilter(this);
		installEventFilter(this);

		//добавляем объект в массив добавляем
		gui_controller.AddWidget(this);
	}
	catch (...) {
	}
}

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
	
	if (target == paint_widget_ && event->type() == QMouseEvent::MouseButtonPress)
	{
		paint_widget_->setColor(color_panel_->currentColor());
		paint_widget_->setDrawer(tools_menu_->getCurrentDrawer());
	}
	if (target == spin_box_)
	{
		paint_widget_->setBrushSize(spin_box_->value());
		paint_widget_->setCursor(paint_widget_->getCursor(spin_box_->value() / 2));
	}
	
	if (event->type() == QEvent::KeyPress)
	{
		
		switch (static_cast<QKeyEvent*>(event)->key())
		{
			case Qt::Key_Shift:
				paint_widget_->setShiftPressed(true);
				break;
			case Qt::Key_Z:
				paint_widget_->undo();
				break;
			case Qt::Key_Y:
				paint_widget_->redo();
				break;
			case Qt::Key_Delete:
				paint_widget_->clear();
				break;
			default:
				break;
		}
	}
	if (event->type() == QEvent::KeyRelease)
	{
		if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Shift)
		{
			paint_widget_->setShiftPressed(false);
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
	tools_menu_->addItem(w, h, drawer, icon);

}

} // namespace XRAD_GUI
