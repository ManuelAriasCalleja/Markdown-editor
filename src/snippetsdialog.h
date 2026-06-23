#ifndef SNIPPETSDIALOG_H
#define SNIPPETSDIALOG_H

#include <QDialog>
#include <QList>

#include "snippets.h"

class QListWidget;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

// Diálogo para gestionar los snippets de usuario: lista de nombres a la izquierda
// y, a la derecha, el nombre y el cuerpo Markdown del seleccionado, más botones
// para añadir/eliminar. Trabaja sobre una copia; el llamador recoge el resultado
// con `snippets()` si el diálogo se acepta.
class SnippetsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SnippetsDialog(const QList<mdsnippet::Snippet> &initial, QWidget *parent = nullptr);

    QList<mdsnippet::Snippet> snippets() const { return m_snippets; }

private:
    void loadSelectionIntoEditors();
    void addSnippet();
    void removeSelected();
    void rebuildList();
    int currentIndex() const;

    QList<mdsnippet::Snippet> m_snippets;
    QListWidget *m_list = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QPlainTextEdit *m_bodyEdit = nullptr;
    QPushButton *m_removeButton = nullptr;
    bool m_loading = false;  // evita realimentar al rellenar los editores
};

#endif // SNIPPETSDIALOG_H
