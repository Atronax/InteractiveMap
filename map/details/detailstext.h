#ifndef DETAILSTEXT_H
#define DETAILSTEXT_H

#include <QGraphicsTextItem>

class DetailsText : public QGraphicsTextItem
{
public:
    DetailsText(const QRectF& bounds, QGraphicsItem* parent = nullptr);
    ~DetailsText();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setForeground(const QColor& color);

private:
    QRectF m_bounds;

    QColor m_foregroundColor;
};

#endif // DETAILSTEXT_H
