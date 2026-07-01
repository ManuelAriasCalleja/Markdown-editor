/// \file
/// \brief Arranque de sesión, reapertura tras cambio de idioma y vigilancia del archivo en disco.

// Sesión y recarga de disco de MainWindow: arranque de sesión (archivo de línea de
// comandos › recuperar borrador › reabrir las pestañas anteriores), reapertura tras
// cambio de idioma, y la vigilancia del archivo en disco (recarga silenciosa o
// pregunta si hay cambios locales). Son métodos de MainWindow, separados de
// mainwindow.cpp para aligerarlo. La gestión de pestañas en sí (addTab/closeTab/
// openPathInTab/setActiveStack) vive en mainwindow.cpp, el shell.

#include "mainwindow.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTextCursor>
#include <QTextEdit>

#include "appsettings.h"
#include "doctemplates.h"
#include "diskwatcher.h"
#include "documentio.h"
#include "editorstack.h"
#include "filecontroller.h"
#include "recentfilesmanager.h"
#include "recoverymanager.h"
#include "splitviewcontroller.h"

void MainWindow::relaunchSession(const QString &reopenPath)
{
    normalizeOutlineWidth();
    // El estado lo decidió la ventana anterior: sin recuperación de borrador. Se
    // reabren todas las pestañas que estaban abiertas (sesión guardada en
    // setLanguage); a falta de sesión, el `reopenPath` suelto (compatibilidad).
    QStringList session = AppSettings::openFiles();
    if (session.isEmpty() && !reopenPath.isEmpty())
        session << reopenPath;
    for (const QString &path : session)
        if (QFileInfo::exists(path))
            openPathInTab(path);
    for (int i = 0; i < m_tabs->count(); ++i)
        if (EditorStack *s = stackAt(i))
            s->file()->startAutosave();
}

void MainWindow::startSession(const QString &cmdLineFile)
{
    // Si el esquema arranca visible, normaliza ya su ancho (con el layout
    // asentado). Si arranca oculto, lo hará el primer F9 (visibilityChanged).
    normalizeOutlineWidth();

    // Prioridad: archivo de la línea de comandos > recuperar borradores > reabrir
    // la sesión. Sin returns prematuros: todas las ramas confluyen en el arranque
    // del autoguardado al final.
    if (!cmdLineFile.isEmpty()) {
        m_stack->file()->openFile(cmdLineFile);
    } else if (const QList<RecoveryManager::Draft> drafts = RecoveryManager::leftoverDrafts();
               !drafts.isEmpty()) {
        // Quedaron borradores de un cierre anómalo (uno por pestaña con cambios sin
        // guardar): ofrecer recuperarlos TODOS, no solo el último.
        QStringList names;
        for (const RecoveryManager::Draft &d : drafts)
            names << (d.originalPath.isEmpty() ? tr("(sin título)")
                                               : QFileInfo(d.originalPath).fileName());
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Recuperar documentos"));
        box.setText(tr("Se encontraron documentos con cambios sin guardar de una "
                       "sesión anterior:\n%1\n\n¿Quieres recuperarlos?")
                        .arg(names.join(QLatin1Char('\n'))));
        QPushButton *recoverBtn = box.addButton(tr("Recuperar"), QMessageBox::AcceptRole);
        box.addButton(tr("Descartar"), QMessageBox::DestructiveRole);
        box.exec();
        if (box.clickedButton() == recoverBtn) {
            // El primero reusa la pestaña inicial; los demás, una pestaña nueva.
            for (int i = 0; i < drafts.size(); ++i) {
                EditorStack *s = (i == 0) ? m_stack : addTab();
                s->file()->loadRecoveredDraft(drafts[i].originalPath, drafts[i].body);
            }
        }
        // Recuperados o descartados, los borradores viejos salen del disco (los
        // recuperados se re-autoguardan ya bajo el slot nuevo de su pestaña).
        for (const RecoveryManager::Draft &d : drafts)
            RecoveryManager::removeDraft(d);
    }

    // Reabrir la sesión de pestañas anterior (documentos guardados). openPathInTab
    // deduplica —si un documento ya está abierto (p. ej. un borrador recuperado con
    // su ruta), salta a él— y reusa la pestaña vacía inicial solo si sigue vacía;
    // así, tras recuperar, se restauran también las pestañas que estaban guardadas.
    if (cmdLineFile.isEmpty()) {
        QStringList session = AppSettings::openFiles();
        if (session.isEmpty()) {
            const QString last = AppSettings::lastFile();  // compatibilidad: una sola
            if (!last.isEmpty())
                session << last;
        }
        for (const QString &path : session)
            if (QFileInfo::exists(path))
                openPathInTab(path);
    }

    // Primer arranque sin nada que abrir (ni archivo, ni borrador, ni sesión previa):
    // muestra el documento de bienvenida en la pestaña inicial vacía, una sola vez. Se
    // carga sin marcar como modificado, para que cerrar no dé la lata.
    if (cmdLineFile.isEmpty() && !AppSettings::welcomeShown() && m_tabs->count() == 1) {
        EditorStack *s = stackAt(0);
        if (s && s->documentIo()->currentFile().isEmpty() && !s->documentIo()->isModified()) {
            s->documentIo()->loadFromString(mdtemplate::welcomeDocument(), false);
            AppSettings::setWelcomeShown(true);
        }
    }

    // Decidida la sesión inicial, ya es seguro autoguardar borradores en cada
    // pestaña (no antes: correría durante el diálogo de recuperación con el
    // documento aún vacío).
    for (int i = 0; i < m_tabs->count(); ++i)
        if (EditorStack *s = stackAt(i))
            s->file()->startAutosave();
}

