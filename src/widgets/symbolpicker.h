#ifndef SYMBOLPICKER_H
#define SYMBOLPICKER_H

#include <QDialog>

// Diálogo no modal tipo «mapa de caracteres»: pestañas por categoría con una
// rejilla de símbolos; al pulsar uno emite symbolChosen() para que quien lo use
// lo inserte en el editor. Permanece abierto, de modo que se pueden insertar
// varios seguidos. No conoce el editor (se comunica por señal).
class SymbolPicker : public QDialog
{
    Q_OBJECT

public:
    explicit SymbolPicker(QWidget *parent = nullptr);

signals:
    void symbolChosen(const QString &symbol);
};

#endif  // SYMBOLPICKER_H
