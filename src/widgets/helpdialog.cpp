/// \file
/// \brief Implementación de la ventana de ayuda (selección de idioma y carga de páginas).

#include "helpdialog.h"

#include "appsettings.h"

#include <QFile>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLocale>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextStream>

namespace {

// Sufijo del archivo de ayuda según el idioma activo de la interfaz. Se replica
// la resolución de main.cpp: si el usuario ha elegido un idioma, ese; si no, el
// del sistema. El español es la base (sufijo vacío: `help-app.md`); el resto de
// idiomas con manual traducido llevan su sufijo (`help-app_de.md`, …); cualquier
// idioma sin manual cae al inglés.
QString helpSuffix()
{
    const QString pref = AppSettings::language();
    const QLocale locale = pref.isEmpty() ? QLocale::system() : QLocale(pref);
    switch (locale.language()) {
    case QLocale::Spanish:    return QString();
    case QLocale::German:     return QStringLiteral("_de");
    case QLocale::French:     return QStringLiteral("_fr");
    case QLocale::Italian:    return QStringLiteral("_it");
    case QLocale::Portuguese: return QStringLiteral("_pt");
    case QLocale::Polish:     return QStringLiteral("_pl");
    case QLocale::Dutch:      return QStringLiteral("_nl");
    case QLocale::Romanian:   return QStringLiteral("_ro");
    default:                  return QStringLiteral("_en");
    }
}

}  // namespace

// Declarada en helpdialog.h (namespace mdhelp) para que tst_help pueda validar
// con ella los índices de los 18 .md de ayuda: cada `](#ancla)` del índice tiene
// que caer en un encabezado real, en los nueve idiomas.
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
    // Tamaño cómodo para leer y, a la vez, dejar el editor visible al lado.
    resize(820, 620);

    m_index = new QListWidget(this);
    m_index->addItem(tr("Uso de la aplicación"));
    m_index->addItem(tr("Markdown"));
    m_index->setFixedWidth(200);

    m_viewer = new QTextBrowser(this);
    // Que la navegación por anclas internas (#enlace) funcione, pero las
    // URLs externas se ignoran aquí (no abrimos navegador desde la ayuda).
    m_viewer->setOpenExternalLinks(false);
    m_viewer->setOpenLinks(true);

    auto *layout = new QHBoxLayout(this);
    layout->addWidget(m_index);
    layout->addWidget(m_viewer, 1);

    const QString suffix = helpSuffix();  // "", "_de", …, "_en"
    connect(m_index, &QListWidget::currentRowChanged, this, [this, suffix](int row) {
        const QString page = row == 1 ? QStringLiteral("help-markdown")
                                      : QStringLiteral("help-app");
        loadPage(QStringLiteral(":/help/") + page + suffix + QStringLiteral(".md"));
    });
    m_index->setCurrentRow(0);
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
