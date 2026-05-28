#ifndef FINDREPLACEBAR_H
#define FINDREPLACEBAR_H

#include <QToolBar>

class QTextEdit;
class QLineEdit;
class QCheckBox;

// Barra inferior de buscar/reemplazar. Encapsula sus propios campos y opera
// sobre el QTextEdit que se le pasa. No conoce la ventana: reporta resultados
// por la señal statusMessage() para que quien la aloje los muestre donde quiera.
class FindReplaceBar : public QToolBar
{
    Q_OBJECT

public:
    explicit FindReplaceBar(QTextEdit *editor, QWidget *parent = nullptr);

    // Cambia el editor sobre el que actúa (p. ej. al pasar a la vista de fuente).
    void setEditor(QTextEdit *editor);

public slots:
    void showFind();      // muestra la barra y enfoca el campo de búsqueda
    void showReplace();   // como showFind() pero enfoca el campo de reemplazo

signals:
    void statusMessage(const QString &text, int timeout);

private:
    void buildUi();
    bool doFind(bool backward);
    void findNext();
    void findPrev();
    void replaceOne();
    void replaceAll();
    void closeBar();

    QTextEdit *m_editor;
    QLineEdit *m_findEdit = nullptr;
    QLineEdit *m_replaceEdit = nullptr;
    QCheckBox *m_caseCheck = nullptr;
};

#endif // FINDREPLACEBAR_H
