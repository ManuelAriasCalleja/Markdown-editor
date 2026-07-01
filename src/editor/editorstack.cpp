/// \file
/// \brief Implementación de EditorStack: cableado del editor y sus colaboradores por documento.

#include "editorstack.h"

#include <QColor>
#include <QMimeData>
#include <QPalette>
#include <QSplitter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextFragment>
#include <QTextFrame>
#include <QTextTable>
#include <QVBoxLayout>

#include "codehighlighter.h"
#include "diagramcontroller.h"
#include "diskwatcher.h"
#include "documentio.h"
#include "exportcontroller.h"
#include "filecontroller.h"
#include "focuseditor.h"
#include "formatcontroller.h"
#include "formulacontroller.h"
#include "insertcontroller.h"
#include "linehighlight.h"
#include "markdownrender.h"
#include "markdowntidy.h"
#include "mathblocks.h"
#include "tableedit.h"
#include "typography.h"
#include "typewriter.h"
#include "outlinepanel.h"
#include "recoverymanager.h"
#include "spellcontroller.h"
#include "splitviewcontroller.h"
#include "tablecontroller.h"
#include "themecontroller.h"

EditorStack::EditorStack(FindReplaceBar *findBar, OutlinePanel *outline,
                         ThemeController *theme, QWidget *parent)
    : QWidget(parent)
    , m_theme(theme)
    , m_findBar(findBar)
    , m_outline(outline)
{
    m_editor = new FocusEditor(this);
    m_editor->setAcceptRichText(true);
    // Nombre accesible (lectores de pantalla): distingue este editor visual del de
    // código fuente, que comparten clase y son visibles a la vez en vista dividida.
    m_editor->setAccessibleName(tr("Editor del documento"));
    // La descripción aclara que el nombre no transmite: es un editor WYSIWYG.
    m_editor->setAccessibleDescription(
        tr("Editor visual: el formato se aplica sobre el texto renderizado, sin ver "
           "la sintaxis Markdown."));
    // Pista sutil en el documento vacío (solo se ve cuando no hay texto).
    m_editor->setPlaceholderText(
        tr("Empieza a escribir. Da formato con la barra o tecleando Markdown."));
    // Resaltado de sintaxis de los bloques de código.
    m_highlighter = new CodeBlockHighlighter(m_editor->document());

    // Previsualización de diagramas (Mermaid/PlantUML) bajo cada bloque de código.
    m_diagrams = new DiagramController(m_editor, this);
    connect(m_editor->document(), &QTextDocument::contentsChanged,
            m_diagrams, &DiagramController::scheduleRefresh);
    // Los fallos de render «asentados» avisan al usuario en la barra de estado.
    connect(m_diagrams, &DiagramController::statusMessage, this, &EditorStack::statusMessage);

    // E/S del documento. El control del tema es ÚNICO de la ventana (lo pasa
    // MainWindow); aquí solo se guarda el puntero —no se posee— y se reapunta a este
    // editor cuando la pestaña se activa (ThemeController::retarget).
    m_documentIo = new DocumentIo(m_editor, this);
    // El color de atenuación del foco de línea se deriva de la paleta; al cambiar
    // de tema hay que rehacerlo (si no, se quedaría con el color del tema anterior).
    connect(m_theme, &ThemeController::themeChanged, this, [this] {
        rebuildExtraSelections(m_editor);
        rebuildExtraSelections(m_split->sourceEditor());
    });

    // Corrector ortográfico: posee el motor (lo enchufa al highlighter) y el menú
    // contextual de sugerencias.
    m_spell = new SpellController(m_editor, m_highlighter, m_documentIo, this);
    connect(m_spell, &SpellController::statusMessage, this, &EditorStack::statusMessage);

    // Al pegar/soltar una imagen, guardarla a disco e insertar `![](ruta)` en vez
    // de incrustarla (el lambda consulta m_insert en tiempo de pegado).
    m_editor->setMimeInsertHandler([this](const QMimeData *src) {
        return m_insert->handlePastedImage(src) || m_insert->handlePastedUrl(src);
    });

    // Vista de código fuente / dividida (posee el editor de fuente y el QSplitter
    // central). Usa la barra de búsqueda y el esquema compartidos de la ventana.
    m_split = new SplitViewController(m_editor, m_findBar, m_outline, m_theme, this);
    connect(m_split->sourceEditor(), &QTextEdit::cursorPositionChanged,
            this, &EditorStack::wordCountShouldUpdate);
    connect(m_split->sourceEditor(), &QTextEdit::cursorPositionChanged,
            this, &EditorStack::cursorMoved);
    connect(m_split->sourceEditor(), &QTextEdit::cursorPositionChanged,
            this, [this] { centerCursorLine(m_split->sourceEditor());
                           rebuildExtraSelections(m_split->sourceEditor()); });
    connect(m_split->sourceEditor(), &QTextEdit::selectionChanged,
            this, &EditorStack::wordCountShouldUpdate);
    connect(m_split, &SplitViewController::wordCountShouldUpdate,
            this, &EditorStack::wordCountShouldUpdate);
    connect(m_split, &SplitViewController::formatActionsShouldUpdate,
            this, [this] { m_format->updateActions(); });
    connect(m_split, &SplitViewController::documentModified,
            this, [this] { emit windowModifiedChanged(true); });
    m_split->setRenderBody([this](const QString &body) { setBodyMarkdown(body); });

    // La vista (QSplitter con WYSIWYG + fuente) es el contenido de este widget.
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_split->splitView());

    // Comandos de formato y sincronización de sus acciones con el cursor.
    m_format = new FormatController(m_editor, m_highlighter, this);
    connect(m_format, &FormatController::statusMessage, this, &EditorStack::statusMessage);

    // Edición de tablas (contextual).
    m_table = new TableController(m_editor, m_split, m_documentIo, this);
    connect(m_split, &SplitViewController::tableActionsShouldUpdate,
            m_table, &TableController::updateActions);
    connect(m_table, &TableController::modifiedChanged,
            this, &EditorStack::windowModifiedChanged);
    connect(m_format, &FormatController::actionsUpdated,
            m_table, &TableController::updateActions);

    // Fórmulas TeX (insertar/editar/proteger).
    m_formula = new FormulaController(m_editor, this);
    connect(m_formula, &FormulaController::statusMessage, this, &EditorStack::statusMessage);

    // Inserción (enlaces, imágenes, tablas, regla).
    m_insert = new InsertController(m_editor, m_documentIo, this);
    connect(m_insert, &InsertController::formatActionsShouldRefresh,
            m_format, &FormatController::updateActions);
    connect(m_insert, &InsertController::tableInserted, this, [this] { styleTables(); });

    // Exportación e impresión.
    m_export = new ExportController(m_editor, m_documentIo, m_split, this);
    connect(m_export, &ExportController::statusMessage, this, &EditorStack::statusMessage);

    // Operaciones de archivo + autoguardado del borrador.
    m_recovery = new RecoveryManager(this);
    m_file = new FileController(m_editor, m_documentIo, m_split, m_recovery, this);
    m_file->setRenderBody([this](const QString &body) { setBodyMarkdown(body); });
    connect(m_file, &FileController::statusMessage, this, &EditorStack::statusMessage);
    connect(m_file, &FileController::windowModifiedChanged,
            this, &EditorStack::windowModifiedChanged);
    connect(m_file, &FileController::loadFailed, this, &EditorStack::loadFailed);
    // Escribir en cualquiera de los dos editores marca el borrador como pendiente.
    connect(m_editor, &QTextEdit::textChanged, m_file, &FileController::markDirty);
    connect(m_split->sourceEditor(), &QTextEdit::textChanged, m_file, &FileController::markDirty);

    // Los botones de formato reflejan lo que hay bajo el cursor.
    connect(m_editor, &QTextEdit::currentCharFormatChanged,
            this, [this] { m_format->updateActions(); });
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, [this] { m_format->updateActions(); });
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, &EditorStack::announceFormulaUnderCursor);
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, &EditorStack::cursorMoved);
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, [this] { centerCursorLine(m_editor); rebuildExtraSelections(m_editor); });
    // Marca «modificado» comparando con la línea base (DocumentIo).
    connect(m_editor->document(), &QTextDocument::contentsChanged, this,
            [this] { emit windowModifiedChanged(m_documentIo->isModified()); });
    // Contador al día con el contenido y la selección.
    connect(m_editor, &QTextEdit::textChanged, this, &EditorStack::wordCountShouldUpdate);
    connect(m_editor, &QTextEdit::selectionChanged, this, &EditorStack::wordCountShouldUpdate);

    // El archivo actual sube a la ventana (título + recientes + vigilancia).
    connect(m_documentIo, &DocumentIo::currentFileChanged,
            this, &EditorStack::currentFileChanged);

    // Vigilancia de cambios externos del archivo abierto.
    m_diskWatcher = new DiskWatcher(this);
    connect(m_documentIo, &DocumentIo::currentFileChanged,
            m_diskWatcher, &DiskWatcher::watch);
    connect(m_diskWatcher, &DiskWatcher::externalChange,
            this, &EditorStack::diskExternalChange);
    connect(m_diskWatcher, &DiskWatcher::vanished, this, &EditorStack::diskVanished);

    // Al cargar: recolorear enlaces, avisar a la ventana (reconstruye el esquema),
    // dar borde a las tablas, aplicar interlineado y tipografía, y reajustar el idioma
    // del corrector. El orden importa (recolor › esquema › tablas › interlineado ›
    // tipografía › corrector), por eso van en esta secuencia.
    connect(m_documentIo, &DocumentIo::documentLoaded, this,
            [this] { m_theme->recolorLinks(m_editor); });
    connect(m_documentIo, &DocumentIo::documentLoaded, this, &EditorStack::documentLoaded);
    connect(m_documentIo, &DocumentIo::documentLoaded, this, [this] { styleTables(); });
    connect(m_documentIo, &DocumentIo::documentLoaded, this,
            [this] { applyLineSpacing(m_editor); });
    connect(m_documentIo, &DocumentIo::documentLoaded, this,
            [this] { mdtypography::apply(m_editor->document()); });
    connect(m_documentIo, &DocumentIo::documentLoaded,
            m_spell, &SpellController::applyLanguage);
    // Tras cargar, reaplica el foco de línea: setTypewriterMode pudo correr con el
    // documento aún vacío (al configurar la pestaña), antes de tener contenido.
    connect(m_documentIo, &DocumentIo::documentLoaded, this, [this] {
        rebuildExtraSelections(m_editor);
        rebuildExtraSelections(m_split->sourceEditor());
    });
}

