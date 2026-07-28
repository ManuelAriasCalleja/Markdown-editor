/// \file
/// \brief Genera las láminas de contacto PNG con las variantes de iconos.
///
/// Uso: ./iconpreview [directorio-de-salida]  (por defecto, el actual).
/// Produce:
///   contacto-claro.png / contacto-oscuro.png  — variantes × tamaños reales (18/22/28/36)
///   detalle-<variante>.png                    — cada juego ampliado 8×, sin suavizar

#include "iconvariants.h"

#include <QApplication>
#include <QDir>
#include <QFont>
#include <QFontMetricsF>
#include <QImage>
#include <QPainter>

using namespace iconvariants;

namespace {

struct VariantRow {
    ListVariant list;
    FormatVariant format;
    const char *name;
};

const VariantRow kRows[] = {
    {ListVariant::Actual, FormatVariant::Actual, "Actual"},
    {ListVariant::Aire,   FormatVariant::Optico, "Aire"},
    {ListVariant::Trazo,  FormatVariant::Optico, "Trazo"},
    {ListVariant::Cifra,  FormatVariant::Optico, "Cifra"},
    {ListVariant::Check,  FormatVariant::Optico, "Check"},
};

const int kSizes[] = {18, 22, 28, 36};

// Los siete iconos de la barra, en el orden en que aparecen en ella.
void paintSet(QPainter &p, const VariantRow &row, const QColor &ink, int px,
              qreal x, qreal y, qreal gap)
{
    const FormatKind fks[] = {FormatKind::Bold, FormatKind::Italic,
                              FormatKind::Underline, FormatKind::Strike};
    for (const FormatKind k : fks) {
        p.drawPixmap(QPointF(x, y), formatPixmap(row.format, k, ink, px, 1.0));
        x += px + gap;
    }
    x += gap * 2;
    const ListKind lks[] = {ListKind::Bullet, ListKind::Numbered, ListKind::Task};
    for (const ListKind k : lks) {
        p.drawPixmap(QPointF(x, y), listPixmap(row.list, k, ink, px, 1.0));
        x += px + gap;
    }
}

qreal setWidth(int px, qreal gap)
{
    return 7 * (px + gap) + gap * 2;
}

void contactSheet(const QString &path, const QColor &bg, const QColor &ink)
{
    const qreal gap = 8;
    const qreal margin = 20;
    const qreal labelW = 90;
    qreal colX = margin + labelW;
    QVector<qreal> colXs;
    for (const int px : kSizes) {
        colXs << colX;
        colX += setWidth(px, gap) + 34;
    }
    const qreal W = colX + margin;
    const qreal rowH = 56;
    const qreal H = margin * 2 + 26 + std::size(kRows) * rowH;

    QImage img(int(W), int(H), QImage::Format_ARGB32_Premultiplied);
    img.fill(bg);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QFont label = p.font();
    label.setPixelSize(12);
    p.setFont(label);
    p.setPen(ink);
    for (int c = 0; c < int(std::size(kSizes)); ++c)
        p.drawText(QPointF(colXs[c], margin + 12),
                   QStringLiteral("%1 px").arg(kSizes[c]));

    for (int r = 0; r < int(std::size(kRows)); ++r) {
        const qreal y = margin + 26 + r * rowH;
        p.setPen(ink);
        p.drawText(QRectF(margin, y, labelW - 10, rowH),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QString::fromLatin1(kRows[r].name));
        for (int c = 0; c < int(std::size(kSizes)); ++c)
            paintSet(p, kRows[r], ink, kSizes[c], colXs[c],
                     y + (rowH - kSizes[c]) / 2 - 4, gap);
    }
    p.end();
    img.save(path);
}

// Ampliación sin suavizado: enseña exactamente qué píxeles pinta cada variante
// al tamaño real de la barra (22 px), que es donde se juega la legibilidad.
void detailSheet(const VariantRow &row, const QString &path,
                 const QColor &bg, const QColor &ink)
{
    const int px = 22, zoom = 8, gap = 6;
    const int setW = int(setWidth(px, gap));
    QImage small(setW, px + 2, QImage::Format_ARGB32_Premultiplied);
    small.fill(bg);
    {
        QPainter p(&small);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);
        paintSet(p, row, ink, px, 0, 1, gap);
    }
    QImage big = small.scaled(small.width() * zoom, small.height() * zoom,
                              Qt::IgnoreAspectRatio, Qt::FastTransformation);
    big.save(path);
}

