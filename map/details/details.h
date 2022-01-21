#ifndef DETAILS_H
#define DETAILS_H

#include <QGraphicsRectItem>

#include <QFont>
#include <QBrush>

#include "../regionofinterest.h"
#include "../helpers/qgraphicsbuttonitem.h"
#include "detailstext.h"

// Details class represents rectangle item, that is used to draw the legend data from the file. It should:
// 1. Have the ability to show \ hide
// 2.                  to store the text
// 3.                  to replace the text

class Details : public QObject, public QGraphicsRectItem
{
    Q_OBJECT

public:
    enum class Shape {RECTANGLE, ROUNDED_RECTANGLE};
    enum class Button {MOVE_UP, MOVE_DN};
    Details(QGraphicsItem* parent = nullptr);
    ~Details();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    // Region specifics
    void setFont  (const QFont& font);
    void setBackground (const QColor& color);
    void setForeground (const QColor& color);
    void setRegionOfInterest (RegionOfInterest* roi);

    // Details animation    
    bool containsAllTheText();
    bool textIsMoving();
    void setTextIsMoving(bool value);
    void moveTextBy(qreal dx, qreal dy);
    void moveTo(qreal x, qreal y, bool updateText);

    // Contents update
    void updateContents();

    // Buttons
    QGraphicsButtonItem* moveUpButton();
    QGraphicsButtonItem* moveDnButton();

private:    
    void makeUI();
    void makePanel();
    void makeButtons();
    void makeText();
    void defaults();

    void createShape(const Shape& shape);

    // Attached region
    RegionOfInterest* m_roi;

    // Shape
    const int DEFAULT_WIDTH = 400;
    const int DEFAULT_HEIGHT = 400;
    QPainterPath m_shape;
    QRectF m_sourceRect;
    QRectF m_detailsRect;
    QLineF m_separator;

    // Contents
    DetailsText* m_detailsText;
    QGraphicsButtonItem* m_moveUpButton;
    QGraphicsButtonItem* m_moveDnButton;
    bool m_detailsTextIsMoving = false;

    // Background and Foreground
    QColor m_backgroundColor = "#bbb";

    // Animation
    void prepareAnimation();
    QTimer m_animationTimer;

public slots:
    void onAnimationTick();
};

#endif // DETAILS_H
