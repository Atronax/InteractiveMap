#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

#include <QRubberBand>
#include <QMouseEvent>

class GraphicsView : public QGraphicsView
{    
    Q_OBJECT

public:
    enum class Mode {VIEW, EDITOR};
    GraphicsView(QGraphicsScene* scene = nullptr);
    ~GraphicsView();

    void mousePressEvent   (QMouseEvent *event);
    void mouseMoveEvent    (QMouseEvent *event);
    void mouseReleaseEvent (QMouseEvent *event);

    void setMode (Mode mode);

private:
    // Mode:
    Mode m_mode;

    // Regions:
    QRubberBand *m_rubberBand;
    QPoint   m_topLeftPosition;
    QPoint   m_bottomRightPosition;

signals:
    void regionSelected(const QPoint& topLeft, const QPoint& bottomRight);
};

#endif // GRAPHICSVIEW_H
