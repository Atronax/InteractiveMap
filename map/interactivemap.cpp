#include "interactivemap.h"

#include <QGraphicsPixmapItem>

#include <QMouseEvent>
#include <QDataStream>
#include <QDateTime>
#include <QFileInfo>

#include <QDebug>

InteractiveMap::InteractiveMap(QWidget *parent)
    : QGraphicsView (parent)
{
    defaults();
    makeCanvas();
    makeDetails();
    makeButtons();
    makeRegions();
}

InteractiveMap::~InteractiveMap()
{
    clearObjects();
    clearScene();
}

void InteractiveMap::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Q && event->modifiers() & Qt::ControlModifier)
        clearObjects();

    if (event->key() == Qt::Key_S && event->modifiers() & Qt::ControlModifier)
        save();

    if (event->key() == Qt::Key_Delete)
        removeSelectedRegions();

    // When the user hits F1, the interactive map enters the view mode:
    if (event->key() == Qt::Key_F1)
    {
        setMode(InteractiveMap::Mode::VIEW);
        emit modeChanged("View mode");
    }

    // When the user hits F2, the interactive map enters the editor mode:
    if (event->key() == Qt::Key_F2)
    {
        setMode(InteractiveMap::Mode::EDITOR);
        emit modeChanged("Editor mode");
    }

    // When the user hits "+" or "-" (or mouse wheel), the view zooms in or zooms out correspondingly:
    // Details instance should have the same geometry as earlier and should move to a new position,
    // since scaled view (0.0f, 0.0f) hasn't the same scene position as earlier.
    // For scaling, we use a pair of variables : {m_scaleFactor} and {m_scaleCurrent}
    // {m_scaleFactor} used to define next step of scaling.
    // {m_scaleCurrent} represents accumulated scaling.
    bool zoomIn  = (event->key() == Qt::Key_Plus);
    bool zoomOut = (event->key() == Qt::Key_Minus);
    bool zoomDefault = (event->key() == Qt::Key_Escape);

    if (zoomIn || zoomOut || zoomDefault)
    {
        if (zoomIn)
        {
            if (m_currentScale <= 3.0f)
            {
                scale(m_scaleFactor, m_scaleFactor);
                m_currentScale *= m_scaleFactor;
            }
        }

        if (zoomOut)
        {
            if (m_currentScale >= 0.2f)
            {
                scale(1.0f/m_scaleFactor, 1.0f/m_scaleFactor);
                m_currentScale /= m_scaleFactor;
            }
        }

        // When the user hits Escape, the default scale of view is restored:
        if (zoomDefault)
        {
            scale(1.0f/m_currentScale, 1.0f/m_currentScale);
            m_currentScale = 1.0f;
        }

        // And dont forget to update the top items positions, when any of the zoom events take place
        updateDetailsPositions(true);
        updateButtons();
    }
}

void InteractiveMap::wheelEvent(QWheelEvent *event)
{
    QPoint delta = event->angleDelta();

    // Zoom in, if user moves mouse wheel forwards.
    // Zoom out, if user moves mouse wheel backwards.    
    if (delta.x() > 0)
    {
        if (m_currentScale <= 3.0f)
        {
            scale(m_scaleFactor, m_scaleFactor);
            m_currentScale *= m_scaleFactor;

            qDebug() << "Current Scale: " << m_currentScale;
        }
    }
    else
    {
        if (m_currentScale >= 1.0f)
        {
            scale(1.0f/m_scaleFactor, 1.0f/m_scaleFactor);
            m_currentScale /= m_scaleFactor;

            qDebug() << "Current Scale: " << m_currentScale;
        }
    }

    updateDetailsPositions(true);
    updateButtons();
}

