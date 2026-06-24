#ifndef SYMBOLPICKER_H
#define SYMBOLPICKER_H

/// \file
/// \brief Diálogo no modal «mapa de caracteres» de símbolos especiales.

#include <QDialog>

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
};

#endif  // SYMBOLPICKER_H
