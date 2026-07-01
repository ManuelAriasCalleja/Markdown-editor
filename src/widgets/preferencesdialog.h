#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

/// \file
/// \brief Ventana de Preferencias: reúne en pestañas ajustes que ya existen en el
///        menú Ver, para descubrirlos de un vistazo.

#include <QDialog>
#include <QList>

class QAction;
class QCheckBox;
class QComboBox;

/// Diálogo de Preferencias. No persiste nada por su cuenta: refleja el estado de las
/// acciones de ajuste que le pasa MainWindow y, al cambiarlas, las **dispara**, de
/// modo que se aplica y se guarda por la misma vía que el menú (sin duplicar lógica).
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    /// Acciones de ajuste que gestiona el diálogo. Las listas son grupos exclusivos
    /// (se muestran como desplegable); el resto, casillas.
    struct Settings {
        QList<QAction *> themeActions;        ///< temas (exclusivos)
        QAction *followSystem = nullptr;      ///< seguir el tema del sistema
        QAction *warmLight = nullptr;         ///< luz cálida nocturna
        QList<QAction *> lineSpacingActions;  ///< interlineado (exclusivos)
        QAction *currentLine = nullptr;       ///< resaltar la línea actual
        QAction *focusMode = nullptr;         ///< modo foco
        QAction *pageNumbers = nullptr;       ///< números de página al imprimir
    };

    explicit PreferencesDialog(const Settings &settings, QWidget *parent = nullptr);

private:
    // Desplegable que refleja/dispara un grupo de acciones exclusivas.
    QComboBox *makeExclusiveCombo(const QList<QAction *> &actions);
    // Casilla que refleja/dispara una acción checkable, con su propio texto de rótulo.
    QCheckBox *makeCheck(QAction *action);
};

#endif  // PREFERENCESDIALOG_H
