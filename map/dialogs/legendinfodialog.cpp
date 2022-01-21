#include "legendinfodialog.h"

#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>

#include <QDebug>

LegendInfoDialog::LegendInfoDialog(RegionOfInterest* roi)
{
    makeUI();
    grabDataFromROI(roi);
}

LegendInfoDialog::~LegendInfoDialog()
{
    clear();
}

const QString &LegendInfoDialog::contentsFilename()
{
    return m_datafile;
}

const QString &LegendInfoDialog::localMapFilename()
{
    return m_localMap;
}

void LegendInfoDialog::makeUI()
{
    l_datafile   = new QLabel("Data file: ");
    l_localMap   = new QLabel("Local map: ");
    l_dataEditor = new QLabel("Data: ");

    le_datafile = new QLineEdit(m_datafile);
    le_localMap = new QLineEdit(m_localMap);

    te_contents = new TextEditor(m_contents);
    te_contents->setMinimumHeight(550);

    pb_chooseDatafile = new QPushButton("Choose legend file...");
    pb_chooseLocalMap = new QPushButton("Choose local map...");
    pb_accept         = new QPushButton("+");
    pb_decline        = new QPushButton("-");

    pb_accept->setFixedSize (BUTTON_SIZE, BUTTON_SIZE);
    pb_decline->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);

    // connect (le_datafile, SIGNAL(textChanged(const QString&)), this, SLOT(onDatafileChanged(const QString&)));
    // connect (le_localMap, SIGNAL(textChanged(const QString&)), this, SLOT(onLocalmapChanged(const QString&)));
    connect (te_contents,       SIGNAL(ctrlEnterPressed()), this, SLOT(onUpdated()));
    connect (pb_chooseDatafile, SIGNAL(clicked()), this, SLOT(onChooseDatafile()));
    connect (pb_chooseLocalMap, SIGNAL(clicked()), this, SLOT(onChooseLocalMap()));
    connect (pb_accept,         SIGNAL(clicked()), this, SLOT(onUpdate()));
    connect (pb_decline,        SIGNAL(clicked()), this, SLOT(reject()));

    m_layout = new QGridLayout (this);
    m_layout->addWidget(l_datafile,        0, 0, 1, 1);
    m_layout->addWidget(le_datafile,       0, 1, 1, 3);
    m_layout->addWidget(pb_chooseDatafile, 0, 4, 1, 1);
    m_layout->addWidget(l_localMap,        1, 0, 1, 1);
    m_layout->addWidget(le_localMap,       1, 1, 1, 3);
    m_layout->addWidget(pb_chooseLocalMap, 1, 4, 1, 1);
    m_layout->addWidget(l_dataEditor,      2, 0, 1, 1);
    m_layout->addWidget(te_contents,       3, 0, 4, 5);
    m_layout->addWidget(pb_accept,         5, 5, 1, 1);
    m_layout->addWidget(pb_decline,        6, 5, 1, 1);

    setLayout(m_layout);

    te_contents->setFocus();
}

void LegendInfoDialog::clear()
{
    l_datafile->deleteLater();
    l_localMap->deleteLater();

    le_datafile->deleteLater();
    le_localMap->deleteLater();

    te_contents->deleteLater();

    pb_chooseDatafile->deleteLater();
    pb_chooseLocalMap->deleteLater();
    pb_accept->deleteLater();
    pb_decline->deleteLater();

    m_layout->deleteLater();
}

void LegendInfoDialog::saveDataAsFile(const QString &filename)
{
    QFile file (filename);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream (&file);

        stream << m_contents;

        file.close();
    }
}

void LegendInfoDialog::loadDataFromFile(const QString &filename)
{
    QFile file (filename);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream stream (&file);

        m_contents = stream.readAll();

        file.close();
    }

    m_datafile = filename;
    le_datafile->setText(m_datafile);
    te_contents->setText(m_contents);
}

void LegendInfoDialog::onDatafileChanged(const QString &text)
{
    m_datafile = text;
}

void LegendInfoDialog::onLocalmapChanged(const QString &text)
{
    m_localMap = text;
}

void LegendInfoDialog::grabDataFromROI(RegionOfInterest* roi)
{
    if (roi->hasAttachedFile())
        loadDataFromFile(roi->attachedFile());

    qDebug() << "Grabbed Data: " << roi->localMap();

    if (roi->hasLocalMap())
        le_localMap->setText(roi->localMap());
}

void LegendInfoDialog::onChooseDatafile()
{
    // Later: make some format for text file to parse it and stylize each blocks of data differently
    QString filename = QFileDialog::getOpenFileName(nullptr, "Choose legend file for this region...", QString(), "Plain Text (*.txt)  \n Rich Text (*.html)");

    m_datafile = filename;
    loadDataFromFile(filename);
}

void LegendInfoDialog::onChooseLocalMap()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Choose local interactive map for this region...", QString(), "Interactive Map File (*.imf)");

    m_localMap = filename;
    le_localMap->setText(m_localMap);
}

void LegendInfoDialog::onUpdate()
{
    bool datafileChanged = (m_datafile != le_datafile->text());
    bool localmapChanged = (m_localMap != le_localMap->text());
    bool contentsChanged = (m_contents != te_contents->toPlainText());

    if (datafileChanged || localmapChanged || contentsChanged)
    {
        m_datafile = le_datafile->text();
        m_localMap = le_localMap->text();
        m_contents = te_contents->toPlainText();
    }

    bool noDatafile = m_datafile.isEmpty() && !m_contents.isEmpty();
    if (noDatafile)
        m_datafile = QFileDialog::getSaveFileName(nullptr, "Save data", QString(), "Data file (*.txt)");

    if (contentsChanged)
        saveDataAsFile(m_datafile);

    accept();
}