void MainWindow::onCurrentFileChanged(const QString &path)
{
    auto *stack = qobject_cast<EditorStack *>(sender());

    // Registra el archivo en recientes y recuerda el último real (para reabrirlo al
    // arrancar; sobrevive a un cierre inesperado). addFile/lastFile ignoran la ruta
    // vacía del documento nuevo.
    if (m_recentFiles)
        m_recentFiles->addFile(path);
    if (!path.isEmpty())
        AppSettings::setLastFile(path);

    if (stack)
        updateTabLabel(stack);

    // Título e indicador [*]: solo manda el documento activo (o una llamada directa
    // sin remitente). Acaba de cargarse/guardarse, así que está limpio.
    if (!stack || stack == m_stack) {
        setWindowModified(false);
        const QString shown = path.isEmpty() ? tr("Sin título")
                                             : QFileInfo(path).fileName();
        setWindowTitle(tr("%1[*] — md-editor").arg(shown));
    }
}

void MainWindow::onDiskExternalChange(const QByteArray &diskBytes)
{
    const QString path = m_stack->documentIo()->currentFile();
    const bool locallyModified =
        m_stack->documentIo()->isModified() || m_stack->split()->isSourceDirty();

    if (!locallyModified) {
        // Sin cambios locales: se recarga sin molestar.
        reloadFromDisk();
        showStatusMessage(tr("El archivo cambió en disco: recargado."), 4000);
        return;
    }

    // Hay cambios locales sin guardar: que decida el usuario.
    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Archivo modificado en disco"));
    box.setText(tr("«%1» ha cambiado en disco y tienes cambios sin guardar.")
                    .arg(QFileInfo(path).fileName()));
    box.setInformativeText(
        tr("¿Recargar la versión del disco (perderás tus cambios) o conservar los tuyos?"));
    QPushButton *reloadButton = box.addButton(tr("Recargar"), QMessageBox::AcceptRole);
    QPushButton *keepButton =
        box.addButton(tr("Conservar los míos"), QMessageBox::RejectRole);
    box.setDefaultButton(keepButton);
    // Mientras el diálogo modal está abierto, suspende la comprobación (su exec
    // corre un bucle anidado y el watcher podría volver a dispararse).
    m_stack->diskWatcher()->setSuspended(true);
    box.exec();
    m_stack->diskWatcher()->setSuspended(false);

    if (box.clickedButton() == reloadButton) {
        reloadFromDisk();
    } else {
        // Conserva lo del usuario; recuerda el contenido del disco como referencia
        // para no volver a preguntar por este mismo cambio externo.
        m_stack->diskWatcher()->setSnapshot(diskBytes);
    }
}

void MainWindow::reloadFromDisk()
{
    const QString path = m_stack->documentIo()->currentFile();
    if (path.isEmpty())
        return;

    // Conserva aproximadamente la posición del cursor del editor activo.
    const int caret = m_stack->activeEditor()->textCursor().position();

    QString error;
    if (!m_stack->documentIo()->load(path, &error)) {  // emite currentFileChanged → revigila
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo recargar el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        return;
    }
    // Si el panel de fuente está visible (modo fuente o vista dividida), su texto
    // plano hay que refrescarlo con lo recargado (load() solo toca el documento
    // WYSIWYG).
    m_stack->split()->refreshSourceFromDocument();
    QTextEdit *ed = m_stack->activeEditor();
    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(qMin(caret, ed->document()->characterCount() - 1));
    ed->setTextCursor(cursor);
}

void MainWindow::revertToSaved()
{
    if (!m_stack)
        return;
    const QString path = m_stack->documentIo()->currentFile();
    if (path.isEmpty())
        return;  // sin archivo no hay nada a lo que revertir

    const bool dirty =
        m_stack->documentIo()->isModified() || m_stack->split()->isSourceDirty();
    if (dirty) {
        const auto answer = QMessageBox::question(
            this, tr("Revertir a lo guardado"),
            tr("Se descartarán los cambios sin guardar de «%1» y se recargará la "
               "versión del disco. ¿Continuar?")
                .arg(QFileInfo(path).fileName()),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;
    }
    reloadFromDisk();
    showStatusMessage(tr("Revertido a la versión guardada."), 4000);
}
