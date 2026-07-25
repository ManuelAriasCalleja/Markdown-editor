#ifndef GOTOHEADINGDIALOG_H
#define GOTOHEADINGDIALOG_H

/// \file
/// \brief Diálogo «quick open» para saltar a un encabezado del documento.

#include "filterlistdialog.h"

#include <QList>

struct OutlineHeading;

/// \brief Diálogo «quick open» para saltar a un encabezado: un campo de filtro sobre una
/// lista de los encabezados del documento (sangrados por nivel), filtrada por
/// subcadena. El comportamiento de teclado lo pone FilterListDialog. Tras aceptar,
/// el bloque elegido se consulta con selectedBlockNumber().
class GoToHeadingDialog : public FilterListDialog
{
    Q_OBJECT

public:
    GoToHeadingDialog(const QList<OutlineHeading> &headings, QWidget *parent = nullptr);

    /// \brief Número de bloque del encabezado seleccionado, o -1 si ninguno visible.
    int selectedBlockNumber() const;

protected:
    void filterChanged(const QString &query) override;
};

#endif  // GOTOHEADINGDIALOG_H