void InteractiveMap::mousePressEvent(QMouseEvent *event)
{
    // Since we need our widget to act differently in case of just viewing the contents of editing them,
    // We need some mechanism to change its states. Let it be "mode" variable and some toggle button.
    switch (m_mode)
    {
        case Mode::VIEW:
        {
            if (event->button() == Qt::LeftButton)
            {
                QGraphicsItem *item = itemAt(event->localPos().toPoint());

                // Grab the item on {mouse position}.
                // Since QGraphicsItem is a base class for all its descendants,
                // just use the {dynamic_cast} to check, whether the item is of the needed type.
                QGraphicsPixmapItem *background = dynamic_cast<QGraphicsPixmapItem*>(item);
                QGraphicsButtonItem *button     = dynamic_cast<QGraphicsButtonItem*>(item);
                RegionOfInterest *region        = dynamic_cast<RegionOfInterest*>   (item);
                Details *details                = dynamic_cast<Details*>            (item);
                DetailsText *detailsText        = dynamic_cast<DetailsText*>        (item);

                bool anyDraggableObjects = region || details || detailsText;

                // If it is the button:
                // Use some method to discriminate the buttons (for example {name} or {text} or whatsoever).
                // Make some reactions on pressing the mouse button over the corresponding item.
                // In these cases we call the slot methods with the same method name as the actual button has.
                if (button)
                {
                    if (button->name() == "Add region")
                        onAddRegion();

                    if (button->name() == "Global map")
                        onGlobalMap();

                    // These should be switched to {pressed state}, when mouse pressed on top of button
                    //                         and {   idle state}, when mouse is pressed no more
                    if (button->name() == "Move up" || button->name() == "Move dn")
                        button->setState(QGraphicsButtonItem::State::PRESSED);
                }
                else if (anyDraggableObjects)
                {
                    // When we select the region, its relevant information should display on details item.
                    if (region)
                        selectRegion(region);

                    // When we select the details, do various freaky stuff with it.
                    // if (details);
                    //      doSomethingScaryWithDetails();

                    // When we select the details text, it should be marked as moving and
                    // - move up, when user moves the mouse up - move dn, when user moves the mouse dn
                    // For this the delta multiplied by some constant could be used:
                    // - delta = (new_pos.y - old_pos.y)*some_constant
                    if (detailsText && event->modifiers() & Qt::ControlModifier)
                        m_details->setTextIsMoving(true);

                    // Activate moving of the items.
                    // When we select {detailsText}, we should move its parent, not the text.
                    b_movingItem = true;
                    m_selectedItem = detailsText ? detailsText->parentItem() : item;
                    m_mouseOldPosition = event->screenPos().toPoint();
                }
                else
                {
                    m_details->hide();

                    QGraphicsView::mousePressEvent(event);
                }
            }
        }
        break;

        case Mode::EDITOR:
        {
            bool makingRegion = event->button() == Qt::LeftButton && (event->modifiers() & Qt::ControlModifier);
            bool selectRegion = event->button() == Qt::LeftButton;

            // Making regions using rubberbands.
            // When making regions of interest using rubberband, we need to calculate its bounding rectangle.
            // Two points are sufficient for this. And we'll get those, when we press and release the mouse button.

            if (makingRegion || selectRegion)
            {
                deselectAllRegions();

                if (makingRegion) b_makingRegion  = true;
                if (selectRegion) b_selectRegions = true;

                if (!m_rubberBand)
                    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

                m_topLeftPosition = event->localPos().toPoint();
                m_rubberBand->setGeometry(QRect(m_topLeftPosition, QSize()));
                m_rubberBand->show();
            }

            // Removing the regions.
            // If user clicked on an item on the scene, and it is of type {RegionOfInterest}, remove it from the list and from the scene.
            if (event->button() == Qt::RightButton)
            {
                QGraphicsItem* item = m_scene->itemAt(mapToScene(event->pos()), QTransform());
                RegionOfInterest* region = dynamic_cast<RegionOfInterest*>(item);

                if (region)
                    removeRegion(region);
            }
        }
        break;
    }
}

