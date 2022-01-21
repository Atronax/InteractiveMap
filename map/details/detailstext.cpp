#include "detailstext.h"

#include <QPainter>
#include <QDebug>

DetailsText::DetailsText(const QRectF& bounds, QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    m_bounds = bounds;

}

DetailsText::~DetailsText()
{

}

void DetailsText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setFont(font());
    painter->setPen(m_foregroundColor);

    painter->drawText(boundingRect(), toPlainText(), QTextOption(Qt::AlignHCenter));
}

void DetailsText::setForeground(const QColor &color)
{
    m_foregroundColor = color;
}
