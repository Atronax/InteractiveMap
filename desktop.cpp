#include "desktop.h"

#include <QGridLayout>
#include <QFileDialog>

#include <QDebug>

Desktop::Desktop(QWidget *parent)
    : QWidget(parent)
{
    init();
    makeUI();
}

Desktop::~Desktop()
{
    clear();
}

void Desktop::init()
{
    m_map = new InteractiveMap(this);
}

void Desktop::makeUI()
{
    pb_placeMap = new QPushButton("Place background map...");
    pb_saveMap = new QPushButton("Save map...");
    pb_loadMap = new QPushButton("Load map...");
    pb_modeIndicator = new QPushButton("View mode");
    pb_updateRegions = new QPushButton("Update");
    pb_fillWithTestData = new QPushButton("Fill");

    pb_modeIndicator->setCheckable(true);

    cb_regionTypeSelector = new QComboBox();
    fillRegionTypes();
    cb_regionTypeSelector->setCurrentIndex(0);

    connect(m_map, SIGNAL(modeChanged(const QString&)), this, SLOT(onModeChanged(const QString&)));
    connect(pb_placeMap, SIGNAL(clicked()), this, SLOT(onPlaceMap()));
    connect(pb_saveMap,  SIGNAL(clicked()), this, SLOT(onSaveMap()));
    connect(pb_loadMap,  SIGNAL(clicked()), this, SLOT(onLoadMap()));
    connect(cb_regionTypeSelector, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
    connect(pb_updateRegions, SIGNAL(clicked()), this, SLOT(onUpdateRegions()));
    connect(pb_fillWithTestData, SIGNAL(clicked()), this, SLOT(onFill()));
    connect(m_map, SIGNAL(resizeDesktop(int, int)), this, SLOT(onResizeDesktop(int, int)));

    m_layout = new QGridLayout (this);
    m_layout->addWidget(m_map,         0, 0, 4, 4);
    m_layout->addWidget(pb_placeMap,   4, 0, 1, 1);
    m_layout->addWidget(pb_modeIndicator, 4, 1, 1, 1);
    m_layout->addWidget(pb_saveMap,    4, 2, 1, 1);
    m_layout->addWidget(pb_loadMap,    4, 3, 1, 1);
    m_layout->addWidget(cb_regionTypeSelector, 5, 3, 1, 1);
    m_layout->addWidget(pb_updateRegions, 5, 1, 1, 1);
    m_layout->addWidget(pb_fillWithTestData, 5, 0, 1, 1);

    setLayout(m_layout);
    // setFixedSize(m_map.width(), m_map.height());
    setFocus();
}

void Desktop::clear()
{
    m_map->deleteLater();

    pb_saveMap->deleteLater();
    pb_loadMap->deleteLater();
    pb_modeIndicator->deleteLater();
    pb_updateRegions->deleteLater();
    pb_fillWithTestData->deleteLater();
    cb_regionTypeSelector->deleteLater();
    m_layout->deleteLater();
}

void Desktop::fillRegionTypes()
{
    cb_regionTypeSelector->addItem("Rectangle");
    cb_regionTypeSelector->addItem("Rounded Rectangle");
    cb_regionTypeSelector->addItem("Ellipse");
    cb_regionTypeSelector->addItem("Circle");

    cb_regionTypeSelector->setCurrentIndex(0);
    cb_regionTypeSelector->activated(0);
}

// Buttons reactions
void Desktop::onSaveMap()
{
    QString filename = QFileDialog::getSaveFileName(nullptr, "Save interactive map as...",
                                                    QString(), "Interactive Map Format (*.imf)");

    saveAs(filename);
}

void Desktop::onLoadMap()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Load interactive map from...",
                                                    QString(), "Interactive Map Format (*.imf)");

    loadFrom(filename);
}

void Desktop::onPlaceMap()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Take an image to use a background map",
                                                    QString(), "Images (*.jpg *.png)");

    m_map->setBackground(filename);
}

void Desktop::onActivated(int index)
{
    // QString activatedShape = cb_regionTypeSelector->itemText(index);
    m_map->setRegionShape(static_cast<RegionOfInterest::ShapeType>(index));
}

void Desktop::onModeChanged(const QString &mode)
{
    pb_modeIndicator->setText(mode);
}

void Desktop::onResizeDesktop(int width, int height)
{
    // resize(m_map->width(), m_map->height());
    int border = 20;
    int buttons = 60;
    setFixedSize(width + border, height + border + buttons);
}

void Desktop::onUpdateRegions()
{
    m_map->updateRegions();
}

void Desktop::onFill()
{
    m_map->fillWithTestData();
}

// Methods to save\restore made maps
void Desktop::saveAs(const QString &filename)
{
    m_map->saveAs(filename);

}

void Desktop::loadFrom(const QString &filename)
{
    m_map->loadFrom(filename);
}
