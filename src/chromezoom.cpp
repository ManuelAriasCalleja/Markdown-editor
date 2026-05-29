#include "chromezoom.h"

#include <QAction>
#include <QChar>
#include <QFontMetrics>
#include <QKeySequence>
#include <QMenu>
#include <QString>

namespace chromezoom {

qreal scaledPointSize(qreal base, int delta)
{
    if (base <= 0)
        return base;  // la fuente no usa puntos: no escalar
    return qMax(qreal(1), base + delta);
}

int menuMinimumWidth(const QMenu &menu)
{
    const QFontMetrics fm(menu.font());
    int textMax = 0;
    int shortcutMax = 0;
    bool anyShortcut = false;
    bool anySubmenu = false;
    bool anyCheckable = false;
    for (const QAction *a : menu.actions()) {
        if (a->isSeparator())
            continue;
        QString t = a->text();
        t.remove(QLatin1Char('&'));  // mnemonic prefix no se pinta
        textMax = qMax(textMax, fm.horizontalAdvance(t));
        const QKeySequence sc = a->shortcut();
        if (!sc.isEmpty()) {
            anyShortcut = true;
            shortcutMax = qMax(
                shortcutMax,
                fm.horizontalAdvance(sc.toString(QKeySequence::NativeText)));
        }
        if (a->menu())
            anySubmenu = true;
        if (a->isCheckable())
            anyCheckable = true;
    }
    if (textMax <= 0)
        return 0;
    // Estimación generosa de paddings/separadores entre columnas. Mejor pasarse y
    // dejar el menú un pelín más ancho que quedarse corto y volver al elide: el
    // estilo del SO añade su propio padding encima, así que el ancho real terminará
    // un poco mayor que este mínimo.
    const int em = fm.horizontalAdvance(QChar('M'));
    int width = textMax + em * 3;      // padding lateral + icono opcional
    if (anyCheckable)
        width += em * 2;               // columna del indicador de check
    if (anyShortcut)
        width += shortcutMax + em * 4; // separación texto–atajo
    if (anySubmenu)
        width += em * 2;               // flecha de submenú
    return width;
}

}  // namespace chromezoom
