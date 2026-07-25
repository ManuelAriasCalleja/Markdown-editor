/// \file
/// \brief Implementación de InsertController: inserción de enlaces, imágenes, tablas, notas al pie y símbolos.

#include "insertcontroller.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QLocale>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QMimeData>
#include <QPalette>
#include <QPushButton>
#include <QSpinBox>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextTable>
#include <QVBoxLayout>

#include "admonitions.h"
#include "documentio.h"
#include "footnotes.h"
#include "markdownrender.h"
#include "outline.h"
#include "symbolpicker.h"
#include "urldetect.h"

// Los textos visibles de las acciones conservan el contexto de traducción
// "MainWindow" (QCoreApplication::translate) para no re-hogar las cadenas ya
// traducidas en los .ts al moverlas desde MainWindow. La excepción es
// twoFieldDialog, que ya usaba QObject::tr (contexto "QObject"): se conserva tal
// cual. Ver la nota de i18n en CLAUDE.md.

namespace {

// Diálogo de dos campos de texto. Con `browseV2`, el segundo campo lleva un
// botón "..." para elegir un archivo. Devuelve true si el usuario acepta.
bool twoFieldDialog(QWidget *parent, const QString &title,
                    const QString &label1, QString &value1,
                    const QString &label2, QString &value2,
                    bool browseV2 = false)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    auto *form = new QFormLayout(&dialog);

    auto *edit1 = new QLineEdit(value1, &dialog);
    form->addRow(label1, edit1);

    auto *edit2 = new QLineEdit(value2, &dialog);
    if (browseV2) {
        auto *row = new QWidget(&dialog);
        auto *h = new QHBoxLayout(row);
        h->setContentsMargins(0, 0, 0, 0);
        h->addWidget(edit2);
        // Al envolver el campo en un contenedor, addRow asocia la etiqueta al
        // contenedor, no al QLineEdit; le damos nombre accesible para que el lector
        // siga leyendo la etiqueta al enfocar el campo (reusa la cadena, sin nueva).
        edit2->setAccessibleName(label2);
        auto *browse = new QPushButton(QObject::tr("..."), row);
        QObject::connect(browse, &QPushButton::clicked, [parent, edit2] {
            const QString f = QFileDialog::getOpenFileName(
                parent, QObject::tr("Elegir imagen"), QString(),
                QObject::tr("Imágenes (*.png *.jpg *.jpeg *.gif *.bmp *.svg);;Todos (*)"));
            if (!f.isEmpty())
                edit2->setText(f);
        });
        h->addWidget(browse);
        form->addRow(label2, row);
    } else {
        form->addRow(label2, edit2);
    }

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return false;
    value1 = edit1->text();
    value2 = edit2->text();
    return true;
}

}  // namespace

