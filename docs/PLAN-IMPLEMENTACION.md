# Plan de implementación — mejoras de la auditoría 2026-06-30

> **✅ COMPLETADO (2026-07-01).** Las 18 mejoras y los refactors R1–R9 están
> implementados, probados y publicados: la primera tanda en la 2.4.0 y el resto en la
> 2.5.0 (una mejora por commit en `main`, con módulo puro + `tst_` + integración +
> traducciones + ayuda + CHANGELOG). Este documento se conserva como registro del
> *cómo*. Las ideas futuras (fuera de este plan) están en
> [`POSIBLES-MEJORAS.md`](POSIBLES-MEJORAS.md).

> Plan de ejecución de las 18 mejoras de la sección «Nueva auditoría
> (2026-06-30)» de [`POSIBLES-MEJORAS.md`](POSIBLES-MEJORAS.md). Aquí está el *cómo*
> (orden, dependencias, módulos, tests, puntos de integración y refactorizaciones
> previas); en la auditoría está el *qué* y el *porqué*.
>
> - **Verificación:** los puntos de integración (`archivo:línea`, señales, hooks y
>   patrones reutilizables) se comprobaron contra el árbol actual el 2026-07-01.
> - **Tecnología:** Qt6 (≥ 6.5) + C++17, sin dependencias nuevas (ni enlazadas ni en
>   runtime).
> - **Filosofía:** módulo puro + `tst_` + integración en el controller; un commit por
>   mejora en `main`; `cmake --build build && ctest --test-dir build` tras cada paso.

---

## Principios de ejecución

- **Una por una, test + commit.** Cada mejora es una unidad independiente y revertible,
  ordenada de menor a mayor riesgo de regresión.
- **Caracterización antes de extraer.** Para lo acoplado a GUI o al round-trip (donde
  no hay test unitario directo), escribir primero un test que fije el comportamiento
  actual y luego refactorizar contra esa red.
- **El refactor es un paso previo, no un añadido.** Cuando una mejora se apoyaría en un
  punto frágil o duplicado, se rediseña ese punto *antes* (en su propio commit), de modo
  que la mejora se monte sobre una base ya saneada. Las refactorizaciones recomendadas
  están etiquetadas `R1`…`R9` y referenciadas desde cada fase.
- **Round-trip primero.** Lo que toca carga/serialización (`mdrender`, `documentMarkdown`)
  va al final, con casos nuevos en `tst_markdownroundtrip` además del `tst_` del módulo.
- **Transversales por mejora:** traducir las cadenas nuevas (`update_translations` + 8
  `.ts`, `tst_translations` falla con `unfinished`); registrar `.cpp/.h` y `tst_*` en
  `CMakeLists.txt`; los atajos nuevos los valida `tst_splitview::noConflictingShortcuts`.
- **Documentación en el MISMO commit de la mejora** (no al final, en una pasada aparte):
  toda mejora con superficie visible para el usuario actualiza, junto al código, la
  **ayuda integrada** (`src/help/help-app*.md`, **los 9 idiomas**, etiquetas idénticas a
  las del menú), y, si procede, la **lista de funciones del README** y una entrada en
  `CHANGELOG.md` (sección `## [Sin publicar]`). Una nueva acción de menú, formato de
  exportación o atajo casi siempre toca la ayuda; verificar tras editar que las 9 copias
  quedan paralelas (grep de la etiqueta nueva en cada fichero).

---

## Orden y fases

