#include "exportcontroller.h"

#include <memory>

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMimeData>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLocale>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QStringConverter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextStream>

#include "appsettings.h"
#include "documentio.h"
#include "exporters.h"
#include "splitviewcontroller.h"

// Los textos visibles conservan el contexto de traducción "MainWindow": se usan
// con QCoreApplication::translate("MainWindow", ...) en vez de tr() (cuyo contexto
// sería "ExportController") para no re-hogar en los 8 .ts las cadenas ya
// traducidas al moverlas desde MainWindow. lupdate las atribuye a "MainWindow" al
// ver el literal. Ver la nota de i18n en CLAUDE.md.

ExportController::ExportController(QTextEdit *editor, DocumentIo *documentIo,
                                   SplitViewController *split, QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_documentIo(documentIo)
    , m_split(split)
    , m_parent(parent)
{
}

QString ExportController::exportTitle() const
{
    return mdexport::frontMatterValue(m_documentIo->frontMatter(),
                                      QStringLiteral("title"));
}

bool ExportController::chooseExportLanguage(mdexport::Language *out)
{
    // Idioma por defecto: `lang`/`language` del front matter; si no, el de la
    // app; si tampoco, el del sistema.
    const QString fm = m_documentIo->frontMatter();
    QString code = mdexport::frontMatterValue(fm, QStringLiteral("lang"));
    if (code.isEmpty())
        code = mdexport::frontMatterValue(fm, QStringLiteral("language"));
    if (code.isEmpty())
        code = AppSettings::language();
    if (code.isEmpty())
        code = QLocale::system().name();  // p. ej. "es_ES"
    const mdexport::Language def = mdexport::languageForCode(code);

    const QList<mdexport::Language> langs = mdexport::languages();
    QStringList names;
    int current = 0;
    for (int i = 0; i < langs.size(); ++i) {
        names << langs.at(i).name;
        if (langs.at(i).code == def.code)
            current = i;
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(
        m_parent, QCoreApplication::translate("MainWindow", "Idioma del documento"),
        QCoreApplication::translate("MainWindow", "Idioma para la exportación:"),
        names, current, /*editable=*/false, &ok);
    if (!ok)
        return false;
    *out = langs.at(names.indexOf(chosen));
    return true;
}

QString ExportController::suggestedExportPath(const QString &ext) const
{
    const QString currentFile = m_documentIo->currentFile();
    if (currentFile.isEmpty())
        return QDir::homePath() + QStringLiteral("/documento.") + ext;
    const QFileInfo info(currentFile);
    return info.absolutePath() + QLatin1Char('/') + info.completeBaseName()
           + QLatin1Char('.') + ext;
}

QString ExportController::promptSavePath(const QString &title, const QString &filter,
                                         const QString &ext)
{
    QString path = QFileDialog::getSaveFileName(
        m_parent, title, suggestedExportPath(ext), filter);
    if (path.isEmpty())
        return QString();
    if (QFileInfo(path).suffix().isEmpty())
        path += QLatin1Char('.') + ext;
    return path;
}

bool ExportController::writeUtf8File(const QString &path, const QString &contents)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(m_parent, QCoreApplication::translate("MainWindow", "Error"),
                             QCoreApplication::translate("MainWindow", "No se pudo escribir:\n%1\n\n%2")
                                 .arg(path, file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << contents;
    file.close();
    return true;
}

bool ExportController::print()
{
    m_split->commitSourceToDocument();  // en modo fuente, imprime el contenido al día
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, m_parent);
    dialog.setWindowTitle(QCoreApplication::translate("MainWindow", "Imprimir"));
    if (dialog.exec() != QDialog::Accepted)
        return false;
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    flat->print(&printer);
    emit statusMessage(QCoreApplication::translate("MainWindow", "Documento enviado a la impresora."), 4000);
    return true;
}

namespace {
// Documento listo para exportar/imprimir con SOLO la selección del editor, o
// nullptr si no hay selección. Pasa por cloneForExport (limpia las propiedades
// de math, conserva el formato visible).
std::unique_ptr<QTextDocument> selectionExportDocument(QTextEdit *editor)
{
    const QTextCursor cursor = editor->textCursor();
    if (!cursor.hasSelection())
        return nullptr;
    QTextDocument selection;
    QTextCursor into(&selection);
    into.insertFragment(cursor.selection());
    return std::unique_ptr<QTextDocument>(mdexport::cloneForExport(&selection));
}
}  // namespace

bool ExportController::printSelection()
{
    m_split->commitSourceToDocument();
    std::unique_ptr<QTextDocument> doc = selectionExportDocument(m_editor);
    if (!doc) {
        emit statusMessage(QCoreApplication::translate("MainWindow", "No hay texto seleccionado."), 4000);
        return false;
    }
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, m_parent);
    dialog.setWindowTitle(QCoreApplication::translate("MainWindow", "Imprimir selección"));
    if (dialog.exec() != QDialog::Accepted)
        return false;
    doc->print(&printer);
    emit statusMessage(QCoreApplication::translate("MainWindow", "Selección enviada a la impresora."), 4000);
    return true;
}

bool ExportController::exportSelectionPdf()
{
    m_split->commitSourceToDocument();
    std::unique_ptr<QTextDocument> doc = selectionExportDocument(m_editor);
    if (!doc) {
        emit statusMessage(QCoreApplication::translate("MainWindow", "No hay texto seleccionado."), 4000);
        return false;
    }
    const QString path = promptSavePath(
        QCoreApplication::translate("MainWindow", "Exportar selección a PDF"),
        QCoreApplication::translate("MainWindow", "PDF (*.pdf)"), QStringLiteral("pdf"));
    if (path.isEmpty())
        return false;
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    doc->print(&printer);
    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a PDF: %1").arg(path), 4000);
    return true;
}

bool ExportController::printPreview()
{
    m_split->commitSourceToDocument();  // en modo fuente, previsualiza lo último
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog dialog(&printer, m_parent);
    dialog.setWindowTitle(
        QCoreApplication::translate("MainWindow", "Vista previa de impresión"));
    // El render se pide cada vez que la vista lo necesita (zoom, cambio de página…).
    connect(&dialog, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter *p) {
        std::unique_ptr<QTextDocument> flat(
            mdexport::cloneForExport(m_editor->document()));
        flat->print(p);
    });
    return dialog.exec() == QDialog::Accepted;
}

