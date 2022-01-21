#ifndef REGIONOFINTEREST_H
#define REGIONOFINTEREST_H

#include <QGraphicsPathItem>
#include <QString>

#include <QTimer>
#include <QPen>

// Leave constructor to make ROI with rubber band.
// Serialize actual bounding rect, when saving the instances.

class RegionOfInterest : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

public:
    enum class ShapeType  {RECTANGLE, ROUNDED_RECTANGLE, ELLIPSE, CIRCLE};
    enum class State {IDLE, ACTIVE};

    RegionOfInterest(QGraphicsItem* parent = nullptr);
    RegionOfInterest(const RegionOfInterest& rhs, QGraphicsItem* parent = nullptr);    
    ~RegionOfInterest();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    const State& state() const;
    const ShapeType& shapeType() const;
    const QString& localMap() const;
    const QString& attachedFile() const;
    const QString& details() const;

    bool hasAttachedFile();
    bool hasLocalMap();
    void setLocalMap (const QString& localMap);
    void setContents (const QString& source);
    void loadDataFormString (const QString& details);

    void setState (const State& state);
    void setShape (const ShapeType& type, const QRectF& bounds);

protected:
    friend QDataStream& operator<< (QDataStream&, const RegionOfInterest&);
    friend QDataStream& operator>> (QDataStream&,       RegionOfInterest&);

private:
    QString generateNameFor (const QString& fullPath);
    QPainterPath makeShapeFor (const ShapeType& path, const QRectF& bbox);

    // Shape
    ShapeType m_shapeType = ShapeType::RECTANGLE;
    QPainterPath m_shape;

    // Selection state
    State m_state = State::IDLE;
    QPen  m_pen;

    // Legend
    QString m_name;
    QString m_attachedLocalMap;
    QString m_attachedContents;
    QString m_text;

    // Animation
    void prepareAnimation();
    QTimer m_animationTimer;
    bool forward = true;

public slots:
    void onAnimationTick();
};


#endif // REGIONOFINTEREST_H
