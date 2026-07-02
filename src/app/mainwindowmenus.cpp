/// \file
/// \brief Construcción de la barra de menús y la barra de formato de MainWindow.

// Construcción de menús y barra de formato de MainWindow. Separado de
// mainwindow.cpp (que se había hecho enorme) para agrupar todo el cableado de
// acciones en un sitio; son métodos de MainWindow, solo que definidos en otra
// unidad de traducción. Ver mainwindow.cpp para el resto.

#include "mainwindow.h"
#include "editorstack.h"
#include "appsettings.h"
#include "blockconstructs.h"
#include "chromezoom.h"
#include "footnotes.h"
#include "tasklist.h"
#include "codehighlighter.h"
#include "distractionfreecontroller.h"
#include "documentio.h"
#include "snippetsdialog.h"
#include "exportcontroller.h"
#include "exporters.h"
#include "filecontroller.h"
#include "formaticons.h"
#include "formatcontroller.h"
#include "formulacontroller.h"
#include "insertcontroller.h"
#include "tablecontroller.h"
#include "findreplacebar.h"
#include "focuseditor.h"
#include "texttransform.h"
#include "richpaste.h"
#include "csvtable.h"
#include "doctemplates.h"
#include "preferencesdialog.h"
#include "themespec.h"
#include "usertemplate.h"
#include "markdownrender.h"
#include "diagramcontroller.h"
#include "mathblocks.h"
#include "outlinepanel.h"
#include "recentfilesmanager.h"
#include "spellchecker.h"
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

namespace {
// Añade el atajo (localizado por `NativeText`) entre paréntesis al final de un
// tooltip; si no hay atajo, deja el texto tal cual.
QString withShortcut(const QString &base, const QKeySequence &shortcut)
{
    if (shortcut.isEmpty())
        return base;
    return base + QStringLiteral(" (%1)").arg(shortcut.toString(QKeySequence::NativeText));
}

// Tooltip = el propio texto de la acción + su atajo. El caso habitual.
void setShortcutTooltip(QAction *action)
{
    action->setToolTip(withShortcut(action->text(), action->shortcut()));
}
} // namespace

void MainWindow::createMenusAndActions()
{
    createFileMenu();
    createEditMenu();
    createFormatActions();
    createInsertMenu();
    createTableMenu();
    createViewMenu();
    createHelpMenu();
    // Las acciones recién creadas se entregan a CADA documento (pestaña) en
    // configureStack(); aquí solo se construyen una vez, compartidas por todos.
}

// Entrega a los colaboradores de un documento las acciones compartidas de la
// ventana: las de modo de vista, las válidas solo en WYSIWYG, y los conjuntos de
// formato y tabla cuyo estado sincronizan con el cursor. Se llama por cada
// pestaña (las acciones son únicas; su estado lo refresca el documento activo).
void MainWindow::configureStack(EditorStack *stack)
{
    stack->split()->setModeActions(m_sourceModeAction, m_splitAction);
    stack->split()->setWysiwygActions(m_wysiwygActions);
    stack->format()->setActions(m_formatActions);
    stack->table()->setActions(m_tableActions);
    stack->table()->updateActions();  // estado inicial (sin tabla bajo el cursor)
    stack->setTypewriterMode(m_typewriterAction && m_typewriterAction->isChecked());
    stack->setCurrentLineHighlight(AppSettings::currentLineHighlight());
    stack->setLineSpacing(AppSettings::lineSpacing());
}

