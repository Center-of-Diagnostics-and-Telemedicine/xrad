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
	
	void SetBrushColor(QColor in_color);
	void SetBrushSize(size_t in_size);

private:
	QPointF     previousPoint;      // Координаты предыдущей точки
	bool ButtonLeft = false, ButtonRight = false;
	QColor color;
	size_t size = 10;


private:
	// Для рисования используем события мыши
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);




};

} //namespace XRAD_GUI

#endif // PAINTSCENE_H