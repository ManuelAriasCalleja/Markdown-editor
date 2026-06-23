// Filtro de eventos y manejo de entrada de MainWindow: el despachador
// eventFilter y sus sub-manejadores (rueda+Ctrl para zoom, arrastrar-soltar,
// enlaces, tareas, notas al pie, menú contextual del corrector, continuación de
// listas en el fuente, protección de fórmulas/shortcodes), más helpers de
// interacción (openLink, goToHeading, footnotes, shortcode). Son métodos de
// MainWindow, separados de mainwindow.cpp para aligerarlo.

#include "mainwindow.h"
#include "editorstack.h"
#include "blockconstructs.h"
#include "footnotes.h"
#include "tasklist.h"
#include "distractionfreecontroller.h"
#include "documentio.h"
#include "exportcontroller.h"
#include "exporters.h"
#include "filecontroller.h"
#include "formatcontroller.h"
#include "formulacontroller.h"
#include "insertcontroller.h"
#include "tablecontroller.h"
#include "findreplacebar.h"
#include "focuseditor.h"
#include "markdownrender.h"
#include "listcontinuation.h"
#include "gotoheadingdialog.h"
#include "diagramcontroller.h"
#include "autopair.h"
#include "splitviewcontroller.h"
#include "mathblocks.h"
#include "outlinepanel.h"
#include "shortcodes.h"
#include "spellcontroller.h"
#include "splitviewcontroller.h"
#include "tableedit.h"
#include "themecontroller.h"
#include <cmath>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLocale>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QResizeEvent>
#include <QToolButton>
#include <QLabel>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextDocumentFragment>
#include <QStatusBar>
#include <QStringList>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QTextFrame>
#include <QTextLength>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QWheelEvent>

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // El documento activo aún puede no estar fijado mientras se construye una
    // pestaña (llegan eventos de layout antes); en ese caso, procesamiento normal.
    if (!m_stack)
        return QMainWindow::eventFilter(watched, event);
    // Despachador: delega en el sub-manejador del objeto vigilado. El primero que
    // consume el evento gana; si ninguno lo hace, sigue el procesamiento normal.
    if (watched == m_stack->editor()->viewport()) {
        if (handleViewportEvent(event))
            return true;
    } else if (watched == m_stack->editor() && event->type() == QEvent::KeyPress) {
        if (handleEditorKeyPress(static_cast<QKeyEvent *>(event)))
            return true;
    }
    // Se comprueba m_stack->split() porque durante su construcción (al reparentar el editor
    // en el QSplitter) ya llegan eventos aquí, antes de que el puntero esté asignado.
    else if (m_stack->split() && watched == m_stack->split()->sourceEditor()
             && event->type() == QEvent::KeyPress) {
        if (handleSourceKeyPress(static_cast<QKeyEvent *>(event)))
            return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

bool MainWindow::handleViewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        return m_stack->spell()->showContextMenu(static_cast<QContextMenuEvent *>(event));
    }
    if (event->type() == QEvent::Wheel) {
        auto *wheel = static_cast<QWheelEvent *>(event);
        if (wheel->modifiers() & Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0)
                zoomInText();
            else if (wheel->angleDelta().y() < 0)
                zoomOutText();
            return true;  // consumimos el evento: no desplazar
        }
    }
    // Arrastrar y soltar un archivo lo abre (en lugar de que el editor lo inserte
    // como texto). El arrastre de texto interno no trae URLs y pasa.
    else if (event->type() == QEvent::DragEnter ||
             event->type() == QEvent::DragMove ||
             event->type() == QEvent::Drop) {
        auto *drop = static_cast<QDropEvent *>(event);
        if (drop->mimeData()->hasUrls()) {
            if (event->type() == QEvent::Drop) {
                const QString path =
                    drop->mimeData()->urls().constFirst().toLocalFile();
                if (!path.isEmpty())
                    openPathInTab(path);  // en una pestaña (como Archivo → Abrir)
            }
            drop->acceptProposedAction();
            return true;
        }
    }
    // Al pasar por encima de un enlace: cursor de mano y pista de cómo abrirlo.
    // (El clic normal coloca el cursor para editar; Ctrl+clic abre.)
    else if (event->type() == QEvent::MouseMove) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->buttons() == Qt::NoButton) {
            // Sobre la casilla de una tarea: cursor de mano y pista de clic.
            if (mdtask::isCheckboxAt(m_stack->editor(), me->position().toPoint())) {
                m_stack->editor()->viewport()->setCursor(Qt::PointingHandCursor);
                statusBar()->showMessage(tr("Clic para marcar o desmarcar la tarea"));
                return true;
            }
            // Sobre una referencia de nota al pie: cursor de mano y pista.
            if (!footnoteRefIdAt(me->position().toPoint()).isEmpty()) {
                m_stack->editor()->viewport()->setCursor(Qt::PointingHandCursor);
                statusBar()->showMessage(tr("Clic para ir a la nota al pie"));
                return true;
            }
            const QString href = m_stack->editor()->anchorAt(me->position().toPoint());
            if (!href.isEmpty()) {
                m_stack->editor()->viewport()->setCursor(Qt::PointingHandCursor);
                statusBar()->showMessage(
                    tr("Ctrl+clic para abrir el enlace: %1").arg(href));
                return true;  // si no, QTextEdit restablecería el cursor a I-beam
            }
            // Acabamos de salir de un enlace: restablece cursor y pista.
            if (m_stack->editor()->viewport()->cursor().shape() == Qt::PointingHandCursor) {
                m_stack->editor()->viewport()->setCursor(Qt::IBeamCursor);
                statusBar()->clearMessage();
            }
        }
    }
    // Doble clic sobre una fórmula renderizada: abre el diálogo de edición con el
    // TeX precargado. Sin esto las fórmulas serían de solo lectura.
    else if (event->type() == QEvent::MouseButtonDblClick) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton
            && m_stack->formula()->editFormulaAt(me->position().toPoint()))
            return true;
    }
    else if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        // Ctrl+clic izquierdo sobre un enlace lo abre en la aplicación externa.
        if (me->button() == Qt::LeftButton
            && (me->modifiers() & Qt::ControlModifier)) {
            const QString href = m_stack->editor()->anchorAt(me->position().toPoint());
            if (!href.isEmpty()) {
                openLink(href);
                return true;  // no mover el cursor de texto
            }
        }
        // Clic izquierdo simple sobre una referencia de nota al pie: salta a su
        // definición al final del documento.
        if (me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier
            && jumpToFootnoteAt(me->position().toPoint()))
            return true;
        // Clic izquierdo simple sobre la casilla de un ítem de tarea: la
        // marca/desmarca (round-trip a `- [x]`/`- [ ]` lo da Qt solo).
        if (me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier
            && mdtask::toggleCheckboxAt(m_stack->editor(), me->position().toPoint()))
            return true;  // consumido: no coloca el cursor ni inicia selección
    }
    return false;
}

