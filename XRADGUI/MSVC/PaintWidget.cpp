#include "pre.h"

#include "paintwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QDebug>

namespace XRAD_GUI
{

	XRAD_USING

		PaintWidget::PaintWidget(QWidget* parent, size_t in_vsize, size_t in_hsize, std::shared_ptr<QImage> in_result) :
		QWidget(parent),
		m_nHsize(in_hsize),
		m_nVsize(in_vsize),
		m_pResult(in_result)
	{


		m_qPTargetPixmap = new QPixmap(m_nHsize, m_nVsize);
		m_qPTargetPixmap->fill();

		initPen(m_qDrawingPen, m_qcolor, 10, Qt::RoundCap);
		initPen(m_qErasingPen, Qt::white, 10, Qt::RoundCap);

	}

	PaintWidget::~PaintWidget()
	{
		delete m_qPTargetPixmap;
	}



	void PaintWidget::mousePressEvent(QMouseEvent* event)
	{
		QPainter PixmapPainter(m_qPTargetPixmap);
		if (event->button() == Qt::MouseButton::LeftButton)
		{
			m_bButtonLeft = true;
			if (m_nDrawer == Hand)
			{
				PixmapPainter.setPen(m_qDrawingPen);
				PixmapPainter.drawPoint(event->pos());
			}

		}
		if (event->button() == Qt::MouseButton::RightButton)
		{
			m_bButtonRight = true;

			PixmapPainter.setPen(m_qErasingPen);
			PixmapPainter.drawPoint(event->pos());
		}


		m_qFigure.setP1(event->pos());
		m_qFigure.setP2(event->pos());
		m_qPreviousPoint = event->pos();
		m_qCurrentPos = event->pos();
		update();
	}

	void PaintWidget::mouseReleaseEvent(QMouseEvent* event)
	{

		if (event->button() == Qt::MouseButton::LeftButton)
			m_bButtonLeft = false;

		if (event->button() == Qt::MouseButton::RightButton)
			m_bButtonRight = false;


		m_qCurrentPos = event->pos();
		update();
	}

	void PaintWidget::paintEvent(QPaintEvent* e)
	{

		static bool wasPressed = false;
		QPainter painter(this);
		QPainter PixmapPainter(m_qPTargetPixmap);

		PixmapPainter.setPen(m_qDrawingPen);
		painter.setPen(m_qDrawingPen);

		if (m_bButtonLeft)
		{
			painter.drawPixmap(0, 0, *m_qPTargetPixmap);
			DrawFigure(painter, m_qFigure.p1(), m_qFigure.p2());

			wasPressed = true;
		}
		else if (wasPressed)
		{
			DrawFigure(PixmapPainter, m_qFigure.p1(), m_qFigure.p2());
			painter.drawPixmap(0, 0, *m_qPTargetPixmap);
			update();
			wasPressed = false;
		}
		else
		{
			painter.drawPixmap(0, 0, *m_qPTargetPixmap);
		}
		*m_pResult = m_qPTargetPixmap->toImage();
		//*m_pResult = m_qPTargetPixmap->toImage();

	}

	void PaintWidget::mouseMoveEvent(QMouseEvent* event)
	{

		QPainter PixmapPainter(m_qPTargetPixmap);
		PixmapPainter.setPen(m_qDrawingPen);


		m_qFigure.setP2(event->pos());

		m_qCurrentPos = event->pos();
		if (m_bButtonLeft)
		{
			if (m_nDrawer == Hand)
			{
				PixmapPainter.setPen(m_qDrawingPen);
				PixmapPainter.drawLine(m_qPreviousPoint, event->pos());
				m_qPreviousPoint = event->pos();
			}
		}
		else if (m_bButtonRight)
		{
			PixmapPainter.setPen(m_qErasingPen);
			PixmapPainter.drawLine(m_qPreviousPoint, event->pos());
			m_qPreviousPoint = event->pos();
		}


		update();
	}


	void PaintWidget::initPen(QPen& pen, const QColor& in_color, int in_width, Qt::PenCapStyle in_style)
	{
		pen.setColor(in_color);
		pen.setWidth(in_width);
		pen.setCapStyle(in_style);
	}

	void PaintWidget::DrawFigure(QPainter& painter, QPoint inp1, QPoint inp2)
	{
		if (m_nDrawer == Line)
			painter.drawLine(QLine(inp1, inp2));
		else if (m_nDrawer == Rect)
			painter.drawRect(QRect(inp1, inp2));
		else if (m_nDrawer == Ellipse)
			painter.drawEllipse(QRect(inp1, inp2));

		//*m_pResult = m_qPTargetPixmap->toImage();
	}

	void PaintWidget::SetDrawer(int in_drawer)
	{
		m_nDrawer = in_drawer;
	}

	void PaintWidget::SetColor(const QColor& in_color)
	{
		m_qcolor = in_color;
		m_qDrawingPen.setColor(m_qcolor);
	}

	void PaintWidget::SetBrushSize(size_t in_size)
	{
		m_nSize = in_size;
		m_qDrawingPen.setWidth(m_nSize);
		m_qErasingPen.setWidth(m_nSize);
	}


	QPoint PaintWidget::GetCurrentBrushPos()
	{
		return m_qCurrentPos;
	}

}//namespace XRAD_GUI