void MainWindow::createFileMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Archivo"));

    QAction *newAction = fileMenu->addAction(tr("&Nuevo"));
    newAction->setShortcut(QKeySequence::New);  // documento nuevo en una pestaña nueva
    connect(newAction, &QAction::triggered, this, &MainWindow::newTab);

    m_templateMenu = fileMenu->addMenu(tr("Nuevo desde &plantilla"));
    rebuildTemplateMenu();

    QAction *openAction = fileMenu->addAction(tr("&Abrir..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openInTab);

    QMenu *recentMenu = fileMenu->addMenu(tr("Abrir &recientes"));
    m_recentFiles = new RecentFilesManager(recentMenu, this);
    connect(m_recentFiles, &RecentFilesManager::fileOpenRequested,
            this, &MainWindow::openPathInTab);

    QMenu *importMenu = fileMenu->addMenu(tr("&Importar"));
    QAction *importHtmlAction = importMenu->addAction(tr("Desde &HTML..."));
    importHtmlAction->setToolTip(
        tr("Convierte una página HTML a Markdown y la abre como documento nuevo"));
    connect(importHtmlAction, &QAction::triggered, this, &MainWindow::importHtml);
    QAction *importEpubAction = importMenu->addAction(tr("Desde &EPUB..."));
    importEpubAction->setToolTip(
        tr("Convierte un libro EPUB a Markdown y lo abre como documento nuevo"));
    connect(importEpubAction, &QAction::triggered, this, &MainWindow::importEpub);
    QAction *importPandocAction = importMenu->addAction(tr("Otros formatos (&Pandoc)..."));
    importPandocAction->setToolTip(
        tr("Importa DOCX, ODT, RTF, LaTeX, reStructuredText… mediante Pandoc (si está instalado)"));
    connect(importPandocAction, &QAction::triggered, this, &MainWindow::importWithPandoc);

    QAction *saveAction = fileMenu->addAction(tr("&Guardar"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, [this] { m_stack->file()->save(); });

    QAction *saveAsAction = fileMenu->addAction(tr("Guardar &como..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, [this] { m_stack->file()->saveAs(); });

    QAction *saveTemplateAction = fileMenu->addAction(tr("Guardar como &plantilla..."));
    saveTemplateAction->setToolTip(
        tr("Guarda el documento actual como una plantilla reutilizable"));
    connect(saveTemplateAction, &QAction::triggered, this, &MainWindow::saveAsTemplate);

    QAction *revertAction = fileMenu->addAction(tr("&Revertir a lo guardado"));
    revertAction->setToolTip(tr("Descarta los cambios y recarga el archivo del disco"));
    connect(revertAction, &QAction::triggered, this, &MainWindow::revertToSaved);
    // Habilitada solo si el documento activo tiene archivo y cambios sin guardar; se
    // recalcula al abrir el menú (sin cablear señales por acción).
    connect(fileMenu, &QMenu::aboutToShow, this, [this, revertAction] {
        revertAction->setEnabled(
            m_stack && !m_stack->documentIo()->currentFile().isEmpty()
            && (m_stack->documentIo()->isModified() || m_stack->split()->isSourceDirty()));
    });

    QAction *openFolderAction = fileMenu->addAction(tr("Abrir &carpeta contenedora"));
    connect(openFolderAction, &QAction::triggered, this, [this] { m_stack->file()->openContainingFolder(); });

    QAction *closeTabAction = fileMenu->addAction(tr("&Cerrar pestaña"));
    closeTabAction->setShortcut(QKeySequence::Close);  // Ctrl+W
    connect(closeTabAction, &QAction::triggered, this, [this] { closeTab(-1); });

    QAction *reopenTabAction = fileMenu->addAction(tr("Reabrir pestaña cerrada"));
    // Ctrl+Shift+T (convención habitual) está ocupado por «Lista de tareas»; se usa Ctrl+Shift+R.
    reopenTabAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    reopenTabAction->setToolTip(withShortcut(
        tr("Vuelve a abrir la última pestaña cerrada"), reopenTabAction->shortcut()));
    connect(reopenTabAction, &QAction::triggered, this, &MainWindow::reopenClosedTab);

    fileMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu(tr("&Exportar"));
    QAction *exportPdfAction = exportMenu->addAction(tr("A PDF..."));
    connect(exportPdfAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportPdf(); });
    QAction *exportHtmlAction = exportMenu->addAction(tr("A HTML..."));
    connect(exportHtmlAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportHtml(); });
    QAction *exportOdfAction = exportMenu->addAction(tr("A ODF (ODT)..."));
    connect(exportOdfAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportOdf(); });
    QAction *exportDocxAction = exportMenu->addAction(tr("A DOCX (Word)..."));
    connect(exportDocxAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportDocx(); });
    QAction *exportLatexAction = exportMenu->addAction(tr("A LaTeX..."));
    connect(exportLatexAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportLatex(); });
    QAction *exportEpubAction = exportMenu->addAction(tr("A EPUB..."));
    connect(exportEpubAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportEpub(); });
    QAction *exportTxtAction = exportMenu->addAction(tr("A texto plano (TXT)..."));
    connect(exportTxtAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportPlainText(); });
    exportMenu->addSeparator();
    QAction *exportSelPdfAction = exportMenu->addAction(tr("Selección a PDF..."));
    exportSelPdfAction->setToolTip(tr("Exporta a PDF solo el texto seleccionado"));
    connect(exportSelPdfAction, &QAction::triggered, this, [this] { m_stack->exporter()->exportSelectionPdf(); });

    fileMenu->addSeparator();

    QAction *printPreviewAction = fileMenu->addAction(tr("&Vista previa de impresión..."));
    connect(printPreviewAction, &QAction::triggered, this, [this] { m_stack->exporter()->printPreview(); });

    QAction *printAction = fileMenu->addAction(tr("&Imprimir..."));
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, [this] { m_stack->exporter()->print(); });

    QAction *printSelAction = fileMenu->addAction(tr("Imprimir &selección..."));
    printSelAction->setToolTip(tr("Imprime solo el texto seleccionado"));
    connect(printSelAction, &QAction::triggered, this, [this] { m_stack->exporter()->printSelection(); });

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction(tr("&Salir"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::rebuildTemplateMenu()
{
    if (!m_templateMenu)
        return;
    m_templateMenu->clear();

    const QList<mdtemplate::DocTemplate> factory = mdtemplate::all();
    const QList<mdusertemplate::UserTemplate> user = AppSettings::userTemplates();

    // Un submenú por categoría no vacía, en orden; en cada uno, primero las de
    // fábrica y luego las del usuario. La creación es perezosa: una categoría sin
    // ninguna plantilla (de fábrica ni de usuario) no aparece.
    for (const mdtemplate::Category cat : mdtemplate::categoriesInOrder()) {
        QMenu *catMenu = nullptr;
        const auto addItem = [&](const QString &name, const QString &body) {
            if (!catMenu)
                catMenu = m_templateMenu->addMenu(mdtemplate::categoryName(cat));
            connect(catMenu->addAction(name), &QAction::triggered, this,
                    [this, body] { addTab(); m_stack->file()->newFromTemplate(body); });
        };
        for (const mdtemplate::DocTemplate &tpl : factory)
            if (tpl.category == cat)
                addItem(tpl.name, tpl.body);
        for (const mdusertemplate::UserTemplate &tpl : user)
            if (tpl.category == cat)
                addItem(tpl.name, tpl.body);
    }

    m_templateMenu->addSeparator();
    connect(m_templateMenu->addAction(tr("Gestionar plantillas...")), &QAction::triggered,
            this, &MainWindow::manageTemplates);
}

void MainWindow::createEditMenu()
{
    QMenu *editMenu = menuBar()->addMenu(tr("&Editar"));

    QAction *undoAction = editMenu->addAction(tr("Deshacer"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this] { m_stack->activeEditor()->undo(); });

    QAction *redoAction = editMenu->addAction(tr("Rehacer"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this] { m_stack->activeEditor()->redo(); });

    editMenu->addSeparator();

    QAction *pastePlainAction = editMenu->addAction(tr("Pegar como texto plano"));
    pastePlainAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    setShortcutTooltip(pastePlainAction);
    connect(pastePlainAction, &QAction::triggered, this, [this] {
        const QString text = QApplication::clipboard()->text();
        if (!text.isEmpty())
            m_stack->activeEditor()->insertPlainText(text);  // ignora formato/HTML del portapapeles
    });

    QAction *pasteMdAction = editMenu->addAction(tr("Pegar como Markdown"));
    pasteMdAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V));
    setShortcutTooltip(pasteMdAction);
    connect(pasteMdAction, &QAction::triggered, this, [this] {
        const QMimeData *mime = QApplication::clipboard()->mimeData();
        if (!mime)
            return;
        // El texto enriquecido (HTML) se normaliza a Markdown en vez de incrustar
        // su formato; sin HTML, se pega el texto plano tal cual.
        if (mime->hasHtml()) {
            const QString md = mdrichpaste::htmlToMarkdown(mime->html());
            if (md.isEmpty())
                return;
            QTextEdit *ed = m_stack->activeEditor();
            if (ed == m_stack->editor())
                ed->textCursor().insertFragment(QTextDocumentFragment::fromMarkdown(md));
            else
                ed->insertPlainText(md);  // la vista de fuente ya es Markdown literal
        } else {
            const QString text = mime->text();
            if (!text.isEmpty())
                m_stack->activeEditor()->insertPlainText(text);
        }
    });

    QAction *copyHtmlAction = editMenu->addAction(tr("Copiar como HTML"));
    connect(copyHtmlAction, &QAction::triggered, this, [this] { m_stack->exporter()->copyHtmlToClipboard(); });

    QAction *copyMdAction = editMenu->addAction(tr("Copiar como Markdown"));
    connect(copyMdAction, &QAction::triggered, this, [this] { m_stack->exporter()->copyMarkdownToClipboard(); });

    editMenu->addSeparator();

    // Transformaciones de texto sobre la selección (o la palabra bajo el cursor,
    // para las de mayúsculas/minúsculas). La lógica vive en mdtext (puro).
    QMenu *transformMenu = editMenu->addMenu(tr("Transformar texto"));
    const auto applyToSelection = [this](QString (*fn)(const QString &)) {
        QTextEdit *ed = m_stack->activeEditor();
        QTextCursor c = ed->textCursor();
        if (!c.hasSelection())
            c.select(QTextCursor::WordUnderCursor);
        if (c.hasSelection())
            c.insertText(fn(c.selectedText()));  // U+2029 sobrevive: preserva párrafos
    };
    QAction *upperAction = transformMenu->addAction(tr("MAYÚSCULAS"));
    connect(upperAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::toUpper); });
    QAction *lowerAction = transformMenu->addAction(tr("minúsculas"));
    connect(lowerAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::toLower); });
    QAction *capAction = transformMenu->addAction(tr("Capitalizar"));
    connect(capAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::capitalize); });
    transformMenu->addSeparator();
    QAction *typographyAction = transformMenu->addAction(tr("Tipografía inteligente"));
    typographyAction->setToolTip(tr("Convierte -- — ... y comillas rectas en sus formas tipográficas"));
    connect(typographyAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::smartTypography); });

    QAction *sortAction = editMenu->addAction(tr("Ordenar líneas"));
    connect(sortAction, &QAction::triggered, this, [this] {
        QTextEdit *ed = m_stack->activeEditor();
        QTextCursor c = ed->textCursor();
        if (!c.hasSelection())
            return;  // ordenar exige un rango explícito de líneas
        QString sel = c.selectedText();
        // selectedText() usa U+2029 entre párrafos; lo paso a '\n' para mdtext y lo
        // restauro antes de reinsertar (insertText vuelve a partir por U+2029).
        sel.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
        QString out = mdtext::sortLines(sel, true);
        out.replace(QLatin1Char('\n'), QChar(QChar::ParagraphSeparator));
        c.insertText(out);
    });

    QAction *cleanAction = editMenu->addAction(tr("Limpiar Markdown"));
    cleanAction->setToolTip(
        tr("Normaliza viñetas, espacios y líneas en blanco (sin tocar el código)"));
    connect(cleanAction, &QAction::triggered, this, [this] { m_stack->cleanMarkdown(); });

    editMenu->addSeparator();

    QAction *findAction = editMenu->addAction(tr("Buscar..."));
    findAction->setShortcut(QKeySequence::Find);                       // Ctrl+F
    connect(findAction, &QAction::triggered, m_findBar, &FindReplaceBar::showFind);

    QAction *replaceAction = editMenu->addAction(tr("Reemplazar..."));
    replaceAction->setShortcut(QKeySequence::Replace);                 // Ctrl+H
    connect(replaceAction, &QAction::triggered, m_findBar, &FindReplaceBar::showReplace);

    editMenu->addSeparator();
    QAction *prefsAction = editMenu->addAction(tr("&Preferencias..."));
    prefsAction->setShortcut(QKeySequence::Preferences);   // Ctrl+, (donde exista)
    prefsAction->setMenuRole(QAction::PreferencesRole);    // en macOS va al menú de app
    connect(prefsAction, &QAction::triggered, this, &MainWindow::openPreferences);
}

