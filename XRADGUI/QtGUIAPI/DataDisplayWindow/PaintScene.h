#ifndef PAINTSCENE_H
#define PAINTSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QDebug>

class PaintScene : public QGraphicsScene
{

	Q_OBJECT

public:
	explicit PaintScene(QObject* parent = 0);
	~PaintScene();
	void SetColor(QColor in_color);

private:
	QPointF     previousPoint;      // ���������� ���������� �����
	bool ButtonLeft = false, ButtonRight = false;
	QColor color;
private:
	// ��� ��������� ���������� ������� ����
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
	
	
	

};

#endif // PAINTSCENE_H