void InteractiveMap::mouseMoveEvent(QMouseEvent *event)
{
    switch (m_mode)
    {
        case Mode::VIEW:
        {
            // Play with this: details should update this position without the text inside, when moving the map.
            //                                                     with    the text inside, when clicking on another region.
            // Another boolean parameter will fit for this.
            if (m_selectedItem != m_details)
                updateDetailsPositions(false);

            updateButtons();

            // Do various stuff with items, when mouse moves over them in view mode.
            // Here we change the state of items, that is dependent on mouse movement (make them active or idle, hovered or default etc).
            // This can be used to change visuals or logic depending on state.
            // For example, when using atlas of spritesheets, we could set another state,
            //              change active spritesheet and use timer to activate the animation reaction on mouse hover.
            QGraphicsItem* item = m_scene->itemAt(mapToScene(event->pos()), QTransform());
            QGraphicsButtonItem* button = dynamic_cast<QGraphicsButtonItem*>(item);
            RegionOfInterest*    region = dynamic_cast<RegionOfInterest*>(item);

            // Recover the default state of buttons (if they were hovered, for example).
            defaultButtons();
            defaultRegions();

            if (button)
            {
                // There is no transition from pressed to hovered state.
                if (!button->isPressed())
                    button->setState(QGraphicsButtonItem::State::HOVERED);                
            }

            else if (region)
                region->setState(RegionOfInterest::State::ACTIVE);

            // When we are moving items, we calculate the delta between current and previous mouse position,
            // And use it to move shapes (bounding rectangles or member variable shape bounding rectangles).
            else if (b_movingItem)
            {
                QPoint mouseCurrentPosition = event->screenPos().toPoint();
                QPoint delta = mouseCurrentPosition - m_mouseOldPosition;
                m_mouseOldPosition = mouseCurrentPosition;

                if (m_details->textIsMoving())
                    m_details->moveTextBy(0.0f, 1.0f*delta.y());
                else
                    m_selectedItem->moveBy(delta.x() / m_currentScale, delta.y() / m_currentScale);

                m_scene->update();
            }
            else
                QGraphicsView::mouseMoveEvent(event);
        }
        break;

        case Mode::EDITOR:
        {
            if (b_makingRegion || b_selectRegions)
            {
                m_bottomRightPosition = event->localPos().toPoint();
                m_rubberBand->setGeometry(QRect(m_topLeftPosition, m_bottomRightPosition).normalized());
            }
        }
        break;
    }
}

void InteractiveMap::mouseReleaseEvent(QMouseEvent *event)
{
    switch (m_mode)
    {
        case Mode::VIEW:
        {
            updateButtons();

            // Reaction on mouse release, when we were moving an item.
            if (b_movingItem)
            {
                RegionOfInterest* roi = dynamic_cast<RegionOfInterest*>(m_selectedItem);
                Details*      details = dynamic_cast<Details*>         (m_selectedItem);

                if (roi)
                    ;

                if (details && m_details->textIsMoving())
                    m_details->setTextIsMoving(false);

                m_selectedItem = nullptr;
                m_mouseOldPosition = QPoint(0,0);
                b_movingItem = false;
            }
            else
                QGraphicsView::mouseReleaseEvent(event);
        }
        break;

        case Mode::EDITOR:
        {            
            // Now, that we have both points describing the region of interest, we can call the method
            // to generate the relevant path and corresponding graphics item to add it to the scene.
            if (b_makingRegion || b_selectRegions)
            {
                m_rubberBand->hide();
                m_topLeftPosition = m_rubberBand->pos();
                m_bottomRightPosition = QPoint(m_rubberBand->pos().x() + m_rubberBand->size().width(),
                                               m_rubberBand->pos().y() + m_rubberBand->size().height());

                if (b_makingRegion)
                {
                    int width = m_bottomRightPosition.x() - m_topLeftPosition.x();
                    int height = m_bottomRightPosition.y() - m_topLeftPosition.y();

                    // if the region is too small, do nothing
                    if (width < THRESHOLD && height < THRESHOLD)
                        return;

                    addRegion(m_topLeftPosition, m_bottomRightPosition);
                    b_makingRegion = false;

                    qDebug() << "View. MRE: " << event->localPos();
                }

                if (b_selectRegions)
                {
                    QList<QGraphicsItem*> selectedItems = m_scene->items(mapToScene(m_rubberBand->geometry()));
                    if (selectedItems.isEmpty())
                        deselectAllRegions();

                    findButton("Statusbar")->setText(QString("Selected: %1").arg(selectedItems.size()));

                    for (int i = 0; i < selectedItems.size(); ++i)
                    {
                        RegionOfInterest* roi = dynamic_cast<RegionOfInterest*>(selectedItems.at(i));
                        if (roi)
                            roi->setState(RegionOfInterest::State::ACTIVE);
                            // selectRegion(roi);
                    }

                    b_selectRegions = false;
                }
            }
        }
        break;
    }
}

void InteractiveMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    switch (m_mode)
    {
        case Mode::VIEW:
        {
            QGraphicsItem *item = itemAt(event->localPos().toPoint());
            RegionOfInterest* region = dynamic_cast<RegionOfInterest*>(item);

            // When user doubleclicked on some region with attached local map,
            // Loads the attached local map and store it in a history list,
            // That allows to move across the local maps.
            if (region && region->hasLocalMap())
                loadFrom(region->localMap());
        }
        break;

        case Mode::EDITOR:
        {
            QGraphicsItem *item = itemAt(event->localPos().toPoint());
            RegionOfInterest* region = dynamic_cast<RegionOfInterest*>(item);

            if (region)
            {
                selectRegion(region);

                // If user doubleclicked the region in editor mode,
                // Show the dialog, that allows to set\replace the additional data for each region.
                LegendInfoDialog dialog (region);
                if (QDialog::Accepted == dialog.exec())
                {
                    region->setContents(dialog.contentsFilename());
                    region->setLocalMap(dialog.localMapFilename());

                    m_details->updateContents();
                }
            }
            else
            {
                // create new region
                RegionOfInterest* roi = addRegion(QSize(50.0f, 50.0f));
                roi->setPos(mapToScene(event->pos().x() - roi->boundingRect().width()  / 2.0f,
                                       event->pos().y() - roi->boundingRect().height() / 2.0f));
            }
        }
        break;
    }
}

void InteractiveMap::clearScene()
{
    m_scene->deleteLater();
    this->deleteLater();
}

void InteractiveMap::clearObjects()
{
    if (m_details)
        m_details->setRegionOfInterest(nullptr);

    removeBackground();
    removeAllRegions();
}

void InteractiveMap::removeBackground()
{
    if (m_background)
    {
        m_scene->removeItem(m_background);

        delete m_background;
        m_background = nullptr;
    }
}

void InteractiveMap::removeAllRegions()
{
    while (!m_regions->isEmpty())
        removeRegion(m_regions->last());
}

void InteractiveMap::removeSelectedRegions()
{
    for (int i = 0; i < m_regions->size(); ++i)
    {
        RegionOfInterest* roi = m_regions->at(i);
        if (roi->state() == RegionOfInterest::State::ACTIVE)
            removeRegion(roi);
    }
}

void InteractiveMap::removeRegion(RegionOfInterest *region)
{
    if (region && m_regions->contains(region))
    {
        m_regions->removeOne(region);
        m_scene->removeItem(region);

        delete region;
        region = nullptr;
    }
}

void InteractiveMap::updateDetailsPositions(bool updateText)
{
    if (m_details)
    {
        int shift = 20;
        QPointF viewport = mapToScene(this->viewport()->width() - m_details->boundingRect().width() - shift, shift);
        m_details->moveTo(viewport.x(), viewport.y(), updateText);
    }
}

void InteractiveMap::updateButtons()
{
    // Since "buttons" are just the graphics items with special use, they behave just like other items.
    // When we move the viewport, they stay on the same scene positions as before, so we need to move them accordingly.
    // For this we use this method.
    // There are some nuances, that should be covered for this case:
    // 1. Conversions between scene and viewport coordinates. For this we use mapTo.. \ mapFrom.. methods of QGraphicsView class.
    // 2. Scaling the distances, when zoom in or zoom out is used. Just divide the distance by current scale factor to place the item at the correct position.

    QGraphicsButtonItem* addRegionButton = findButton("Add region");
    QGraphicsButtonItem* globalMapButton  = findButton("Global map");
    QGraphicsButtonItem* statusbarButton = findButton("Statusbar");

    if (addRegionButton && globalMapButton && statusbarButton)
    {        
        int vw = viewport()->geometry().width();
        int vh = viewport()->geometry().height();
        int arbw = addRegionButton->boundingRect().width();

        QPointF viewport = mapToScene(0.0f, 0.0f);
        int vx = viewport.x();
        int vy = viewport.y();

        addRegionButton->moveTo(vx + 10.0f          / m_currentScale, vy + 10.0f  / m_currentScale);
        globalMapButton->moveTo(vx + (20.0f + arbw) / m_currentScale, vy + 10.0f  / m_currentScale);
        statusbarButton->moveTo(vx + 20.0f          / m_currentScale, vy + 930.0f / m_currentScale);
        // statusbarButton->setBounds(QRectF((vx + 20.0f) / m_currentScale, (vy + vh - 60.0f) / m_currentScale, (vw - 20.0f) / m_currentScale, 60.0f / m_currentScale));
        // statusbarButton->update();

    }
}

