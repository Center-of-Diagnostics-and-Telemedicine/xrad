#include "pre.h"

#include "paintwidget.h"
#include <QBrush>
#include <QDebug>
#include <QPainter>
#include <QPen>

namespace XRAD_GUI {

XRAD_USING



PaintWidget::PaintWidget(QWidget* parent, size_t in_vsize, size_t in_hsize, std::shared_ptr<QImage> in_result) :
	QWidget(parent),
	m_width(in_hsize),
	m_height(in_vsize),
	m_presult(in_result)
{
	m_ptarget_pixmap = new QPixmap(int(m_width), int(m_height));
	m_ptarget_pixmap->fill();


	initPen(m_drawing_pen, m_color, 10, Qt::RoundCap);
	m_drawing_pen.setJoinStyle(Qt::MiterJoin);
	initPen(m_erasing_pen, Qt::white, 10, Qt::RoundCap);


}

PaintWidget::~PaintWidget()
{
	delete m_ptarget_pixmap;
}

//events

void PaintWidget::mousePressEvent(QMouseEvent* event)
{
	QPainter PixmapPainter(m_ptarget_pixmap);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		m_left_btn_pressed = true;
		if (m_drawer == Drawers::Hand)
		{
			PixmapPainter.setPen(m_drawing_pen);
			PixmapPainter.drawPoint(event->pos());
		}
		if (m_drawer == Drawers::Eraser)
		{
			PixmapPainter.setPen(m_erasing_pen);
			PixmapPainter.drawPoint(event->pos());
		}
		if (m_drawer == Drawers::Filler)
		{
			PixmapPainter.drawImage(QPoint(0, 0), getFilledImageFromPoint(*m_presult, event->pos(), m_color.rgb()));

		}
	}
	if (event->button() == Qt::MouseButton::RightButton)
	{
		m_right_btn_pressed = true;

		PixmapPainter.setPen(m_erasing_pen);
		PixmapPainter.drawPoint(event->pos());
	}


	m_figure.setP1(event->pos());
	m_figure.setP2(event->pos());
	m_previous_pos = event->pos();
	m_current_pos = event->pos();

	m_last_state.push(*m_presult);

	update();
}

void PaintWidget::mouseReleaseEvent(QMouseEvent* event)
{

	if (event->button() == Qt::MouseButton::LeftButton)
		m_left_btn_pressed = false;

	if (event->button() == Qt::MouseButton::RightButton)
		m_right_btn_pressed = false;


	m_last_state.push(*m_presult);
	m_current_pos = event->pos();
	update();


}

void PaintWidget::paintEvent(QPaintEvent* event)
{

	static bool wasPressed = false;
	QPainter painter(this);
	QPainter PixmapPainter(m_ptarget_pixmap);

	PixmapPainter.setPen(m_drawing_pen);
	painter.setPen(m_drawing_pen);



	if (m_left_btn_pressed)
	{
		painter.drawPixmap(0, 0, *m_ptarget_pixmap);
		drawFigure(painter, m_figure.p1(), m_figure.p2());

		wasPressed = true;
	}
	else if (wasPressed)
	{
		drawFigure(PixmapPainter, m_figure.p1(), m_figure.p2());
		painter.drawPixmap(0, 0, *m_ptarget_pixmap);
		update();
		wasPressed = false;
	}
	else
	{

		painter.drawPixmap(0, 0, *m_ptarget_pixmap);
		update();
	}
	*m_presult = m_ptarget_pixmap->toImage();
	update();
}

void PaintWidget::mouseMoveEvent(QMouseEvent* event)
{

	QPainter PixmapPainter(m_ptarget_pixmap);
	PixmapPainter.setPen(m_drawing_pen);


	if (event->modifiers() ==Qt::ShiftModifier)
	{
		int wid = event->pos().x() - m_figure.p1().x();
		int hei = event->pos().y() - m_figure.p1().y();

		int mid = (abs(wid) + abs(hei)) / 2;

		QPoint middle;

		if (wid <= 0 && hei <= 0)
			middle = QPoint(m_figure.p1().x() - mid, m_figure.p1().y() - mid);
		if (wid >= 0 && hei >= 0)
			middle = QPoint(m_figure.p1().x() + mid, m_figure.p1().y() + mid);
		if (wid <= 0 && hei >= 0)
			middle = QPoint(m_figure.p1().x() - mid, m_figure.p1().y() + mid);
		if (wid >= 0 && hei <= 0)
			middle = QPoint(m_figure.p1().x() + mid, m_figure.p1().y() - mid);

		m_figure.setP2(middle);


	}
	else
		m_figure.setP2(event->pos());


	m_current_pos = event->pos();
	if (m_left_btn_pressed)
	{
		if (m_drawer == Drawers::Hand)
		{
			PixmapPainter.setPen(m_drawing_pen);
			PixmapPainter.drawLine(m_previous_pos, event->pos());
			m_previous_pos = event->pos();
		}
		if (m_drawer == Drawers::Eraser)
		{
			PixmapPainter.setPen(m_erasing_pen);
			PixmapPainter.drawLine(m_previous_pos, event->pos());
			m_previous_pos = event->pos();
		}
	}
	else if (m_right_btn_pressed)
	{
		PixmapPainter.setPen(m_erasing_pen);
		PixmapPainter.drawLine(m_previous_pos, event->pos());
		m_previous_pos = event->pos();
	}

}