void MainWindow::openPreferences()
{
    // Reúne las acciones de ajuste ya existentes; el diálogo las refleja y las dispara
    // (aplican y persisten por su propia vía, sin duplicar lógica).
    PreferencesDialog::Settings s;
    for (const mdtheme::ThemeSpec &spec : mdtheme::allThemes())
        if (QAction *action = m_themeActions.value(spec.id))
            s.themeActions << action;
    s.followSystem = m_followSystemAction;
    s.warmLight = m_warmLightAction;
    s.lineSpacingActions = m_lineSpacingGroup->actions();
    s.currentLine = m_currentLineAction;
    s.focusMode = m_typewriterAction;
    s.pageNumbers = m_pageNumbersAction;

    PreferencesDialog dialog(s, this);
    dialog.exec();
}

void MainWindow::createFormatActions()
{
    // --- Acciones de formato (compartidas por el menú y la barra) ---
    // Tabla de descriptores: todas son checkable y solo difieren en texto, atajo
    // y la acción a ejecutar. El tooltip se compone del texto más el atajo en su
    // forma nativa y localizada (p. ej. «Strg+B» en alemán), derivado del propio
    // atajo: así no hay que traducirlo a mano ni puede desincronizarse.
    using Mutator = std::function<void(QTextCharFormat &, const QTextCharFormat &)>;
    const auto charToggle = [this](Mutator m) {
        return [this, m = std::move(m)] { m_stack->format()->toggleCharFormat(m); };
    };

    struct FormatActionDef {
        QAction **slot;             // dónde guardar el QAction creado
        QString text;
        QKeySequence shortcut;      // vacío = sin atajo
        std::function<void()> handler;
        QString tooltip;            // globo descriptivo; vacío = usar `text`
    };

    const FormatActionDef defs[] = {
        {&m_boldAction, tr("Negrita"), QKeySequence::Bold,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontWeight(cur.fontWeight() >= QFont::Bold ? QFont::Normal : QFont::Bold);
         })},
        {&m_italicAction, tr("Cursiva"), QKeySequence::Italic,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontItalic(!cur.fontItalic());
         })},
        {&m_underlineAction, tr("Subrayado"), QKeySequence::Underline,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             // toMarkdown() de Qt serializa el subrayado como `_texto_` (su
             // dialecto reserva `*…*` para la cursiva), así que hace round-trip.
             f.setFontUnderline(!cur.fontUnderline());
         })},
        {&m_strikeAction, tr("Tachado"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X),
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontStrikeOut(!cur.fontStrikeOut());
         })},
        {&m_codeAction, tr("Código"), QKeySequence(Qt::CTRL | Qt::Key_E),
         charToggle([this](QTextCharFormat &f, const QTextCharFormat &cur) {
             const bool enable = !cur.fontFixedPitch();
             f.setFontFixedPitch(enable);
             // El conversor a Markdown emite `código` con fuente de paso fijo.
             f.setFontFamilies({enable ? QStringLiteral("monospace")
                                       : m_stack->editor()->document()->defaultFont().family()});
         }),
         tr("Código en línea")},
        {&m_linkAction, tr("Enlace"), QKeySequence(Qt::CTRL | Qt::Key_K),
         [this] { m_stack->insert()->insertLink(); }, tr("Insertar o editar enlace")},
        {&m_quoteAction, tr("❝ Cita"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q),
         [this] { m_stack->format()->toggleBlockquote(); }, tr("Convertir en cita")},
        {&m_codeBlockAction, tr("Bloque"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K),
         [this] { m_stack->format()->toggleCodeBlock(); }, tr("Bloque de código")},
        {&m_h1Action, tr("H1"), QKeySequence(Qt::CTRL | Qt::Key_1),
         [this] { m_stack->format()->applyHeading(1); }},
        {&m_h2Action, tr("H2"), QKeySequence(Qt::CTRL | Qt::Key_2),
         [this] { m_stack->format()->applyHeading(2); }},
        {&m_h3Action, tr("H3"), QKeySequence(Qt::CTRL | Qt::Key_3),
         [this] { m_stack->format()->applyHeading(3); }},
        {&m_h4Action, tr("H4"), QKeySequence(Qt::CTRL | Qt::Key_4),
         [this] { m_stack->format()->applyHeading(4); }},
        {&m_h5Action, tr("H5"), QKeySequence(Qt::CTRL | Qt::Key_5),
         [this] { m_stack->format()->applyHeading(5); }},
        {&m_h6Action, tr("H6"), QKeySequence(Qt::CTRL | Qt::Key_6),
         [this] { m_stack->format()->applyHeading(6); }},
        {&m_bulletAction, tr("Lista de viñetas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U),
         [this] { m_stack->format()->applyList(QTextListFormat::ListDisc); }},
        {&m_numberedAction, tr("Lista numerada"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O),
         [this] { m_stack->format()->applyList(QTextListFormat::ListDecimal); }},
        {&m_taskAction, tr("Lista de tareas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T),
         [this] { m_stack->format()->toggleTaskItem(); }},
    };

    for (const FormatActionDef &d : defs) {
        auto *action = new QAction(d.text, this);
        action->setCheckable(true);
        if (!d.shortcut.isEmpty())
            action->setShortcut(d.shortcut);
        action->setToolTip(withShortcut(d.tooltip.isEmpty() ? d.text : d.tooltip,
                                        d.shortcut));
        connect(action, &QAction::triggered, this, d.handler);
        *d.slot = action;
        m_wysiwygActions.append(action);  // sin sentido en la vista de fuente
    }

    // --- Menú Formato (las mismas acciones, accesibles por teclado) ---
    QMenu *formatMenu = menuBar()->addMenu(tr("&Formato"));
    formatMenu->addAction(m_boldAction);
    formatMenu->addAction(m_italicAction);
    formatMenu->addAction(m_underlineAction);
    formatMenu->addAction(m_strikeAction);
    formatMenu->addAction(m_codeAction);
    formatMenu->addAction(m_linkAction);

    QAction *highlightAction = formatMenu->addAction(tr("Resaltar"));
    highlightAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    highlightAction->setToolTip(withShortcut(
        tr("Resaltar el texto seleccionado (==marca==)"), highlightAction->shortcut()));
    connect(highlightAction, &QAction::triggered, this,
            [this] { m_stack->format()->insertHighlight(); });
    m_wysiwygActions.append(highlightAction);

    QAction *superAction = formatMenu->addAction(tr("Superíndice"));
    superAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Equal));
    superAction->setToolTip(withShortcut(
        tr("Superíndice del texto seleccionado (^x^)"), superAction->shortcut()));
    connect(superAction, &QAction::triggered, this,
            [this] { m_stack->format()->toggleSuperscript(); });
    m_wysiwygActions.append(superAction);

    QAction *subAction = formatMenu->addAction(tr("Subíndice"));
    subAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Minus));
    subAction->setToolTip(withShortcut(
        tr("Subíndice del texto seleccionado (~x~)"), subAction->shortcut()));
    connect(subAction, &QAction::triggered, this,
            [this] { m_stack->format()->toggleSubscript(); });
    m_wysiwygActions.append(subAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_h1Action);
    formatMenu->addAction(m_h2Action);
    formatMenu->addAction(m_h3Action);
    formatMenu->addAction(m_h4Action);
    formatMenu->addAction(m_h5Action);
    formatMenu->addAction(m_h6Action);

    m_promoteHeadingAction = formatMenu->addAction(tr("Promover encabezado"));
    m_promoteHeadingAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
    m_promoteHeadingAction->setToolTip(withShortcut(
        tr("Sube el encabezado un nivel (hacia H1)"), m_promoteHeadingAction->shortcut()));
    connect(m_promoteHeadingAction, &QAction::triggered, this,
            [this] { m_stack->format()->promoteHeading(); });
    m_wysiwygActions.append(m_promoteHeadingAction);

    m_demoteHeadingAction = formatMenu->addAction(tr("Degradar encabezado"));
    m_demoteHeadingAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
    m_demoteHeadingAction->setToolTip(withShortcut(
        tr("Baja el encabezado un nivel (hacia H6)"), m_demoteHeadingAction->shortcut()));
    connect(m_demoteHeadingAction, &QAction::triggered, this,
            [this] { m_stack->format()->demoteHeading(); });
    m_wysiwygActions.append(m_demoteHeadingAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_bulletAction);
    formatMenu->addAction(m_numberedAction);
    formatMenu->addAction(m_taskAction);

    m_indentAction = formatMenu->addAction(tr("Aumentar sangría"));
    m_indentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    connect(m_indentAction, &QAction::triggered, this, [this] { m_stack->format()->indentList(); });
    m_wysiwygActions.append(m_indentAction);

    m_outdentAction = formatMenu->addAction(tr("Disminuir sangría"));
    m_outdentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    connect(m_outdentAction, &QAction::triggered, this, [this] { m_stack->format()->outdentList(); });
    m_wysiwygActions.append(m_outdentAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_quoteAction);
    formatMenu->addAction(m_codeBlockAction);

    m_langAction = formatMenu->addAction(tr("Lenguaje del bloque..."));
    m_langAction->setToolTip(tr("Fija el lenguaje del bloque de código (resaltado)"));
    connect(m_langAction, &QAction::triggered, this, [this] { m_stack->format()->setCodeLanguage(); });
    m_wysiwygActions.append(m_langAction);

    // Conjunto de acciones que FormatController sincroniza con el formato bajo el
    // cursor (las comparten este menú y la barra de botones). Se guardan para
    // entregárselas a cada documento en configureStack().
    m_formatActions = {
        m_boldAction, m_italicAction, m_underlineAction, m_strikeAction, m_codeAction,
        m_linkAction, m_quoteAction, m_codeBlockAction, m_langAction,
        m_h1Action, m_h2Action, m_h3Action, m_h4Action, m_h5Action, m_h6Action,
        m_bulletAction, m_numberedAction, m_taskAction, m_indentAction, m_outdentAction};
}

