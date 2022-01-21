#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QTextEdit>

class TextEditor : public QTextEdit
{
    Q_OBJECT

public:
    TextEditor(const QString& text, QWidget* parent = nullptr);

    void keyPressEvent(QKeyEvent *e) override;

signals:
    void ctrlEnterPressed();
};

#endif // TEXTEDITOR_H
