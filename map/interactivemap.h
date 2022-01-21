#ifndef INTERACTIVEMAP_H
#define INTERACTIVEMAP_H

#include <QWidget>

#include <QGraphicsScene>
#include <QGraphicsView>

#include <QRubberBand>
#include <QPixmap>
#include <QList>

#include <QTimer>

#include "regionofinterest.h"
#include "dialogs/legendinfodialog.h"
#include "helpers/qgraphicsbuttonitem.h"
#include "details/details.h"

// 1. We could collapse both images and draw them on widget using paint event or something like that,
//    but for simplicity reasons the canvas will be used. Besides, this would allow to easily make
//    interactive legend windows without frames, move the items using mouse or move them between layers.
// 2. Regions of interest are some sort of shapes, that have
//    - visual representation (shapes are drawed on top of image using instances of *ShapeItem class).
//    - logic
//    1. When user clicks LMB inside the shape, the details item should load new data from connected file of legend details.
//    2. When user clicks RMB inside the shape, the window to set\replace connected file with legend details should open.

class InteractiveMap : public QGraphicsView
{
    Q_OBJECT

public:
    enum class Mode {VIEW, EDITOR};
    enum class ImageType {BACKGROUND, FOREGROUND};    

    InteractiveMap (QWidget *parent = nullptr);
    ~InteractiveMap ();

    void setMode (const Mode& mode);

    // Key and mouse event handlers
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;    

    void setBackground (const QString& filename);
    void setRegionShape (const RegionOfInterest::ShapeType& pathType);

    // Serialization methods
    void save();
    void saveAs   (const QString& filename);
    void loadFrom (const QString& filename);

    // Helper methods
    void fillWithTestData();
    void updateRegions();


protected:
    friend QDataStream& operator<< (QDataStream&, const InteractiveMap&);
    friend QDataStream& operator>> (QDataStream&,       InteractiveMap&);

private:
    // Constructor methods
    void makeUI();    
    void makeCanvas();
    void makeDetails();
    void makeButtons();
    void makeRegions();
    void defaults();

    // Cleaning methods
    void clearScene();
    void clearObjects();
    void removeBackground();
    void removeAllRegions();
    void removeSelectedRegions();
    void removeRegion (RegionOfInterest* region);

    // Default methods
    void defaultButtons();
    void defaultRegions();

    // Update methods
    void updateDetailsPositions(bool updateText);
    void updateButtons();

    // Mode:
    Mode m_mode;

    // Canvas:
    const int WIDTH = 1000;
    const int HEIGHT = 1000;
    QGraphicsScene *m_scene;

    // Background and foreground:
    // - background image used a map
    // - generated  image used to represent roi, that could be collapsed with background and saved for other purposes
    // - filename for background map, used to store the IMF file
    // collapse the regions with background and store it as a separate pixmap
    // QGraphicsPixmapItem *m_foreground;
    QGraphicsPixmapItem *m_background;
    QString              m_backgroundPath;

    // Regions of interest:
    // - methods to add\remove roi
    // - methods to change\remove connected legend file
    RegionOfInterest* addRegion (RegionOfInterest* rhs);
    RegionOfInterest* addRegion (const QSize& size);
    RegionOfInterest* addRegion (const QPointF& topLeft, const QPointF& bottomRight);
    void deselectAllRegions ();
    void selectRegion (RegionOfInterest* region);
    QList<RegionOfInterest*> *m_regions;
    RegionOfInterest* m_selectedRegion;

    // Buttons:
    // These are used to generate standard regions, when are clicked. And draw some statistics on used objects.    
    QGraphicsButtonItem* makeButton (const QString& name, const QGraphicsButtonItem::Shape& shape, const QSizeF& size, const QPointF& position);
    QGraphicsButtonItem* findButton (const QGraphicsButtonItem::State& state);
    QGraphicsButtonItem* findButton (const QString& name);
    QList<QGraphicsButtonItem*> *m_buttons;

    // Details:
    Details *m_details;

    // Operating the items on scene:
    QGraphicsItem* m_selectedItem;
    bool b_movingItem = false;
    QPoint m_mouseOldPosition;

    // Global-local maps
    int m_currentLevel = 0;
    QStringList m_globalIMF;

    // Editor:
    RegionOfInterest::ShapeType m_currentShape;

    // Generate regions using rubberband
    QRubberBand *m_rubberBand;
    QPoint   m_topLeftPosition;
    QPoint   m_bottomRightPosition;
    const int THRESHOLD = 100;
    bool b_makingRegion = false;
    bool b_selectRegions = false;

    // Scale interactive map using mouse wheel or plus\minus
    float m_currentScale = 1.0f;
    float m_scaleFactor = 1.2f;

    // Saving, loading
    QString m_currentMapFilename;

signals:
    void modeChanged (const QString& mode);
    void resizeDesktop (int width, int height);

public slots:
    void onAddRegion();
    void onGlobalMap();
};

#endif // INTERACTIVEMAP_H
