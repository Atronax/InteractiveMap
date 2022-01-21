#include "QGraphicsButtonItem.h"

#include <QFileInfo>
#include <QPainter>
#include <QDebug>

QGraphicsButtonItem::QGraphicsButtonItem(const QString& name, QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setName(name);
    setRect(0,0,82,82);
    setShape(Shape::RECTANGLE);
    setBackgroundColor(Qt::white);
    setForegroundColor(Qt::black);    
}

void QGraphicsButtonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (hasSpritesheet)
    {
        painter->drawImage(boundingRect(), m_spritesheet->currentFrame());
    }

    else if (hasImage)
    {
        painter->drawImage(boundingRect(), m_image);
    }

    else if (hasText)
    {
        // background shape
        painter->drawPath(m_shape);
        painter->fillPath(m_shape, m_backgroundColor);

        painter->setPen(m_foregroundColor);
        painter->drawText(boundingRect(), m_text, QTextOption(Qt::AlignCenter));
    }
}

QRectF QGraphicsButtonItem::boundingRect() const
{
    return m_shape.boundingRect();
}

QPainterPath QGraphicsButtonItem::shape() const
{
    return m_shape;
}

void QGraphicsButtonItem::setName(const QString &name)
{
    m_name = name;
}

void QGraphicsButtonItem::setState(const QGraphicsButtonItem::State &state)
{
    if (m_state == state)
        return;

    switch(state)
    {
        case State::IDLE:
        qDebug() << "Idle state";
        m_state = State::IDLE;

        if (hasImage)
        {
            qDebug() << "changed image to idle";
            setImage(m_imageIdle);
        }

        if (hasText)
        {
            setBackgroundColor(Qt::white);
            setForegroundColor(Qt::black);
        }
        break;

        case State::PRESSED:
        m_state = State::PRESSED;
        break;

        case State::HOVERED:
        qDebug() << "Hovered state";
        m_state = State::HOVERED;

        if (hasImage)
        {
            qDebug() << "change image to hovered";
            setImage(m_imageHovered);
        }

        if (hasText)
        {
            setBackgroundColor(Qt::black);
            setForegroundColor(Qt::white);
        }
        break;
    }

    update();
}

void QGraphicsButtonItem::setText(const QString &text)
{
    hasText = true;
    m_text = text;

    update();
}

void QGraphicsButtonItem::setImage(const QImage &image)
{
    m_image = image.scaled(boundingRect().size().toSize());
    qDebug() << "S: " << m_image.size();

    hasImage = true;

    setState(State::IDLE);
}

void QGraphicsButtonItem::setSpritesheet(Spritesheet *spritesheet)
{
    m_spritesheet = spritesheet;

    hasSpritesheet = true;

    setState (State::IDLE);
}

void QGraphicsButtonItem::setImages(const QImage &imageIdle, const QImage &imageHovered)
{
    m_imageIdle = imageIdle;
    m_imageHovered = imageHovered;

    hasImage = true;
}

void QGraphicsButtonItem::loadImages(const QString &imageIdle, const QString &imageHovered)
{
    bool bothImagesExist = checkImagesExistence (imageIdle, imageHovered);
    if (bothImagesExist)
    {
        qDebug() << "Both images exists";
        m_imageIdle    = QImage(imageIdle);
        m_imageHovered = QImage(imageHovered);

        hasImage = true;
    }

    setState(State::IDLE);
    update();
}

void QGraphicsButtonItem::moveTo(qreal x, qreal y)
{
    setPos(x, y);
    update();
}

void QGraphicsButtonItem::clear()
{
    if (m_spritesheet)
        m_spritesheet->deleteLater();
}

const QString &QGraphicsButtonItem::name() const
{
    return m_name;
}

bool QGraphicsButtonItem::checkImagesExistence(const QString &imageIdle, const QString &imageHovered)
{
    QFileInfo infoIdle (imageIdle);
    QFileInfo infoHovered (imageHovered);

    bool imageIdleExists = infoIdle.exists() && (infoIdle.suffix() == "jpg" || infoIdle.suffix() == "png");
    bool imageHoveredExists = infoHovered.exists() && (infoHovered.suffix() == "jpg" || infoHovered.suffix() == "png");

    if (imageIdleExists && imageHoveredExists)
        return true;
    else
        return false;
}

void QGraphicsButtonItem::setShape(const QGraphicsButtonItem::Shape &shape)
{
    m_shapeType = shape;
    m_shape = makeShape(shape);
}

void QGraphicsButtonItem::setBackgroundColor(const QColor &background)
{
    m_backgroundColor = QBrush(background);
}

void QGraphicsButtonItem::setForegroundColor(const QColor &foreground)
{
    m_foregroundColor = foreground;
}

void QGraphicsButtonItem::setBounds(const QRectF &bounds)
{
    m_bounds = bounds;
    m_shape = makeShape(m_shapeType);
}

const QString &QGraphicsButtonItem::text() const
{
    return m_text;
}

const QGraphicsButtonItem::State &QGraphicsButtonItem::state() const
{
    return m_state;
}

bool QGraphicsButtonItem::isHovered() const
{
    return (m_state == State::HOVERED);
}

bool QGraphicsButtonItem::isPressed() const
{
    return (m_state == State::PRESSED);
}

QPainterPath QGraphicsButtonItem::makeShape(const QGraphicsButtonItem::Shape &shape)
{
    QPainterPath path;

    switch(shape)
    {
        case Shape::RECTANGLE:
            path.addRect(m_bounds);
        break;

        case Shape::HEX:
            path = makeHex(m_bounds);
        break;
    }

    return path;
}

QPainterPath QGraphicsButtonItem::makeHex(const QRectF &bounds)
{
    int x = bounds.x();
    int y = bounds.y();
    int w = bounds.width();
    int h = bounds.height();

    // Hex can be represented by 6 points:
    // 0. move to first point
    // 1. pt1 (x      , y + h/4  )
    // 2. pt2 (x + w/2, y        )
    // 3. pt3 (x + w  , y + h/4  )
    // 4. pt4 (x + w  , y + 3*h/4)
    // 5. pt5 (x + w/2, y + h    )
    // 6. pt6 (x      , y + 3h/4 )

    // So, lets build it
    QPainterPath path;
    path.moveTo(x, y + h/4);
    path.lineTo(x+w/2, y);
    path.lineTo(x+w, y+h/4);
    path.lineTo(x+w, y+3*h/4);
    path.lineTo(x+w/2,y+h);
    path.lineTo(x, y+3*h/4);

    return path;
}
