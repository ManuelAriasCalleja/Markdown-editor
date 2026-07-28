/// \file
/// \brief Pintado de las variantes de iconos del banco de pruebas.

#include "iconvariants.h"

#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

namespace iconvariants {
namespace {

QPixmap canvas(int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    return pm;
}

// --- Dígitos dibujados a trazo -------------------------------------------
//
// El problema del icono de lista numerada es que sus dígitos son glifos de
// fuente: a 18-24 px el «1 2 3» ocupa 6-7 px de alto, con astas de menos de un
// píxel, y el antialiasing lo convierte en tres manchas grises que ni se leen
// ni pegan con el resto del icono (líneas de 2 px bien negras). Dibujarlos con
// el MISMO trazo que las líneas de texto los mete en la misma familia óptica:
// pierden detalle tipográfico, pero ganan peso y silueta.
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
    case 3:
        path.moveTo(x + 0.10 * w, y + 0.14 * h);
        path.cubicTo(x + 0.60 * w, y - 0.14 * h,
                     x + 1.05 * w, y + 0.22 * h,
                     x + 0.50 * w, y + 0.48 * h);
        path.cubicTo(x + 1.12 * w, y + 0.54 * h,
                     x + 0.62 * w, y + 1.16 * h,
                     x + 0.08 * w, y + 0.86 * h);
        break;
    default:
        break;
    }
    p.drawPath(path);
}

// Marca de verificación suelta, inscrita en `box`.
void drawCheck(QPainter &p, const QRectF &box, const QColor &color, qreal stroke)
{
    QPen chk(color, stroke);
    chk.setCapStyle(Qt::RoundCap);
    chk.setJoinStyle(Qt::RoundJoin);
    p.setPen(chk);
    p.setBrush(Qt::NoBrush);
    QPolygonF check;
    check << QPointF(box.left() + box.width() * 0.06, box.center().y() + box.height() * 0.06)
          << QPointF(box.left() + box.width() * 0.36, box.center().y() + box.height() * 0.34)
          << QPointF(box.left() + box.width() * 0.98, box.center().y() - box.height() * 0.36);
    p.drawPolyline(check);
}

// Casilla de tarea con su marca de verificación opcional.
void drawCheckbox(QPainter &p, const QRectF &box, bool checked,
                  const QColor &color, qreal stroke)
{
    QPen boxPen(color, stroke);
    boxPen.setJoinStyle(Qt::MiterJoin);
    p.setPen(boxPen);
    p.setBrush(Qt::NoBrush);
    const qreal r = box.width() * 0.18;
    p.drawRoundedRect(box, r, r);
    if (checked)
        drawCheck(p, box.adjusted(box.width() * 0.20, box.height() * 0.20,
                                  -box.width() * 0.20, -box.height() * 0.20),
                  color, stroke * 1.1);
}

