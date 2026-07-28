/// \file
/// \brief Implementación del pintado de los iconos monocromos de la barra de formato.
///
/// PROPUESTA — sustituto listo para copiar sobre `src/widgets/formaticons.cpp`.
/// Cambia el dibujo de los tres iconos de lista y el centrado de los cuatro de
/// formato de carácter; `contrastingInk` y `makeTableIcon` quedan como estaban.
/// La cabecera `formaticons.h` NO cambia: misma API.

#include "formaticons.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
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

namespace {

// Lienzo transparente para un icono monocromo de `px` puntos lógicos, a la
// densidad `dpr` de la pantalla.
QPixmap iconCanvas(int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    return pm;
}

// Geometría común de los tres iconos de lista, en fracciones del lado del icono.
// DOS filas, no tres: a 18-22 px (el tamaño real en la barra) tres filas dejan
// al marcador —lo único que distingue un icono de otro— unos 5 px de alto, y los
// tres botones se ven como la misma mancha gris. Con dos filas el marcador gana
// la mitad del alto del icono y cada botón recupera su silueta.
constexpr qreal kRow0        = 0.30;   // centro de la primera fila
constexpr qreal kRowGap      = 0.40;   // paso entre filas
constexpr qreal kLineLeft    = 0.52;   // inicio de la «línea de texto»
constexpr qreal kLineRight   = 0.94;
constexpr qreal kStroke      = 0.115;  // grosor del trazo
constexpr qreal kMarkerLeft  = 0.05;
constexpr qreal kMarkerWidth = 0.32;
constexpr qreal kMarkerHigh  = 0.30;   // alto del marcador (deja aire entre filas)

// Dígito dibujado a trazo, inscrito en `b`. No se usa una fuente: a este tamaño
// el «1 2» de una fuente ocupa 6-7 px con astas de menos de un píxel, y el
// antialiasing lo deja en dos manchas grises que ni se leen ni pegan con las
// líneas de al lado (trazo pleno de 2 px). Dibujados con el mismo trazo que el
// resto del icono pierden detalle tipográfico pero ganan peso y silueta.
void drawDigit(QPainter &p, int digit, const QRectF &b)
{
    const qreal x = b.left(), y = b.top(), w = b.width(), h = b.height();
    QPainterPath path;
    switch (digit) {
    case 1:
        path.moveTo(x + 0.10 * w, y + 0.30 * h);
        path.lineTo(x + 0.52 * w, y);
        path.lineTo(x + 0.52 * w, y + h);
        break;
    case 2:
        path.moveTo(x + 0.08 * w, y + 0.28 * h);
        path.cubicTo(x + 0.10 * w, y - 0.06 * h,
                     x + 0.95 * w, y - 0.02 * h,
                     x + 0.86 * w, y + 0.40 * h);
        path.lineTo(x + 0.08 * w, y + h);
        path.lineTo(x + 0.94 * w, y + h);
        break;
    default:
        break;
    }
    p.drawPath(path);
}

}  // namespace

