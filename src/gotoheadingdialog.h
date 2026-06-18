#ifndef GOTOHEADINGDIALOG_H
#define GOTOHEADINGDIALOG_H

#include <QDialog>
#include <QList>

struct OutlineHeading;
class QLineEdit;
class QListWidget;

// Diálogo «quick open» para saltar a un encabezado: un campo de filtro sobre una
// lista de los encabezados del documento (sangrados por nivel). Se filtra por
// subcadena al teclear; las flechas arriba/abajo desde el filtro mueven la
// selección de la lista; Intro o doble clic aceptan. Tras aceptar, el bloque
// elegido se consulta con selectedBlockNumber().
class GoToHeadingDialog : public QDialog
{
    Q_OBJECT

public:
    GoToHeadingDialog(const QList<OutlineHeading> &headings, QWidget *parent = nullptr);

    // Número de bloque del encabezado seleccionado, o -1 si ninguno visible.
    int selectedBlockNumber() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QLineEdit *m_filter;
    QListWidget *m_list;
};

#endif  // GOTOHEADINGDIALOG_H
