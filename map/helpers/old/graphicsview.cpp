#include "graphicsview.h"

#include <QDebug>

GraphicsView::GraphicsView(QGraphicsScene *scene)
    : QGraphicsView (scene)
{
    m_rubberBand = nullptr;
}

GraphicsView::~GraphicsView()
{
    m_rubberBand->deleteLater();
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "in GVMP";

    switch (m_mode)
    {
        case Mode::VIEW:
        break;

        case Mode::EDITOR:
        {
            if (event->button() == Qt::LeftButton)
            {
                m_topLeftPosition = event->localPos().toPoint();

                if (!m_rubberBand)
                    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

                m_rubberBand->setGeometry(QRect(m_topLeftPosition, QSize()));
                m_rubberBand->show();

                qDebug() << "Top left: " << m_topLeftPosition;
                qDebug() << "View. MPE: " << event->localPos();
            }

            if (event->button() == Qt::RightButton)
            {

            }
        }
        break;
    }
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_mode)
    {
        case Mode::VIEW:
        break;

        case Mode::EDITOR:
        {
            m_bottomRightPosition = event->localPos().toPoint();
            qDebug() << "BB: " << m_topLeftPosition << " - " << m_bottomRightPosition;

            m_rubberBand->setGeometry(QRect(m_topLeftPosition, m_bottomRightPosition).normalized());

            qDebug() << "View. MME: " << event->localPos();
        }
        break;
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    switch (m_mode)
    {
        case Mode::VIEW:
        break;

        case Mode::EDITOR:
        {
            m_rubberBand->hide();
            m_topLeftPosition = m_rubberBand->pos();
            m_bottomRightPosition = QPoint(m_rubberBand->pos().x() + m_rubberBand->size().width(),
                                           m_rubberBand->pos().y() + m_rubberBand->size().height());

            emit regionSelected(m_topLeftPosition, m_bottomRightPosition);

            qDebug() << "View. MRE: " << event->localPos();
        }
        break;
    }
}

void GraphicsView::setMode(GraphicsView::Mode mode)
{
    m_mode = mode;
}
