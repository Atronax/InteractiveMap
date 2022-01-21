#ifndef LEGENDINFODIALOG_H
#define LEGENDINFODIALOG_H

#include <QDialog>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

#include "../regionofinterest.h"
#include "../helpers/texteditor.h"

// 1. Global map <---> Local maps
// 2. Details text editor

class LegendInfoDialog : public QDialog
{
    Q_OBJECT

public:
    LegendInfoDialog(RegionOfInterest* roi);
    ~LegendInfoDialog();

    const QString& contentsFilename();
    const QString& localMapFilename();


private:
    void makeUI();
    void clear();

    void grabDataFromROI (RegionOfInterest* roi);
    void saveDataAsFile  (const QString& filename);
    void loadDataFromFile (const QString& filename);

    QString m_datafile;
    QString m_localMap;
    QString m_contents;

    // User interface
    const int BUTTON_SIZE = 40;
    QLabel*      l_datafile;
    QLabel*      l_localMap;
    QLabel*      l_dataEditor;
    QLineEdit*   le_datafile;
    QLineEdit*   le_localMap;
    TextEditor*  te_contents;

    QPushButton* pb_chooseDatafile;
    QPushButton* pb_chooseLocalMap;
    QPushButton* pb_update;
    QPushButton* pb_accept;
    QPushButton* pb_decline;

    QGridLayout* m_layout;

public slots:
    void onDatafileChanged(const QString&);
    void onLocalmapChanged(const QString&);
    void onChooseDatafile();
    void onChooseLocalMap();
    void onUpdate();
};

#endif // LEGENDINFODIALOG_H
