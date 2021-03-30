#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include "pre.h"

#include <QLine>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QWidget>
#include <stack>




namespace XRAD_GUI 
{

XRAD_USING

enum Drawers
{
	Hand,
	Line,
	Rect,
	Ellipse,
	Eraser,
	Filler
};

class PaintWidget : public QWidget {
	Q_OBJECT


public:


	explicit PaintWidget(QWidget* parent, size_t in_vsize, size_t in_hsize, std::shared_ptr<QImage> in_result);
	~PaintWidget();

	//setters
	void setDrawer(int drawer);
	void setColor(const QColor&);
	void setBrushSize(size_t);
	void setShiftPressed(bool is);




	//getters
	QPixmap getCursor(size_t radius);
	QImage image();
	QColor color();
	size_t brushSize();


	void init(int x, int y, size_t width, size_t height, int drawer, const QColor& color, size_t brush_size);
	void clear();
	void undo();
	void redo();


protected:
	//events
	void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void paintEvent(QPaintEvent* event);

private: //methods

	QImage getFilledImageFromPoint(const QImage& image, const QPoint& point, const QRgb& color);

	void initPen(QPen&, const QColor&, int brush_size, Qt::PenCapStyle);

	void drawFigure(QPainter&, const QPoint&, const QPoint&);
	void fillFromPoint(const QPoint&, const QColor&);

private: //fields


	std::stack<QImage> last_state_;
	std::stack<QImage> pre_lastState_;

	QPixmap* ptarget_pixmap_;

	QLine figure_;
	QColor color_ = Qt::black;
	QPen drawing_pen_, erasing_pen_;
	QPoint current_pos_, previous_pos_;


	std::shared_ptr<QImage> p_result_;

	size_t brush_size_ = 10;
	size_t width_, height_;

	int drawer_;

	bool is_shift_pressed_ = false;
	bool is_L_button_pressed_ = false, is_R_button_pressed_ = false;

};

} // namespace XRAD_GUI
#endif // PAINTWIDGET_H