void MainWindow::createInsertMenu()
{
    QMenu *insertMenu = menuBar()->addMenu(tr("&Insertar"));

    QAction *insLink = insertMenu->addAction(tr("Enlace..."));
    connect(insLink, &QAction::triggered, this, [this] { m_stack->insert()->insertLink(); });

    QAction *insImage = insertMenu->addAction(tr("Imagen..."));
    connect(insImage, &QAction::triggered, this, [this] { m_stack->insert()->insertImage(); });

    QAction *insPasteImage = insertMenu->addAction(tr("Pegar imagen"));
    insPasteImage->setToolTip(tr("Guarda la imagen del portapapeles y la inserta"));
    connect(insPasteImage, &QAction::triggered, this, [this] { m_stack->insert()->pasteImageFromClipboard(); });

    QAction *insTable = insertMenu->addAction(tr("Tabla..."));
    connect(insTable, &QAction::triggered, this, [this] { m_stack->insert()->insertTable(); });

    QAction *insTableClip = insertMenu->addAction(tr("Tabla desde el portapapeles"));
    insTableClip->setToolTip(tr("Convierte los datos TSV/CSV del portapapeles en una tabla"));
    connect(insTableClip, &QAction::triggered, this, [this] {
        const mdcsvtable::Delimited data =
            mdcsvtable::detectDelimited(QApplication::clipboard()->text());
        if (!data.ok) {
            showStatusMessage(tr("El portapapeles no contiene datos de tabla (TSV/CSV)."));
            return;
        }
        m_stack->insertMarkdown(mdcsvtable::toMarkdownTable(data.rows));
    });

    QAction *insRule = insertMenu->addAction(tr("Regla horizontal"));
    connect(insRule, &QAction::triggered, this, [this] { m_stack->insert()->insertHorizontalRule(); });

    QAction *insToc = insertMenu->addAction(tr("Índice (TOC)"));
    insToc->setToolTip(tr("Inserta un índice con los encabezados del documento"));
    connect(insToc, &QAction::triggered, this, [this] { m_stack->insert()->insertTableOfContents(); });

    QAction *insFormula = insertMenu->addAction(tr("Fórmula..."));
    insFormula->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    setShortcutTooltip(insFormula);
    connect(insFormula, &QAction::triggered, this, [this] { m_stack->formula()->insertFormula(); });

    QAction *insFootnote = insertMenu->addAction(tr("Nota al pie"));
    insFootnote->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    insFootnote->setToolTip(withShortcut(
        tr("Inserta una referencia [^n] y su definición al final del documento"),
        insFootnote->shortcut()));
    connect(insFootnote, &QAction::triggered, this, [this] { m_stack->insert()->insertFootnote(); });

    // Admoniciones / callouts (`> [!NOTE]`, etc.). Los nombres se traducen; el
    // marcador insertado es el keyword canónico en inglés (lo que entiende GitHub).
    QMenu *admonitionMenu = insertMenu->addMenu(tr("Admonición"));
    admonitionMenu->setToolTip(tr("Inserta un bloque destacado (nota, aviso, etc.)"));
    const QList<QPair<QString, QString>> admonitions = {
        {QStringLiteral("NOTE"), tr("Nota")},
        {QStringLiteral("TIP"), tr("Consejo")},
        {QStringLiteral("IMPORTANT"), tr("Importante")},
        {QStringLiteral("WARNING"), tr("Advertencia")},
        {QStringLiteral("CAUTION"), tr("Precaución")},
    };
    for (const auto &a : admonitions) {
        const QString keyword = a.first;
        QAction *act = admonitionMenu->addAction(a.second);
        connect(act, &QAction::triggered, this,
                [this, keyword] { m_stack->insert()->insertAdmonition(keyword); });
    }

    QAction *insSymbol = insertMenu->addAction(tr("Símbolos especiales..."));
    insSymbol->setToolTip(tr("Inserta símbolos no habituales, por categorías"));
    connect(insSymbol, &QAction::triggered, this, [this] { m_stack->insert()->insertSymbol(); });

    QAction *insDate = insertMenu->addAction(tr("Fecha"));
    insDate->setToolTip(tr("Inserta la fecha actual en formato local"));
    connect(insDate, &QAction::triggered, this, [this] { m_stack->insert()->insertDate(); });

    QAction *insDateTime = insertMenu->addAction(tr("Fecha y hora"));
    insDateTime->setToolTip(tr("Inserta la fecha y la hora actuales en formato local"));
    connect(insDateTime, &QAction::triggered, this, [this] { m_stack->insert()->insertDateTime(); });

    // Snippets de usuario: submenú dinámico (uno por snippet + «Gestionar…»). NO
    // va en m_wysiwygActions: insertSnippet funciona también en la vista de fuente
    // (inserta el Markdown crudo), y «Gestionar» debe estar siempre disponible.
    m_snippetsMenu = insertMenu->addMenu(tr("Snippet"));
    rebuildSnippetsMenu();

    // Insertar tampoco aplica en la vista de fuente.
    m_wysiwygActions << insLink << insImage << insPasteImage << insTable << insRule
                     << insToc << insFormula << insFootnote
                     << admonitionMenu->menuAction() << insSymbol
                     << insDate << insDateTime;
}

