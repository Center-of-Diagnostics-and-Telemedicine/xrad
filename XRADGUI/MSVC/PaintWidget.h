#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QLine>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QWidget>

#include "pre.h"

namespace XRAD_GUI {

XRAD_USING

class PaintWidget : public QWidget {
    Q_OBJECT

public:
    explicit PaintWidget(QWidget* parent,
        size_t in_vsize,
        size_t in_hsize,
        std::shared_ptr<QImage> in_result);
    ~PaintWidget();

    void SetDrawer(int);
    void SetColor(const QColor&);
    void SetBrushSize(size_t);

    QPoint GetCurrentBrushPos();

    enum Drawers {
        Hand,
        Line,
        Rect,
        Ellipse
    };

protected:
    void mouseMoveEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);

private:
    void initPen(QPen&, const QColor&, int, Qt::PenCapStyle);
    void DrawFigure(QPainter&, QPoint, QPoint);

private:
    QPixmap* m_qPTargetPixmap;

    QLine m_qFigure;
    QColor m_qcolor = Qt::black;
    QPen m_qDrawingPen, m_qErasingPen;
    QPoint m_qCurrentPos, m_qPreviousPoint;

    std::shared_ptr<QImage> m_pResult;

    size_t m_nSize = 10;
    size_t m_nHsize, m_nVsize;

    int m_nDrawer;

    bool m_bButtonLeft = false, m_bButtonRight = false;
};

} // namespace XRAD_GUI
#endif // PAINTWIDGET_H
