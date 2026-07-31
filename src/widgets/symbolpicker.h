#ifndef SYMBOLPICKER_H
#define SYMBOLPICKER_H

/// \file
/// \brief Diálogo no modal «mapa de caracteres» de símbolos especiales.

#include <QDialog>
#include <QList>

class QToolButton;

/// \brief Diálogo no modal tipo «mapa de caracteres»: pestañas por categoría con una
/// rejilla de símbolos; al pulsar uno emite symbolChosen() para que quien lo use
/// lo inserte en el editor. Permanece abierto, de modo que se pueden insertar
/// varios seguidos. No conoce el editor (se comunica por señal).
class SymbolPicker : public QDialog
{
    Q_OBJECT

public:
    explicit SymbolPicker(QWidget *parent = nullptr);

signals:
    /// \brief Emite el símbolo pulsado para que el receptor lo inserte.
    void symbolChosen(const QString &symbol);

protected:
    // Reajusta los símbolos cuando cambia la fuente del diálogo (el zoom de la
    // interfaz se la fija desde MainWindow).
    void changeEvent(QEvent *event) override;

private:
    // Da a los botones la fuente de los símbolos (algo mayor que la del diálogo) y
    // el tamaño de celda que le corresponde. Nada de esto puede ser fijo en píxeles:
    // con el zoom subido, los glifos no cabrían en la casilla.
    void updateSymbolMetrics();

    QList<QToolButton *> m_buttons;  // los botones de todas las categorías
};

#endif  // SYMBOLPICKER_H