void MainWindow::rebuildSnippetsMenu()
{
    if (!m_snippetsMenu)
        return;
    m_snippetsMenu->clear();
    const QList<mdsnippet::Snippet> snippets = AppSettings::snippets();
    for (const mdsnippet::Snippet &s : snippets) {
        QAction *act = m_snippetsMenu->addAction(s.name);
        const QString body = s.body;
        connect(act, &QAction::triggered, this, [this, body] { m_stack->insertSnippet(body); });
    }
    if (snippets.isEmpty()) {
        QAction *empty = m_snippetsMenu->addAction(tr("(sin snippets)"));
        empty->setEnabled(false);
    }
    m_snippetsMenu->addSeparator();
    QAction *manage = m_snippetsMenu->addAction(tr("Gestionar snippets..."));
    connect(manage, &QAction::triggered, this, [this] {
        SnippetsDialog dlg(AppSettings::snippets(), this);
        if (dlg.exec() == QDialog::Accepted) {
            AppSettings::setSnippets(dlg.snippets());
            rebuildSnippetsMenu();
        }
    });
}

void MainWindow::createTableMenu()
{
    // --- Menú Tabla (operaciones sobre la tabla bajo el cursor) ---
    // Sus acciones se habilitan solo cuando el cursor está dentro de una tabla
    // (ver updateTableActions), de ahí que no vayan en m_wysiwygActions.
    QMenu *tableMenu = menuBar()->addMenu(tr("&Tabla"));

    QAction *aRowAbove = tableMenu->addAction(tr("Insertar fila encima"));
    connect(aRowAbove, &QAction::triggered, this, [this] { m_stack->table()->insertRow(false); });
    QAction *aRowBelow = tableMenu->addAction(tr("Insertar fila debajo"));
    connect(aRowBelow, &QAction::triggered, this, [this] { m_stack->table()->insertRow(true); });
    QAction *aColLeft = tableMenu->addAction(tr("Insertar columna a la izquierda"));
    connect(aColLeft, &QAction::triggered, this, [this] { m_stack->table()->insertColumn(false); });
    QAction *aColRight = tableMenu->addAction(tr("Insertar columna a la derecha"));
    connect(aColRight, &QAction::triggered, this, [this] { m_stack->table()->insertColumn(true); });

    tableMenu->addSeparator();
    QAction *aDelRow = tableMenu->addAction(tr("Eliminar fila"));
    connect(aDelRow, &QAction::triggered, this, [this] { m_stack->table()->deleteRow(); });
    QAction *aDelCol = tableMenu->addAction(tr("Eliminar columna"));
    connect(aDelCol, &QAction::triggered, this, [this] { m_stack->table()->deleteColumn(); });

    tableMenu->addSeparator();
    QMenu *alignMenu = tableMenu->addMenu(tr("Alinear columna"));
    QAction *aLeft = alignMenu->addAction(tr("Izquierda"));
    connect(aLeft, &QAction::triggered, this, [this] { m_stack->table()->alignColumn(Qt::AlignLeft); });
    QAction *aCenter = alignMenu->addAction(tr("Centrar"));
    connect(aCenter, &QAction::triggered, this, [this] { m_stack->table()->alignColumn(Qt::AlignHCenter); });
    QAction *aRight = alignMenu->addAction(tr("Derecha"));
    connect(aRight, &QAction::triggered, this, [this] { m_stack->table()->alignColumn(Qt::AlignRight); });

    tableMenu->addSeparator();
    QMenu *sortMenu = tableMenu->addMenu(tr("Ordenar filas por columna"));
    QAction *aSortAsc = sortMenu->addAction(tr("Ascendente"));
    connect(aSortAsc, &QAction::triggered, this, [this] { m_stack->table()->sortByColumn(true); });
    QAction *aSortDesc = sortMenu->addAction(tr("Descendente"));
    connect(aSortDesc, &QAction::triggered, this, [this] { m_stack->table()->sortByColumn(false); });

    m_tableActions = {aRowAbove, aRowBelow, aColLeft, aColRight,
                      aDelRow, aDelCol, alignMenu->menuAction(),
                      aLeft, aCenter, aRight,
                      sortMenu->menuAction(), aSortAsc, aSortDesc};
}