Una dependencia dura condiciona todo el orden: el resaltado de la línea actual (#12) y
el de todas las coincidencias (#13) comparten el dueño único de `setExtraSelections`,
hoy `EditorStack::applyLineFocus` (`editorstack.cpp:322-362`). Hay que aplicar el
refactor `R1` *antes* de tocar cualquiera de los dos.

### Fase 1 — Export / portapapeles (aislado, sin riesgo de round-trip)

- **#1 · Copiar como Markdown** — contrapartida de «Pegar como Markdown» / «Copiar como HTML».
  - **Puro:** `mdrichpaste::fragmentToMarkdown()` (crea un `QTextDocument` con el fragmento
    y lo pasa por `mdtable::documentMarkdown`); `tst_richpaste`.
  - **Integración:** `ExportController::copyMarkdownToClipboard()` (selección vs documento,
    `mime->setText(md)`); acción en *Editar* junto a «Copiar como HTML» (`mainwindowmenus.cpp:274-275`).
  - **Riesgo:** bajo. **Refactor:** `R3` (helper de portapapeles compartido con `copyHtmlToClipboard`).
- **#2 · Exportar a .txt** — encaja en el patrón declarativo existente.
  - **Integración:** nuevo descriptor en `ExportController::runExport` (`exportcontroller.cpp:128-162`)
    con `write = writeUtf8File(path, doc->toPlainText(), err)` (helper ya en `:112-126`),
    `useFlatClone=true`; entrada en *Archivo → Exportar*; caso en `tst_goldenexport`.
  - **Riesgo:** muy bajo. Valida el patrón `FileExporter`; no se debe caso-especiar.
- **#3 · Metadatos del PDF (título/autor)** — hoy el PDF no usa front matter.
  - **Puro:** `mdexport::pdfDocumentInfo(frontMatter)` → `{title, creator}`; `tst_exporters`.
  - **Integración:** `setDocName()/setCreator()` en `exportPdf` (`exportcontroller.cpp:272-274`)
    y `exportSelectionPdf` (`:226-228`); título de `exportTitle()`, autor de
    `frontMatterValue(fm,"author")` (`author`→`setCreator`; Qt6 no tiene `setAuthor`).
  - **Riesgo:** bajo. **Refactor:** `R2` (consolidación ligera de la configuración del `QPrinter`).
- **#4 · Revertir a lo guardado** — la lógica ya existe (`reloadFromDisk()`, `mainwindowsession.cpp:176-200`).
  - **Integración:** acción en *Archivo* con `QMessageBox` de confirmación; habilitada solo si
    `currentFile()` no vacío y modificado. Sin módulo nuevo.
  - **Riesgo:** bajo.

### Fase 2 — Navegación y barra de estado (UI aislada)

- **#5 · Ir a línea (Ctrl+L)** — complementa «Ir a encabezado» (Ctrl+G).
  - **Puro:** `mdnav::clampLine(req, blockCount)`; `tst_gotoline`.
  - **Integración:** `QInputDialog::getInt` + `findBlockByNumber` sobre `activeEditor()`;
    **no** añadir a `m_wysiwygActions` (útil en fuente/dividida).
  - **Riesgo:** bajo.
- **#6 · Indicador Ln/Col** — engancha al movimiento de cursor.
  - **Puro:** `mdstats::lineColumnOf(text, pos)`; `tst_docstats`.
  - **Integración:** `QLabel` permanente espejo de `m_countLabel` (`mainwindow.cpp:249-252`);
    conmutable en *Ver*, persistido (`AppSettings::showLineColumn`).
  - **Riesgo:** bajo-medio. **Refactor:** `R4` (señal explícita `cursorMoved()`; hoy el WYSIWYG
    no propaga `cursorPositionChanged` a la ventana).
- **#7 · Pegar TSV/CSV como tabla** — el hueco real es texto plano TAB/coma (el HTML ya lo cubre «Pegar como Markdown»).
  - **Puro:** `mdcsvtable::detectDelimited` (conservador) + `toMarkdownTable` (escapa `|`); `tst_csvtable`.
  - **Integración:** inserta con `fromMarkdown`; acción *Insertar → Tabla desde portapapeles*.
  - **Riesgo:** bajo-medio.
- **#8 · Reabrir pestaña cerrada** — hoy `closeTab` descarta la ruta sin guardarla.
  - **Puro:** pila acotada `closedtabs::push/pop`; `tst_closedtabstack`.
  - **Integración:** `closeTab` (`mainwindow.cpp:659-673`) apila `documentIo()->currentFile()`
    antes de `removeTab/reset`; un slot la desapila con `openPathInTab`. Solo rutas en disco.
  - **Riesgo:** bajo-medio. **Atajo:** Ctrl+Shift+T está ocupado (Lista de tareas) → elegir otro.

### Fase 3 — Paleta de comandos (#9, máximo impacto en usabilidad)

- **#9 · Paleta de comandos (Ctrl+Shift+P)** — acceso por teclado a las ~80 acciones.
  - **Puro:** `mdcommands::collectCommands(menuBar)` (recorre `findChildren<QAction*>()`, salta
    separadores/contenedores/deshabilitadas, arma ruta+atajo) + `filterCommands` difuso; `tst_commands`.
  - **Integración:** `CommandPaletteDialog` clonando `GoToHeadingDialog` (`gotoheadingdialog.cpp`:
    filtro + `QListWidget` + `eventFilter` + `selectedXxx()` tras `exec()`); guarda `QAction*` en
    `Qt::UserRole`, dispara `action->trigger()` al aceptar.
  - **Riesgo:** medio, pero aislado.

### Fase 4 — extraSelections (refactor habilitador + dependientes)

- **R1 (prerrequisito) · Compositor de `extraSelections`** — convertir `applyLineFocus` (único
  `setExtraSelections`, `editorstack.cpp:330,361`) en `rebuildExtraSelections()` que **fusiona**
  tres capas: atenuación del modo foco (existente) + línea actual (#12) + coincidencias (#13).
  Caracterización antes de extraer. Sin cambio de comportamiento visible.
- **#12 · Resaltar la línea actual** — **depende de R1.**
  - **Puro:** `linehighlight::currentLineColor` (mezcla `Base`→`Highlight`); `tst_linehighlight`.
  - **Integración:** `ExtraSelection` con `FullWidthSelection`, recalculada en `cursorPositionChanged`
    (ya conectado, `editorstack.cpp:160-161` y `:96-98`); conmutable en *Ver*.
  - **Riesgo:** medio.
- **#13 · Búsqueda «N de M» + resaltar todas** — **depende de R1.**
  - **Puro:** `mdfind::matchOrdinal` (ver `R5`); `tst_findmatches`.
  - **Integración:** contador («x de N») como `QLabel` en la barra; señal nueva
    `FindReplaceBar::highlightMatches(ranges)` al `EditorStack` activo, que las inyecta como capa.
  - **Riesgo:** medio. **Refactor:** `R5` (extraer el bucle de coincidencias de `replaceAll`).

### Fase 5 — Operaciones estructurales del editor (medio)

- **#11 · Promover/degradar encabezado (Ctrl+[ / Ctrl+])**
  - **Puro:** `mdoutline::shiftHeadingLevels(md, delta, fromOrdinal, subtree)` con clamp [1,6]; `tst_outline`.
  - **Integración:** variante por cursor en `FormatController` **sin** la semántica *toggle* de
    `applyHeading` (`formatcontroller.cpp:53-55`, que borraría el encabezado al promover).
  - **Riesgo:** medio. **Refactor:** `R6` (separar `setHeadingLevel` absoluto del toggle).
- **#10 · Ordenar filas de tabla por columna**
  - **Puro:** `mdtablesort::sortedOrder(cells, col, numérica/alfabética)`; `tst_tablesort`.
  - **Integración:** `TableController::sortRows` moviendo `QTextDocumentFragment` por celda
    (preserva formato y `MathObject`); reusa el patrón `cellAt(r,c).firstCursorPosition()/lastCursorPosition()`
    (hoy solo en `alignColumn`, `tablecontroller.cpp:76-83`); `m_tableActions` habilita por contexto.
  - **Riesgo:** medio.
- **#17 · Comandos de línea (mover/duplicar/borrar/unir)**
  - **Puro:** `mdmoveline` que respeta fences y no toca front matter; `tst_moveline`.
  - **Integración:** operar sobre el **cuerpo Markdown** y recargar con `setBodyMarkdown` (patrón de
    `cleanMarkdown`/`moveSection`); en fuente es trivial. Atajos Alt+↑/↓, Ctrl+J (Ctrl+K y
    Ctrl+Shift+K están ocupados).
  - **Riesgo:** medio. **Refactor:** `R8` (edición estructural vía cuerpo Markdown, no mutando `QTextBlock`s).
- **#18 · Esquema: filtro en vivo + plegado persistente**
  - **Puro:** `mdoutline::visibleOrdinals(headings, filtro)` (conserva ancestros); `tst_outline`.
  - **Integración:** `QLineEdit` de filtro + recordar ramas plegadas entre reconstrucciones (hoy
    `rebuild()` hace `expandAll()` incondicional, `outlinepanel.cpp:299`) + «Expandir/Plegar todo».
  - **Riesgo:** medio. **Refactor:** `R7` (separar construcción del modelo de la aplicación del estado de vista).

### Fase 6 — Round-trip / paginación (lo más sensible, al final)

- **#15 · Marca `==texto==`** (resaltado)
  - **Puro:** `mdmark::spansIn` + pasada en `mdrender::renderPasses`; presentación con
    `QPalette::Highlight` (sin `setStyleSheet`); `tst_markhighlight` + caso en `tst_markdownroundtrip`.
  - **Integración:** acción que **inserta los `==` literales** (no un char-format).
  - **Riesgo:** medio (round-trip seguro: `=` no es delimitador en md4c → texto literal).
  - **Refactor:** `R9` (centralizar la extensión en `mdrender::renderPasses`).
- **#14 · Cabecera/pie con nº de página**
  - **Puro:** `mdprintdecor` (`headerFooterText` + `bodyRect`); `tst_printdecor`.
  - **Integración:** helper `paintPaginated(QPrinter*, QTextDocument*, …)` con `QPainter`/`newPage()`
    que sustituye los 5 call sites de `print()` (`exportcontroller.cpp:174,208,229,245,277`); flags en `AppSettings`.
  - **Riesgo:** medio-alto (reescribe la paginación). **Refactor:** `R2` (el helper único de impresión es la base).
- **#16 · Superíndice/subíndice `^x^` / `~x~`** — el más delicado.
  - **Puro:** `mdsupsub` (proteger + render + reinyección en `documentMarkdown`); `tst_supersub`.
  - **Integración:** `~x~` choca con `~~tachado~~` GFM → proteger el `~` simple con centinela PUA
    antes de `setMarkdown`; `AlignSuperScript` pasa a compartirse entre 3 features.
  - **Riesgo:** medio-alto (toca la protección de carga y `documentMarkdown`).
  - **Refactor:** `R9` (`UserProperty` propia para desambiguar `AlignSuperScript`).

---

## Refactorizaciones recomendadas

Cada una elimina un riesgo concreto de regresión o una duplicación que la mejora
agravaría. Van en su propio commit, antes (prerrequisito) o junto a la mejora que la dispara.

- **R1 · Compositor de `extraSelections`** — *prerrequisito de #12 y #13.* Hoy `applyLineFocus`
  es el único que llama `setExtraSelections`; añadir línea actual y coincidencias llamándolo cada
  uno se pisaría entre sí y borraría la atenuación del modo foco. Patrón: composición por capas
  (proveedores independientes fusionados en un único `rebuildExtraSelections`). Red: caracterización
  del modo foco (`tst_typewriter`) + un test de integración que verifique que las tres capas coexisten.
- **R2 · Choke point de impresión/PDF** — *dispara #3, base de #14.* Seis call sites construyen su
  propio `QPrinter` y llaman `print()` (`exportcontroller.cpp:174,208,229,245,277` + `copyHtml`).
  Patrón: extraer la configuración del printer (`configurePrinter`, incl. metadatos de #3) y un único
  `renderToPrinter(doc, printer)`; #14 sustituye ahí dentro `print()` por `paintPaginated`. Consolidación
  ligera al llegar a #3; el helper de paginación al llegar a #14. Red: `tst_goldenexport` + comprobar
  que las 5 rutas no cambian de salida (nº de páginas / no-crash).
- **R3 · Helper de portapapeles compartido** — *dispara #1.* `copyHtmlToClipboard` y el nuevo
  `copyMarkdownToClipboard` comparten «commit source → clon/serialización → `QMimeData`». Patrón: DRY,
  extraer el tronco común y parametrizar el serializador (HTML vs Markdown). Riesgo bajo.
- **R4 · Señal explícita `EditorStack::cursorMoved()`** — *dispara #6.* Hoy el WYSIWYG no propaga
  `cursorPositionChanged` a la ventana y el de fuente lo hace de refilón vía `wordCountShouldUpdate`
  (`editorstack.cpp:94-95`). Patrón: señal con nombre propio en vez de efecto colateral; deshace la
  asimetría y da un punto limpio para Ln/Col (y futuros indicadores de cursor).
- **R5 · Extraer el bucle de coincidencias de `replaceAll`** — *dispara #13.* El contador vive
  embebido en `replaceAll`, acoplado a la inserción (`findreplacebar.cpp:226-245`). Patrón: extraer una
  función pura de iteración/conteo (`mdfind`) que `replaceAll` reusa; un solo camino de *matching* para
  contar, resaltar y reemplazar. Reduce el riesgo sobre la ruta ya probada.
- **R6 · Separar `setHeadingLevel` absoluto del toggle** — *dispara #11.* `applyHeading` es un toggle
  (`formatcontroller.cpp:53-55`); promover/degradar **no** puede reusarlo (borraría el encabezado).
  Patrón: extraer la mutación nuclear (nivel + `FontSizeAdjustment` + peso) a `setHeadingLevel(int)` y
  dejar el toggle como envoltura delgada, evitando duplicar la lógica de formato.
- **R7 · OutlinePanel: modelo vs estado de vista** — *dispara #18.* `rebuild()` hace `clear()` +
  `expandAll()` incondicional y pierde el plegado (`outlinepanel.cpp:266,299`). Patrón: separar la
  construcción del árbol (modelo) de la aplicación del estado de vista; capturar el conjunto plegado
  (keyed por blockNumber/texto) y reaplicarlo, y derivar la visibilidad de `visibleOrdinals`. Red:
  `tst_outline` para la función pura + caracterización del `rebuild`.
- **R8 · Edición estructural vía cuerpo Markdown** — *dispara #17 (reutilizable por #10).* Reordenar
  `QTextBlock`s en crudo en WYSIWYG es arriesgado (tablas, `MathObject`, notas). Principio de diseño,
  no refactor de código existente: enrutar la edición por el cuerpo Markdown + `setBodyMarkdown` (patrón
  ya probado en `cleanMarkdown`/`moveSection`), que minimiza regresiones por construcción.
- **R9 · Desambiguar `AlignSuperScript` + punto de extensión único** — *dispara #15 y #16.* Las nuevas
  extensiones inline deben entrar solo por `mdrender::renderPasses` (punto único ya existente). Para
  #16, `AlignSuperScript` pasaría a compartirse entre fórmulas y texto sup/sub: patrón de marca con una
  `UserProperty` propia para que la edición atómica de fórmulas y la serialización (`documentMarkdown`)
  no confundan ambos casos.

---

## Alcance

Quedan **fuera** (distribución/infra, no features Qt puras; la auditoría los marca aparte): packaging
nativo, firma/notarización, auto-update y analítica de descargas. Este plan cubre solo las 18 mejoras
Qt-puras de la auditoría 2026-06-30.