bool MainWindow::handleEditorKeyPress(QKeyEvent *ke)
{
    // Interceptación de teclas sobre el editor WYSIWYG para proteger las fórmulas
    // renderizadas frente a edición accidental con el teclado.
    //
    // Antes de un pegado, ajusta la selección para que coja los grupos de math
    // enteros (no a media fórmula). El paste real lo sigue haciendo QTextEdit con
    // la selección ya extendida, así que aquí NO se consume.
    if (ke->matches(QKeySequence::Paste))
        m_stack->formula()->guardPasteAgainstMath();
    if (m_stack->formula()->handleMathKeyPress(ke))
        return true;
    // Shortcodes `:nombre:`: al teclear el ':' de cierre, si delante hay un
    // `:nombre:` conocido se sustituye por su símbolo. Insertamos el ':' y
    // expandimos nosotros (solo en el editor WYSIWYG, solo al teclear).
    if (ke->text() == QStringLiteral(":")
        && !(ke->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
        QTextCursor cursor = m_stack->editor()->textCursor();
        cursor.insertText(QStringLiteral(":"));
        expandShortcodeBefore(cursor);
        return true;
    }
    return applyAutoPair(m_stack->editor(), ke);
}

bool MainWindow::handleSourceKeyPress(QKeyEvent *ke)
{
    // Continuación inteligente de listas en el editor de código fuente.
    const auto mods = ke->modifiers() & ~Qt::KeypadModifier;
    if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
        && mods == Qt::NoModifier && continueSourceList())
        return true;
    return applyAutoPair(m_stack->split()->sourceEditor(), ke);
}

bool MainWindow::applyAutoPair(QTextEdit *ed, QKeyEvent *ke)
{
    // Solo caracteres imprimibles sin modificadores de comando (Shift sí: hace
    // falta para `(`, `{`…). Backspace/Enter/flechas traen texto vacío o de
    // control y `mdautopair::apply` los ignoraría igualmente.
    if (ke->text().isEmpty()
        || (ke->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)))
        return false;
    QTextCursor cursor = ed->textCursor();
    if (!mdautopair::apply(cursor, ke->text().at(0), mdautopair::defaultPairs()))
        return false;
    ed->setTextCursor(cursor);
    return true;
}

bool MainWindow::continueSourceList()
{
    QTextEdit *source = m_stack->split()->sourceEditor();
    QTextCursor cursor = source->textCursor();
    if (cursor.hasSelection())
        return false;  // con selección, Enter la reemplaza: comportamiento normal

    const mdlist::Continuation c = mdlist::analyze(cursor.block().text());
    if (!c.isItem)
        return false;

    cursor.beginEditBlock();
    if (c.empty) {
        // Ítem vacío: se sale de la lista quitando el marcador (la línea queda en
        // blanco), en vez de insertar otro ítem.
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    } else {
        // Nueva línea con la misma sangría y marcador (número incrementado,
        // tarea sin marcar).
        cursor.insertText(QLatin1Char('\n') + c.nextPrefix);
    }
    cursor.endEditBlock();
    source->setTextCursor(cursor);
    return true;
}

