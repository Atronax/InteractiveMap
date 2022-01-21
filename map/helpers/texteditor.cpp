#include "texteditor.h"

#include <QKeyEvent>

TextEditor::TextEditor(const QString &text, QWidget *parent)
    : QTextEdit(text, parent)
{

}

void TextEditor::keyPressEvent(QKeyEvent *event)
{
    // If we press both control and enter, the signal {ctrlEnterPressed} is emitted.
    // We can connect them to some slot to add some sort of reactions for it.
    if (event->key() == Qt::Key_Return && event->modifiers() | Qt::ControlModifier)
    {
        emit ctrlEnterPressed();
    }
    else
        QTextEdit::keyPressEvent(event);
}