// --- Variante «Actual» (copia literal de src/widgets/formaticons.cpp) -----
void paintActual(QPainter &p, ListKind kind, const QColor &color, qreal N)
{
    const qreal markerRight = N * 0.42;
    const qreal lineRight   = N * 0.88;
    const qreal rows[3] = {N * 0.26, N * 0.5, N * 0.74};
    const qreal stroke = qMax(qreal(1.5), N * 0.10);

    QPen linePen(color, stroke);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    for (const qreal y : rows)
        p.drawLine(QPointF(markerRight, y), QPointF(lineRight, y));

    switch (kind) {
    case ListKind::Bullet: {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        const qreal r = N * 0.10;
        for (const qreal y : rows)
            p.drawEllipse(QPointF(N * 0.16, y), r, r);
        break;
    }
    case ListKind::Numbered: {
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
    case ListKind::Task: {
        const qreal s = N * 0.22;
        QPen boxPen(color, qMax(qreal(1.2), N * 0.09));
        boxPen.setJoinStyle(Qt::MiterJoin);
        for (int i = 0; i < 3; ++i) {
            const qreal y = rows[i];
            const QRectF box(N * 0.08, y - s / 2, s, s);
            p.setPen(boxPen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(box);
            if (i == 0) {
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
}

// --- Variantes nuevas -----------------------------------------------------
//
// Parámetros comunes: `rows` filas de «línea de texto» (2 o 3) y una columna de
// marcador a la izquierda. Con 2 filas el marcador cabe al doble de tamaño sin
// tocar el ancho del icono, que es de donde sale la legibilidad.
struct Layout {
    int rows;
    qreal firstRow, rowGap;   // en fracción de N
    qreal lineLeft, lineRight;
    qreal strokeFactor;
    qreal markerWidth;        // ancho de la columna del marcador
    qreal markerHeight;       // alto máximo del marcador (para que no se toquen)
    bool  vectorDigits;
    bool  bareChecks;         // tareas: marcas sueltas en vez de casillas
};

void paintList(QPainter &p, const Layout &lay, ListKind kind,
               const QColor &color, qreal N)
{
    const qreal stroke = qMax(qreal(1.5), N * lay.strokeFactor);
    QVector<qreal> rows;
    for (int i = 0; i < lay.rows; ++i)
        rows << N * (lay.firstRow + i * lay.rowGap);

    QPen linePen(color, stroke);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    for (const qreal y : rows)
        p.drawLine(QPointF(N * lay.lineLeft, y), QPointF(N * lay.lineRight, y));

    const qreal mw = N * lay.markerWidth;    // ancho útil del marcador
    const qreal mh = N * lay.markerHeight;   // alto útil del marcador
    const qreal mx = N * 0.05;               // margen izquierdo

    switch (kind) {
    case ListKind::Bullet: {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        const qreal r = qMin(mw, mh) * 0.36;
        for (const qreal y : rows)
            p.drawEllipse(QPointF(mx + mw / 2, y), r, r);
        break;
    }
    case ListKind::Numbered: {
        if (lay.vectorDigits) {
            QPen digitPen(color, stroke * 0.82);
            digitPen.setCapStyle(Qt::RoundCap);
            digitPen.setJoinStyle(Qt::RoundJoin);
            p.setPen(digitPen);
            p.setBrush(Qt::NoBrush);
            const qreal w = mw * 0.74;
            for (int i = 0; i < rows.size(); ++i)
                drawDigit(p, i + 1, QRectF(mx, rows[i] - mh / 2, w, mh));
        } else {
            QFont f = p.font();
            f.setPixelSize(qMax(6, int(mh * 1.30)));  // el px de fuente ≈ alto de caja
            f.setBold(true);
            p.setFont(f);
            p.setPen(color);
            for (int i = 0; i < rows.size(); ++i)
                p.drawText(QRectF(mx, rows[i] - mh, mw, mh * 2),
                           Qt::AlignHCenter | Qt::AlignVCenter,
                           QString::number(i + 1));
        }
        break;
    }
    case ListKind::Task: {
        const qreal s = qMin(mw, mh);
        for (int i = 0; i < rows.size(); ++i) {
            const QRectF box(mx, rows[i] - s / 2, s, s);
            if (lay.bareChecks)
                drawCheck(p, box, color, stroke);
            else
                drawCheckbox(p, box, i == 0, color, qMax(qreal(1.2), stroke * 0.80));
        }
        break;
    }
    }
}

//                 filas  1ª     paso  línea izq/der  trazo  marc.ancho/alto  díg  ✓
const Layout kAire  {2, 0.30, 0.40, 0.52, 0.94, 0.115, 0.32, 0.30, false, false};
const Layout kTrazo {3, 0.23, 0.27, 0.46, 0.92, 0.095, 0.30, 0.22, true,  false};
const Layout kCifra {2, 0.30, 0.40, 0.52, 0.94, 0.115, 0.30, 0.30, true,  false};
const Layout kCheck {2, 0.30, 0.40, 0.52, 0.94, 0.115, 0.32, 0.30, true,  true};

} // namespace

QPixmap listPixmap(ListVariant v, ListKind k, const QColor &color, int px, qreal dpr)
{
    QPixmap pm = canvas(px, dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    const qreal N = px;
    switch (v) {
    case ListVariant::Actual:  paintActual(p, k, color, N); break;
    case ListVariant::Aire:    paintList(p, kAire, k, color, N); break;
    case ListVariant::Trazo:   paintList(p, kTrazo, k, color, N); break;
    case ListVariant::Cifra:   paintList(p, kCifra, k, color, N); break;
    case ListVariant::Check:   paintList(p, kCheck, k, color, N); break;
    }
    p.end();
    return pm;
}

QPixmap formatPixmap(FormatVariant v, FormatKind k, const QColor &color, int px, qreal dpr)
{
    QPixmap pm = canvas(px, dpr);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const qreal N = px;
    QChar glyph;
    QFont f = p.font();
    f.setWeight(QFont::DemiBold);
    // La variante óptica deja sitio bajo la letra para el subrayado y encoge un
    // punto la caja, porque centrar por caja de línea (lo actual) deja el glifo
    // bajo y el subrayado se come el borde del icono.
    f.setPixelSize(int(N * (v == FormatVariant::Optico ? 0.62 : 0.66)));
    switch (k) {
    case FormatKind::Bold:      glyph = u'N'; f.setWeight(QFont::Black); break;
    case FormatKind::Italic:    glyph = u'C'; f.setItalic(true);    break;
    case FormatKind::Underline: glyph = u'S'; f.setUnderline(true); break;
    case FormatKind::Strike:    glyph = u'T'; f.setStrikeOut(true); break;
    }
    p.setFont(f);
    p.setPen(color);

    if (v == FormatVariant::Actual) {
        p.drawText(QRectF(0, 0, N, N), Qt::AlignCenter, QString(glyph));
    } else if (k == FormatKind::Underline) {
        // El subrayado de la FUENTE se pega al borde inferior del icono (y a
        // tamaños pequeños se recorta). Se pinta a mano, con el mismo grosor que
        // el resto de la familia y a distancia fija de la base de la letra.
        // El tachado, en cambio, se deja a la fuente: dibujarlo a mano lo alarga
        // más allá de la «T» y el icono acaba pareciendo un «+».
        QFont plain = f;
        plain.setUnderline(false);
        p.setFont(plain);
        const QFontMetricsF fm(plain);
        const QRectF tight = fm.tightBoundingRect(QString(glyph));
        const QPointF baseline(N / 2 - tight.center().x(), N / 2 - tight.center().y());
        p.drawText(baseline, QString(glyph));

        const qreal half = tight.width() * 0.62;
        QPen rule(color, qMax(qreal(1.4), N * 0.085));
        rule.setCapStyle(Qt::RoundCap);
        p.setPen(rule);
        const qreal y = baseline.y() + N * 0.16;
        p.drawLine(QPointF(N / 2 - half, y), QPointF(N / 2 + half, y));
    } else {
        // Centrado ÓPTICO: se mide la mancha real del glifo (tightBoundingRect)
        // y se coloca su centro en el centro del icono. Con AlignCenter, Qt
        // centra la caja de línea —que incluye ascendente y descendente vacíos—,
        // así que una mayúscula sin descendente queda visiblemente alta.
        const QFontMetricsF fm(f);
        QRectF tight = fm.tightBoundingRect(QString(glyph));
        // La cursiva se sale por la derecha: se compensa medio ángulo.
        const qreal bias = (k == FormatKind::Italic) ? -N * 0.03 : 0;
        const QPointF baseline(N / 2 - tight.center().x() + bias,
                               N / 2 - tight.center().y());
        p.drawText(baseline, QString(glyph));
    }

    p.end();
    return pm;
}

} // namespace iconvariants