void ExportController::copyHtmlToClipboard()
{
    m_split->commitSourceToDocument();  // en modo fuente, copia lo último
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    auto *mime = new QMimeData;  // el portapapeles toma su propiedad
    mime->setHtml(flat->toHtml());
    mime->setText(flat->toPlainText());  // reserva para destinos sin formato
    QApplication::clipboard()->setMimeData(mime);
    emit statusMessage(
        QCoreApplication::translate("MainWindow", "Copiado como HTML al portapapeles."),
        4000);
}

bool ExportController::exportPdf()
{
    m_split->commitSourceToDocument();
    const QString path = promptSavePath(QCoreApplication::translate("MainWindow", "Exportar a PDF"), QCoreApplication::translate("MainWindow", "PDF (*.pdf)"),
                                        QStringLiteral("pdf"));
    if (path.isEmpty())
        return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    flat->print(&printer);

    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a PDF: %1").arg(path), 4000);
    return true;
}

bool ExportController::exportHtml()
{
    m_split->commitSourceToDocument();
    const QString path = promptSavePath(QCoreApplication::translate("MainWindow", "Exportar a HTML"),
                                        QCoreApplication::translate("MainWindow", "HTML (*.html *.htm)"), QStringLiteral("html"));
    if (path.isEmpty())
        return false;

    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    if (!writeUtf8File(path, flat->toHtml()))
        return false;

    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a HTML: %1").arg(path), 4000);
    return true;
}

bool ExportController::exportOdf()
{
    m_split->commitSourceToDocument();
    mdexport::Language language;
    if (!chooseExportLanguage(&language))
        return false;
    const QString path = promptSavePath(QCoreApplication::translate("MainWindow", "Exportar a ODF"),
                                        QCoreApplication::translate("MainWindow", "Documento ODF (*.odt)"), QStringLiteral("odt"));
    if (path.isEmpty())
        return false;

    QString error;
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    if (!mdexport::writeOdf(flat.get(), path, language, exportTitle(), &error)) {
        QMessageBox::warning(m_parent, QCoreApplication::translate("MainWindow", "Error"),
                             QCoreApplication::translate("MainWindow", "No se pudo exportar a ODF:\n%1\n\n%2").arg(path, error));
        return false;
    }
    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a ODF: %1").arg(path), 4000);
    return true;
}

bool ExportController::exportDocx()
{
    m_split->commitSourceToDocument();
    mdexport::Language language;
    if (!chooseExportLanguage(&language))
        return false;
    const QString path = promptSavePath(
        QCoreApplication::translate("MainWindow", "Exportar a DOCX"),
        QCoreApplication::translate("MainWindow", "Documento Word (*.docx)"),
        QStringLiteral("docx"));
    if (path.isEmpty())
        return false;

    QString error;
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    // El clon no hereda la baseUrl; la copiamos para que se resuelvan las imágenes
    // de ruta relativa al embeberlas en el .docx.
    flat->setBaseUrl(m_editor->document()->baseUrl());
    if (!mdexport::writeDocx(flat.get(), path, language, exportTitle(), &error)) {
        QMessageBox::warning(m_parent, QCoreApplication::translate("MainWindow", "Error"),
                             QCoreApplication::translate("MainWindow",
                                 "No se pudo exportar a DOCX:\n%1\n\n%2").arg(path, error));
        return false;
    }
    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a DOCX: %1").arg(path), 4000);
    return true;
}

bool ExportController::exportLatex()
{
    m_split->commitSourceToDocument();
    mdexport::Language language;
    if (!chooseExportLanguage(&language))
        return false;
    const QString path = promptSavePath(QCoreApplication::translate("MainWindow", "Exportar a LaTeX"),
                                        QCoreApplication::translate("MainWindow", "Documento LaTeX (*.tex)"), QStringLiteral("tex"));
    if (path.isEmpty())
        return false;

    if (!writeUtf8File(path,
                       mdexport::toLatex(m_editor->document(), language, exportTitle())))
        return false;

    emit statusMessage(QCoreApplication::translate("MainWindow", "Exportado a LaTeX: %1").arg(path), 4000);
    return true;
}
