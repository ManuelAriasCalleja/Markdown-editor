/// \file
/// \brief Implementación de la ventana de ayuda (selección de idioma y carga de páginas).

#include "helpdialog.h"

#include "appsettings.h"
#include "chromezoom.h"
#include "langtag.h"

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLocale>
#include <QScreen>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>

namespace {

// Idioma activo de la interfaz, con la misma resolución que main.cpp: si el usuario
// ha elegido uno, ese; si no, el del sistema.
QString activeLanguage()
{
    const QString pref = AppSettings::language();
    return pref.isEmpty() ? QLocale::system().name() : pref;
}

}  // namespace

// El sufijo sale de la etiqueta canónica y se comprueba contra los recursos: antes
// era una lista de `case` sobre QLocale::Language que había que ampliar a mano con
// cada manual nuevo y que, además, no podía distinguir el chino simplificado del
// tradicional (`QLocale::Chinese` vale para los dos).
QString mdhelp::helpSuffixForLanguage(const QString &code)
{
    const QString tag = mdlang::canonicalTag(code);
    if (tag.isEmpty() || tag == QStringLiteral("es"))
        return QString();  // el español es la base, sin sufijo
    QString suffix = QLatin1Char('_') + tag;
    if (QFile::exists(QStringLiteral(":/help/help-app%1.md").arg(suffix))
        && QFile::exists(QStringLiteral(":/help/help-markdown%1.md").arg(suffix))) {
        return suffix;
    }
    return QStringLiteral("_en");  // idioma sin manual traducido
}

// Declarada en helpdialog.h (namespace mdhelp) para que tst_help pueda validar
// con ella los índices de los 20 .md de ayuda: cada `](#ancla)` del índice tiene
// que caer en un encabezado real, en los diez idiomas.
QString mdhelp::headingSlug(const QString &text)
{
    const QString decomposed = text.normalized(QString::NormalizationForm_D);
    QString slug;
    slug.reserve(decomposed.size());
    bool pendingSeparator = false;
    for (const QChar c : decomposed) {
        if (c.category() == QChar::Mark_NonSpacing
            || c.category() == QChar::Mark_SpacingCombining
            || c.category() == QChar::Mark_Enclosing)
            continue;  // diacrítico: á → a
        if (c.isLetterOrNumber()) {
            if (pendingSeparator && !slug.isEmpty())
                slug += QLatin1Char('-');
            pendingSeparator = false;
            slug += c.toLower();
        } else if (c.isSpace() || c == QLatin1Char('-')) {
            pendingSeparator = true;  // separador (colapsa runs a un guion)
        }
        // El resto de puntuación se descarta (no genera guion).
    }
    return slug;
}

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Ayuda"));
    updateWindowSize();

    m_index = new QListWidget(this);
    m_index->addItem(tr("Uso de la aplicación"));
    m_index->addItem(tr("Markdown"));
    updateIndexWidth();

    m_viewer = new QTextBrowser(this);
    // Que la navegación por anclas internas (#enlace) funcione, pero las
    // URLs externas se ignoran aquí (no abrimos navegador desde la ayuda).
    m_viewer->setOpenExternalLinks(false);
    m_viewer->setOpenLinks(true);

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(m_index);
    layout->addWidget(m_viewer, 1);

    const QString suffix = mdhelp::helpSuffixForLanguage(activeLanguage());
    connect(m_index, &QListWidget::currentRowChanged, this, [this, suffix](int row) {
        const QString page = row == 1 ? QStringLiteral("help-markdown")
                                      : QStringLiteral("help-app");
        loadPage(QStringLiteral(":/help/") + page + suffix + QStringLiteral(".md"));
    });
    m_index->setCurrentRow(0);
}

void HelpDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    // La fuente la fija MainWindow (zoom de la interfaz) después de construir el
    // diálogo, y puede volver a cambiarla mientras está abierto: el ancho del índice
    // y el de la propia ventana tienen que seguirla.
    if (event->type() == QEvent::FontChange) {
        updateIndexWidth();
        updateWindowSize();
    }
}

void HelpDialog::updateWindowSize()
{
    // Tamaño cómodo para leer y, a la vez, dejar el editor visible al lado… con la
    // fuente base. Cuando el zoom de la interfaz la amplía, en ese mismo ancho cabe
    // menos texto y las líneas se parten de forma poco natural, así que la ventana
    // crece en la misma proporción (sin pasar de lo que quepa en la pantalla).
    static const QSize kBaseSize(820, 620);
    const qreal appPoints = QApplication::font().pointSizeF();
    const qreal points = font().pointSizeF();
    if (appPoints <= 0 || points <= 0)
        return;
    QSize limit;
    if (const QScreen *scr = screen())
        limit = scr->availableGeometry().size() * 0.9;
    resize(chromezoom::scaledWindowSize(kBaseSize, points / appPoints, limit));
}

void HelpDialog::updateIndexWidth()
{
    const QFontMetrics fm(m_index->font());
    int text = 0;
    for (int row = 0; row < m_index->count(); ++row)
        text = qMax(text, fm.horizontalAdvance(m_index->item(row)->text()));
    // Margen para el relleno del ítem y el marco. Se expresa en alturas de línea
    // (no en píxeles fijos) para que crezca con la fuente igual que el texto.
    m_index->setFixedWidth(text + fm.height() * 3 + m_index->frameWidth() * 2);
}

void HelpDialog::loadPage(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_viewer->setPlainText(tr("No se pudo cargar la ayuda."));
        return;
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    m_viewer->setMarkdown(in.readAll());
    // Qt no genera anclas con nombre para los encabezados al renderizar Markdown,
    // así que los enlaces internos del índice (#seccion) no tendrían destino y al
    // pulsarlos el visor no se desplazaría. Recorremos los encabezados y a cada uno
    // le ponemos como ancla su slug, para que QTextBrowser::scrollToAnchor (que
    // dispara el clic) encuentre el destino.
    QTextDocument *doc = m_viewer->document();
    for (QTextBlock b = doc->begin(); b != doc->end(); b = b.next()) {
        if (b.blockFormat().headingLevel() <= 0)
            continue;
        const QString slug = mdhelp::headingSlug(b.text());
        if (slug.isEmpty())
            continue;
        QTextCursor cur(b);
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        QTextCharFormat fmt;
        // scrollToAnchor solo localiza fragmentos con isAnchor(); un ancla con
        // nombre pero sin href no se pinta como enlace (el encabezado no cambia).
        fmt.setAnchor(true);
        fmt.setAnchorNames({slug});
        cur.mergeCharFormat(fmt);
    }
    // Vuelve arriba al cambiar de página (si no, conservaría el scroll
    // de la página anterior, que confunde).
    m_viewer->verticalScrollBar()->setValue(0);
}