QTextEdit *EditorStack::activeEditor() const
{
    return m_split->activeEditor();
}

void EditorStack::styleTables()
{
    QTextDocument *doc = m_editor->document();
    const QColor borderColor = m_editor->palette().color(QPalette::Mid);

    // El borde es presentación pura: no lo serializa toMarkdown(), así que ni el
    // round-trip ni el estado «modificado» se ven afectados. Aun así preservamos
    // la marca de modificado de Qt, como hace recolorLinks().
    const bool wasModified = doc->isModified();
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    const QList<QTextFrame *> frames = doc->rootFrame()->childFrames();
    for (QTextFrame *frame : frames) {
        auto *table = qobject_cast<QTextTable *>(frame);
        if (!table)
            continue;
        QTextTableFormat fmt = table->format();
        fmt.setBorder(1);
        fmt.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        fmt.setBorderBrush(borderColor);
        fmt.setBorderCollapse(true);  // un solo trazo entre celdas, no doble
        fmt.setCellPadding(4);
        fmt.setCellSpacing(0);
        table->setFormat(fmt);
    }
    cursor.endEditBlock();
    doc->setModified(wasModified);
}

void EditorStack::applyLineSpacing(QTextEdit *ed)
{
    QTextDocument *doc = ed->document();
    // Presentación pura: el interlineado no lo serializa toMarkdown(), así que ni
    // el round-trip ni «modificado» se ven afectados. Preservamos la marca de Qt
    // como en styleTables()/recolorLinks().
    const bool wasModified = doc->isModified();
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    QTextBlockFormat fmt;
    fmt.setLineHeight(m_lineSpacing, QTextBlockFormat::ProportionalHeight);
    cursor.select(QTextCursor::Document);
    cursor.mergeBlockFormat(fmt);
    cursor.endEditBlock();
    doc->setModified(wasModified);
}

