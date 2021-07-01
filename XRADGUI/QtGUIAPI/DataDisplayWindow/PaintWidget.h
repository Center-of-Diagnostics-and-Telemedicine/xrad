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


XRAD_USING


namespace xrad
{
	enum Drawers
	{
		Hand,
		Line,
		Rect,
		Ellipse,
		Eraser,
		Filler
	};
};

namespace XRAD_GUI 
{

class PaintWidget : public QWidget {
	Q_OBJECT


public:


	explicit PaintWidget(QWidget* parent, size_t in_vsize, size_t in_hsize, std::shared_ptr<QImage> in_result);
	~PaintWidget();

	//setters
	void setDrawer(int drawer);
	void setColor(const QColor&);
	void setBrushSize(size_t);
	void setImage(const QImage&);



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


	std::stack<QImage> m_last_state;
	std::stack<QImage> m_pre_last_state;

	QPixmap* m_ptarget_pixmap;

	QLine m_figure;
	QColor m_color = Qt::black;
	QPen m_drawing_pen, m_erasing_pen;
	QPoint m_current_pos, m_previous_pos;


	std::shared_ptr<QImage> m_presult;

	size_t m_brush_size = 10;
	size_t m_width, m_height;

	int m_drawer;

	
	bool m_left_btn_pressed = false, m_right_btn_pressed = false;

};

} // namespace XRAD_GUI
#endif // PAINTWIDGET_H
