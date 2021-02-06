#include "pre.h"
#include "PaintScene.h"

namespace XRAD_GUI
{

	XRAD_USING


		PaintScene::PaintScene(QObject* parent) : QGraphicsScene(parent)
	{

	}

	PaintScene::~PaintScene()
	{

	}

	void PaintScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		// ��� ������� ������ ���� ������������ ������

		if (event->button() == Qt::MouseButton::LeftButton)
		{
			ButtonLeft = true;
			ButtonRight = false;
			addEllipse(event->scenePos().x() - size / 2,
				event->scenePos().y() - size / 2,
				size,
				size,
				QPen(Qt::NoPen),
				QBrush(color));
			// ��������� ���������� ����� �������
			previousPoint = event->scenePos();
		}
		else if (event->button() == Qt::MouseButton::RightButton)
		{
			ButtonLeft = false;
			ButtonRight = true;
			addEllipse(event->scenePos().x() - size / 2,
				event->scenePos().y() - size / 2,
				size,
				size,
				QPen(Qt::NoPen),
				QBrush(Qt::white));
			// ��������� ���������� ����� �������
			previousPoint = event->scenePos();

		}

	}

	void PaintScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		// ������������ ����� � �������������� ���������� ����������
		if (ButtonLeft)
		{
			addLine(previousPoint.x(),
				previousPoint.y(),
				event->scenePos().x(),
				event->scenePos().y(),
				QPen(color, size, Qt::SolidLine, Qt::RoundCap));
			// ��������� ������ � ���������� ����������
			previousPoint = event->scenePos();
		}
		else if (ButtonRight)
		{
			addLine(previousPoint.x(),
				previousPoint.y(),
				event->scenePos().x(),
				event->scenePos().y(),
				QPen(Qt::white, size, Qt::SolidLine, Qt::RoundCap));
			// ��������� ������ � ���������� ����������
			previousPoint = event->scenePos();
		}


	}


	void PaintScene::SetBrushColor(QColor in_color)
	{
		color = in_color;
	}

	void PaintScene::SetBrushSize(size_t in_size)
	{
		size = in_size;
	}
}//namespace XRAD_GUI