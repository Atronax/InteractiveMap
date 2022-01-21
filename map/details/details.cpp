#include "details.h"

#include <QPainter>
#include <QDebug>

Details::Details(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    defaults();
    makeUI();
    hide();

    prepareAnimation();
}

Details::~Details()
{

}

void Details::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->drawPath(m_shape);
    painter->fillPath(m_shape, QBrush(m_backgroundColor));
}

QRectF Details::boundingRect() const
{
    return m_shape.boundingRect();
}

QPainterPath Details::shape() const
{
    return m_shape;
}

void Details::setFont(const QFont& font)
{
    m_detailsText->setFont(font);
    update();
}

void Details::setBackground(const QColor& color)
{
    m_backgroundColor = color;
    update();
}

void Details::setForeground(const QColor& color)
{
    m_detailsText->setForeground(color);
    update();
}

void Details::setRegionOfInterest(RegionOfInterest *roi)
{
    m_roi = roi;

    if (m_roi)
    {
        m_detailsText->setPlainText(m_roi->details());
        m_detailsText->setToolTip(m_roi->attachedFile());
    }

    update();
}

bool Details::containsAllTheText()
{
    // If there is no region of interest, we return true, since the details is just an empty string.
    if (!m_roi)
        return true;

    int symbols_total = m_roi->details().size();
    int char_width  = QFontMetrics(m_detailsText->font()).averageCharWidth();
    int char_height = QFontMetrics(m_detailsText->font()).height();
    int symbols_per_line = m_detailsText->textWidth() / char_width; // px size

    int lines = symbols_total / symbols_per_line;
    int maxLines = m_detailsText->boundingRect().height() / char_height;

    qDebug() << "SPL:" << symbols_per_line;
    qDebug() << "Lines: " << lines;
    qDebug() << "DTBR: " << m_detailsText->boundingRect();
    qDebug() << "Can contain: " << maxLines;

    return (lines <= maxLines);
}

bool Details::textIsMoving()
{
    return m_detailsTextIsMoving;
}

void Details::setTextIsMoving(bool value)
{
    m_detailsTextIsMoving = value;
}

void Details::moveTextBy(qreal dx, qreal dy)
{
    // Do nothing, if the panel is invisible.
    if (!isVisible())
        return;

    // Move the bounding rectangle of details text by (dx, dy).
    m_detailsText->moveBy(dx, dy);

    // Check, whether the bounding rectangle of text and panel are still intersecting, and if not,
    // Move the bounding rectangle of text to some position below the bounding rectangle of panel.
    // And vise versa.
    int shift = 20;
    int textRectangleLowestY  = m_detailsText->pos().y() + m_detailsText->boundingRect().height() + shift;
    int textRectangleHighestY = m_detailsText->pos().y() - shift;
    int viewRectangleHighestY = m_shape.boundingRect().topLeft().y() - shift;
    int viewRectangleLowestY  = m_shape.boundingRect().bottomLeft().y() + shift;

    bool rectanglesAreNotIntersecting = (textRectangleLowestY  < viewRectangleHighestY) ||
                                        (textRectangleHighestY > viewRectangleLowestY);

    // When the bounding regions are not intersecting, move the {detailsText} bounding rectangle to new position,
    // Depending on the direction, in which the rectangle was moving.
    if (rectanglesAreNotIntersecting)
    {
        int detailsX = m_detailsText->pos().x();
        int detailsY = (textRectangleLowestY < viewRectangleHighestY) ? m_shape.boundingRect().bottomLeft().y() + shift :
                       (textRectangleHighestY > viewRectangleLowestY) ? m_shape.boundingRect().topLeft().y() - m_detailsText->boundingRect().height() - shift : m_detailsText->pos().y();

        m_detailsText->setPos(detailsX, detailsY);
    }
}

void Details::moveTo(qreal x, qreal y, bool updateText)
{
    setPos(x, y);
    // QPointF delta = QPointF(0.0f, m_detailsText->pos().y() - pos().y());
    if (updateText)
        m_detailsText->setPos(m_shape.boundingRect().topLeft());

    update();
}

void Details::updateContents()
{
    m_detailsText->setPlainText(m_roi->details());
    update();
}

QGraphicsButtonItem *Details::moveUpButton()
{
    return m_moveUpButton;
}

QGraphicsButtonItem *Details::moveDnButton()
{
    return m_moveDnButton;
}

void Details::makeUI()
{
    makePanel();
    makeText();
    makeButtons();
}

void Details::defaults()
{
    m_roi = nullptr;

    setFlags(QGraphicsItem::ItemDoesntPropagateOpacityToChildren |
             QGraphicsItem::ItemClipsToShape |
             QGraphicsItem::ItemClipsChildrenToShape);
}

void Details::createShape(const Details::Shape &shape)
{
    switch (shape)
    {
        case Shape::RECTANGLE:
        m_shape.addRect(QRectF(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT));
        break;

        case Shape::ROUNDED_RECTANGLE:
        m_shape.addRoundedRect(QRectF(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT), 20, 20);
        break;
    }

    m_sourceRect = QRectF(m_shape.boundingRect().x(),
                          m_shape.boundingRect().y() + 10,
                          m_shape.boundingRect().width(),
                          40);

    m_separator = QLine(QPoint(m_shape.boundingRect().x() + 20,
                               m_shape.boundingRect().y() + 55),
                        QPoint(m_shape.boundingRect().x() + m_shape.boundingRect().width() - 20,
                               m_shape.boundingRect().y() + 55));

    m_detailsRect = QRectF(m_shape.boundingRect().x(),
                           m_shape.boundingRect().y() + 40 + 20,
                           m_shape.boundingRect().width(),
                           m_shape.boundingRect().height() - 40 - 20);
}

void Details::prepareAnimation()
{
    int fps = 30;

    connect(&m_animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTick()));
    m_animationTimer.start(1000/fps);
}

void Details::onAnimationTick()
{
    // if (containsAllTheText()) return;

    // these are called once each timer tick (currently at 30 fps)
    moveTextBy(0.0f, -0.3f);

    // when button is pressed, the method is called each 1000/30 milliseconds
    if(m_moveUpButton->isPressed())
        moveTextBy(0.0f, -10.0f);

    if (m_moveDnButton->isPressed())
        moveTextBy(0.0f,  10.0f);
}

void Details::makePanel()
{
    createShape(Shape::RECTANGLE);
}

void Details::makeButtons()
{
    m_moveUpButton = new QGraphicsButtonItem("Move up", this);
    m_moveUpButton->loadImages("D:/button_up_idle.png", "D:/button_up_hovered.png");
    m_moveUpButton->setBounds(QRectF(0, 0, 40, 40));    
    m_moveUpButton->setPos(boundingRect().bottomRight().x() - 60, boundingRect().bottomRight().y() - 40*2 - 20*2);

    m_moveDnButton = new QGraphicsButtonItem("Move dn", this);
    m_moveDnButton->loadImages("D:/button_dn_idle.png", "D:/button_dn_hovered.png");
    m_moveDnButton->setBounds(QRectF(0, 0, 40, 40));
    m_moveDnButton->setPos(boundingRect().bottomRight().x() - 60, boundingRect().bottomRight().y() - 40  - 20);

    update();
}

void Details::makeText()
{
    m_detailsText = new DetailsText(m_detailsRect, this);
    m_detailsText->setPos(m_detailsRect.topLeft());
    m_detailsText->setTextWidth(m_shape.boundingRect().width());
}
