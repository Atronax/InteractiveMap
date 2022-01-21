#include "regionofinterest.h"

#include <QTextStream>
#include <QFileInfo>
#include <QFile>

#include <QPainter>
#include <QBrush>
#include <QPen>

#include <QDebug>

RegionOfInterest::RegionOfInterest(QGraphicsItem *parent)
    : QGraphicsPathItem (parent)
{
    setState(State::IDLE);
    setShape(ShapeType::RECTANGLE, QRectF(0,0,0,0));

    prepareAnimation();
}

RegionOfInterest::RegionOfInterest(const RegionOfInterest &rhs, QGraphicsItem *parent)
    : QGraphicsPathItem (parent)
{
    setState(State::IDLE);
    setShape(rhs.shapeType(), rhs.boundingRect());
    setPos(rhs.pos());
    setContents(rhs.attachedFile());
    setLocalMap(rhs.localMap());

    prepareAnimation();
}

RegionOfInterest::~RegionOfInterest()
{
}

void RegionOfInterest::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(m_pen);
    painter->drawPath(m_shape);    
}

QRectF RegionOfInterest::boundingRect() const
{
    return m_shape.boundingRect();
}

QPainterPath RegionOfInterest::shape() const
{
    return m_shape;
}

const RegionOfInterest::State &RegionOfInterest::state() const
{
    return m_state;
}

const QString &RegionOfInterest::details() const
{
    return m_text;
}

bool RegionOfInterest::hasAttachedFile()
{
    return !m_attachedContents.isEmpty();
}

bool RegionOfInterest::hasLocalMap()
{
    qDebug() << "This ROI has ALC: " << m_attachedLocalMap;

    return !m_attachedLocalMap.isEmpty();
}

void RegionOfInterest::setLocalMap(const QString &localMap)
{
    m_attachedLocalMap = localMap;

    qDebug() << "New local map has been set for " << attachedFile() << ": " << m_attachedLocalMap;
}

const QString &RegionOfInterest::attachedFile() const
{
    return m_attachedContents;
}

const RegionOfInterest::ShapeType& RegionOfInterest::shapeType() const
{
    return m_shapeType;
}

const QString &RegionOfInterest::localMap() const
{
    return m_attachedLocalMap;
}

void RegionOfInterest::setContents(const QString &filename)
{
    if (filename.isEmpty())
    {
        m_attachedContents = "";
        m_name = "";
        m_text = "";

        setToolTip("");
        return;
    }

    m_attachedContents = filename;
    m_name = generateNameFor(filename);
    setToolTip(m_name);

    QFile file (filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream (&file);
        m_text = stream.readAll();

        file.close();
    }
}

void RegionOfInterest::loadDataFormString(const QString &details)
{
    m_text = details;
}

void RegionOfInterest::setState(const RegionOfInterest::State &state)
{
    switch (state)
    {
        case State::IDLE:
        m_state = State::IDLE;
        m_pen = QPen(Qt::white);
        break;

        case State::ACTIVE:
        m_state = State::ACTIVE;
        m_pen = QPen(Qt::green, 3);
        break;
    }

    update();
}

void RegionOfInterest::setShape(const RegionOfInterest::ShapeType &type, const QRectF& bounds)
{
    m_shapeType = type;
    m_shape = makeShapeFor(m_shapeType, bounds);
}

QString RegionOfInterest::generateNameFor(const QString &fullPath)
{
    QFileInfo fi (fullPath);
    if (!fi.exists())
        return "";

    QString suffix = fi.suffix();
    QString name = fi.fileName();
    name.truncate(name.indexOf(suffix) - 1);

    return name;
}

QPainterPath RegionOfInterest::makeShapeFor(const RegionOfInterest::ShapeType &type, const QRectF &bounds)
{
    QPainterPath path;

    switch (type)
    {
        case ShapeType::RECTANGLE:
        {
            path.addRect(bounds);
        }
        break;

        case ShapeType::ROUNDED_RECTANGLE:
        {
            path.addRoundedRect(bounds, 10, 10);
        }
        break;

        case ShapeType::CIRCLE:
        {
            // the center of the circle will be the center of the rectangle
            // the radius of the circle is the minimum value of width/2 or height/2
            QPoint center = bounds.center().toPoint();
            int halfW = bounds.width() / 2;
            int halfH = bounds.height() / 2;
            int radius = (halfW < halfH) ? halfW : halfH;
            path.addEllipse(center, radius, radius);
        }
        break;

        case ShapeType::ELLIPSE:
        {
            path.addEllipse(bounds);
        }
        break;
    }

    return path;
}

void RegionOfInterest::prepareAnimation()
{
    int fps = 14;

    connect (&m_animationTimer, SIGNAL(timeout()), this, SLOT(onAnimationTick()));
    m_animationTimer.start(1000/fps);
}

void RegionOfInterest::onAnimationTick()
{
    if (m_state != State::ACTIVE)
        return;    

    if (forward)
    {
        m_pen.setWidth(m_pen.width() + 1);
        if (m_pen.width() == 7)
            forward = false;
    }
    else
    {
        m_pen.setWidth(m_pen.width() - 1);
        if (m_pen.width() == 1)
            forward = true;
    }   

    update();
    // update(QRectF(pos().x(), pos().y(), m_shape.boundingRect().width(), m_shape.boundingRect().height()));
}

// Serialization
QDataStream& operator<< (QDataStream &out, const RegionOfInterest &roi)
{
    // When we serialize objects of RegionOfInterest class, we should store:
    // 1. Its type (since it is enum, we store the corresponding number)
    // 2. Its bounding rectangle (to restore the same shape when loading)
    // 3. Load the data form the connected filename or plain text

    qDebug() << "------------";
    qDebug() << "Saving ROI: ";
    qDebug() << "ROI position: " << roi.pos();
    qDebug() << "ROI bounding rect: " << roi.boundingRect();

    out << static_cast<int>(roi.m_shapeType)
        << roi.pos()
        << roi.boundingRect()
        << roi.m_attachedContents
        << roi.m_attachedLocalMap;

    return out;
}

QDataStream& operator>> (QDataStream &in, RegionOfInterest &roi)
{
    int typeIndex;
    QPointF position;
    QRectF boundingRect;
    QString attachedContents;
    QString attachedLocalMap;

    in >> typeIndex >> position >> boundingRect >> attachedContents >> attachedLocalMap;

    qDebug() << "-----------------------------------------------------";
    qDebug() << "Loading region: ";
    qDebug() << "Region restored position: "  << position;
    qDebug() << "Region restored bounds: "    << boundingRect;
    qDebug() << "Region restored contents: "  << attachedContents;
    qDebug() << "Region restored local map: " << attachedLocalMap;

    roi.setShape(static_cast<RegionOfInterest::ShapeType>(typeIndex), boundingRect);
    roi.setPos(position);
    roi.setContents(attachedContents);
    roi.setLocalMap(attachedLocalMap);

    return in;
}
