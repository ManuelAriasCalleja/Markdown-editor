#ifndef TABLECONTROLLER_H
#define TABLECONTROLLER_H

/// \file
/// \brief Edición contextual de la tabla bajo el cursor (filas, columnas, alineación) y estado de sus acciones.

#include <QList>
#include <QObject>
#include <Qt>

class QAction;
class QTextEdit;
class DocumentIo;
class SplitViewController;

/// Operaciones sobre la tabla bajo el cursor del editor WYSIWYG: insertar/eliminar
/// filas y columnas y alinear una columna. También habilita/inhabilita las
/// acciones del menú Tabla según si el cursor está dentro de una tabla.
///
/// Trabaja con la QTextTable del cursor (no posee estado del documento); recibe el
/// editor, el SplitViewController (para saber si el panel activo es WYSIWYG) y el
/// DocumentIo (para la marca «modificado»). Avisa de cambios de «modificado» por
/// señal.
class TableController : public QObject
{
    Q_OBJECT

public:
    TableController(QTextEdit *editor, SplitViewController *split,
                    DocumentIo *documentIo, QObject *parent);

    /// Acciones contextuales del menú Tabla, que updateActions habilita solo cuando
    /// el cursor está dentro de una tabla.
    void setActions(const QList<QAction *> &actions) { m_actions = actions; }

    /// Habilita/inhabilita las acciones según si hay una tabla bajo el cursor en el
    /// editor WYSIWYG activo.
    void updateActions();

public slots:
    void insertRow(bool below);              // inserta una fila encima/debajo
    void insertColumn(bool right);           // inserta una columna a izda./dcha.
    void deleteRow();                        // elimina la fila actual
    void deleteColumn();                     // elimina la columna actual
    void alignColumn(Qt::Alignment alignment);  // alinea la columna actual

signals:
    /// El contenido difiere (o no) del guardado tras editar la tabla.
    void modifiedChanged(bool modified);

private:
    QTextEdit *m_editor = nullptr;
    SplitViewController *m_split = nullptr;
    DocumentIo *m_documentIo = nullptr;
    QList<QAction *> m_actions;
};

#endif // TABLECONTROLLER_H