void MainWindow::createViewMenu()
{
    QMenu *viewMenu = menuBar()->addMenu(tr("&Ver"));

    m_sourceModeAction = viewMenu->addAction(tr("Código fuente Markdown"));
    m_sourceModeAction->setCheckable(true);
    m_sourceModeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
    setShortcutTooltip(m_sourceModeAction);
    connect(m_sourceModeAction, &QAction::toggled, this, [this](bool on) { m_stack->split()->toggleSourceMode(on); });

    m_splitAction = viewMenu->addAction(tr("Vista dividida"));
    m_splitAction->setCheckable(true);
    // Ctrl+Shift+D (no Ctrl+\\: en teclados español/ISO la «\» exige AltGr y el
    // atajo resulta imposible de pulsar). «D» de «Dividida».
    m_splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    m_splitAction->setToolTip(withShortcut(
        tr("Editar WYSIWYG y código fuente a la vez, lado a lado"),
        m_splitAction->shortcut()));
    connect(m_splitAction, &QAction::toggled, this, [this](bool on) { m_stack->split()->toggleSplitView(on); });

    m_distractionAction = viewMenu->addAction(tr("Sin distracciones"));
    m_distractionAction->setCheckable(true);
    m_distractionAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_distractionAction->setToolTip(
        tr("Pantalla completa, sin barras, con el texto centrado (ESC o F11 para salir)"));
    // La conexión con el controlador se hace en el ctor (m_distraction se crea
    // después de los menús, tras la barra de formato que oculta/muestra).

    m_typewriterAction = viewMenu->addAction(tr("Modo foco"));
    m_typewriterAction->setCheckable(true);
    m_typewriterAction->setChecked(AppSettings::typewriterMode());
    // F12: junto a F11 (sin distracciones), agrupa los dos modos inmersivos. F10
    // no se usa: la reserva Qt para abrir la barra de menús.
    m_typewriterAction->setShortcut(QKeySequence(Qt::Key_F12));
    m_typewriterAction->setToolTip(withShortcut(
        tr("Centra la línea del cursor y atenúa todo salvo el párrafo actual"),
        m_typewriterAction->shortcut()));
    connect(m_typewriterAction, &QAction::toggled, this, [this](bool on) {
        AppSettings::setTypewriterMode(on);
        for (int i = 0; i < m_tabs->count(); ++i)
            if (EditorStack *s = stackAt(i))
                s->setTypewriterMode(on);
    });

    m_currentLineAction = viewMenu->addAction(tr("Resaltar la línea actual"));
    m_currentLineAction->setCheckable(true);
    m_currentLineAction->setChecked(AppSettings::currentLineHighlight());
    m_currentLineAction->setToolTip(tr("Marca con un fondo sutil la línea del cursor"));
    connect(m_currentLineAction, &QAction::toggled, this, [this](bool on) {
        AppSettings::setCurrentLineHighlight(on);
        for (int i = 0; i < m_tabs->count(); ++i)
            if (EditorStack *s = stackAt(i))
                s->setCurrentLineHighlight(on);
    });

    m_pageNumbersAction = viewMenu->addAction(tr("Números de página al imprimir"));
    m_pageNumbersAction->setCheckable(true);
    m_pageNumbersAction->setChecked(AppSettings::printPageNumbers());
    m_pageNumbersAction->setToolTip(tr("Añade el número de página en el pie al imprimir o exportar a PDF"));
    connect(m_pageNumbersAction, &QAction::toggled, this,
            [](bool on) { AppSettings::setPrintPageNumbers(on); });

    // Interlineado del editor (presentación pura, no afecta al Markdown). Submenú
    // de opciones excluyentes; el valor se persiste y se empuja a todas las
    // pestañas. Los rótulos van por QT_TRANSLATE_NOOP para que lupdate los extraiga
    // (el bucle los traduce con tr() en runtime, como los nombres de tema).
    QMenu *spacingMenu = viewMenu->addMenu(tr("Interlineado"));
    m_lineSpacingGroup = new QActionGroup(this);
    m_lineSpacingGroup->setExclusive(true);
    const struct { const char *label; int percent; } kSpacingOptions[] = {
        {QT_TRANSLATE_NOOP("MainWindow", "Sencillo"), 100},
        {QT_TRANSLATE_NOOP("MainWindow", "1,5 líneas"), 150},
        {QT_TRANSLATE_NOOP("MainWindow", "Doble"), 200},
    };
    const int currentSpacing = AppSettings::lineSpacing();
    for (const auto &opt : kSpacingOptions) {
        QAction *action = spacingMenu->addAction(tr(opt.label));
        action->setCheckable(true);
        action->setChecked(opt.percent == currentSpacing);
        m_lineSpacingGroup->addAction(action);
        const int percent = opt.percent;
        connect(action, &QAction::triggered, this, [this, percent] {
            AppSettings::setLineSpacing(percent);
            for (int i = 0; i < m_tabs->count(); ++i)
                if (EditorStack *s = stackAt(i))
                    s->setLineSpacing(percent);
        });
    }

    // Esquema (índice): toggleViewAction muestra/oculta el dock y mantiene su
    // marca sincronizada con la visibilidad del panel automáticamente.
    m_outlineAction = m_outline->toggleViewAction();
    m_outlineAction->setText(tr("Esquema"));
    m_outlineAction->setShortcut(QKeySequence(Qt::Key_F9));
    setShortcutTooltip(m_outlineAction);
    viewMenu->addAction(m_outlineAction);

    QAction *goToHeadingAction = viewMenu->addAction(tr("Ir a encabezado..."));
    goToHeadingAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    goToHeadingAction->setToolTip(withShortcut(
        tr("Salta a un encabezado del documento"), goToHeadingAction->shortcut()));
    connect(goToHeadingAction, &QAction::triggered, this, &MainWindow::goToHeading);

    QAction *goToLineAction = viewMenu->addAction(tr("Ir a línea..."));
    goToLineAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_L));
    goToLineAction->setToolTip(withShortcut(
        tr("Salta a un número de línea"), goToLineAction->shortcut()));
    connect(goToLineAction, &QAction::triggered, this, &MainWindow::goToLine);

    QAction *commandPaletteAction = viewMenu->addAction(tr("Paleta de comandos..."));
    commandPaletteAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    commandPaletteAction->setToolTip(withShortcut(
        tr("Busca y ejecuta cualquier acción por su nombre"),
        commandPaletteAction->shortcut()));
    connect(commandPaletteAction, &QAction::triggered, this, &MainWindow::commandPalette);

    viewMenu->addSeparator();

    QAction *zoomInAction = viewMenu->addAction(tr("Aumentar letra"));
    zoomInAction->setShortcuts({QKeySequence::ZoomIn,
                                QKeySequence(Qt::CTRL | Qt::Key_Equal)});  // Ctrl++ y Ctrl+=
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomInText);

    QAction *zoomOutAction = viewMenu->addAction(tr("Reducir letra"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);                     // Ctrl+-
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOutText);

    QAction *zoomResetAction = viewMenu->addAction(tr("Tamaño normal"));
    zoomResetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));      // Ctrl+0
    connect(zoomResetAction, &QAction::triggered, this, &MainWindow::resetZoom);

    viewMenu->addSeparator();

    QAction *statsAction = viewMenu->addAction(tr("Estadísticas del documento..."));
    connect(statsAction, &QAction::triggered, this, &MainWindow::showDocumentStatistics);

    QAction *wordCountAction = viewMenu->addAction(tr("Mostrar contador de palabras"));
    wordCountAction->setCheckable(true);
    // m_countLabel aún no existe aquí (los menús se crean antes que la barra de
    // estado); el estado inicial sale del ajuste, con el que arrancará el label.
    wordCountAction->setChecked(AppSettings::showWordCount());
    connect(wordCountAction, &QAction::toggled, this, [this](bool on) {
        m_countLabel->setVisible(on);
        AppSettings::setShowWordCount(on);
    });

    QAction *lineColAction = viewMenu->addAction(tr("Mostrar línea y columna"));
    lineColAction->setCheckable(true);
    lineColAction->setChecked(AppSettings::showLineColumn());
    connect(lineColAction, &QAction::toggled, this, [this](bool on) {
        m_lineColLabel->setVisible(on);
        AppSettings::setShowLineColumn(on);
        if (on)
            updateLineColumn();  // rellena el texto al activarlo (antes estaba vacío y oculto)
    });

    QAction *spellAction = viewMenu->addAction(tr("Corrección ortográfica"));
    spellAction->setCheckable(true);
    spellAction->setChecked(AppSettings::spellCheck());
    spellAction->setToolTip(tr("Subraya las palabras mal escritas según el idioma del documento"));
    connect(spellAction, &QAction::toggled, this, [this](bool on) {
        // Aplica a TODAS las pestañas (cada una tiene su SpellController): si no, las
        // de segundo plano seguirían subrayando con el ajuste desincronizado.
        for (int i = 0; i < m_tabs->count(); ++i)
            if (EditorStack *s = stackAt(i))
                s->spell()->setEnabled(on);  // persiste, carga/descarga y rehace el resaltado
    });

    // Idioma del corrector: «Automático» (deduce del documento) o uno fijo de los
    // diccionarios disponibles. El nombre legible sale del propio locale.
    QMenu *spellLangMenu = viewMenu->addMenu(tr("Idioma de corrección"));
    auto *spellLangGroup = new QActionGroup(this);
    spellLangGroup->setExclusive(true);
    const QString currentSpellLang = AppSettings::spellLanguage();
    QAction *autoLangAction = spellLangMenu->addAction(tr("Automático (según el documento)"));
    autoLangAction->setCheckable(true);
    autoLangAction->setChecked(currentSpellLang.isEmpty());
    spellLangGroup->addAction(autoLangAction);
    connect(autoLangAction, &QAction::triggered, this, [this] {
        for (int i = 0; i < m_tabs->count(); ++i)
            if (EditorStack *s = stackAt(i))
                s->spell()->setLanguageOverride(QString());
    });
    const QStringList spellLangs = SpellChecker::availableLanguages();
    if (!spellLangs.isEmpty())
        spellLangMenu->addSeparator();
    for (const QString &code : spellLangs) {
        QAction *langAct = spellLangMenu->addAction(SpellController::languageLabel(code));
        langAct->setCheckable(true);
        langAct->setChecked(code == currentSpellLang);
        spellLangGroup->addAction(langAct);
        connect(langAct, &QAction::triggered, this, [this, code] {
            for (int i = 0; i < m_tabs->count(); ++i)
                if (EditorStack *s = stackAt(i))
                    s->spell()->setLanguageOverride(code);
        });
    }

    viewMenu->addSeparator();
    QMenu *themeMenu = viewMenu->addMenu(tr("Tema"));
    auto *themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    // Literales de los nombres de tema traducibles, marcados para que lupdate
    // los extraiga (el bucle los traduce en runtime con tr(), que no expone el
    // literal). Deben coincidir con los displayName de los temas con
    // nameTranslated == true en mdtheme::allThemes().
    [[maybe_unused]] static const char *const kThemeNameMarkers[] = {
        QT_TRANSLATE_NOOP("MainWindow", "Claro"),
        QT_TRANSLATE_NOOP("MainWindow", "Oscuro"),
        QT_TRANSLATE_NOOP("MainWindow", "Alto contraste"),
    };

    // Una entrada por tema del catálogo (mdtheme::allThemes()): añadir un tema
    // allí lo trae al menú sin tocar esto. Los nombres con nameTranslated pasan
    // por tr(); los de nombre propio (GitHub, Monokai) se muestran verbatim.
    for (const mdtheme::ThemeSpec &spec : mdtheme::allThemes()) {
        const QString name = spec.nameTranslated
            ? tr(spec.displayName.toUtf8().constData())
            : spec.displayName;
        QAction *action = themeMenu->addAction(name);
        action->setCheckable(true);
        themeGroup->addAction(action);
        m_themeActions.insert(spec.id, action);
        const mdtheme::ThemeId id = spec.id;
        connect(action, &QAction::triggered, this,
                [this, id] { m_stack->theme()->applyTheme(id); });
    }

    // El control del tema avisa para mantener marcada la acción del tema activo.
    // (Los iconos de la barra NO se regeneran aquí: esta conexión es de un solo
    // stack y no seguiría a otras pestañas; lo hace MainWindow::changeEvent ante
    // ApplicationPaletteChange, que cubre cualquier origen del cambio de tema.)
    connect(m_stack->theme(), &ThemeController::themeChanged, this,
            [this](mdtheme::ThemeId id) {
        if (QAction *action = m_themeActions.value(id))
            action->setChecked(true);
    });

    themeMenu->addSeparator();
    m_followSystemAction = themeMenu->addAction(tr("Seguir el sistema"));
    m_followSystemAction->setCheckable(true);
    m_followSystemAction->setChecked(m_stack->theme()->followsSystem());
    m_followSystemAction->setToolTip(
        tr("Usa el tema claro u oscuro según la configuración del sistema operativo"));
    // Mientras se sigue el SO, la elección manual de tema queda deshabilitada.
    auto setManualThemeEnabled = [this](bool follow) {
        for (QAction *action : m_themeActions.values())
            action->setEnabled(!follow);
    };
    setManualThemeEnabled(m_stack->theme()->followsSystem());
    connect(m_followSystemAction, &QAction::toggled, this,
            [this, setManualThemeEnabled](bool on) {
                m_stack->theme()->setFollowSystem(on);
                setManualThemeEnabled(on);
            });

    themeMenu->addSeparator();
    m_warmLightAction = themeMenu->addAction(tr("Luz cálida nocturna"));
    m_warmLightAction->setCheckable(true);
    m_warmLightAction->setChecked(m_stack->theme()->isWarmLight());
    m_warmLightAction->setToolTip(
        tr("Tiñe el fondo del editor de tono ámbar según la hora, más cálido de noche"));
    connect(m_warmLightAction, &QAction::toggled, this,
            [this](bool on) { m_stack->theme()->setWarmLight(on); });

    // --- Idioma (se aplica al instante: recrea la ventana, ver setLanguage) ---
    // Código de locale por idioma; "" = automático (el del sistema). Los
    // nombres se muestran en su propia lengua (autónimos), no se traducen.
    // Añadir un idioma = una línea aquí + su .ts en CMakeLists.txt.
    struct Lang { QString code; QString name; };
    const QList<Lang> languages = {
        { QString(),               tr("Automático (sistema)") },
        { QStringLiteral("es"),    QStringLiteral("Español") },
        { QStringLiteral("en"),    QStringLiteral("English") },
        { QStringLiteral("de"),    QStringLiteral("Deutsch") },
        { QStringLiteral("fr"),    QStringLiteral("Français") },
        { QStringLiteral("it"),    QStringLiteral("Italiano") },
        { QStringLiteral("pt"),    QStringLiteral("Português") },
        { QStringLiteral("pl"),    QStringLiteral("Polski") },
        { QStringLiteral("nl"),    QStringLiteral("Nederlands") },
        { QStringLiteral("ro"),    QStringLiteral("Română") },
    };

    QMenu *langMenu = viewMenu->addMenu(tr("Idioma"));
    auto *langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);
    const QString currentLang = AppSettings::language();
    for (const Lang &lang : languages) {
        QAction *action = langMenu->addAction(lang.name);
        action->setCheckable(true);
        action->setChecked(lang.code == currentLang);
        langGroup->addAction(action);
        connect(action, &QAction::triggered, this,
                [this, code = lang.code] { setLanguage(code); });
    }
}