void InteractiveMap::makeCanvas()
{    
    m_scene = new QGraphicsScene (0, 0, WIDTH, HEIGHT, this);    

    // View:
    setScene(m_scene);
    setBackgroundBrush(QBrush("#222"));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setInteractive(true);
    setMouseTracking(true);    
}

void InteractiveMap::makeDetails()
{
    m_details = new Details();   
    m_details->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    m_details->setZValue(2.0f);
    m_details->setBackground(QColor(0,0,0,155));
    m_details->setForeground(QColor(255,255,255));
    m_details->setFont(QFont("UKIJ Diwani Kawak", 6)); // "DS UncialFunnyHand", 9)); // ("ATLANTIDA", 7));

    m_scene->addItem(m_details);
}

void InteractiveMap::makeButtons()
{
    m_buttons = new QList<QGraphicsButtonItem*>();

    makeButton("Add region", QGraphicsButtonItem::Shape::RECTANGLE, QSizeF( 60.0f, 60.0f), QPointF( 50.0f, 10.0f));
    makeButton("Global map", QGraphicsButtonItem::Shape::RECTANGLE, QSizeF(100.0f, 60.0f), QPointF(150.0f, 10.0f));
    makeButton("Statusbar" , QGraphicsButtonItem::Shape::RECTANGLE, QSizeF(1000.0f - 20.0f*2, 60.0f), QPointF(20.0f, 1000.0f - 60.0f - 20.0f));

    // globalMapButton->setSpritesheet(new Spritesheet("D:/icebum.png", 100, 100, 30, globalMapButton));

    // Also add details buttons to list for them to be affected by InteractiveMap methods.
    m_buttons->append(m_details->moveUpButton());
    m_buttons->append(m_details->moveDnButton());
}

void InteractiveMap::makeRegions()
{
    m_regions = new QList<RegionOfInterest*>();
}

void InteractiveMap::defaults()
{
    m_regions    = nullptr;
    m_rubberBand = nullptr;
    m_background = nullptr;

    m_currentShape = RegionOfInterest::ShapeType::CIRCLE;
    m_mode = Mode::VIEW;
}

void InteractiveMap::fillWithTestData()
{
    setBackground("D:/World Map.jpg");

    m_currentShape = RegionOfInterest::ShapeType::CIRCLE;
    RegionOfInterest* roi_soul = addRegion(QPointF(0,0), QPointF(50,50));
    roi_soul->moveBy(500, 500);
    roi_soul->setContents("D:/Душа.txt"); // Венгерберг.txt");

    int count = 20;
    int s = 50;
    for (int i = 0; i < count; ++i)
    {
        int x = 50 * (rand()%(WIDTH/s));
        int y = 50 * (rand()%(HEIGHT/s));
        int t = rand()%4;
        QString file = (t == 0) ? "D:/Душа.txt" : ((t == 1) ? "D:/Тифон.txt" : ((t == 2) ? "D:/Долина.txt" : ("D:/Луч.txt")));

        RegionOfInterest* roi = addRegion(QPointF(x,y), QPointF(x+s,y+s));
        roi->setContents(file);
    }

    int spacing = 10;
    int size = 50;
    for (int x = 10; x < 1000; x += size + spacing)
        addRegion(QPointF(x, 930), QPointF(x + size, 930 + size));
}

RegionOfInterest *InteractiveMap::addRegion(RegionOfInterest *roi)
{
    roi->setZValue(1.0f);

    m_regions->append(roi);
    m_scene->addItem(roi);

    return roi;
}

RegionOfInterest *InteractiveMap::addRegion(const QSize &size)
{
    RegionOfInterest *roi = new RegionOfInterest;
    roi->setShape(m_currentShape, QRectF(0.0f, 0.0f, size.width(), size.height()));

    return addRegion(roi);
}

