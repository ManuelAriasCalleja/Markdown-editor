#include "formaticons.h"

#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QPolygonF>

#include <cmath>

namespace formaticons {

QColor contrastingInk(const QColor &background)
{
    auto channel = [](int c) {
        const double v = c / 255.0;
        return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    const double L = 0.2126 * channel(background.red())
                   + 0.7152 * channel(background.green())
                   + 0.0722 * channel(background.blue());
    return L > 0.5 ? QColor(0x1a, 0x1a, 0x1a) : QColor(0xf0, 0xf0, 0xf0);
}

// Dibuja un icono monocromo para los botones de lista, del color dado (el del
// texto de los botones, para que siga al tema claro/oscuro). Tres filas con una
// «línea de texto» a la derecha y, a la izquierda, el marcador propio de cada
// tipo: viñetas, números o casillas de verificación.
QIcon makeListIcon(ListIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal N = px;                       // se pinta en coordenadas lógicas
    const qreal markerRight = N * 0.42;       // fin de la zona del marcador
    const qreal lineRight   = N * 0.88;       // fin de la línea de texto
    const qreal rows[3] = {N * 0.26, N * 0.5, N * 0.74};
    // Trazo grueso: a tamaños de icono pequeños un trazo más fino se ve
    // «aguado» y pierde contraste contra el fondo, aunque la tinta sea
    // máxima. Se pisa un poco más que el grosor «de proporción» natural.
    const qreal stroke = qMax(qreal(1.5), N * 0.10);

    QPen linePen(color, stroke);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    for (const qreal y : rows)
        p.drawLine(QPointF(markerRight, y), QPointF(lineRight, y));

    switch (kind) {
    case ListIconKind::Bullet: {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        const qreal r = N * 0.10;
        for (const qreal y : rows)
            p.drawEllipse(QPointF(N * 0.16, y), r, r);
        break;
    }
    case ListIconKind::Numbered: {
        QFont f = p.font();
        f.setPixelSize(int(N * 0.34));
        f.setBold(true);
        p.setFont(f);
        p.setPen(color);
        const char *nums[3] = {"1", "2", "3"};
        for (int i = 0; i < 3; ++i)
            p.drawText(QRectF(0, rows[i] - N * 0.22, markerRight - N * 0.10, N * 0.44),
                       Qt::AlignRight | Qt::AlignVCenter, QString::fromLatin1(nums[i]));
        break;
    }
    case ListIconKind::Task: {
        const qreal s = N * 0.22;
        QPen boxPen(color, qMax(qreal(1.2), N * 0.09));
        boxPen.setJoinStyle(Qt::MiterJoin);
        for (int i = 0; i < 3; ++i) {
            const qreal y = rows[i];
            const QRectF box(N * 0.08, y - s / 2, s, s);
            p.setPen(boxPen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(box);
            if (i == 0) {  // la primera casilla, marcada
                QPen chk(color, qMax(qreal(1.2), N * 0.10));
                chk.setCapStyle(Qt::RoundCap);
                chk.setJoinStyle(Qt::RoundJoin);
                p.setPen(chk);
                QPolygonF check;
                check << QPointF(box.left() + s * 0.18, y + s * 0.02)
                      << QPointF(box.left() + s * 0.42, y + s * 0.28)
                      << QPointF(box.left() + s * 0.84, y - s * 0.30);
                p.drawPolyline(check);
            }
        }
        break;
    }
    }

    p.end();
    return QIcon(pm);
}

// Dibuja el icono de un botón de formato de carácter: la inicial española del
// efecto, pintada con ese mismo efecto, de modo que la letra se explica sola
// (N negrita, C cursiva, S subrayada, T tachada). Monocromo, del color del
// texto, para seguir al tema como los iconos de lista.
QIcon makeFormatIcon(FormatIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const qreal N = px;
    QChar glyph;
    QFont f = p.font();
    f.setPixelSize(int(N * 0.66));
    // Todos los glifos arrancan con peso DemiBold para que C/S/T no se vean
    // más finos —y por tanto más «aguados»— que la N. La negrita se sigue
    // distinguiendo porque sube a Black; el efecto característico de cada
    // botón (cursiva, subrayado, tachado) se mantiene aparte.
    f.setWeight(QFont::DemiBold);
    switch (kind) {
    case FormatIconKind::Bold:      glyph = u'N'; f.setWeight(QFont::Black); break;
    case FormatIconKind::Italic:    glyph = u'C'; f.setItalic(true);    break;
    case FormatIconKind::Underline: glyph = u'S'; f.setUnderline(true); break;
    case FormatIconKind::Strike:    glyph = u'T'; f.setStrikeOut(true); break;
    }
    p.setFont(f);
    p.setPen(color);
    p.drawText(QRectF(0, 0, N, N), Qt::AlignCenter, QString(glyph));

    p.end();
    return QIcon(pm);
}

} // namespace formaticons
