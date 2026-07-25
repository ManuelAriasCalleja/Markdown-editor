#ifndef FILTERLISTDIALOG_H
#define FILTERLISTDIALOG_H

/// \file
/// \brief Base de los diálogos «quick open»: un campo de filtro sobre una lista.

#include <QDialog>

class QEvent;
class QLineEdit;
class QListWidget;

/// \brief Armazón común de los diálogos de filtro rápido (la paleta de comandos y el
/// salto a encabezado): un `QLineEdit` sobre un `QListWidget`, con el
/// comportamiento de teclado que se espera de un «quick open»:
///
///   - las flechas y AvPág/RePág mueven la selección **sin sacar el foco del
///     filtro**, para poder seguir escribiendo mientras se navega;
///   - Intro en el filtro, o activar/doble clic en la lista, aceptan el diálogo.
///
/// Lo que cambia en cada uno —los rótulos, el tamaño y qué se muestra al filtrar—
/// lo pone la clase derivada: llena la lista en su constructor y responde a
/// `filterChanged`. Los textos se traducen en la derivada, así que cada diálogo
/// conserva su contexto de traducción.
class FilterListDialog : public QDialog
{
    Q_OBJECT

public:
    /// `title` rotula la ventana, `filterPlaceholder` el campo (y le sirve de nombre
    /// accesible, porque el placeholder no lo lee un lector de pantalla) y
    /// `listName` es el nombre accesible de la lista.
    FilterListDialog(const QString &title, const QString &filterPlaceholder,
                     const QString &listName, QWidget *parent = nullptr);

protected:
    QLineEdit *filterEdit() const { return m_filter; }
    QListWidget *list() const { return m_list; }

    /// Se llama al teclear en el filtro. La derivada decide qué mostrar.
    virtual void filterChanged(const QString &query) = 0;

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QLineEdit *m_filter;
    QListWidget *m_list;
};

#endif  // FILTERLISTDIALOG_H