RegionOfInterest* InteractiveMap::addRegion(const QPointF &topLeft, const QPointF &bottomRight)
{
    RegionOfInterest *roi = new RegionOfInterest;
    roi->setShape(m_currentShape, QRectF(topLeft, bottomRight));

    return addRegion(roi);
}

void InteractiveMap::deselectAllRegions()
{
    for (int i = 0; i < m_regions->size(); ++i)
    {
        RegionOfInterest* roi = m_regions->at(i);
        if (roi->state() == RegionOfInterest::State::ACTIVE)
            roi->setState(RegionOfInterest::State::IDLE);
    }
}

void InteractiveMap::selectRegion(RegionOfInterest *region)
{
    // Default the regions state
    for (int i = 0; i < m_regions->size(); ++i)
        m_regions->at(i)->setState(RegionOfInterest::State::IDLE);

    // Select region and make it active
    m_selectedRegion = region;
    region->setState(RegionOfInterest::State::ACTIVE);

    // Update details position
    int shift = 20.0f;
    QPointF topRight = mapToScene(geometry().topRight().x() - m_details->boundingRect().width() - shift*2, shift);
    m_details->moveTo(topRight.x(), topRight.y(), true);
    m_details->setRegionOfInterest(region);
    m_details->show();
}

void InteractiveMap::defaultButtons()
{
    // Return all the buttons to idle state.
    for (int i = 0; i < m_buttons->size(); ++i)
    {
        QGraphicsButtonItem* button = m_buttons->at(i);
        button->setState(QGraphicsButtonItem::State::IDLE);
    }
}

void InteractiveMap::defaultRegions()
{
    for (int i = 0; i < m_regions->size(); ++i)
    {
        RegionOfInterest *region = m_regions->at(i);
        if (region != m_selectedRegion)
            region->setState(RegionOfInterest::State::IDLE);
    }
}

QGraphicsButtonItem *InteractiveMap::makeButton(const QString& name, const QGraphicsButtonItem::Shape& shape, const QSizeF& size, const QPointF& position)
{
    QGraphicsButtonItem *button = new QGraphicsButtonItem(name);

    button->setText(name);
    button->setFlag(QGraphicsItem::ItemIgnoresTransformations);
    button->setBounds(QRectF(0,0,size.width(),size.height()));
    button->setShape(shape);
    button->setPos(position);
    button->setZValue(2.0f);

    m_buttons->append(button);
    m_scene->addItem(button);

    return button;
}

QGraphicsButtonItem *InteractiveMap::findButton(const QGraphicsButtonItem::State &state)
{
    // Look for the button with specific state.

    // Return {nullptr}, if there are no buttons in list.
    if (m_buttons == nullptr || m_buttons->isEmpty())
        return nullptr;

    // Or a {button}, if it is found.
    for (int i = 0; i < m_buttons->size(); ++i)
    {
        QGraphicsButtonItem* button = m_buttons->at(i);
        if (button->state() == state)
            return button;
    }

    return nullptr;
}

QGraphicsButtonItem *InteractiveMap::findButton(const QString &name)
{
    // Look for the button with specific name.

    // Return {nullptr}, if there are no buttons in list.
    if (m_buttons == nullptr || m_buttons->isEmpty())
        return nullptr;

    // Or a {button}, if it is found.
    for (int i = 0; i < m_buttons->size(); ++i)
    {
        QGraphicsButtonItem* button = m_buttons->at(i);
        if (button->name() == name)
            return button;
    }

    return nullptr;
}

void InteractiveMap::setMode(const Mode& mode)
{
    m_mode = mode;
}

