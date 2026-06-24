#include "filecontroller.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>

#include "appsettings.h"
#include "documentio.h"
#include "focuseditor.h"
#include "recoverymanager.h"
#include "splitviewcontroller.h"
#include "tableedit.h"

// Los textos visibles conservan el contexto de traducción "MainWindow": se usan
// con QCoreApplication::translate("MainWindow", ...) en vez de tr() para no
// re-hogar en los 8 .ts las cadenas ya traducidas al moverlas desde MainWindow
// (lupdate las atribuye a "MainWindow" al ver el literal). Ver CLAUDE.md.

FileController::FileController(QTextEdit *editor, DocumentIo *documentIo,
                               SplitViewController *split, RecoveryManager *recovery,
                               QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_documentIo(documentIo)
    , m_split(split)
    , m_recovery(recovery)
    , m_parent(parent)
{
    m_autosaveTimer = new QTimer(this);
    m_autosaveTimer->setInterval(5000);
    connect(m_autosaveTimer, &QTimer::timeout, this, &FileController::autosaveDraft);
}

void FileController::startAutosave()
{
    // Lo abierto/recuperado al arrancar no cuenta como cambio pendiente.
    m_autosaveDirty = false;
    m_autosaveTimer->start();
}

QString FileController::currentBody() const
{
    return m_split->isSourceDirty() ? m_split->sourceEditor()->toPlainText()
                                    : mdtable::documentMarkdown(m_editor->document());
}

void FileController::newFile()
{
    m_split->toggleSourceMode(false);  // un documento nuevo se edita en WYSIWYG
    if (!maybeSave())
        return;
    rememberCursorPosition();  // guarda dónde estaba el documento que se reemplaza
    m_documentIo->reset();
}

void FileController::newFromTemplate(const QString &body)
{
    m_split->toggleSourceMode(false);  // la plantilla se edita en WYSIWYG
    if (!maybeSave())
        return;
    rememberCursorPosition();  // guarda dónde estaba el documento que se reemplaza
    m_documentIo->loadFromString(body);
}

void FileController::openFileDialog()
{
    const QString currentFile = m_documentIo->currentFile();
    const QString startDir = currentFile.isEmpty()
        ? QDir::homePath()
        : QFileInfo(currentFile).absolutePath();

    const QString path = QFileDialog::getOpenFileName(
        m_parent,
        QCoreApplication::translate("MainWindow", "Abrir archivo Markdown"),
        startDir,
        QCoreApplication::translate("MainWindow",
            "Archivos Markdown (*.md *.markdown *.mdown *.mkd);;Todos los archivos (*)"));

    if (!path.isEmpty())
        openFile(path);
}

void FileController::openFile(const QString &path)
{
    m_split->toggleSourceMode(false);  // el archivo cargado se muestra en WYSIWYG
    if (!maybeSave())
        return;

    rememberCursorPosition();  // guarda dónde estaba el documento que se reemplaza

    QString error;
    if (!m_documentIo->load(path, &error)) {
        QMessageBox::warning(m_parent, QCoreApplication::translate("MainWindow", "Error"),
                             QCoreApplication::translate("MainWindow",
                                 "No se pudo abrir el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        emit loadFailed(path);  // si venía de recientes y ya no es accesible
        return;
    }
    // El front matter se conserva pero no se muestra: avisamos para que no
    // parezca que se ha perdido.
    if (m_documentIo->hasFrontMatter())
        emit statusMessage(QCoreApplication::translate("MainWindow",
                               "%1 — front matter conservado").arg(path), 0);
    else
        emit statusMessage(path, 0);

    restoreCursorPosition();  // reabre el documento donde se dejó
}

void FileController::openContainingFolder()
{
    const QString file = m_documentIo->currentFile();
    if (file.isEmpty()) {
        QMessageBox::information(
            m_parent,
            QCoreApplication::translate("MainWindow", "Abrir carpeta contenedora"),
            QCoreApplication::translate("MainWindow",
                "Guarda el documento primero para abrir su carpeta."));
        return;
    }
    QDesktopServices::openUrl(
        QUrl::fromLocalFile(QFileInfo(file).absolutePath()));
}

bool FileController::save()
{
    const QString currentFile = m_documentIo->currentFile();
    return currentFile.isEmpty() ? saveAs() : writeToFile(currentFile);
}

bool FileController::saveAs()
{
    const QString currentFile = m_documentIo->currentFile();
    QString path = QFileDialog::getSaveFileName(
        m_parent,
        QCoreApplication::translate("MainWindow", "Guardar como"),
        currentFile.isEmpty() ? QDir::homePath() : currentFile,
        QCoreApplication::translate("MainWindow",
            "Archivos Markdown (*.md *.markdown);;Todos los archivos (*)"));

    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".md");

    return writeToFile(path);
}

bool FileController::writeToFile(const QString &path)
{
    m_split->commitSourceToDocument();  // si estamos en fuente, aplica los cambios primero
    QString error;
    if (!m_documentIo->write(path, &error)) {
        QMessageBox::warning(m_parent, QCoreApplication::translate("MainWindow", "Error"),
                             QCoreApplication::translate("MainWindow",
                                 "No se pudo guardar el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        return false;
    }
    // Guardado en disco: el borrador de recuperación ya no hace falta.
    m_recovery->clearDraft();
    m_autosaveDirty = false;
    emit statusMessage(QCoreApplication::translate("MainWindow", "Guardado: %1").arg(path), 4000);
    return true;
}

bool FileController::maybeSave()
{
    m_split->commitSourceToDocument();  // que isModified vea también los cambios del fuente
    if (!m_documentIo->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        m_parent, QCoreApplication::translate("MainWindow", "md-editor"),
        QCoreApplication::translate("MainWindow",
            "El documento tiene cambios sin guardar.\n¿Quieres guardarlos?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        return true;  // Discard
    }
}

void FileController::autosaveDraft()
{
    if (!m_autosaveDirty)
        return;  // nada cambió desde el último volcado: no reescribir
    m_autosaveDirty = false;

    // Hay cambios sin guardar si el documento difiere de lo último guardado, o si
    // se está editando en modo fuente (cuyos cambios aún no se han volcado).
    const bool modified = m_documentIo->isModified() || m_split->isSourceDirty();
    if (modified)
        m_recovery->saveDraft(m_documentIo->currentFile(), currentBody());
    else
        m_recovery->clearDraft();  // el documento coincide con el disco
}

void FileController::rememberCursorPosition()
{
    const QString file = m_documentIo->currentFile();
    if (!file.isEmpty())
        AppSettings::setCursorPosition(file, m_editor->textCursor().position());
}

void FileController::restoreCursorPosition()
{
    const QString file = m_documentIo->currentFile();
    if (file.isEmpty())
        return;
    const int pos = AppSettings::cursorPosition(file);
    if (pos <= 0)
        return;
    QTextCursor cursor = m_editor->textCursor();
    // characterCount() incluye el carácter final de bloque; el último índice
    // válido es uno menos. Acotamos por si el archivo encogió desde la última vez.
    cursor.setPosition(qMin(pos, m_editor->document()->characterCount() - 1));
    m_editor->setTextCursor(cursor);
    m_editor->ensureCursorVisible();
}

bool FileController::recoverDraft()
{
    if (!m_recovery->hasDraft())
        return false;

    const QString body = m_recovery->draftBody();
    const QString original = m_recovery->draftOriginalPath();

    if (!original.isEmpty() && QFileInfo::exists(original)) {
        // Abre el documento guardado (fija ruta, baseUrl y la línea base) y le
        // superpone el cuerpo del borrador: así queda asociado a su archivo pero
        // marcado como modificado (el borrador difiere de lo guardado).
        openFile(original);
        if (m_renderBody)
            m_renderBody(body);
    } else {
        // Sin archivo asociado (o ya no existe): se recupera como sin título.
        m_split->toggleSourceMode(false);
        m_documentIo->reset();
        if (m_renderBody)
            m_renderBody(body);
    }

    emit windowModifiedChanged(m_documentIo->isModified());
    emit statusMessage(QCoreApplication::translate("MainWindow",
                           "Documento recuperado de la sesión anterior"), 5000);
    return true;
}