void EditorStack::setLineSpacing(int percent)
{
    // Solo el editor WYSIWYG: el de fuente repuebla su texto plano en cada
    // sincronización de la vista dividida y perdería el formato de bloque.
    m_lineSpacing = percent;
    applyLineSpacing(m_editor);
}

void EditorStack::setBodyMarkdown(const QString &body)
{
    // El flag anti-bucle del controlador envuelve la sustitución para que los
    // contentsChanged que provoca (incluido el de recolorLinks) no realimenten la
    // sincronización de la vista dividida. Se guarda/restaura por reentrancia.
    const bool wasSyncing = m_split->beginProgrammaticChange();
    // Suspendemos el repintado mientras se reconstruye: setMarkdown, las pasadas
    // de render, el estilo de tablas y el recoloreado de enlaces repintarían cada
    // uno por separado (parpadeo al volver al WYSIWYG desde la vista de fuente).
    const bool wasUpdating = m_editor->updatesEnabled();
    m_editor->setUpdatesEnabled(false);
    mdrender::setMarkdownWithExtensions(m_editor, body);
    styleTables();
    applyLineSpacing(m_editor);
    mdtypography::apply(m_editor->document());
    m_theme->recolorLinks(m_editor);
    m_editor->setUpdatesEnabled(wasUpdating);
    m_outline->rebuild(m_editor->document());
    m_split->endProgrammaticChange(wasSyncing);
}

