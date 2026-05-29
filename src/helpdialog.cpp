#include "helpdialog.h"

#include "appsettings.h"

#include <QFile>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLocale>
#include <QScrollBar>
#include <QTextBrowser>
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

} // namespace

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
    // Vuelve arriba al cambiar de página (si no, conservaría el scroll
    // de la página anterior, que confunde).
    m_viewer->verticalScrollBar()->setValue(0);
}
