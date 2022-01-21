#ifndef DESKTOP_H
#define DESKTOP_H

// Interactive map should have:
// 1. methods to show map as background \ overlapping transparent image with regions of interest
// 2. methods to add\remove regions of interest
// 3. methods to add\remove\replace information (filename) for existing region of interest
// 4. methods to react for clicking on these regions of interest
// 5. methods to show set information (loaded from file) on region of interest as a frameless window or whatsoever
// 6. methods to save\restore interactive map (operators <<, >> and datastreams + wrappers)

#include "map/interactivemap.h"

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>

class Desktop : public QWidget
{
    Q_OBJECT

public:
    Desktop(QWidget *parent = nullptr);
    ~Desktop();

    void saveAs   (const QString& filename);
    void loadFrom (const QString& filename);

private:
    // Init:
    void init();
    void makeUI();
    void clear();

    // Path type selector
    void fillRegionTypes();

    // User Interface:
    InteractiveMap *m_map;    
    QPushButton *pb_saveMap;  // save interactive map
    QPushButton *pb_loadMap;  // load interactive map
    QPushButton *pb_placeMap; // place background map
    QPushButton *pb_modeIndicator; // toggle button to start\end the editing
    QPushButton *pb_updateRegions; // helper button to update region data without need to restart the app
    QPushButton *pb_fillWithTestData; // helper button to fill the scene with test data
    QComboBox   *cb_regionTypeSelector; // selector for path type, that are used to draw regions of interest
    QGridLayout *m_layout;

public slots:
    void onSaveMap();
    void onLoadMap();
    void onPlaceMap();
    void onActivated(int);

    void onModeChanged (const QString& mode);
    void onResizeDesktop(int width, int height);
    void onUpdateRegions();
    void onFill();

};
#endif // DESKTOP_H