InsertController::InsertController(QTextEdit *editor, DocumentIo *documentIo, QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_documentIo(documentIo)
    , m_parent(parent)
{
}

void InsertController::insertLink()
{
    QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat current = m_editor->currentCharFormat();

    QString text = cursor.selectedText();
    QString url = current.isAnchor() ? current.anchorHref() : QString();

    if (!twoFieldDialog(m_parent, QCoreApplication::translate("MainWindow", "Enlace"),
                        QCoreApplication::translate("MainWindow", "Texto:"), text,
                        QCoreApplication::translate("MainWindow", "URL:"), url))
        return;

    const QPalette pal = m_parent->palette();
    QTextCharFormat fmt;
    if (url.isEmpty()) {
        // Sin URL no se crea ningún enlace: se inserta texto plano (un enlace sin
        // destino, [texto](), no aporta nada y confunde). Si tampoco hay texto,
        // no hay nada que insertar.
        if (text.isEmpty())
            return;
        fmt.setAnchor(false);
        fmt.setAnchorHref(QString());
        fmt.setForeground(pal.color(QPalette::Text));
        fmt.setFontUnderline(false);
        cursor.insertText(text, fmt);
    } else {
        fmt.setAnchor(true);
        fmt.setAnchorHref(url);
        fmt.setForeground(pal.color(QPalette::Link));
        fmt.setFontUnderline(true);
        if (text.isEmpty())
            text = url;
        // insertText reemplaza la selección si la hay.
        cursor.insertText(text, fmt);
    }
    m_editor->setFocus();
    emit formatActionsShouldRefresh();
}

void InsertController::insertImage()
{
    QString alt = QCoreApplication::translate("MainWindow", "imagen");
    QString path;
    if (!twoFieldDialog(m_parent, QCoreApplication::translate("MainWindow", "Insertar imagen"),
                        QCoreApplication::translate("MainWindow", "Texto alternativo:"), alt,
                        QCoreApplication::translate("MainWindow", "Ruta o URL:"), path,
                        /*browseV2=*/true))
        return;
    if (path.isEmpty())
        return;

    // Si el documento está guardado y la imagen es local, usar ruta relativa
    // para que el Markdown sea portable.
    const QString currentFile = m_documentIo->currentFile();
    if (!currentFile.isEmpty() && !path.contains(QLatin1String("://"))) {
        const QDir docDir(QFileInfo(currentFile).absolutePath());
        const QString rel = docDir.relativeFilePath(path);
        if (!rel.startsWith(QLatin1String("..")))
            path = rel;
    }

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    // mdrender::imageMarkdown y no un `![%1](%2)` a mano: si el usuario deja vacío
    // el texto alternativo, `setMarkdown` descartaría la imagen al reabrir el
    // documento (y una ruta con espacios cortaría el enlace).
    cursor.insertFragment(
        QTextDocumentFragment::fromMarkdown(mdrender::imageMarkdown(path, alt)));
    cursor.endEditBlock();
    m_editor->setFocus();
}

void InsertController::pasteImageFromClipboard()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime || !handlePastedImage(mime))
        QMessageBox::information(
            m_parent, QCoreApplication::translate("MainWindow", "Pegar imagen"),
            QCoreApplication::translate("MainWindow",
                "El portapapeles no contiene ninguna imagen."));
}

bool InsertController::handlePastedImage(const QMimeData *source)
{
    if (!source || !source->hasImage())
        return false;
    const QImage image = qvariant_cast<QImage>(source->imageData());
    if (image.isNull())
        return false;

    // Texto alternativo de la imagen (puede dejarse vacío). Si se cancela, no se
    // inserta nada pero el evento se considera gestionado (no incrustar la imagen).
    bool ok = false;
    const QString alt = QInputDialog::getText(
        m_parent, QCoreApplication::translate("MainWindow", "Pegar imagen"),
        QCoreApplication::translate("MainWindow", "Texto alternativo:"),
        QLineEdit::Normal, QCoreApplication::translate("MainWindow", "imagen pegada"), &ok);
    if (!ok)
        return true;

    // Nombre con marca de tiempo para evitar colisiones; si aun así existe (mismo
    // segundo), se añade un sufijo incremental.
    const QString stamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const auto fileNameFor = [&stamp](int n) {
        return n == 0 ? QStringLiteral("imagen-%1.png").arg(stamp)
                      : QStringLiteral("imagen-%1-%2.png").arg(stamp).arg(n);
    };

    QString savePath;  // ruta absoluta donde se escribe el PNG
    QString mdPath;    // lo que va dentro de ![](...)

    const QString currentFile = m_documentIo->currentFile();
    if (!currentFile.isEmpty()) {
        // Documento guardado: junto al .md, con ruta relativa para que sea
        // portable (igual criterio que insertImage()).
        const QDir dir(QFileInfo(currentFile).absolutePath());
        int n = 0;
        do {
            savePath = dir.filePath(fileNameFor(n++));
        } while (QFileInfo::exists(savePath));
        mdPath = dir.relativeFilePath(savePath);
    } else {
        // Documento sin guardar: no hay carpeta de referencia; se pide dónde
        // guardarla y se inserta la ruta elegida.
        savePath = QFileDialog::getSaveFileName(
            m_parent, QCoreApplication::translate("MainWindow", "Guardar imagen pegada"),
            QDir::home().filePath(fileNameFor(0)),
            QCoreApplication::translate("MainWindow", "Imagen PNG (*.png)"));
        if (savePath.isEmpty())
            return true;  // cancelado: gestionado (no incrustar la imagen)
        mdPath = savePath;
    }

    if (!image.save(savePath, "PNG")) {
        QMessageBox::warning(
            m_parent, QCoreApplication::translate("MainWindow", "Pegar imagen"),
            QCoreApplication::translate("MainWindow",
                "No se pudo guardar la imagen en «%1».").arg(savePath));
        return true;
    }

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(
        QTextDocumentFragment::fromMarkdown(mdrender::imageMarkdown(mdPath, alt)));
    cursor.endEditBlock();
    m_editor->setFocus();
    return true;
}