QString EditorStack::formulaAtCursor(int *start) const
{
    QTextDocument *doc = m_editor->document();
    const int pos = m_editor->textCursor().position();
    // Formato del carácter que empieza en `p` (el que hay a la derecha de esa
    // posición). Todos los caracteres de un mismo fragmento comparten formato.
    const auto fmtAt = [doc](int p) -> QTextCharFormat {
        if (p < 0)
            return {};
        const QTextBlock b = doc->findBlock(p);
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment f = it.fragment();
            if (f.isValid() && p >= f.position() && p < f.position() + f.length())
                return f.charFormat();
        }
        return {};
    };
    // El cursor está «sobre» la fórmula si la toca por cualquiera de sus bordes:
    // mira el carácter a la derecha (pos) y, si no, el de la izquierda (pos-1).
    int anchor = pos;
    QTextCharFormat f = fmtAt(pos);
    if (!f.property(mdmath::IsMathProperty).toBool()) {
        anchor = pos - 1;
        f = fmtAt(anchor);
    }
    if (!f.property(mdmath::IsMathProperty).toBool())
        return QString();
    const QString tex = f.property(mdmath::MathTexProperty).toString();
    // Retrocede hasta el inicio del grupo (mismos IsMath + MathTex) para tener un
    // ancla estable con la que deduplicar el anuncio.
    int s = anchor;
    while (s > 0) {
        const QTextCharFormat pf = fmtAt(s - 1);
        if (!pf.property(mdmath::IsMathProperty).toBool()
            || pf.property(mdmath::MathTexProperty).toString() != tex)
            break;
        --s;
    }
    if (start)
        *start = s;
    return tex;
}

