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
	width_(in_hsize),
	height_(in_vsize),
	p_result_(in_result)
{
	ptarget_pixmap_ = new QPixmap(int(width_), int(height_));
	ptarget_pixmap_->fill();


	initPen(drawing_pen_, color_, 10, Qt::RoundCap);
	drawing_pen_.setJoinStyle(Qt::MiterJoin);
	initPen(erasing_pen_, Qt::white, 10, Qt::RoundCap);


}

PaintWidget::~PaintWidget()
{
	delete ptarget_pixmap_;
}

//events

void PaintWidget::mousePressEvent(QMouseEvent* event)
{
	QPainter PixmapPainter(ptarget_pixmap_);

	if (event->button() == Qt::MouseButton::LeftButton)
	{
		is_L_button_pressed_ = true;
		if (drawer_ == Drawers::Hand)
		{
			PixmapPainter.setPen(drawing_pen_);
			PixmapPainter.drawPoint(event->pos());
		}
		if (drawer_ == Drawers::Eraser)
		{
			PixmapPainter.setPen(erasing_pen_);
			PixmapPainter.drawPoint(event->pos());
		}
		if (drawer_ == Drawers::Filler)
		{
			PixmapPainter.drawImage(QPoint(0, 0), getFilledImageFromPoint(*p_result_, event->pos(), color_.rgb()));

		}
	}
	if (event->button() == Qt::MouseButton::RightButton)
	{
		is_R_button_pressed_ = true;

		PixmapPainter.setPen(erasing_pen_);
		PixmapPainter.drawPoint(event->pos());
	}


	figure_.setP1(event->pos());
	figure_.setP2(event->pos());
	previous_pos_ = event->pos();
	current_pos_ = event->pos();

	last_state_.push(*p_result_);

	update();
}

void PaintWidget::mouseReleaseEvent(QMouseEvent* event)
{

	if (event->button() == Qt::MouseButton::LeftButton)
		is_L_button_pressed_ = false;

	if (event->button() == Qt::MouseButton::RightButton)
		is_R_button_pressed_ = false;


	last_state_.push(*p_result_);
	current_pos_ = event->pos();
	update();


}

void PaintWidget::paintEvent(QPaintEvent* event)
{

	static bool wasPressed = false;
	QPainter painter(this);
	QPainter PixmapPainter(ptarget_pixmap_);

	PixmapPainter.setPen(drawing_pen_);
	painter.setPen(drawing_pen_);



	if (is_L_button_pressed_)
	{
		painter.drawPixmap(0, 0, *ptarget_pixmap_);
		drawFigure(painter, figure_.p1(), figure_.p2());

		wasPressed = true;
	}
	else if (wasPressed)
	{
		drawFigure(PixmapPainter, figure_.p1(), figure_.p2());
		painter.drawPixmap(0, 0, *ptarget_pixmap_);
		update();
		wasPressed = false;
	}
	else
	{

		painter.drawPixmap(0, 0, *ptarget_pixmap_);
		update();
	}
	*p_result_ = ptarget_pixmap_->toImage();
	update();
}

void PaintWidget::mouseMoveEvent(QMouseEvent* event)
{

	QPainter PixmapPainter(ptarget_pixmap_);
	PixmapPainter.setPen(drawing_pen_);


	if (is_shift_pressed_ == true)
	{
		int wid = event->pos().x() - figure_.p1().x();
		int hei = event->pos().y() - figure_.p1().y();

		int mid = (abs(wid) + abs(hei)) / 2;

		QPoint middle;

		if (wid <= 0 && hei <= 0)
			middle = QPoint(figure_.p1().x() - mid, figure_.p1().y() - mid);
		if (wid >= 0 && hei >= 0)
			middle = QPoint(figure_.p1().x() + mid, figure_.p1().y() + mid);
		if (wid <= 0 && hei >= 0)
			middle = QPoint(figure_.p1().x() - mid, figure_.p1().y() + mid);
		if (wid >= 0 && hei <= 0)
			middle = QPoint(figure_.p1().x() + mid, figure_.p1().y() - mid);

		figure_.setP2(middle);


	}
	else
		figure_.setP2(event->pos());


	current_pos_ = event->pos();
	if (is_L_button_pressed_)
	{
		if (drawer_ == Drawers::Hand)
		{
			PixmapPainter.setPen(drawing_pen_);
			PixmapPainter.drawLine(previous_pos_, event->pos());
			previous_pos_ = event->pos();
		}
		if (drawer_ == Drawers::Eraser)
		{
			PixmapPainter.setPen(erasing_pen_);
			PixmapPainter.drawLine(previous_pos_, event->pos());
			previous_pos_ = event->pos();
		}
	}
	else if (is_R_button_pressed_)
	{
		PixmapPainter.setPen(erasing_pen_);
		PixmapPainter.drawLine(previous_pos_, event->pos());
		previous_pos_ = event->pos();
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
	QImage ci = QImage(int(width_), int(height_), QImage::Format_RGBA8888);

	for (size_t i = 0; i < width_; i++)
	{
		for (size_t j = 0; j < height_; j++)
		{
			ci.setPixel(int(i), int(j), 0xffffffff);
		}
	}
	*ptarget_pixmap_ = QPixmap::fromImage(ci);
}

void PaintWidget::undo()
{
	if (!last_state_.empty() && !is_L_button_pressed_)
	{
		pre_lastState_.push(*p_result_);
		*ptarget_pixmap_ = QPixmap::fromImage(last_state_.top());
		last_state_.pop();
		update();
	}

}

void PaintWidget::redo()
{
	if (!pre_lastState_.empty() && !is_L_button_pressed_)
	{
		last_state_.push(*p_result_);
		*ptarget_pixmap_ = QPixmap::fromImage(pre_lastState_.top());
		pre_lastState_.pop();
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

	if (drawer_ == Drawers::Line)
		painter.drawLine(QLine(inp1, inp2));
	else if (drawer_ == Drawers::Rect)
		painter.drawRect(QRect(inp1, inp2));
	else if (drawer_ == Drawers::Ellipse)
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
	*ptarget_pixmap_ = QPixmap::fromImage(getFilledImageFromPoint(*p_result_, point, color.rgb()));
}




//setters
void PaintWidget::setDrawer(int in_drawer)
{
	drawer_ = in_drawer;
}

void PaintWidget::setColor(const QColor& in_color)
{
	color_ = in_color;
	drawing_pen_.setColor(color_);
}

void PaintWidget::setBrushSize(size_t in_size)
{
	brush_size_ = in_size;
	drawing_pen_.setWidth(int(brush_size_));
	erasing_pen_.setWidth(int(brush_size_));
}

void PaintWidget::setShiftPressed(bool is)
{
	is_shift_pressed_ = is;
}

void PaintWidget::setImage(const QImage& img)
{
	*ptarget_pixmap_ = QPixmap::fromImage(img);
}




//getters
QImage PaintWidget::image()
{
	return *p_result_;
}

QColor PaintWidget::color()
{
	return color_;
}

size_t PaintWidget::brushSize()
{
	return brush_size_;
}





} // namespace XRAD_GUI