void InsertController::insertTable()
{
    // Las tablas no se anidan: insertar una dentro de la celda de otra crea una
    // QTextTable anidada que corrompe la serialización Markdown de AMBAS. Si el
    // cursor está dentro de una tabla, se avisa y no se inserta.
    if (m_editor->textCursor().currentTable()) {
        QMessageBox::information(
            m_parent, QCoreApplication::translate("MainWindow", "Insertar tabla"),
            QCoreApplication::translate("MainWindow",
                "No se puede insertar una tabla dentro de otra. Coloca el cursor "
                "fuera de la tabla."));
        return;
    }

    // Un solo diálogo con columnas y filas a la vez (más legible que dos seguidos).
    QDialog dlg(m_parent);
    dlg.setWindowTitle(QCoreApplication::translate("MainWindow", "Insertar tabla"));
    auto *colsSpin = new QSpinBox(&dlg);
    colsSpin->setRange(1, 20);
    colsSpin->setValue(2);
    auto *rowsSpin = new QSpinBox(&dlg);
    rowsSpin->setRange(1, 100);
    rowsSpin->setValue(2);
    auto *form = new QFormLayout;
    form->addRow(QCoreApplication::translate("MainWindow", "Columnas:"), colsSpin);
    form->addRow(QCoreApplication::translate("MainWindow", "Filas de datos:"), rowsSpin);
    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    auto *layout = new QVBoxLayout(&dlg);
    layout->addLayout(form);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const int cols = colsSpin->value();
    const int rows = rowsSpin->value();

    QString header = QStringLiteral("|");
    QString separator = QStringLiteral("|");
    for (int c = 0; c < cols; ++c) {
        header += QStringLiteral("   |");
        separator += QStringLiteral("---|");
    }
    QString row = QStringLiteral("|");
    for (int c = 0; c < cols; ++c)
        row += QStringLiteral("   |");

    QString md = header + QLatin1Char('\n') + separator + QLatin1Char('\n');
    for (int r = 0; r < rows; ++r)
        md += row + QLatin1Char('\n');

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(QTextDocumentFragment::fromMarkdown(md));
    cursor.endEditBlock();
    emit tableInserted();  // que la tabla recién creada muestre sus bordes
    m_editor->setFocus();
}

void InsertController::insertTableOfContents()
{
    const QList<OutlineHeading> headings = mdoutline::headingsOf(m_editor->document());
    if (headings.isEmpty()) {
        QMessageBox::information(
            m_parent, QCoreApplication::translate("MainWindow", "Insertar índice"),
            QCoreApplication::translate("MainWindow",
                "El documento no tiene encabezados."));
        return;
    }

    const QString md = mdoutline::tableOfContentsMarkdown(headings);
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(QTextDocumentFragment::fromMarkdown(md));
    cursor.endEditBlock();
    m_editor->setFocus();
}

void InsertController::insertFootnote()
{
    QTextDocument *doc = m_editor->document();
    // El id se calcula sobre el texto plano: las referencias `[^id]` siguen ahí
    // literalmente (el superíndice no cambia el texto), así que nextId las ve.
    const QString id = mdfootnote::nextId(doc->toPlainText());

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();

    // 1) Referencia `[^id]` en el cursor, con estilo de superíndice. Con una
    //    selección activa, insertText reemplaza desde selectionStart, así que el
    //    inicio real de la referencia es ahí (no position(), que puede ser el fin
    //    de la selección: el formato caería sobre el texto siguiente).
    const int refStart = cursor.selectionStart();
    const QString ref = QStringLiteral("[^%1]").arg(id);
    cursor.insertText(ref);
    QTextCursor refFmt(doc);
    refFmt.setPosition(refStart);
    refFmt.setPosition(refStart + ref.length(), QTextCursor::KeepAnchor);
    refFmt.mergeCharFormat(mdfootnote::refFormat(id));

    // 2) Definición `[^id]: ` en un bloque nuevo al final del documento, con
    //    formato limpio (ni superíndice ni el del bloque anterior).
    QTextCursor end(doc);
    end.movePosition(QTextCursor::End);
    end.insertBlock(QTextBlockFormat(), QTextCharFormat());
    end.insertText(QStringLiteral("[^%1]: ").arg(id));
    const int defPos = end.position();
    cursor.endEditBlock();

    // 3) Lleva el cursor a la definición para teclear la nota.
    QTextCursor place(doc);
    place.setPosition(defPos);
    m_editor->setTextCursor(place);
    m_editor->setFocus();
}