void MainWindow::openLink(const QString &href)
{
    if (href.isEmpty())
        return;
    // Los enlaces relativos se resuelven respecto al directorio del documento
    // (la misma baseUrl con la que se cargaron imágenes/enlaces al abrir).
    QUrl url(href);
    if (url.isRelative())
        url = m_stack->editor()->document()->baseUrl().resolved(url);
    // Si el enlace apunta a un archivo Markdown local existente, lo abrimos en una
    // pestaña (como Archivo → Abrir) en vez de delegar en el SO: como md-editor es
    // el manejador por defecto de los .md, QDesktopServices lanzaría otra instancia
    // de la propia aplicación en lugar de reusar esta ventana.
    if (url.isLocalFile()) {
        const QString path = url.toLocalFile();
        const QString suffix = QFileInfo(path).suffix().toLower();
        if ((suffix == QLatin1String("md") || suffix == QLatin1String("markdown"))
            && QFileInfo::exists(path)) {
            openPathInTab(path);
            return;
        }
    }
    QDesktopServices::openUrl(url);
}

void MainWindow::normalizeOutlineWidth()
{
    // El ancho del dock se persiste en windowState y puede volver
    // desproporcionado (p. ej. heredado del modo sin distracciones, que lo
    // ensancha a propósito). Solo aplica fuera de ese modo y con el dock
    // visible; si ocupa más de un tercio de la ventana lo devolvemos a un
    // ancho de lectura cómodo para que el editor no quede en una franja.
    if (!m_outline->isVisible() || (m_distraction && m_distraction->isActive()))
        return;
    constexpr int kNormalOutlineWidth = 280;
    if (m_outline->width() > qMax(kNormalOutlineWidth, width() / 3))
        resizeDocks({m_outline}, {kNormalOutlineWidth}, Qt::Horizontal);
}

void MainWindow::goToHeading()
{
    const QList<OutlineHeading> headings = mdoutline::headingsOf(m_stack->editor()->document());
    if (headings.isEmpty()) {
        showStatusMessage(tr("El documento no tiene encabezados."));
        return;
    }
    GoToHeadingDialog dialog(headings, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const int blockNumber = dialog.selectedBlockNumber();
    const QTextBlock block = m_stack->editor()->document()->findBlockByNumber(blockNumber);
    if (!block.isValid())
        return;
    QTextCursor cursor(block);
    m_stack->editor()->setTextCursor(cursor);
    m_stack->editor()->ensureCursorVisible();
    m_stack->editor()->setFocus();
}

void MainWindow::expandShortcodeBefore(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    const int blockStart = block.position();
    const int end = cursor.position() - blockStart;  // tras el ':' de cierre, en el bloque
    const QString text = block.text();
    if (end < 2 || text.at(end - 1) != QLatin1Char(':'))
        return;
    // Busca el ':' de apertura hacia atrás; el nombre solo admite [A-Za-z0-9_].
    int open = -1;
    for (int i = end - 2; i >= 0; --i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char(':')) {
            open = i;
            break;
        }
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_')))
            return;  // carácter inválido en el nombre: no es un shortcode
    }
    if (open < 0)
        return;
    const QString name = text.mid(open + 1, (end - 1) - (open + 1));
    const QString symbol = mdshortcode::expand(name);
    if (symbol.isEmpty())
        return;
    // Sustituye `:nombre:` (ambos dos puntos incluidos) por el símbolo.
    QTextCursor replace(m_stack->editor()->document());
    replace.setPosition(blockStart + open);
    replace.setPosition(blockStart + end, QTextCursor::KeepAnchor);
    replace.insertText(symbol);
}

QString MainWindow::footnoteRefIdAt(const QPoint &viewportPos) const
{
    const int pos = m_stack->editor()->cursorForPosition(viewportPos).position();
    // charFormat() devuelve el formato del carácter inmediatamente anterior al
    // cursor; probamos pos-1 y pos para cubrir el carácter bajo el clic.
    QTextCursor probe(m_stack->editor()->document());
    probe.setPosition(pos > 0 ? pos - 1 : 0);
    QTextCharFormat cf = probe.charFormat();
    if (!cf.boolProperty(mdfootnote::IsFootnoteRefProperty)) {
        probe.setPosition(pos);
        cf = probe.charFormat();
        if (!cf.boolProperty(mdfootnote::IsFootnoteRefProperty))
            return QString();
    }
    return cf.property(mdfootnote::FootnoteIdProperty).toString();
}

bool MainWindow::jumpToFootnoteAt(const QPoint &viewportPos)
{
    const QString id = footnoteRefIdAt(viewportPos);
    if (id.isEmpty())
        return false;
    const int blockNo = mdfootnote::definitionBlockNumber(m_stack->editor()->document(), id);
    if (blockNo < 0) {
        showStatusMessage(tr("La nota [^%1] no tiene definición").arg(id));
        return false;
    }
    const QTextBlock block = m_stack->editor()->document()->findBlockByNumber(blockNo);
    QTextCursor dest(block);
    dest.movePosition(QTextCursor::EndOfBlock);
    m_stack->editor()->setTextCursor(dest);
    m_stack->editor()->ensureCursorVisible();
    m_stack->editor()->setFocus();
    return true;
}
