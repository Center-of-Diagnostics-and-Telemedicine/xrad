#include "pre.h"
#include "PaintScene.h"

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
		addEllipse(event->scenePos().x() - 5,
			event->scenePos().y() - 5,
			10,
			10,
			QPen(Qt::NoPen),
			QBrush(color));
		// ��������� ���������� ����� �������
		previousPoint = event->scenePos();
	}
	else if (event->button() == Qt::MouseButton::RightButton)
	{
		ButtonLeft = false;
		ButtonRight = true;
		addEllipse(event->scenePos().x() - 5,
			event->scenePos().y() - 5,
			10,
			10,
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
			QPen(color, 10, Qt::SolidLine, Qt::RoundCap));
		// ��������� ������ � ���������� ����������
		previousPoint = event->scenePos();
	}
	else if (ButtonRight)
	{
		addLine(previousPoint.x(),
			previousPoint.y(),
			event->scenePos().x(),
			event->scenePos().y(),
			QPen(Qt::white, 15, Qt::SolidLine, Qt::RoundCap));
		// ��������� ������ � ���������� ����������
		previousPoint = event->scenePos();
	}


}


void PaintScene::SetColor(QColor in_color)
{
	color = in_color;
}