QList<QTextEdit::ExtraSelection> EditorStack::focusDimSelections(QTextEdit *ed) const
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!ed || !m_typewriter)
        return selections;

    // Color apagado: el texto mezclado a medias con el fondo del editor. Funciona
    // igual en temas claros y oscuros (acerca el texto al fondo, sin fijar gris).
    const QColor text = ed->palette().color(QPalette::Text);
    const QColor base = ed->palette().color(QPalette::Base);
    const QColor dim(
        (text.red() + base.red()) / 2,
        (text.green() + base.green()) / 2,
        (text.blue() + base.blue()) / 2);

    const QTextCursor cur = ed->textCursor();
    const QTextBlock block = cur.block();
    // El total es la última posición de cursor VÁLIDA: characterCount() incluye el
    // separador final de bloque, pero setPosition(characterCount()) está fuera de
    // rango (la posición máxima es characterCount()-1). Pasarlo entero dejaría el
    // último tramo con un setPosition fallido → selección vacía → sin atenuar.
    const int lastPos = ed->document()->characterCount() - 1;
    for (const mdtypewriter::Range &r :
         mdtypewriter::dimRanges(lastPos,
                                 block.position(), block.position() + block.length())) {
        QTextEdit::ExtraSelection sel;
        sel.cursor = QTextCursor(ed->document());
        sel.cursor.setPosition(r.start);
        sel.cursor.setPosition(r.start + r.length, QTextCursor::KeepAnchor);
        sel.format.setForeground(dim);
        selections.append(sel);
    }
    return selections;
}

QList<QTextEdit::ExtraSelection> EditorStack::currentLineSelections(QTextEdit *ed) const
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!ed || !m_currentLineHighlight)
        return selections;
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(mdlinehighlight::currentLineColor(
        ed->palette().color(QPalette::Base), ed->palette().color(QPalette::Highlight)));
    // FullWidthSelection pinta el fondo de toda la línea; requiere que el cursor no
    // tenga selección propia.
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.cursor = ed->textCursor();
    sel.cursor.clearSelection();
    selections.append(sel);
    return selections;
}

QList<QTextEdit::ExtraSelection> EditorStack::matchSelections(QTextEdit *ed) const
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!ed || ed != m_matchEditor || m_searchMatches.isEmpty())
        return selections;
    // Fondo translúcido (el texto se sigue leyendo) derivado del color de selección.
    QColor bg = ed->palette().color(QPalette::Highlight);
    bg.setAlpha(90);
    QTextDocument *doc = ed->document();
    const int lastPos = doc->characterCount() - 1;
    for (const mdfind::Match &m : m_searchMatches) {
        if (m.start < 0 || m.length <= 0 || m.start + m.length > lastPos)
            continue;  // defensivo: el documento pudo cambiar desde el cálculo
        QTextEdit::ExtraSelection sel;
        sel.cursor = QTextCursor(doc);
        sel.cursor.setPosition(m.start);
        sel.cursor.setPosition(m.start + m.length, QTextCursor::KeepAnchor);
        sel.format.setBackground(bg);
        selections.append(sel);
    }
    return selections;
}

void EditorStack::rebuildExtraSelections(QTextEdit *ed)
{
    if (!ed)
        return;
    // Fusiona todas las capas y aplica en una sola llamada (este es el único
    // setExtraSelections del componente). El orden importa cuando dos capas se
    // solapan: las posteriores pintan encima. La línea actual va primero (fondo);
    // las coincidencias encima; la atenuación del foco (foreground) al final.
    QList<QTextEdit::ExtraSelection> selections;
    selections += currentLineSelections(ed);
    selections += matchSelections(ed);
    selections += focusDimSelections(ed);
    ed->setExtraSelections(selections);
}

void EditorStack::setTypewriterMode(bool on)
{
    m_typewriter = on;
    centerCursorLine(activeEditor());  // centra ya, sin esperar a moverse
    // Aplica/limpia la atenuación en ambos editores (cualquiera puede estar visible
    // en la vista dividida).
    rebuildExtraSelections(m_editor);
    rebuildExtraSelections(m_split->sourceEditor());
}

void EditorStack::setCurrentLineHighlight(bool on)
{
    m_currentLineHighlight = on;
    // Recompone las capas en ambos editores (cualquiera puede estar visible).
    rebuildExtraSelections(m_editor);
    rebuildExtraSelections(m_split->sourceEditor());
}

void EditorStack::setSearchMatches(const QList<mdfind::Match> &matches)
{
    m_searchMatches = matches;
    // Las posiciones son del editor activo (donde busca la barra); solo ahí se pintan.
    m_matchEditor = activeEditor();
    rebuildExtraSelections(m_editor);
    rebuildExtraSelections(m_split->sourceEditor());
}

