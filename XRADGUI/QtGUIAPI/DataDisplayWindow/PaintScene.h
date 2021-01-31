#ifndef PAINTSCENE_H
#define PAINTSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QDebug>


namespace XRAD_GUI
{

XRAD_USING

class PaintScene : public QGraphicsScene
{
	Q_OBJECT

public:
	explicit PaintScene(QObject* parent = 0);
	~PaintScene();
	void SetColor(QColor in_color);

private:
	QPointF     previousPoint;      // Координаты предыдущей точки
	bool ButtonLeft = false, ButtonRight = false;
	QColor color;
private:
	// Для рисования используем события мыши
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);




};

} //namespace XRAD_GUI

#endif // PAINTSCENE_H