void InteractiveMap::setBackground(const QString &filename)
{
    // Check, whether the parameter has any data.
    // Do nothing, if it is null or empty.
    if (filename.isNull() || filename.isEmpty())
        return;

    // Check, whether the file is of image type.
    bool isImage = false;
    QFileInfo fi (filename);
    if (fi.suffix() == "jpg" || fi.suffix() == "png")
        isImage = true;

    // If it is:
    // - store the filename in member variable
    // - remove previous background;
    // - add new background;
    // - resize the scene to fit it.
    if (isImage)
    {
        removeBackground();

        m_backgroundPath = filename;
        m_background = new QGraphicsPixmapItem(QPixmap(filename));
        m_background->setZValue(-1.0f);
        m_scene->addItem(m_background);

        int border = 50, points = 50;
        m_scene->setSceneRect(m_background->boundingRect().x() - border,       m_background->boundingRect().y() - border,
                              m_background->boundingRect().width() + border*2, m_background->boundingRect().height() + border*2 + points);

        update();
    }
}

void InteractiveMap::setRegionShape(const RegionOfInterest::ShapeType &shape)
{
    m_currentShape = shape;

    // update all the regions
    for (int i = 0; i < m_regions->size(); ++i)
    {
        RegionOfInterest* roi = m_regions->at(i);
        roi->setShape(m_currentShape, roi->boundingRect());
        roi->update();
    }
}

void InteractiveMap::save()
{

    findButton("Statusbar")->setText(QString("%1: Saved map into file: %2").arg(QDateTime::currentDateTime().time().toString("hh:mm")).arg(m_currentMapFilename));
    saveAs(m_currentMapFilename);
}

void InteractiveMap::updateRegions()
{
    for (int i = 0; i < m_regions->size(); ++i)
    {
        RegionOfInterest* roi = m_regions->at(i);
        roi->setContents(roi->attachedFile());
    }

    m_details->update();
}

void InteractiveMap::saveAs(const QString &filename)
{
    QFile file (filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QDataStream stream (&file);
        stream << *this;

        file.close();
    }
}

void InteractiveMap::loadFrom(const QString &filename)
{
    QFile file (filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QDataStream stream (&file);

        // Store the filename of current map.
        m_currentMapFilename = filename;

        // Special case for global maps:
        if (QFileInfo(filename).fileName().startsWith("g_"))
            m_globalIMF.clear();

        // Push the filename of loading map into the history list.
        // It will be used to move across local maps in both directions:
        // - from global to local or from local to global.
        ++m_currentLevel;
        m_globalIMF.append(filename);

        // Clear all the objects, that are in the scene currently.
        defaultButtons();
        clearObjects();

        stream >> *this;

        file.close();
    }

    m_details->setRegionOfInterest(nullptr);
    m_details->hide();
}

// Serialization friend functions
QDataStream& operator<<(QDataStream& out, const InteractiveMap& im)
{
    out << im.m_backgroundPath;

    out << im.m_regions->size();
    for (int i = 0; i < im.m_regions->size(); ++i)
        out << *im.m_regions->at(i);

    return out;
}

QDataStream& operator>>(QDataStream& in, InteractiveMap& im)
{
    QString backgroundPath;
    int countOfRegions;

    in >> backgroundPath >> countOfRegions;

    im.clearObjects();
    im.setBackground(backgroundPath);

    RegionOfInterest currentRegion;
    for (int i = 0; i < countOfRegions; ++i)
    {
        in >> currentRegion;
        im.addRegion(new RegionOfInterest(currentRegion));
    }

    return in;
}

// SLOTS: Interaction with buttons
void InteractiveMap::onAddRegion()
{
    // We're creating new region just below the graphics item, that plays the role of the button.
    //

    QGraphicsButtonItem* makeRegionButton = findButton("Add region");

    qreal size  = 50.0f;
    qreal shift = 20.0f;
    qreal x = makeRegionButton->pos().x() + 0.0f;
    qreal y = makeRegionButton->pos().y() + makeRegionButton->boundingRect().height() + shift;

    RegionOfInterest* regionExists = dynamic_cast<RegionOfInterest*>(m_scene->itemAt(x + size/2.0f, y + size/2.0f, QTransform()));
    if (!regionExists)
    {
        RegionOfInterest* region = addRegion(QSize(size, size));
        region->setPos(x, y);
    }
}

void InteractiveMap::onGlobalMap()
{
    qDebug() << "Global map button reaction";

    if (m_globalIMF.size() > 1)
    {
        QString currentMap = m_globalIMF.takeLast();
        QString previousMap = m_globalIMF.takeLast();
        loadFrom(previousMap);
    }
}