void MainWindow::createHelpMenu()
{
    QMenu *helpMenu = menuBar()->addMenu(tr("A&yuda"));
    QAction *manualAction = helpMenu->addAction(tr("&Manual"));
    manualAction->setShortcut(QKeySequence::HelpContents);  // F1 en Linux/Windows
    connect(manualAction, &QAction::triggered, this, &MainWindow::showHelpDialog);
    helpMenu->addSeparator();
    QAction *aboutAction = helpMenu->addAction(tr("&Acerca de"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
}

void MainWindow::createFormatToolBar()
{
    QToolBar *bar = addToolBar(tr("Formato"));
    m_formatToolBar = bar;
    bar->setObjectName(QStringLiteral("formatToolBar"));  // para save/restoreState
    bar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    bar->addAction(m_boldAction);
    bar->addAction(m_italicAction);
    bar->addAction(m_underlineAction);
    bar->addAction(m_strikeAction);
    bar->addAction(m_codeAction);
    bar->addAction(m_linkAction);
    bar->addSeparator();
    bar->addAction(m_h1Action);
    bar->addAction(m_h2Action);
    bar->addAction(m_h3Action);
    bar->addAction(m_h4Action);
    bar->addSeparator();
    bar->addAction(m_bulletAction);
    bar->addAction(m_numberedAction);
    bar->addAction(m_taskAction);
    bar->addSeparator();
    bar->addAction(m_quoteAction);
    bar->addAction(m_codeBlockAction);

    // Los botones de formato de carácter y de lista se muestran como iconos (el
    // resto de la barra es de texto): se ponen en modo solo-icono y se les genera
    // el icono según el tema. El tamaño lo fija updateToolBarIcons() a partir de
    // la fuente de la barra.
    for (QAction *a : {m_boldAction, m_italicAction, m_underlineAction, m_strikeAction,
                       m_bulletAction, m_numberedAction, m_taskAction})
        if (auto *btn = qobject_cast<QToolButton *>(bar->widgetForAction(a)))
            btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    updateToolBarIcons();
}

void MainWindow::updateToolBarIcons()
{
    using namespace formaticons;
    if (!m_formatToolBar)
        return;
    // El icono sigue a la altura del texto de la barra (la de los botones «H1»,
    // «H2»…) para que los botones de icono se vean igual de grandes y acompañen
    // al zoom de la interfaz.
    const int px =
        qMax(18, qRound(QFontMetricsF(m_formatToolBar->font()).height() * 1.3));
    m_formatToolBar->setIconSize(QSize(px, px));
    const qreal dpr = m_formatToolBar->devicePixelRatioF();
    // El color del glifo se deriva del fondo de la barra (no se lee de la
    // paleta) para garantizar contraste sea cual sea el tema: tinta oscura
    // sobre fondos luminosos, tinta clara sobre fondos oscuros. Los botones de
    // la barra no pintan fondo de botón salvo en hover, así que el contraste
    // real es contra el color Window de la barra.
    //
    // Se consulta la paleta de la aplicación, no la del widget: cuando esta
    // función se llama desde la señal themeChanged del ThemeController, la
    // paleta de qApp ya está actualizada, pero la del widget aún arrastra la
    // anterior hasta que Qt propague el cambio. Si leyéramos la del widget,
    // los iconos se regenerarían con el color del tema previo y se verían
    // «aguados» al cambiar de tema.
    const QColor color =
        contrastingInk(qApp->palette().color(QPalette::Window));
    m_boldAction->setIcon(makeFormatIcon(FormatIconKind::Bold, color, px, dpr));
    m_italicAction->setIcon(makeFormatIcon(FormatIconKind::Italic, color, px, dpr));
    m_underlineAction->setIcon(makeFormatIcon(FormatIconKind::Underline, color, px, dpr));
    m_strikeAction->setIcon(makeFormatIcon(FormatIconKind::Strike, color, px, dpr));
    m_bulletAction->setIcon(makeListIcon(ListIconKind::Bullet, color, px, dpr));
    m_numberedAction->setIcon(makeListIcon(ListIconKind::Numbered, color, px, dpr));
    m_taskAction->setIcon(makeListIcon(ListIconKind::Task, color, px, dpr));
}