// Dibuja un icono monocromo para los botones de lista, del color dado (el del
// texto de los botones, para que siga al tema claro/oscuro). Dos filas con una
// «línea de texto» a la derecha y, a la izquierda, el marcador propio de cada
// tipo: viñetas, cifras o marcas de verificación.
QIcon makeListIcon(ListIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm = iconCanvas(px, dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal N = px;                       // se pinta en coordenadas lógicas
    // Trazo grueso: a tamaños de icono pequeños un trazo más fino se ve
    // «aguado» y pierde contraste contra el fondo, aunque la tinta sea máxima.
    const qreal stroke = qMax(qreal(1.5), N * kStroke);
    const qreal rows[2] = {N * kRow0, N * (kRow0 + kRowGap)};

    QPen linePen(color, stroke);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    for (const qreal y : rows)
        p.drawLine(QPointF(N * kLineLeft, y), QPointF(N * kLineRight, y));

    const qreal mx = N * kMarkerLeft;
    const qreal mw = N * kMarkerWidth;
    const qreal mh = N * kMarkerHigh;

    switch (kind) {
    case ListIconKind::Bullet: {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        const qreal r = qMin(mw, mh) * 0.36;
        for (const qreal y : rows)
            p.drawEllipse(QPointF(mx + mw / 2, y), r, r);
        break;
    }
    case ListIconKind::Numbered: {
        QPen digitPen(color, stroke * 0.82);
        digitPen.setCapStyle(Qt::RoundCap);
        digitPen.setJoinStyle(Qt::RoundJoin);
        p.setPen(digitPen);
        p.setBrush(Qt::NoBrush);
        for (int i = 0; i < 2; ++i)
            drawDigit(p, i + 1, QRectF(mx, rows[i] - mh / 2, mw * 0.74, mh));
        break;
    }
    case ListIconKind::Task: {
        // Marcas de verificación SIN casilla: una casilla de 6 px con una marca
        // dentro es un borrón a los tamaños de la barra (y la marca es lo que
        // significa «tarea»; el recuadro solo le roba sitio).
        QPen chk(color, stroke);
        chk.setCapStyle(Qt::RoundCap);
        chk.setJoinStyle(Qt::RoundJoin);
        p.setPen(chk);
        p.setBrush(Qt::NoBrush);
        for (const qreal y : rows) {
            QPolygonF check;
            check << QPointF(mx + mw * 0.06, y + mh * 0.06)
                  << QPointF(mx + mw * 0.36, y + mh * 0.34)
                  << QPointF(mx + mw * 0.98, y - mh * 0.36);
            p.drawPolyline(check);
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
    QPixmap pm = iconCanvas(px, dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const qreal N = px;
    QChar glyph;
    QFont f = p.font();
    // Todos los glifos arrancan con peso DemiBold para que C/S/T no se vean
    // más finos —y por tanto más «aguados»— que la N. La negrita se sigue
    // distinguiendo porque sube a Black; el efecto característico de cada
    // botón (cursiva, subrayado, tachado) se mantiene aparte.
    f.setWeight(QFont::DemiBold);
    f.setPixelSize(int(N * 0.62));  // algo menor que antes: deja sitio al subrayado
    switch (kind) {
    case FormatIconKind::Bold:      glyph = u'N'; f.setWeight(QFont::Black); break;
    case FormatIconKind::Italic:    glyph = u'C'; f.setItalic(true);    break;
    case FormatIconKind::Underline: glyph = u'S';                       break;
    case FormatIconKind::Strike:    glyph = u'T'; f.setStrikeOut(true); break;
    }
    p.setFont(f);
    p.setPen(color);

    // Centrado ÓPTICO: se mide la mancha real del glifo (tightBoundingRect) y se
    // pone su centro en el del icono. Con AlignCenter, Qt centra la caja de
    // línea —que incluye ascendente y descendente vacíos—, así que una mayúscula
    // sin descendente queda visiblemente alta dentro del botón.
    const QFontMetricsF fm(f);
    const QRectF tight = fm.tightBoundingRect(QString(glyph));
    const qreal bias = (kind == FormatIconKind::Italic) ? -N * 0.03 : 0;  // la cursiva se va a la derecha
    const QPointF baseline(N / 2 - tight.center().x() + bias,
                           N / 2 - tight.center().y());
    p.drawText(baseline, QString(glyph));

    // El subrayado se pinta a mano: el de la fuente cae pegado al borde inferior
    // del icono (y a 18 px se recorta). El tachado, en cambio, se deja a la
    // fuente: dibujarlo a mano lo alarga más allá de la «T» y parece un «+».
    if (kind == FormatIconKind::Underline) {
        const qreal half = tight.width() * 0.62;
        QPen rule(color, qMax(qreal(1.4), N * 0.085));
        rule.setCapStyle(Qt::RoundCap);
        p.setPen(rule);
        const qreal y = baseline.y() + N * 0.16;
        p.drawLine(QPointF(N / 2 - half, y), QPointF(N / 2 + half, y));
    }

    p.end();
    return QIcon(pm);
}

// Iconos de la barra flotante de tabla. Filas = barras horizontales; columnas =
// barras verticales; el «+»/«−» (insertar/eliminar) va separado de las barras para
// que se lea. La alineación usa las tres líneas clásicas pegadas a un lado.
// SIN CAMBIOS respecto al original.
QIcon makeTableIcon(TableIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm = iconCanvas(px, dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal N = px;
    const qreal stroke = qMax(qreal(1.5), N * 0.10);
    QPen pen(color, stroke);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);

    // Dibuja un «+» (o «−») centrado en (cx, cy).
    const auto badge = [&](qreal cx, qreal cy, bool plus) {
        const qreal r = N * 0.13;
        p.drawLine(QPointF(cx - r, cy), QPointF(cx + r, cy));
        if (plus)
            p.drawLine(QPointF(cx, cy - r), QPointF(cx, cy + r));
    };

    switch (kind) {
    case TableIconKind::RowInsert:
    case TableIconKind::RowDelete: {
        const qreal ys[3] = {N * 0.28, N * 0.5, N * 0.72};
        for (const qreal y : ys)
            p.drawLine(QPointF(N * 0.36, y), QPointF(N * 0.88, y));
        badge(N * 0.16, N * 0.5, kind == TableIconKind::RowInsert);
        break;
    }
    case TableIconKind::ColInsert:
    case TableIconKind::ColDelete: {
        const qreal xs[3] = {N * 0.28, N * 0.5, N * 0.72};
        for (const qreal x : xs)
            p.drawLine(QPointF(x, N * 0.36), QPointF(x, N * 0.88));
        badge(N * 0.5, N * 0.16, kind == TableIconKind::ColInsert);
        break;
    }
    case TableIconKind::AlignLeft:
    case TableIconKind::AlignCenter:
    case TableIconKind::AlignRight: {
        const qreal ys[3] = {N * 0.28, N * 0.5, N * 0.72};
        for (int i = 0; i < 3; ++i) {
            const qreal w = (i == 1) ? N * 0.42 : N * 0.72;  // línea del medio más corta
            qreal x0 = N * 0.14;
            if (kind == TableIconKind::AlignRight)
                x0 = N * 0.86 - w;
            else if (kind == TableIconKind::AlignCenter)
                x0 = (N - w) / 2;
            p.drawLine(QPointF(x0, ys[i]), QPointF(x0 + w, ys[i]));
        }
        break;
    }
    }

    p.end();
    return QIcon(pm);
}

} // namespace formaticons