//operations
void PaintWidget::init(int x, int y, size_t w, size_t h, int drawer, const QColor& color, size_t brush_size)
{
	setGeometry(x, y, int(w), int(h));
	setDrawer(drawer);
	setColor(color);
	setBrushSize(brush_size);
}

void PaintWidget::clear()
{
	QImage ci = QImage(int(m_width), int(m_height), QImage::Format_RGBA8888);

	for (size_t i = 0; i < m_width; i++)
	{
		for (size_t j = 0; j < m_height; j++)
		{
			ci.setPixel(int(i), int(j), 0xffffffff);
		}
	}
	*m_ptarget_pixmap = QPixmap::fromImage(ci);
}

void PaintWidget::undo()
{
	if (!m_last_state.empty() && !m_left_btn_pressed)
	{
		m_pre_last_state.push(*m_presult);
		*m_ptarget_pixmap = QPixmap::fromImage(m_last_state.top());
		m_last_state.pop();
		update();
	}

}

void PaintWidget::redo()
{
	if (!m_pre_last_state.empty() && !m_left_btn_pressed)
	{
		m_last_state.push(*m_presult);
		*m_ptarget_pixmap = QPixmap::fromImage(m_pre_last_state.top());
		m_pre_last_state.pop();
		update();
	}
}


//supporting methods

QPixmap PaintWidget::getCursor(size_t in_radius)
{

	size_t radius = in_radius > 3 ? in_radius : 3;


	QPixmap result_pxmp;
	QImage result_img(int(radius) * 2, int(radius) * 2, QImage::Format_RGBA8888);

	float thickness = radius < 30 ? 1.7 : float(radius) / 20;
	float	circle_radius = radius - thickness;

	auto alpha = [](float d) -> unsigned int {return unsigned int(255. * sqrt(d)) << 24; };
	int	black = 0x000000;
	int white = 0xFFFFFF;


	for (size_t i = 0; i < 2 * radius; i++)
	{
		for (size_t j = 0; j < 2 * radius; j++)
		{
			float delta = hypot(float(i) - radius, float(j) - radius) - circle_radius;
			auto pt = QPoint(int(i), int(j));
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

void PaintWidget::initPen(QPen& pen, const QColor& in_color, int in_width, Qt::PenCapStyle in_style)
{
	pen.setColor(in_color);
	pen.setWidth(in_width);
	pen.setCapStyle(in_style);
}

void PaintWidget::drawFigure(QPainter& painter, const  QPoint& inp1, const  QPoint& inp2)
{

	if (m_drawer == Drawers::Line)
		painter.drawLine(QLine(inp1, inp2));
	else if (m_drawer == Drawers::Rect)
		painter.drawRect(QRect(inp1, inp2));
	else if (m_drawer == Drawers::Ellipse)
		painter.drawEllipse(QRect(inp1, inp2));

}

QImage PaintWidget::getFilledImageFromPoint(const QImage& img, const QPoint& point, const QRgb& color)
{
	QImage result = img;
	QRgb targetColor = result.pixel(point.x(), point.y());

	std::stack<QPoint> points;
	points.push(point);
	if (targetColor == color)
	{
		return result;
	}

	while (!points.empty())
	{
		int x = points.top().x();
		int y = points.top().y();
		points.pop();

		if (result.pixel(x - 1, y) == targetColor)
		{
			result.setPixel(x - 1, y, color);
			points.push(QPoint(x - 1, y));
		}
		if (result.pixel(x + 1, y) == targetColor)
		{
			result.setPixel(x + 1, y, color);
			points.push(QPoint(x + 1, y));
		}
		if (result.pixel(x, y - 1) == targetColor)
		{
			result.setPixel(x, y - 1, color);
			points.push(QPoint(x, y - 1));
		}
		if (result.pixel(x, y + 1) == targetColor)
		{
			result.setPixel(x, y + 1, color);
			points.push(QPoint(x, y + 1));
		}
	}

	return result;
}

void PaintWidget::fillFromPoint(const QPoint& point, const QColor& color)
{
	*m_ptarget_pixmap = QPixmap::fromImage(getFilledImageFromPoint(*m_presult, point, color.rgb()));
}




//setters
void PaintWidget::setDrawer(int in_drawer)
{
	m_drawer = in_drawer;
}

void PaintWidget::setColor(const QColor& in_color)
{
	m_color = in_color;
	m_drawing_pen.setColor(m_color);
}

void PaintWidget::setBrushSize(size_t in_size)
{
	m_brush_size = in_size;
	m_drawing_pen.setWidth(int(m_brush_size));
	m_erasing_pen.setWidth(int(m_brush_size));
}



void PaintWidget::setImage(const QImage& img)
{
	*m_ptarget_pixmap = QPixmap::fromImage(img);
}




//getters
QImage PaintWidget::image()
{
	return *m_presult;
}

QColor PaintWidget::color()
{
	return m_color;
}

size_t PaintWidget::brushSize()
{
	return m_brush_size;
}





} // namespace XRAD_GUI