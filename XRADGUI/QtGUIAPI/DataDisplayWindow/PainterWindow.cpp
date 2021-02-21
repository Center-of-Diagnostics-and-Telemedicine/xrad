#include "PainterWindow.h"
#include "pre.h"

#include "FileSaveUtils.h"
#include "GUIController.h"
#include "ThreadGUI.h"
#include <XRADQt/QtStringConverters.h>
#include <XRADSystem/TextFile.h>

namespace XRAD_GUI {

	XRAD_USING

		//
		//--------------------------------------------------------------

		PainterWindow::PainterWindow(const QString& in_title, size_t in_vsize, size_t in_hsize, shared_ptr<QImage> in_result, GUIController& gc)
		: DataDisplayWindow(gc),
		m_nVSize(in_vsize),
		m_nHSize(in_hsize),
		m_sTitle(in_title),
		m_pResult(in_result)
	{
		try {
			ui.setupUi(this);

			// QHBoxLayout* hbox = new QHBoxLayout();

			pw = new PaintWidget(this, m_nVSize, m_nHSize, m_pResult);

			setWindowTitle(in_title);
			setFixedSize(QSize(int(m_nHSize) + 10, int(m_nVSize) + 100));
			setMinimumSize(460, int(m_nVSize) + 50);

			*m_pResult = QImage(QSize(int(m_nHSize), int(m_nVSize)), QImage::Format_RGB888);

			setLayout(ui.verticalLayout);

			ui.verticalLayout->addWidget(pw);

			ui.red_spinBox->installEventFilter(this);
			ui.green_spinBox->installEventFilter(this);
			ui.blue_spinBox->installEventFilter(this);
			ui.size_spinBox->installEventFilter(this);
			ui.comboBox->installEventFilter(this);

			pw->installEventFilter(this);
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
		if (target == ui.comboBox) {
			if (ui.comboBox->currentText() == "Hand") {
				pw->SetDrawer(PaintWidget::Drawers::Hand);
			}
			else if (ui.comboBox->currentText() == "Rect") {
				pw->SetDrawer(PaintWidget::Drawers::Rect);
			}
			else if (ui.comboBox->currentText() == "Ellipse") {
				pw->SetDrawer(PaintWidget::Drawers::Ellipse);
			}
			else if (ui.comboBox->currentText() == "Line") {
				pw->SetDrawer(PaintWidget::Drawers::Line);
			}
		}
		if (target == ui.red_spinBox || target == ui.green_spinBox || target == ui.blue_spinBox) {
			pw->SetColor(QColor::fromRgb(ui.red_spinBox->value(),
				ui.green_spinBox->value(),
				ui.blue_spinBox->value()));
			ui.color_view_widget->setStyleSheet(
				GetStringStyleSheet(ui.red_spinBox->value(),
					ui.green_spinBox->value(),
					ui.blue_spinBox->value()));
		}
		if (target == ui.size_spinBox) {
			pw->SetBrushSize(ui.size_spinBox->value());
			pw->setCursor(GetCursor(ui.size_spinBox->value() / 2));
		}

		return QObject::eventFilter(target, event);
	}

	void PainterWindow::keyPressEvent(QKeyEvent* event)
	{
		if (event->type() == QEvent::KeyPress) {
			switch (event->key()) {
			case Qt::Key_Escape:
				emit signal_esc();
				break;
			case Qt::Key_Shift:
				emit signal_esc();
				break;
			};
		}
		return QWidget::keyPressEvent(event);
	}

	QPixmap
		PainterWindow::GetCursor(size_t in_radius)
	{
		// radius = 10;
		size_t radius = in_radius > 3 ? in_radius : 3;

		QPixmap result_pxmp;
		QImage result_img(int(radius) * 2, int(radius) * 2, QImage::Format_RGBA8888);

		float thickness = radius < 30 ? 1.7 : float(radius) / 20;
		float circle_radius = radius - thickness;

		auto alpha = [](float d) -> unsigned int {
			return unsigned int(255. * sqrt(d)) << 24;
		};
		int black = 0;
		int white = 0xFFFFFF;

		for (size_t i = 0; i < 2 * radius; i++) {
			for (size_t j = 0; j < 2 * radius; j++) {
				float delta = hypot(float(i) - radius, float(j) - radius) - circle_radius;
				auto pt = QPoint(int(i), int(j));
				float d = fabs(delta) / thickness;

				if (d < 1) {
					int color = delta < 0 ? black : white;

					result_img.setPixel(pt, color | alpha(1 - d));
				}
				else
					result_img.setPixel(pt, 0x00000000);
			}
		}
		result_pxmp = QPixmap::fromImage(result_img);

		return result_pxmp;
	}

	QString
		PainterWindow::GetStringStyleSheet(int r, int g, int b)
	{
		QString result;

		result.append("background-color: rgb(");
		result.append(QString::number(r));
		result.append(", ");
		result.append(QString::number(g));
		result.append(", ");
		result.append(QString::number(b));
		result.append(", ");

		result.append(");");

		return result;
	}
} // namespace XRAD_GUI
