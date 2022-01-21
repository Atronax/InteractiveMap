#ifndef QGRAPHICSBUTTONITEM_H
#define QGRAPHICSBUTTONITEM_H

#include <QGraphicsRectItem>
#include <QBrush>

#include "spritesheet.h"

class QGraphicsButtonItem : public QGraphicsRectItem
{
public:
    enum class Shape {RECTANGLE, HEX};
    enum class State {HOVERED, PRESSED, IDLE};

    QGraphicsButtonItem(const QString& name, QGraphicsItem* parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    bool isHovered() const;
    bool isPressed() const;

    const QString& name() const;
    const QString& text() const;
    const State&  state() const;

    void setName (const QString& name);
    void setState (const State& state);
    void setImage (const QImage& image);
    void setSpritesheet (Spritesheet* spritesheet);
    void setBackgroundColor(const QColor& background);
    void setForegroundColor(const QColor& foreground);
    void setBounds(const QRectF& bounds);
    void setShape(const Shape& shape);
    void setText(const QString& text);

    void setImages (const QImage&  imageIdle, const QImage&  imageHovered);
    void loadImages(const QString& imageIdle, const QString& imageHovered);

    void moveTo(qreal x, qreal y);

private:
    void clear();
    bool checkImagesExistence (const QString& imageIdle, const QString& imageHovered);    

    QPainterPath makeShape(const Shape&  shape);
    QPainterPath makeHex  (const QRectF& bounds);

    // Basis
    QString m_name;
    State m_state;
    Shape m_shapeType;

    // Shape
    QRectF m_bounds;
    QPainterPath m_shape;

    // Text
    bool hasText = false;
    QString m_text;
    QBrush  m_backgroundColor;
    QColor  m_foregroundColor;

    // Image
    bool hasImage = false;
    QImage m_image;
    QImage m_imageIdle;
    QImage m_imageHovered;

    // Spritesheet
    bool hasSpritesheet = false;
    Spritesheet* m_spritesheet = nullptr;

};

#endif // QGRAPHICSBUTTONITEM_H