void InsertController::insertAdmonition(const QString &keyword)
{
    // Formato de cita canónico (el mismo que produce Qt para "> x"), para que la
    // admonición serialice como cita y round-tripee.
    static const QTextBlockFormat quoteFmt = [] {
        QTextDocument tmp;
        tmp.setMarkdown(QStringLiteral("> x"));
        return tmp.firstBlock().blockFormat();
    }();

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    // La admonición empieza en su propio bloque: si el actual tiene texto, abre uno.
    if (!cursor.block().text().isEmpty()) {
        cursor.movePosition(QTextCursor::EndOfBlock);
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }
    cursor.setBlockFormat(quoteFmt);
    cursor.setCharFormat(QTextCharFormat());
    cursor.insertText(QStringLiteral("[!%1]").arg(keyword));  // marcador literal
    cursor.insertBlock(quoteFmt, QTextCharFormat());          // línea de contenido (cita)
    const int contentPos = cursor.position();
    cursor.endEditBlock();

    // Estiliza el callout y deja el cursor en la línea de contenido.
    mdadmonition::renderAdmonitionsInDocument(m_editor->document());
    QTextCursor place(m_editor->document());
    place.setPosition(contentPos);
    m_editor->setTextCursor(place);
    m_editor->setFocus();
}

bool InsertController::handlePastedUrl(const QMimeData *source)
{
    if (!source || !source->hasText())
        return false;
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.hasSelection())
        return false;
    const QString selected = cursor.selectedText();
    // Selección multibloque: el separador de párrafo (U+2029) no cabe en el texto
    // del enlace; se deja el pegado normal.
    if (selected.contains(QChar::ParagraphSeparator))
        return false;
    const QString url = source->text().trimmed();
    if (!mdurl::looksLikeUrl(url))
        return false;
    cursor.insertText(QStringLiteral("[%1](%2)").arg(selected, url));
    return true;
}

void InsertController::insertDate()
{
    m_editor->insertPlainText(
        QLocale::system().toString(QDate::currentDate(), QLocale::LongFormat));
    m_editor->setFocus();
}

void InsertController::insertDateTime()
{
    const QLocale loc = QLocale::system();
    const QDateTime now = QDateTime::currentDateTime();
    m_editor->insertPlainText(loc.toString(now.date(), QLocale::LongFormat)
                              + QStringLiteral(", ")
                              + loc.toString(now.time(), QLocale::ShortFormat));
    m_editor->setFocus();
}

void InsertController::insertSymbol()
{
    // Diálogo no modal y reutilizado: se queda abierto para insertar varios
    // símbolos seguidos, cada uno en la posición actual del cursor.
    if (!m_symbolPicker) {
        m_symbolPicker = new SymbolPicker(m_parent);
        connect(m_symbolPicker, &SymbolPicker::symbolChosen, this,
                [this](const QString &symbol) { m_editor->insertPlainText(symbol); });
    }
    m_symbolPicker->show();
    m_symbolPicker->raise();
    m_symbolPicker->activateWindow();
}

void InsertController::insertHorizontalRule()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertBlock();
    // La regla horizontal es un bloque con esta propiedad (lo que produce el
    // lector de Markdown de Qt y exporta como '- - -').
    QTextBlockFormat hr;
    hr.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth,
                   QTextLength(QTextLength::PercentageLength, 100));
    cursor.setBlockFormat(hr);
    // Bloque normal a continuación para seguir escribiendo.
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    cursor.endEditBlock();
    m_editor->setFocus();
}