// Maqueta de la barra de formato tal cual es: los botones de icono conviven con
// botones de TEXTO («H1», «Cita»…). Es el contexto real en el que se juzgan, y
// donde se ve si el peso del icono pega con el del texto de al lado.
void toolbarMock(const QString &path, const QColor &bg, const QColor &ink)
{
    const int px = 22;
    const qreal padX = 10, gap = 4, rowH = 46, margin = 16, labelW = 70;
    const char *textButtons1[] = {"Código", "Enlace"};
    const char *headings[] = {"H1", "H2", "H3", "H4"};
    const char *textButtons2[] = {"Cita", "Bloque de código"};

    QImage probe(1, 1, QImage::Format_ARGB32_Premultiplied);
    QPainter mp(&probe);
    QFont barFont = mp.font();
    barFont.setPixelSize(14);
    const QFontMetricsF fm(barFont);
    mp.end();

    qreal w = margin + labelW;
    auto advanceIcon = [&] { w += px + padX + gap; };
    auto advanceText = [&](const char *t) {
        w += fm.horizontalAdvance(QString::fromUtf8(t)) + padX * 2 + gap;
    };
    for (int i = 0; i < 4; ++i) advanceIcon();
    for (const char *t : textButtons1) advanceText(t);
    w += 12;
    for (const char *t : headings) advanceText(t);
    w += 12;
    for (int i = 0; i < 3; ++i) advanceIcon();
    w += 12;
    for (const char *t : textButtons2) advanceText(t);
    w += margin;

    const int rows = int(std::size(kRows));
    QImage img(int(w), int(margin * 2 + rows * rowH), QImage::Format_ARGB32_Premultiplied);
    img.fill(bg);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setFont(barFont);

    for (int r = 0; r < rows; ++r) {
        const qreal cy = margin + r * rowH + rowH / 2;
        p.setPen(ink);
        p.drawText(QRectF(margin, cy - rowH / 2, labelW - 8, rowH),
                   Qt::AlignLeft | Qt::AlignVCenter, QString::fromLatin1(kRows[r].name));
        qreal x = margin + labelW;
        const auto icon = [&](const QPixmap &pm) {
            p.drawPixmap(QPointF(x + padX / 2, cy - px / 2.0), pm);
            x += px + padX + gap;
        };
        const auto text = [&](const char *t) {
            const QString s = QString::fromUtf8(t);
            const qreal tw = fm.horizontalAdvance(s) + padX * 2;
            p.setPen(ink);
            p.drawText(QRectF(x, cy - rowH / 2, tw, rowH), Qt::AlignCenter, s);
            x += tw + gap;
        };
        const auto sep = [&] {
            QPen sepPen(ink, 1);
            p.setPen(sepPen);
            p.setOpacity(0.30);
            p.drawLine(QPointF(x + 5, cy - 11), QPointF(x + 5, cy + 11));
            p.setOpacity(1.0);
            x += 12;
        };
        for (const FormatKind k : {FormatKind::Bold, FormatKind::Italic,
                                   FormatKind::Underline, FormatKind::Strike})
            icon(formatPixmap(kRows[r].format, k, ink, px, 1.0));
        for (const char *t : textButtons1) text(t);
        sep();
        for (const char *t : headings) text(t);
        sep();
        for (const ListKind k : {ListKind::Bullet, ListKind::Numbered, ListKind::Task})
            icon(listPixmap(kRows[r].list, k, ink, px, 1.0));
        sep();
        for (const char *t : textButtons2) text(t);
    }
    p.end();
    img.save(path);
}

} // namespace

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    const QString outDir = argc > 1 ? QString::fromLocal8Bit(argv[1])
                                    : QDir::currentPath();
    QDir().mkpath(outDir);

    const QColor lightBg(0xf3, 0xf3, 0xf3), lightInk(0x1a, 0x1a, 0x1a);
    const QColor darkBg(0x2b, 0x2b, 0x2b),  darkInk(0xf0, 0xf0, 0xf0);

    toolbarMock(outDir + "/barra-claro.png", lightBg, lightInk);
    toolbarMock(outDir + "/barra-oscuro.png", darkBg, darkInk);
    contactSheet(outDir + "/contacto-claro.png", lightBg, lightInk);
    contactSheet(outDir + "/contacto-oscuro.png", darkBg, darkInk);
    for (const VariantRow &row : kRows)
        detailSheet(row, outDir + QStringLiteral("/detalle-%1.png")
                                      .arg(QString::fromLatin1(row.name).toLower()),
                    lightBg, lightInk);
    return 0;
}