void EditorStack::cleanMarkdown()
{
    QTextEdit *active = activeEditor();
    if (active == m_split->sourceEditor()) {
        const QString cleaned = mdtidy::tidy(active->toPlainText());
        if (cleaned == active->toPlainText())
            return;
        QTextCursor c = active->textCursor();
        c.beginEditBlock();
        c.select(QTextCursor::Document);
        c.insertText(cleaned);
        c.endEditBlock();
    } else {
        const QString md = mdtable::documentMarkdown(m_editor->document());
        const QString cleaned = mdtidy::tidy(md);
        if (cleaned != md)
            setBodyMarkdown(cleaned);  // re-render desde el Markdown normalizado
    }
}

void EditorStack::applyLineOp(mdmoveline::Result (*op)(const QStringList &, int))
{
    QTextEdit *ed = activeEditor();
    if (ed != m_split->sourceEditor())
        return;  // los comandos de línea solo tienen sentido en la vista de código
    const QStringList lines = ed->toPlainText().split(QLatin1Char('\n'));
    const QTextCursor cur = ed->textCursor();
    const int line = cur.blockNumber();
    const int col = cur.positionInBlock();

    const mdmoveline::Result r = op(lines, line);
    if (r.lines == lines)
        return;  // sin cambios (borde): no ensuciar el documento

    QTextCursor edit(ed->document());
    edit.beginEditBlock();
    edit.select(QTextCursor::Document);
    edit.insertText(r.lines.join(QLatin1Char('\n')));
    edit.endEditBlock();

    // Restaura el cursor en la línea resultante, con la misma columna (acotada).
    const int target = qBound(0, r.line, ed->document()->blockCount() - 1);
    const QTextBlock nb = ed->document()->findBlockByNumber(target);
    QTextCursor nc(nb);
    nc.setPosition(nb.position() + qMin(col, qMax(0, nb.length() - 1)));
    ed->setTextCursor(nc);
    ed->setFocus();
}

void EditorStack::moveLineUp() { applyLineOp(mdmoveline::moveUp); }
void EditorStack::moveLineDown() { applyLineOp(mdmoveline::moveDown); }
void EditorStack::duplicateLine() { applyLineOp(mdmoveline::duplicate); }
void EditorStack::deleteLine() { applyLineOp(mdmoveline::removeLine); }
void EditorStack::joinLines() { applyLineOp(mdmoveline::joinNext); }

void EditorStack::insertMarkdown(const QString &markdown)
{
    if (markdown.isEmpty())
        return;
    QTextEdit *ed = activeEditor();
    if (ed == m_split->sourceEditor()) {
        ed->insertPlainText(markdown);  // la fuente es Markdown literal
    } else {
        QTextCursor cursor = ed->textCursor();
        cursor.beginEditBlock();
        cursor.insertFragment(QTextDocumentFragment::fromMarkdown(markdown));
        cursor.endEditBlock();
    }
    ed->setFocus();
}

void EditorStack::insertSnippet(const QString &body)
{
    insertMarkdown(body);  // un snippet es Markdown insertado en el editor activo
}

void EditorStack::centerCursorLine(QTextEdit *ed)
{
    if (!m_typewriter || !ed)
        return;
    QScrollBar *vbar = ed->verticalScrollBar();
    if (!vbar)
        return;
    const int centerY = ed->cursorRect().center().y();
    vbar->setValue(mdtypewriter::centeredScrollValue(
        vbar->value(), centerY, ed->viewport()->height(),
        vbar->minimum(), vbar->maximum()));
}

void EditorStack::announceFormulaUnderCursor()
{
    int start = -1;
    const QString tex = formulaAtCursor(&start);
    if (tex.isEmpty()) {
        m_lastFormulaStart = -1;
        return;
    }
    if (start == m_lastFormulaStart)
        return;   // misma fórmula (solo nos movemos dentro): no repetir
    m_lastFormulaStart = start;
    emit statusMessage(tr("Fórmula: %1").arg(tex), 0);
}
