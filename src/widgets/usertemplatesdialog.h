#ifndef USERTEMPLATESDIALOG_H
#define USERTEMPLATESDIALOG_H

/// \file
/// \brief Diálogo de gestión de las plantillas de usuario (y «Guardar como plantilla…»).

#include <QDialog>
#include <QList>

#include "usertemplate.h"

class QListWidget;
class QLineEdit;
class QComboBox;
class QPlainTextEdit;
class QPushButton;

/// \brief Diálogo para gestionar las plantillas de usuario: lista a la izquierda y, a
/// la derecha, el nombre, la categoría y el cuerpo Markdown de la seleccionada, con
/// botones para añadir/eliminar. Trabaja sobre una copia; el llamador recoge el
/// resultado con `templates()` si el diálogo se acepta. Para «Guardar como
/// plantilla…», el llamador siembra una plantilla nueva con `startNewFromBody()`.
class UserTemplatesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserTemplatesDialog(const QList<mdusertemplate::UserTemplate> &initial,
                                 QWidget *parent = nullptr);

    /// \brief Lista de plantillas editada; léela tras aceptar el diálogo.
    QList<mdusertemplate::UserTemplate> templates() const { return m_templates; }

    /// \brief Añade una plantilla nueva con `body` (el documento actual), la
    /// selecciona y enfoca el nombre. Para el flujo «Guardar como plantilla…».
    void startNewFromBody(const QString &body);

private:
    void loadSelectionIntoEditors();
    void addTemplate();
    void removeSelected();
    void rebuildList();
    int currentIndex() const;
    QString labelFor(const mdusertemplate::UserTemplate &t) const;

    QList<mdusertemplate::UserTemplate> m_templates;
    QListWidget *m_list = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_categoryCombo = nullptr;
    QPlainTextEdit *m_bodyEdit = nullptr;
    QPushButton *m_removeButton = nullptr;
    bool m_loading = false;  // evita realimentar al rellenar los editores
};

#endif  // USERTEMPLATESDIALOG_H
