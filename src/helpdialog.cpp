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

// Indica si el idioma activo de la interfaz es el español, para escoger qué
// archivo de ayuda mostrar. Se replica la resolución de main.cpp: si el
// usuario ha elegido un idioma, ese; si no, el del sistema. El resto de
// idiomas reciben la versión en inglés (no se traduce el manual a los ocho
// idiomas de la UI).
bool helpInSpanish()
{
    const QString pref = AppSettings::language();
    const QLocale locale = pref.isEmpty() ? QLocale::system() : QLocale(pref);
    return locale.language() == QLocale::Spanish;
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

    const QString suffix = helpInSpanish() ? QStringLiteral(".md")
                                           : QStringLiteral("_en.md");
    connect(m_index, &QListWidget::currentRowChanged, this, [this, suffix](int row) {
        const QString page = row == 1 ? QStringLiteral("help-markdown")
                                      : QStringLiteral("help-app");
        loadPage(QStringLiteral(":/help/") + page + suffix);
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
