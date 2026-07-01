# Posibles mejoras

Lista de mejoras propuestas para **md-editor**, agrupadas y priorizadas. El
proyecto es sólido (~9k LOC, 23 ficheros de test, arquitectura por controllers,
9 idiomas), así que las mejoras son sobre todo de alcance y distribución.

> Leyenda: ✅ hecho · ⬜ pendiente. Las marcas reflejan el estado tras la última
> tanda de trabajo (cambio de idioma en caliente, casillas interactivas, notas al
> pie, reordenación de secciones y este CHANGELOG).

## 🎯 Alto impacto (poco esfuerzo / mucho valor)

1. ✅ **CI/CD con GitHub Actions** — automatizar el build de los 3 ejecutables
   (Linux/Windows/macOS) y la publicación de releases con tags. *Hecho:*
   `.github/workflows/release.yml` compila al empujar un tag `vX.Y.Z` (Linux
   AppImage, Windows ZIP portable, macOS DMG universal) y publica la Release con
   notas autogeneradas; `ci.yml` compila y pasa los tests en cada push/PR. Las
   actions están ya en Node.js 24 (`@v5`). *Corregido (v2.4.0):* el job de Linux
   moría por OOM del runner al compilar los ~44 ejecutables de test en paralelo
   (`cmake --build --parallel` sin tope de trabajos → SIGTERM, `exit 143`); ahora
   los tres jobs de release compilan solo `--target md-editor` (los tests ya los
   ejecuta `ci.yml`, la release no los necesita). La primera release v2.4.0 falló
   por esto y se rehízo re-etiquetando el tag sobre el commit del arreglo.
2. **Packaging para gestores nativos** — Flatpak/AppStream o AUR (Linux),
   winget/Chocolatey (Windows), Homebrew cask (macOS). Multiplica la visibilidad
   frente al `.AppImage`/`.zip` suelto y mejora la confianza.
3. **Firma de binarios** — macOS y Windows no están firmados (Ctrl-clic → Abrir).
   Firmar (y notarizar en Mac) elimina la fricción de instalación, que es donde
   se pierden usuarios.
4. **Auto-actualización o aviso de nueva versión** — al ser descarga manual, los
   usuarios de una versión antigua nunca sabrán de la nueva. Un chequeo ligero
   contra la API de releases bastaría.

## ✨ Funcionalidad

5. **Corrector ortográfico** — casi imprescindible en un editor de texto; Qt +
   Hunspell encaja bien con los 9 idiomas que ya soporta.

   **✅ Fase 1 hecha.** *Hecho:* módulo puro `spellscan` (`mdspell`):
   `tokenize` (extrae palabras Unicode con apóstrofos internos, descarta tokens
   con dígitos, separa por guion) y `pickDictionary` (elige el `.aff/.dic` para
   un idioma), con `tst_spellscan`. Hunspell ya es **dependencia opcional** en
   CMake (`SPELL_CHECK` → `HAVE_HUNSPELL`): si falta `libhunspell-dev`, el build
   sigue verde y el corrector queda inactivo. Además, clase motor `SpellChecker`
   (`spellchecker.{h,cpp}`): envuelve Hunspell tras un *pimpl* (el header no
   incluye Hunspell), **siempre se compila** (stub inerte sin el `-dev`), con
   carga perezosa de UN idioma, descubrimiento de diccionarios en rutas estándar
   (`pickDictionary`), codificación según el `.aff`, e ignorar/personal.
   `tst_spellchecker` (se salta si no hay diccionarios). Ya integrado en el
   resaltado: `CodeBlockHighlighter::highlightSpelling` subraya las erratas en la
   rama no-código (saltando código en línea, fórmulas y enlaces) con
   `SpellCheckUnderline`; `MainWindow::applySpellLanguage` carga el diccionario
   del idioma del documento (front matter › ajuste › locale) tras cada carga y al
   arrancar, y la lista personal se persiste en `AppSettings::personalDictionary`.
   También hay **menú contextual** (`MainWindow::showSpellContextMenu`, vía el
   filtro de eventos del viewport): sobre una errata, antepone hasta 8 sugerencias
   (un clic reemplaza) + «añadir al diccionario» / «ignorar», sobre el menú
   estándar. Y el interruptor **Ver → Corrección ortográfica** (acción checkable,
   `AppSettings::spellCheck`): al desactivar, descarga el diccionario (sin huella
   de memoria) y limpia el subrayado. **Ver → Idioma de corrección** permite fijar
   un idioma o dejarlo en automático (`AppSettings::spellLanguage`). Y el
   **empaquetado** para Windows/Mac está resuelto: Hunspell se enlaza **estático**
   (`SPELL_CHECK_STATIC`, el motor va dentro del binario — `ldd` no muestra
   dependencia), así no hay que desplegar ninguna biblioteca; y los `dictionaries/`
   (con README y `scripts/bundle-dictionaries.sh`) se instalan junto a la app donde
   `searchPaths` los busca. Resultado: paquete autocontenido. Fase 1 (y sus
   opcionales) completa. Si el corrector está activo pero **no hay diccionario**
   para el idioma (degrada en silencio), un aviso en la barra de estado lo indica
   —solo cuando el problema está presente— para que el usuario sepa por qué no
   subraya.

   **Análisis (multiplataforma y dependencias).** Qt6 no trae corrector
   (`QSpellChecker` no existe), hay que aportar el motor. La opción que mantiene
   el carácter portable del proyecto (sin `#ifdef Q_OS_*`) es **Hunspell** (C++
   puro; lo usan Chrome, Firefox, LibreOffice). Alternativas descartadas: APIs
   nativas (NSSpellChecker / ISpellChecker / enchant) obligan a código por
   plataforma y Objective-C++ en Mac; KDE Sonnet arrastra KDE Frameworks.
   - *¿Funciona en Win/Linux/Mac?* Sí con Hunspell: mismo C++ en los tres y la
     integración (subrayado `QTextCharFormat::SpellCheckUnderline` + menú de
     sugerencias) es API pura de Qt. **Matiz**: `QTextDocument` admite un solo
     `QSyntaxHighlighter` y ya lo ocupa `CodeBlockHighlighter`; el subrayado hay
     que integrarlo ahí o aplicarlo por `QTextEdit::setExtraSelections`.
   - *¿Dependencias / todo en el ejecutable?* El **motor** sería la primera
     dependencia de terceros, pero se puede vendorizar y enlazar estático → viaja
     dentro del binario. Los **diccionarios** (.aff/.dic) son datos (~30–50 MB
     los 9 idiomas): o embebidos en `.qrc` (binario más grande + extraer a
     temporal, porque Hunspell lee de rutas de fichero), o como ficheros junto al
     binario (vía `install.sh`), o descarga bajo demanda.
   - *Avisos:* licencias de diccionarios variadas (GPL/LGPL/MPL/BSD según idioma,
     revisar dado lo sensible que es la licencia aquí); y es la mejora de
     funcionalidad más cara tras multi-documento (1ª dependencia externa +
     integración en el highlighter único + empaquetado en 3 SO).
6. ✅ **Contador de palabras / tiempo de lectura / estadísticas** del documento.
   *Hecho:* módulo puro `docstats`, contador con tiempo de lectura en la barra de
   estado y diálogo *Ver → Estadísticas del documento…*.
7. ✅ **Pestañas o multi-documento** — *Hecho:* `MainWindow` es un *shell* con un
   `QTabWidget` de `EditorStack` (un documento por pestaña, cada uno dueño de sus
   ~15 colaboradores); `addTab`/`closeTab`/`openPathInTab` gestionan el ciclo de
   vida y la sesión (`AppSettings::openFiles`) reabre todas las pestañas al
   arrancar. Ver «Edición por pestañas» en `CLAUDE.md`.
8. ✅ **Export a DOCX** — ya hay PDF/HTML/ODT/LaTeX; `.docx` es el formato que más
   pide quien no usa Markdown. *Hecho:* serializador OOXML propio
   (`mdexport::writeDocx`) empaquetado con el QZip privado de Qt (sin
   dependencias), con encabezados, formato de carácter, listas, tablas, citas,
   código, enlaces (campo HYPERLINK) e imágenes embebidas; idioma y título
   incrustados. *Archivo → Exportar → A DOCX (Word)*.
9. **Diagramas** (Mermaid/PlantUML) — complementaría el soporte TeX existente.

   **✅ Hecho.** Enfoque elegido: **CLI externo opcional** (degradación
   elegante, como el corrector), ambos motores. *Hecho (backend):* módulo puro
   `diagram` (`mddiagram::kindForLanguage` clasifica ```mermaid/plantuml/puml/uml)
   y `DiagramRenderer` (ejecuta `plantuml`/`mmdc` async vía `QProcess`, cachea por
   fuente, emite `rendered`/`failed`). Sin dependencia de terceros enlazada: solo
   lanza procesos si existen. `tst_diagram` + `tst_diagramrenderer` (renderiza
   PlantUML real a PNG; QSKIP si la herramienta falta). *Hecho (Fase 2):*
   `DiagramController` coloca la imagen renderizada en un bloque de presentación
   **bajo** el bloque de código (opción «imagen debajo»); `diagramdoc` la marca y
   `documentMarkdown` la elimina del clon, así que el round-trip / `isModified` /
   guardado no la ven. Debounce 600 ms, caché por fuente, limpieza de previews
   huérfanas, y aviso en la barra de estado si hay diagramas y falta la
   herramienta. Completo.
   *Tensiona la filosofía:* requiere un motor externo (JS/Java).
10. ✅ **Insertar índice (TOC)** y ✅ **footnotes**. *Hechos:*
    `mdoutline::tableOfContentsMarkdown` + *Insertar → Índice (TOC)*; y el módulo
    puro `mdfootnote` + *Insertar → Nota al pie* (referencia `[^n]` autonumerada y
    su definición, render como superíndice y salto a la definición con un clic).
    El round-trip se respeta: Qt deja `[^id]` como texto literal y solo hace falta
    proteger el `:` de las definiciones de una palabra (que md4c tomaría por
    definición de enlace).
11. ✅ **Tema automático según el sistema** (seguir el modo claro/oscuro del SO).
    *Hecho:* opción *Ver → Tema → Seguir el sistema* vía `QStyleHints`.

## 🧹 Calidad de código

12. ✅ **Descomponer `mainwindow.cpp`** — *Hecho:* de 2155 a **671 líneas** (−69 %).
    Pasos: (a) iconos de la barra a un módulo puro `formaticons`; (b) construcción
    de menús + barra a `mainwindowmenus.cpp`; (c) filtro de eventos/entrada a
    `mainwindowinput.cpp`; (d) el corrector ortográfico a un `SpellController` real
    (posee motor + estado + menú contextual); (e) zoom/escalado de interfaz a
    `mainwindowzoom.cpp`; (f) sesión + recarga de disco a `mainwindowsession.cpp`.
    Los grupos (b), (c), (e) y (f) son métodos de `MainWindow` en otra unidad de
    traducción (como `texparser`), sin clases ni cambios de lógica. Queda en
    `mainwindow.cpp` el *shell*: ciclo de vida, pestañas (`addTab`/`closeTab`/
    `openPathInTab`/`setActiveStack`), idioma, estadísticas y *glue*.
13. ✅ **`mathblocks.cpp` (982 líneas)** — *Hecho:* extraído el motor de parseo
    TeX→runs/Unicode a `src/texparser.cpp` (puro, ~480 líneas: `renderTexAsRuns`,
    `texToUnicode`, `wrapTex` y sus tablas/helpers). `mathblocks.cpp` queda en
    ~510 líneas (scanning del fuente + integración con `QTextDocument`). Header
    único `mathblocks.h` (cero cambios en consumidores); cobertura intacta en
    `tst_mathblocks`/`tst_formulacontroller`.
14. **Static analysis en CI** — `clang-tidy` + compilar tests con ASAN/UBSAN.
    Para un parser con round-trip, los sanitizers cazan bugs de memoria que los
    tests funcionales no ven.
15. **Fuzzing del round-trip Markdown** — ya existe `tst_markdownroundtrip`; un
    fuzzer (libFuzzer) sobre "parse→serialize→parse == idempotente" da robustez
    con poco código.

## ✅ Mejoras baratas (hechas)

Lote de mejoras de bajo coste, **sin dependencias nuevas** y portables a los 3 SO
(Qt6 puro), ya implementadas:

- **Vista previa de impresión** — `ExportController::printPreview()`
  (`QPrintPreviewDialog`), *Archivo → Vista previa de impresión*.
- **Recordar la posición del cursor por archivo** — mapa acotado en `AppSettings`;
  `FileController` la guarda al cambiar de documento y al cerrar, y la restaura al
  abrir.
- **Abrir la carpeta contenedora** — `FileController::openContainingFolder()` vía
  `QDesktopServices`, *Archivo → Abrir carpeta contenedora*.
- **Pegar como texto plano** — *Editar* (Ctrl+Shift+V).
- **Buscar con regex y palabra completa** — casillas en `FindReplaceBar` (regex
  validada + `FindWholeWords`).
- **Copiar como HTML al portapapeles** — `ExportController::copyHtmlToClipboard()`,
  *Editar → Copiar como HTML*.
- **Transformar texto y ordenar líneas** — módulo puro `mdtext`
  (MAYÚSCULAS/minúsculas/Capitalizar/ordenar líneas), *Editar → Transformar texto*.
- **Cambio de idioma sin reiniciar** — *Ver → Idioma* recrea la ventana al vuelo
  (traductores intercambiables en `main()`), conservando el documento.
- **Casillas de tarea interactivas** — módulo `mdtask`; clic sobre la casilla
  marca/desmarca (`QTextBlockFormat::marker()`), round-trip nativo de Qt.
- **Notas al pie** — ver punto 10.
- **Reordenar secciones desde el esquema** — `mdoutline::moveSection` (pura) +
  arrastre en el panel de índice; mueve la sección con su contenido y subsecciones.

## ✨ Funcionalidad pendiente (nuevas ideas, encajan con la filosofía)

Sin dependencias nuevas y con el patrón habitual (función pura + `tst_`).

### Rápidas (poco coste, autocontenidas) — ✅ hechas

- ✅ **Insertar fecha/hora** — *Insertar → Fecha / Fecha y hora* (localizado).
- ✅ **Exportar/imprimir solo la selección** — *Imprimir selección* + *Exportar →
  Selección a PDF*.
- ✅ **Auto-enlazar al pegar URLs** — `mdurl::looksLikeUrl` +
  `InsertController::handlePastedUrl`.
- ✅ **Ir a encabezado (Ctrl+G)** — `GoToHeadingDialog` (quick open con filtro).
- ✅ **Tipografía inteligente** — `mdtext::smartTypography` (guiones, puntos
  suspensivos y comillas), en *Transformar texto*.
- ✅ **Shortcodes `:nombre:`** — `mdshortcode::expand` + expansión al teclear.

### Medias (Qt puro)

- ✅ **Pegar texto enriquecido como Markdown** — *Hecho:* módulo puro
  `richpaste` (`mdrichpaste::htmlToMarkdown`) pasa el HTML del portapapeles por un
  `QTextDocument` auxiliar y lo serializa con la ruta canónica
  (`mdtable::documentMarkdown`), normalizando el formato del origen al subconjunto
  de Markdown del editor en vez de incrustarlo. *Editar → Pegar como Markdown*
  (Ctrl+Alt+V); sin HTML, pega texto plano.
- ✅ **Plantillas de documento** — *Hecho:* módulo `doctemplates`
  (`mdtemplate::all()`) con 10 esqueletos Markdown (acta, nota diaria, blog,
  README, carta, informe, lista de tareas, certificado, práctica de asignatura,
  examen) en *Archivo → Nuevo desde plantilla*. Los textos van por `tr()` (no
  `.qrc`), traducidos a los 9 idiomas; `DocumentIo::loadFromString` los abre como
  documento nuevo, modificado y sin ruta. (Los **snippets de usuario** quedan
  pendientes.)
- ✅ **Admoniciones/callouts** (`> [!NOTE]`, `> [!WARNING]`) con round-trip.
  *Hecho:* módulo puro `admonitions` (`mdadmonition`); estilo de callout (fondo +
  título en color) al maquetar e inserción desde *Insertar → Admonición*. El
  marcador sobrevive sin escape (`unescapeMarkers` en `documentMarkdown`), apto
  para GitHub.
- ✅ **Matemáticas "Nivel 2"** — *Hecho:* layout 2D con un `QTextObjectInterface`
  propio (`MathObject`) sobre un motor de cajas puro (`mathlayout`): fracciones
  apiladas con barra real y grandes operadores (`\sum`/`\int`/`\prod`…) con
  límites encima/debajo. Solo se activa (objeto) cuando la fórmula lo necesita
  (`needsTwoDLayout`); el resto sigue como runs. Escala con el zoom (mide con la
  fuente del documento) y `cloneForExport` lo expande a runs inline para que
  HTML/ODF/PDF/DOCX se exporten igual que antes. **2ª tanda:** añadidos `\sqrt`
  con vínculo (e índice `\sqrt[n]`) y matrices (`pmatrix`/`bmatrix`/…) con
  delimitadores dibujados. **3ª tanda:** `\binom` apilado, `\begin{cases}` (llave
  izquierda + celdas alineadas), acentos (`\hat`/`\bar`/`\vec`/`\tilde`/`\dot`/…
  por caracteres combinantes Unicode, también inline) y `\text{…}`/`\mathrm{…}`.
  El centrado vertical de las 2D inline **no es viable** con `QTextObjectInterface`
  (Qt ancla el objeto por su base al baseline y no expone ascenso/descenso); queda
  como limitación. **4ª tanda (cobertura de comandos):** ampliada la tabla de
  glifos (delimitadores `\langle`/`\lceil`/`\lfloor`/`\Vert`, relaciones y
  conjuntos `\perp`/`\parallel`/`\prec`/`\sqsubseteq`/…, grandes operadores inline
  `\bigcup`/`\bigvee`/`\bigoplus`/…, más flechas y `\dagger`/`\therefore`); además
  `\left`/`\right` (emiten el delimitador, sin escalar), `\not X` (negación con
  combinante), `\quad`/`\qquad`, `\underline` y `\mathcal`/`\mathscr`/`\mathfrak`
  (letras script/fraktur del plano astral). Todo inline + 2D, con tests en
  `tst_mathblocks`. *Descartado por coste/encaje:* entornos multilínea con `&`
  (`align`/`aligned`, exigen layout nuevo), `\overbrace`/`\underbrace` y
  `\xrightarrow` (2D propio), y el escalado real de `\left`/`\right`.
- ✅ **Export a EPUB** — *Hecho:* `mdexport::writeEpub` empaqueta un EPUB 3
  (mimetype + OPF + nav.xhtml + toc.ncx + XHTML) con el QZip privado, reutilizando
  el HTML de Qt saneado a XHTML (`htmlBodyToXhtml`) e incrustando las imágenes como
  PNG. Idioma y título del front matter. *Archivo → Exportar → A EPUB*.

### Pulidos de edición pendientes (Qt puro, sin dependencias)

Lo poco barato/mediano que aún encaja con la filosofía tras la comparación con
otros editores (Typora, Ghostwriter…). El grueso de funcionalidad está maduro: el
mayor margen restante es de **distribución** (puntos 2-4 de arriba), no de features.

- ✅ **Snippets de usuario** — *Hecho:* fragmentos Markdown reutilizables que el
  usuario define e inserta por nombre. Módulo puro `snippets` (`mdsnippet`:
  modelo + (de)serialización para QSettings, con `tst_snippets`); se persisten en
  `AppSettings::snippets()`. Diálogo gestor `SnippetsDialog` (añadir/editar/
  eliminar) y submenú dinámico *Insertar → Snippet* (uno por snippet +
  «Gestionar…»). `EditorStack::insertSnippet` inserta el cuerpo crudo en la vista
  de fuente y renderizado (`QTextDocumentFragment::fromMarkdown`) en WYSIWYG, así
  que funciona en ambos modos. (La expansión al teclear un disparador queda como
  posible añadido futuro; se evitó por solaparse con los shortcodes `:nombre:`.)
- ✅ **Modo foco (máquina de escribir + foco de línea)** — *Hecho:* *Ver → Modo
  foco* es un único interruptor que (a) mantiene la línea del cursor centrada en
  vertical mientras se escribe y (b) atenúa todo el documento salvo el párrafo del
  cursor. Lógica pura en `typewriter` (`mdtypewriter::centeredScrollValue` para el
  scroll y `mdtypewriter::dimRanges` para los tramos a apagar, con `tst_typewriter`);
  la integración vive en `EditorStack::centerCursorLine` y `applyLineFocus`
  (recalculan en cada `cursorPositionChanged` y tras cargar, en ambos editores). El
  color apagado es la mezcla a medias del texto con el fondo (se rehace al cambiar
  de tema); la atenuación usa `QTextEdit::extraSelections`, así que no toca el
  documento ni el round-trip. Interruptor por ventana aplicado a todas las pestañas
  y persistido en `AppSettings::typewriterMode`.
- ✅ **Limpieza/normalización del Markdown** — *Hecho:* módulo puro `markdowntidy`
  (`mdtidy::tidy`, con `tst_markdowntidy`), acción *Editar → «Limpiar Markdown»*
  vía `EditorStack::cleanMarkdown` (en vista fuente sobre su texto; en WYSIWYG
  re-serializa, limpia y recarga). Conservador: recorta espacios finales
  (preservando el salto duro de 2+ espacios), colapsa líneas en blanco múltiples,
  normaliza viñetas a `- ` y el espacio tras los `#`, y deja un único salto final.
  No toca el interior de fences ``` ``` ```/`~~~` ni las reglas temáticas.
- ✅ **Auto-emparejado** — *Hecho:* módulo puro `autopair` (`mdautopair::apply`
  sobre un `QTextCursor`, con `tst_autopair`), enganchado en los dos manejadores de
  teclado (WYSIWYG y fuente) vía `MainWindow::applyAutoPair`. Al teclear `(`/`[`/
  `{`/`` ` `` auto-cierra (cursor en medio), envuelve la selección si la hay, y
  hace *type-over* del cierre tecleado delante de su gemelo. No auto-cierra si lo
  siguiente es palabra (evita «()texto»). Se omiten `*`/`_` a propósito (chocan con
  listas, `snake_case` y la multiplicación; el énfasis va por la barra). En WYSIWYG
  la protección de fórmulas actúa antes, así que no interfiere dentro de una math.

*Con tensión de filosofía, dejados fuera a propósito:* árbol de carpetas lateral
(empuja hacia «workspace»/PKM) e importar `.docx`/`.odt` a Markdown vía Pandoc
opcional (añadiría otra herramienta externa, como los diagramas).

### Nueva auditoría (2026-06-30): oportunidades Qt puro, coste bajo/medio — ✅ COMPLETADA

> **✅ Las 18 mejoras (+ refactors R1–R9) están hechas y publicadas** (2.4.0 y 2.5.0,
> 2026-07-01). Las marcas de abajo lo reflejan.
>
> **Plan de ejecución:** el *cómo* de estas 18 mejoras (orden por riesgo, dependencias,
> módulos/tests, puntos de integración y refactorizaciones previas) vive en
> [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md).

Resultado de una auditoría sistemática del código (8 exploradores por subsistema +
verificación adversarial contra el repo: 47 candidatos en bruto → 18 confirmados, 0
rechazados). Las 18 se comprobaron como **no implementadas, sin dependencias nuevas
(ni enlazadas ni en runtime) y con el round-trip a salvo**; ninguna estaba ya
descartada. Patrón habitual: módulo puro + `tst_` + integración en el controller.
Coste y confianza según la verificación.

#### Coste bajo, confianza alta (primera tanda recomendada) — ✅ entregadas en 2.4.0

> Las 8 de esta tanda se implementaron en la versión **2.4.0** (fases 1–2 del
> [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md)): cada una con módulo puro + `tst_` +
> ayuda en los 9 idiomas + CHANGELOG.

- ✅ **Copiar como Markdown** (selección o documento) — contrapartida de «Pegar como
  Markdown» y «Copiar como HTML». *Impl.:* `mdrichpaste::fragmentToMarkdown` (crea un
  `QTextDocument` con el fragmento y lo pasa por `mdtable::documentMarkdown`) +
  `ExportController::copyMarkdownToClipboard` (clon de `copyHtmlToClipboard`) + acción
  en *Editar* junto a «Copiar como HTML». `tst_richpaste`.
- ✅ **Exportar a texto plano (.txt)** — encaja en el patrón declarativo
  `FileExporter`/`runExport`. *Impl.:* `ExportController::exportPlainText` con lambda
  `writeUtf8File(path, doc->toPlainText())`; entrada en *Archivo → Exportar*.
- ✅ **Metadatos del PDF (título/autor) desde el front matter** — hoy el PDF no usa
  `title`/`author`. *Impl.:* `mdexport::pdfDocumentInfo(frontMatter)` +
  `printer.setDocName()/setCreator()` en `exportPdf`/`exportSelectionPdf` (`author` →
  `setCreator`; Qt6 no tiene `setAuthor`). `tst_exporters`.
- ✅ **Revertir a lo guardado** (recarga manual con confirmación) — la lógica ya
  existe: `MainWindow::reloadFromDisk()`, hoy solo la invoca el `DiskWatcher`. *Impl.:*
  acción en *Archivo* → `revertToSaved()` con `QMessageBox` de confirmación; habilitada
  solo si hay archivo y cambios.
- ✅ **Ir a línea (Ctrl+L)** — complementa «Ir a encabezado» (Ctrl+G). *Impl.:*
  `mdnav::clampLine` + `QInputDialog::getInt` + `findBlockByNumber` sobre
  `activeEditor()`; **no** añadir a `m_wysiwygActions` (útil en fuente/dividida).
  `tst_gotoline`.
- ✅ **Indicador Ln/Col en la barra de estado** — engancha a la señal ya cableada
  `wordCountShouldUpdate`. *Impl.:* `mdstats::lineColumnOf` + `QLabel` permanente espejo
  de `m_countLabel`; conmutable en *Ver* y persistido. `tst_docstats`.
- ✅ **Pegar TSV/CSV del portapapeles como tabla** — *matiz:* el HTML de hojas de
  cálculo ya lo cubre «Pegar como Markdown»; el hueco real es **texto plano** TSV/CSV.
  *Impl.:* módulo puro `mdcsvtable` (`detectDelimited` conservador + `toMarkdownTable`
  escapando `|`), inserta con `fromMarkdown`; acción *Insertar → Tabla desde
  portapapeles* y, opcional, en la cadena de pegado solo para TAB. `tst_csvtable`.
- ✅ **Reabrir pestaña cerrada** — *matiz:* `Ctrl+Shift+T` está ocupado (Lista de
  tareas) → usar otro atajo. *Impl.:* pila pura `session::pushClosed/popClosed`;
  `closeTab` apila la ruta y un slot la desapila con `openPathInTab`. Solo recupera
  documentos con ruta en disco. `tst_closedtabstack`.

#### Coste medio, confianza alta

- ✅ **Paleta de comandos (Ctrl+Shift+P)** *(2.5.0)* — acceso por teclado a las ~80 acciones sin
  memorizar atajos; la de mayor impacto en usabilidad. *Impl.:* módulo puro
  `mdcommands` (`collectCommands` recorre `menuBar()->actions()` recursivamente, salta
  separadores/contenedores/deshabilitadas, arma ruta+atajo; `filterCommands` difuso) +
  `CommandPaletteDialog` (clon de `GoToHeadingDialog`) que dispara `action->trigger()`.
  `tst_commands`.
- ✅ **Ordenar filas de tabla por columna** *(2.5.0)* (numérica/alfabética, contextual en menú
  Tabla). *Impl.:* módulo puro `mdtablesort::sortedOrder` + `TableController::sortRows`;
  preservar formato/fórmulas moviendo `QTextDocumentFragment` por celda;
  `m_tableActions` habilita por contexto. `tst_tablesort`.
- ✅ **Promover/degradar nivel de encabezado** *(2.5.0)* (relativo, p. ej. Ctrl+[ / Ctrl+]) — hoy
  `applyHeading` solo fija niveles absolutos. *Impl.:* `mdoutline::shiftHeadingLevels`
  (clamp [1,6] y subárbol opcional) + variante cursor en `FormatController` **sin** la
  semántica *toggle* (que borraría el encabezado al promover). `tst_outline`.
- ✅ **Resaltar la línea actual del cursor** *(2.5.0)* — *Impl.:* `linehighlight::currentLineColor`
  (mezcla `Base`→`Highlight`) + `ExtraSelection` con `FullWidthSelection`; **refactor**
  de `applyLineFocus` a una lista fusionada (hoy es el dueño único de `extraSelections`).
  Conmutable en *Ver*. `tst_linehighlight`.
- ✅ **Búsqueda «N de M» + resaltar todas las coincidencias** *(2.5.0)* — el contador es de coste
  bajo (reusa el bucle de `replaceAll`); resaltar-todas es medio porque comparte
  `extraSelections` con el modo foco (mismo refactor a lista fusionada). *Impl.:*
  `mdfind::matchOrdinal` + `QLabel` en la barra; señal `highlightMatches` de
  `FindReplaceBar` al `EditorStack` activo. `tst_findmatches`.
- ✅ **Cabecera/pie con número de página al imprimir y en PDF** *(2.5.0)* («3 / 10», título,
  fecha). *Impl.:* módulo puro `mdprintdecor` (`headerFooterText` + `bodyRect`) + helper
  `paintPaginated(QPrinter*, QTextDocument*)` con `QPainter`/`newPage()` que sustituye
  los 5 `print()` actuales; flags en `AppSettings`. `tst_printdecor`.

#### Extensiones Markdown inline (round-trip con matiz, confianza media)

- ✅ **Marca `==texto==`** (resaltado) *(2.5.0)* — `=` no es delimitador en md4c, así que
  round-trip-ea como texto literal; solo presentación por `QPalette::Highlight` (sin
  `setStyleSheet`). Coste bajo. *Impl.:* módulo puro `mdmark::spansIn` + pasada de
  render en `mdrender::renderPasses` + acción que **inserta** los `==` literales (no un
  char-format). `tst_markhighlight` + caso en `tst_markdownroundtrip`.
- ✅ **Superíndice/subíndice de texto `^x^` / `~x~`** *(2.5.0)* — *matiz:* `~x~` choca con
  `~~tachado~~` de GFM → proteger el `~` simple con centinela PUA antes de `setMarkdown`
  (sobrescribe el tachado de tilde simple; precedente en Typora); `AlignSuperScript`
  pasaría a compartirse entre 3 features → desambiguar con una `UserProperty` nueva.
  *Impl.:* módulo puro `mdsupsub` (proteger + render + reinyección en
  `documentMarkdown`). `tst_supersub`.

#### Coste medio, confianza media

- ✅ **Comandos de línea: mover / duplicar / borrar / unir** *(2.5.0, solo vista de código)* (Alt+↑/↓, Ctrl+J…) —
  *matiz crítico:* reordenar `QTextBlock`s en crudo en el editor WYSIWYG es arriesgado
  (tablas, objetos `MathObject`, notas) → operar sobre el **cuerpo Markdown** y recargar
  con `setBodyMarkdown` (patrón de `cleanMarkdown`/`moveSection`); en la vista de fuente
  es trivial. `Ctrl+Shift+K` (Bloque) y `Ctrl+K` (Enlace) están ocupados → usar
  `Alt+↑/↓`, `Ctrl+J`, etc. *Impl.:* módulo puro `mdmoveline` que respeta los fences y
  no toca el front matter. `tst_moveline`.
- ✅ **Panel de esquema: filtro en vivo + plegado persistente** *(2.5.0)* — hoy `rebuild()` hace
  `expandAll()` incondicional en cada edición (molesto en documentos largos). *Impl.:*
  `mdoutline::visibleOrdinals` (conserva ancestros) + `QLineEdit` de filtro; recordar
  las ramas plegadas entre reconstrucciones; acciones «Expandir/Plegar todo».
  `tst_outline`.

*Fuera de alcance por ser distribución/infra (no feature Qt pura):* los puntos 2–4 de
arriba (packaging nativo, firma/notarización, auto-update) y la analítica (#18).

### Pulido de UX: reducir la ventaja de Typora (2026-07-01)

De comparar md-editor con Typora/MarkText/Ghostwriter/Obsidian: la funcionalidad está
madura, pero Typora se percibe **más pulido**. Esa brecha no es magia; se reduce a tres
cosas —fluidez de entrada, tipografía del documento y *affordances* en el bloque—. Todo
lo de abajo es Qt puro, sin dependencias, **sin `setStyleSheet`** (theming por
paleta/formatos, como el resto del proyecto) y con el round-trip a salvo.

#### Alto impacto (cierran la mayor parte de la brecha)

- ✅ **Reglas de entrada («teclea Markdown y se formatea»)** — lo que hace *mágico* a
  Typora: teclear `## `, `**x**`, `> ` o ``` ```lang ``` y que se transforme en el sitio,
  sin barra ni atajos. Hoy en md-editor se formatea con Ctrl+B/barra. *Impl.:* módulo puro
  `mdinputrules` (sobre un `QTextCursor`, detecta el patrón recién tecleado y aplica el
  formato equivalente) enganchado en `handleEditorKeyPress` —mismo patrón que `mdautopair`/
  `mdshortcode`/continuación de listas, que ya cubren media familia—. Cubre encabezados
  `#`…`######`, `**`/`*`/`~~`/`` ` ``, cita `> `, listas `- `/`1. `, regla `---` y fence
  ``` ```lang ```. Produce los **mismos formatos que la barra**, así que el round-trip es
  idéntico. `tst_inputrules`. Coste medio, confianza alta. **La que más se nota.**
  *(Hecho: solo marcadores de bloque `#`/`>`/`-`/`1.`; las inline `**x**`/`` `x` `` quedan
  pendientes de una segunda tanda.)*
- ✅ **Pasada de tipografía del documento renderizado** — el render de Qt sale plano; el de
  Typora está compuesto (ritmo de encabezados, interlineado, citas, código, ancho de
  lectura). *Hecho:* módulo puro `mdtypography` (`markdown/typography.{h,cpp}`) con
  `apply(doc)` que recorre los bloques y mergea `QTextBlockFormat` — márgenes de encabezado
  por nivel, espacio entre párrafos, panel de código con fondo translúcido + sangría, y
  cita con fondo tenue; presentación pura (preserva `isModified`, no toca el round-trip),
  aplicada en `EditorStack` tras `styleTables`/`applyLineSpacing`. Tinte **translúcido**
  (como las admoniciones) para no reaplicar al cambiar de tema; salta tablas, ítems de lista
  y las citas que ya son admoniciones. `tst_typography`. La **barra lateral** de la cita
  (como `QTextBlockFormat` no tiene borde de bloque) se pinta en `FocusEditor::paintEvent`
  con `cursorRect` (coordenadas de viewport: respeta scroll, sangría y columna centrada):
  gris en una cita normal, del color del acento en una admonición (completa el «callout»);
  el color lo decide `mdtypography::quoteBarColor` (puro, testado). *Pendiente de una posible
  tanda futura:* el **ancho de lectura máximo**.
- ⬜ **Ventana de Preferencias única** — hoy los ajustes están repartidos por *Ver → …* +
  `AppSettings`. *Impl.:* diálogo `PreferencesDialog` con pestañas (General, Editor,
  Apariencia, Exportación) que **reúne** lo que ya expone `AppSettings` (nada nuevo que
  persistir). Mejora el descubrimiento y aparenta madurez de golpe. Coste medio, riesgo
  bajo, confianza alta. **La que más madura aparenta.**

#### Affordances en el bloque

- ⬜ **Barra flotante de tabla + navegación por celdas** — punto fuerte reconocido de
  Typora. *Impl.:* un `QWidget` superpuesto al viewport que aparece cuando el cursor está
  en una tabla (añadir/quitar fila-columna, alinear, mover columna, borrar), reusando
  `TableController`; y **Tab/Shift+Tab** entre celdas (Tab en la última añade fila).
  Posicionado a mano sobre el viewport, sin stylesheet. Coste medio.
- ⬜ **Bloque de código: etiqueta de lenguaje + botón copiar** — mostrar el lenguaje en el
  fence, un desplegable para cambiarlo (rehace el resaltado vía `LanguageRegistry`) y un
  botón *copiar* al pasar el ratón. *Impl.:* overlay/acciones sobre el grupo de bloques de
  código. Coste bajo-medio, alto retorno percibido.

#### Toques menores de acabado

- ⬜ **Documento de bienvenida en el primer arranque** (reusa `doctemplates`) y
  **placeholder** sutil en el documento vacío. Coste bajo.
- ⬜ **Popover al pasar por un enlace** (hoy la URL solo sale en la barra de estado). Coste bajo.

*Ya en el roadmap y también «pulido» de UX (no duplicar):* la **paleta de comandos** (#9),
el **filtro + plegado del esquema** (#18) y **resaltar la línea actual** (#12) de la
auditoría 2026-06-30.

*Fuera de alcance a propósito (fricción con la arquitectura):* estética tipo CSS pesada
(esquinas redondeadas, sombras, animaciones) —md-editor es paleta/formatos, sin
`setStyleSheet`; imitar el CSS de Typora empuja hacia donde Electron gana— y **redimensionar
imágenes con tiradores** (Typora usa `width` de CSS; en `QTextDocument` complicaría el
round-trip; a lo sumo, alineación).

### Plantillas por profesión: agrupar y ampliar «Nuevo desde plantilla» (2026-07-01)

Ampliar *Archivo → Nuevo desde plantilla* **agrupando** las plantillas en submenús por
categoría profesional y **añadiendo** más (para programadores, docentes, abogados,
académicos…). Diseño acordado.

**Coste clave — i18n.** Hoy, en `doctemplates.cpp`, el **nombre y el cuerpo completo** de
cada plantilla pasan por `translate("MainWindow", …)`, así que cada plantilla nueva hay que
traducirla a los 8 idiomas (y `tst_translations` lo exige). Además, las plantillas
**jurídicas dependen de la jurisdicción** (una «demanda» española no equivale a un
*pleading* de *common law*, ni a los sistemas alemán/polaco/rumano): traducirlas produciría
documentos legalmente sin sentido. Por eso el enfoque es **mixto**.

**Categorías (7)** — un submenú por categoría; una categoría vacía no se muestra:

| Categoría | De fábrica (traducidas ×9) | Vía plantillas de usuario |
|---|---|---|
| Personal / General | nota diaria, carta, lista de tareas, certificado (+ diario) | — |
| Programación | README + **CHANGELOG, ADR, informe de bug** | sí |
| Académico / Investigación | **artículo (IMRyD), abstract, informe de laboratorio** | sí |
| Docencia | examen, práctica (+ opc. rúbrica) | sí |
| Empresa / Negocios | acta de reunión (+ opc. orden del día, memo) | sí |
| Derecho | — (jurisdicción) | **todas** |
| Sanidad / Escritura | — (regulado / blog ya existe) | sí |

**Enfoque mixto:** de fábrica traducidas **solo** donde la convención es global y luce las
fortalezas de md-editor (Programación → código/diagramas; Académico → TeX + LaTeX + notas al
pie); **plantillas de usuario** para el resto (Derecho, Sanidad y lo local), que esquivan a la
vez el coste de traducción y el problema de jurisdicción.

**Fases (de barato a caro):**

- ⬜ **Fase A — Agrupar (casi gratis).** *Impl.:* `mdtemplate::Category` (enum) +
  `categoryName()` por `tr()` + campo `category` en `DocTemplate`; recategorizar las 10
  actuales; el menú monta submenús por categoría. Solo se traducen los ~7 nombres de
  categoría. `tst_doctemplates`. Riesgo bajo. **Reorganiza el menú sin escribir plantillas
  nuevas.**
- ⬜ **Fase B — Plantillas de usuario (núcleo escalable).** *Impl.:* *Archivo → Guardar como
  plantilla…* (pide nombre + categoría), persistidas en `AppSettings` (modelo puro tipo
  `snippets`: `mdusertemplate` de (de)serialización + `tst_usertemplate`), diálogo gestor, y
  el menú mezcla usuario + fábrica por categoría. Esquiva i18n y jurisdicción. Riesgo medio.
  Es lo que hace útiles a Derecho/Sanidad sin traducir nada.
- ⬜ **Fase C — Plantillas de fábrica curadas.** *Impl.:* las ~6‑8 nuevas de Programación/
  Académico, cada una nombre + cuerpo por `tr()`, traducidas a los 9 idiomas (la parte cara
  pero acotada); Docencia/Empresa opc. 1‑2; Derecho/Sanidad cero de fábrica.

### Fuente configurable del documento y la exportación (2026-07-01)

Elegir la **familia tipográfica global** del documento (la del editor en pantalla
y/o la de la salida a PDF/HTML/ODF/EPUB), como preferencia de **presentación**.

**Contexto.** Markdown no expresa la tipografía: solo estructura y semántica. No hay
forma de poner «distintas fuentes» por fragmento salvo HTML incrustado —desactivado
a propósito aquí (`MarkdownNoHTML`), no haría round-trip— o CSS de exportación. Lo
único compatible es una fuente **global**, que no se serializa al `.md`. Por eso NO
se contempla un selector de fuente por texto (se perdería al guardar).

- ⬜ **Fuente global.** Un ajuste (en *Ver* y/o en el diálogo de exportación) para la
  familia de cuerpo y, aparte, la de código (monoespaciada). Persistir en
  `AppSettings`; aplicar a `document()->defaultFont()` (pantalla) y, en exportación,
  a `cloneForExport` (que hoy ya normaliza el **tamaño** a 11 pt; añadiría la
  **familia**). Riesgo bajo, es presentación pura. Encaja con el arreglo reciente del
  tamaño de letra en impresión/exportación (el zoom de pantalla ya no se cuela).

### Importar desde otros formatos (2026-07-01)

Hoy no hay importación real: *Abrir* acepta `.md`/`.markdown`/`.txt` pero **todo se
carga como Markdown** (`setMarkdownWithExtensions`); un `.html` se vería como texto
literal (por `MarkdownNoHTML`). Un *Archivo → Importar* convertiría a Markdown y
abriría el resultado como documento **nuevo sin título** (vía `loadFromString`, como
las plantillas), para no pisar el original.

**Piezas ya existentes que reutilizar:**

- `mdrichpaste::htmlToMarkdown` (HTML → Markdown con `QTextDocument::setHtml` +
  `mdtable::documentMarkdown`), hoy solo para *Pegar como Markdown*.
- Patrón de herramienta externa por `QProcess` con degradación elegante
  (`DiagramRenderer` con plantuml/mmdc).
- QZip privado de Qt (ya usado para exportar DOCX/ODF/EPUB) sirve para **leer** esos
  zips al importar.
- El *handler* de pegado que guarda imágenes a disco con ruta relativa (patrón para
  extraer las imágenes embebidas/referenciadas de lo importado).

**Formatos y viabilidad:**

| Formato | Mecanismo | Coste | Calidad | Deps |
|---|---|---|---|---|
| HTML (.html) | reusar `htmlToMarkdown` | muy bajo | buena (HTML sencillo) | Qt puro |
| EPUB (.epub) | QZip → XHTML → `htmlToMarkdown` | bajo-medio | buena | Qt puro |
| DOCX (.docx) | QZip → `document.xml` (`QXmlStreamReader`) → Markdown | medio-alto | media (lossy) | Qt puro |
| ODT (.odt) | igual, `content.xml` (ODF) | medio-alto | media | Qt puro |
| RTF, LaTeX, reST, MediaWiki, org… | `pandoc -f X -t markdown` por `QProcess` | bajo/formato | alta | Pandoc opcional |
| PDF | — | — | muy lossy | fuera de alcance |

**Recomendación (coste/beneficio):**

- ✅ **HTML → Markdown** — fruta madura: reusa `htmlToMarkdown`, Qt puro, alto valor.
  *Hecho:* *Archivo → Importar → Desde HTML…* (`MainWindow::importHtml`) → módulo puro
  `mdimport` (`markdown/htmlimport.{h,cpp}`: `decodeHtml`/`charsetOf`, decodificación
  charset-aware BOM › `<meta charset>` › UTF-8) → `mdrichpaste::htmlToMarkdown` →
  `newFromTemplate` en pestaña nueva (documento sin título, no pisa el original).
  `tst_htmlimport`. *Limitación:* las imágenes con ruta relativa apuntan a la ubicación
  del HTML original (no se copian ni reescriben); windows-1252 cae a UTF-8 (Qt6 core no
  lo decodifica). *Pendiente:* **Pandoc opcional** para DOCX/ODT/RTF/LaTeX/…
- ⬜ **Pandoc opcional** — máximo apalancamiento: una integración `QProcess` cubre
  DOCX/ODT/RTF/LaTeX/reST/… con calidad alta y degradación elegante, sin dependencia
  enlazada (mismo patrón que los diagramas).
- ⬜ **Importador nativo DOCX/ODT** (sin Pandoc) — solo si se quiere sin herramientas
  externas; más trabajo y lossy. DOCX es el más interesante (cierra el círculo con el
  export DOCX ya existente).

Caveats transversales: extraer las **imágenes** a disco junto al `.md` y abrir como
documento nuevo modificado para no sobrescribir la fuente.

### Robustez

- ✅ **ASAN/UBSAN + clang-tidy en CI** — *Hecho:* opción CMake `ENABLE_SANITIZERS`
  (ASan+UBSan, con `vptr` desactivado por el ruido de Qt) y `.clang-tidy` con
  checks de alta señal; dos *jobs* en `ci.yml` que corren la suite bajo sanitizers
  y clang-tidy (fail-on-warning) en cada push/PR. La primera pasada cazó un bug de
  un test (`QChar` de un carácter astral) y 3 avisos de clang-tidy, ya corregidos.
- ✅ **Fuzzing del round-trip** Markdown — *Hecho:* `tst_roundtripfuzz` genera 6000
  documentos adversarios con un PRNG determinista (semillas fijas → reproducible) y
  los pasa, dos veces, por la ruta real `setMarkdownWithExtensions` →
  `documentMarkdown`, exigiendo que **no crashee** (la red de verdad bajo
  ASan/UBSan, lo corre CI). *No* exige convergencia: el fuzzeo destapó que
  `QTextDocument::toMarkdown` de Qt **no es idempotente** por motivos ajenos a la
  app (la maquinaria propia —fórmulas, tablas, notas, admoniciones— sí converge en
  aislamiento; lo verifica `tst_markdownroundtrip`). Limitaciones de Qt halladas,
  por si compensa mitigarlas algún día en `documentMarkdown`:
  - **Code spans con `\` o `&` — *mitigado*.** Qt los escapa al serializar pero los
    re-lee literales → se duplicaban en cada guardado (`` `C:\x` `` se corrompía).
    `mdcodespan::unescapeInlineCode` (en `documentMarkdown`) revierte ese
    sobre-escapado dentro de los code spans en línea; ya converge.
  - **`<...>` — *mitigado*.** Qt lo trataba como HTML en línea y **se tragaba ese
    texto y el de alrededor** al cargar (pérdida de datos). El editor carga y guarda
    con el flag `MarkdownNoHTML` (`mdrender::kMarkdownFeatures`): los `<...>` son
    texto literal —lo correcto en un WYSIWYG que no ejecuta HTML— y el round-trip
    converge.
  - **Line-wrapping / fence tras lista — *no se tocan* (cosmético).** Qt parte las
    líneas a ~80 columnas (un corte dentro de un `*…*` lo vuelve literal) e indenta
    el cierre de un *fence* que sigue a una lista. Ambos **se estabilizan en una
    pasada** (no crecen, no hay pérdida de datos); arreglarlos exigiría des-wrappear
    la salida de Qt, con riesgo alto y valor casi nulo.
- ✅ **Golden tests de exportadores** — *Hecho:* `tst_goldenexport` fija la salida
  exacta de los serializadores propios y deterministas para un documento canónico,
  contra referencias en `tests/golden/` (LaTeX, el XML de DOCX/ODF, las piezas del
  EPUB con uuid/fecha fijos, el saneado HTML→XHTML). Falla ante cualquier cambio de
  salida; se regeneran con `UPDATE_GOLDEN=1`. No se fija el HTML de
  `QTextDocument::toHtml` (es de Qt y cambia entre versiones: avisaría de cambios de
  Qt, no de regresiones nuestras).
- ✅ **Accesibilidad** — desarrollado en su propia sección, [♿ Accesibilidad](#-accesibilidad)
  (los dos únicos puntos sin cerrar quedaron descartados: foco explícito por coste y
  prueba con lector real por falta de medios).

### Hechas en esta tanda

- ✅ **Símbolos especiales por categorías** — *Insertar → Símbolos especiales…*
  (módulo `mdsymbols` + diálogo `SymbolPicker`, 8 categorías).

## ♿ Accesibilidad

Ya hay una **primera capa** de accesibilidad (ver lo marcado ✅ abajo): nombres
accesibles en los widgets sin etiqueta y los mensajes de estado anunciados a los
lectores. Sobre esa base se apoya el resto. Hay además base previa aprovechable —el
tema de **alto contraste** real (21:1), el zoom de interfaz persistente, ~33 atajos
de teclado, 25 tooltips y el alt text de las imágenes—, pero un lector de pantalla
(NVDA/JAWS en Windows, Orca en Linux, VoiceOver en macOS) todavía recibe poca
información semántica en algunos puntos. Todo lo de abajo es **Qt6 puro, sin
dependencias nuevas**: Qt traduce sola su API de accesibilidad a AT-SPI/UIA/
NSAccessibility en cada SO, así que encaja con la filosofía del proyecto. *La
validación con un lector real (Orca/NVDA/VoiceOver) queda **descartada** por falta de
medios: todo está cableado contra la API de Qt, que es lo máximo verificable sin AT en
vivo.*

> Leyenda: ✅ ya existe · 🚧 parcial · ⬜ pendiente · 🚫 descartado (no viable / no-objetivo).

### Alto impacto / poco esfuerzo

- ✅ **Nombres y descripciones accesibles** — *Hecho (nombres):* `setAccessibleName`
  en los widgets de los que un lector no podía derivar nombre: editor WYSIWYG y
  editor de fuente (que comparten clase y coexisten en vista dividida), los campos
  Buscar/Reemplazar de la barra de búsqueda, el árbol del esquema, el contador de
  palabras y el filtro/lista del diálogo «Ir a encabezado». Los `QAction` (las ~33
  acciones de menús y los botones **solo-icono** de la barra) ya exponen su `text()`
  como nombre accesible de serie. *Hecho (descripciones):* `setAccessibleDescription`
  donde el nombre se queda corto — el editor WYSIWYG (aclara que el formato se aplica
  sobre el texto renderizado) y el árbol del esquema (pista de uso). Los **diálogos**
  propios quedan cubiertos por el *buddy* automático de `QFormLayout` (ver punto de
  abajo).
- ✅ **Anunciar los mensajes de estado efímeros** — *Hecho:* helper único
  `MainWindow::showStatusMessage` por el que pasa el feedback importante (guardado,
  exportado, «no encontrado», nº de reemplazos, regex inválida, diccionario ausente,
  cambios/recarga en disco, «sin encabezados», nota sin definición); además de
  mostrarlo en la barra, lo anuncia con `QAccessibleAnnouncementEvent` (Qt 6.8+,
  guardado con `QT_VERSION_CHECK`; degrada a solo-visual en 6.5–6.7) sin mover el
  foco y solo si hay un lector activo. Las pistas de *hover* y el banner de arranque
  se dejan en `showMessage` directo, sin anunciar (serían ruido). Los
  `QMessageBox`/`QInputDialog` ya eran accesibles de serie.
- ✅ **Anunciar la fórmula bajo el cursor** — *Hecho:* una fórmula 2D vive como un
  único `ObjectReplacementCharacter` que un lector salta en silencio, y las inline
  son glifos Unicode sin estructura; el TeX es lo legible. `EditorStack`, en
  `cursorPositionChanged` del editor WYSIWYG, detecta si el cursor está sobre un
  fragmento de fórmula (`IsMathProperty`/`MathTexProperty`) y anuncia su TeX
  («Fórmula: \frac{a}{b}») vía el mismo `showStatusMessage` → anuncio. Se deduplica
  por el inicio del grupo (`formulaAtCursor`): suena una vez al entrar, no al
  moverse dentro. Cubierto por `tst_formulacontroller`.
- ✅ **Etiquetas asociadas en los diálogos** — *Hecho/verificado:* los diálogos con
  `QFormLayout` (fórmula, insertar enlace, tabla, estadísticas) ya asocian
  etiqueta↔campo **automáticamente** (`addRow(QString, campo)` crea la `QLabel` y le
  fija el campo como *buddy*), así que el lector lee la etiqueta al enfocar el campo;
  no hacía falta tocarlos. El único hueco era `GoToHeadingDialog`, que usaba solo
  `placeholderText` (que los lectores no leen): ahora su campo de filtro toma ese
  texto como nombre accesible y la lista de encabezados tiene nombre propio. Los
  mnemónicos `&` (Alt+letra) se **descartan**: la posición del `&` varía por idioma
  (coste × 9 + riesgo de conflicto) y los campos ya se alcanzan con Tab.

### Información no transmitida solo por color (daltonismo)

- ✅ **Admoniciones** — *Revisado:* el render colorea la línea del marcador pero
  **no la oculta**, así que el tipo lo transmite el propio **texto** visible
  (`[!NOTE]`, `[!WARNING]`…), no solo el color (`admonitions.cpp`): la distinción no
  depende del color y un usuario con deuteranopia/protanopia lee igual el tipo. El
  **glifo** decorativo por tipo (estilo GitHub) se **descarta**: exigiría insertar
  en el documento texto que no es Markdown, lo que rompería la regex que detecta el
  marcador (`^\s*\[!…\]\s*$`), la detección de límites del callout, el flujo de
  inserción y el round-trip — coste/riesgo alto para una ganancia marginal sobre el
  keyword que ya se ve. (Las erratas del corrector ya llevan subrayado ondulado
  además del rojo; las fórmulas se reconocen por su forma renderizada.)
- ✅ **Verificador de contraste de los temas (función pura + `tst_`)** — *Ya existía:*
  `mdtheme::contrastRatio` (WCAG 2.x, linealización sRGB; `themespec.cpp`) y
  `tst_themespec` verifican el contraste de cada `ThemeSpec`, con un listón **más
  alto** que el AA propuesto: texto de cuerpo a **AAA (≥7:1)** sobre página y cromo,
  y enlaces, texto secundario, botones, selección, tooltips y los 5 colores de
  sintaxis a **AA (≥4.5:1)**, en los 6 temas. Blinda la paleta contra regresiones al
  editar el catálogo.
- ✅ **Tinta de los iconos de la barra de formato** — *Arreglado:* los iconos
  monocromos generados (negrita/cursiva/subrayado/tachado y los de listas) se
  «hornean» en un `QPixmap` con un color y a una `devicePixelRatio` concretos, así
  que hay que **regenerarlos** cuando cambia cualquiera de los dos —si no, se quedan
  con la tinta del tema anterior (sin contraste) o, en HiDPI, borrosos hasta el
  primer relayout—. Dos fallos cazados aquí: (a) en el arranque se generaban durante
  la construcción, antes de que la ventana tuviera pantalla, a `dpr = 1`; ahora
  `MainWindow::showEvent` los rehornea con la dpr definitiva. (b) solo se
  regeneraban vía la señal `themeChanged` de un único stack (no seguía a otras
  pestañas); ahora `MainWindow::changeEvent` los recolorea ante
  `ApplicationPaletteChange`/`PaletteChange` (y `DevicePixelRatioChange` en Qt 6.6+),
  leyendo `qApp` ya actualizada, de modo que su tinta **contrasta con el fondo de la
  barra** igual que el texto en cualquier tema, esquema del SO o densidad de
  pantalla. Test `toolbarIconInkContrastsWithTheme` en `tst_chromezoom`.

### Navegación por teclado y foco

- ✅ **Orden de tabulación** — *Verificado:* en todos los diálogos propios (fórmula,
  insertar enlace/imagen, tabla, ir a encabezado), en la barra de búsqueda y en la
  vista dividida el orden de **construcción** de los widgets ya coincide con el orden
  lógico/visual, así que el Tab fluye bien sin `setTabOrder`; ambos editores son
  `StrongFocus` y alcanzables con teclado. Dos arreglos puntuales encontrados al
  revisar: la **vista previa** de la fórmula (solo lectura) ya no roba el Tab
  (`Qt::ClickFocus`), y el campo de ruta del diálogo de imagen recupera su nombre
  accesible (al ir envuelto en un contenedor perdía la asociación con su etiqueta).
- 🚫 **Indicador de foco visible** — *Descartado (coste/beneficio):* el foco lo
  dibuja el `QStyle` usando el rol `Highlight` de cada tema, que el verificador de
  contraste ya garantiza legible (en alto contraste, amarillo puro sobre negro — muy
  visible en botones, menús, listas y el árbol del esquema). En los editores el foco
  se ve por el cursor (como en cualquier editor de texto). Un *focus ring* explícito
  más fuerte chocaría con la arquitectura —el theming es **100 % por paleta, sin un
  solo `setStyleSheet` en el proyecto**— y obligaría a pintarlo a mano sobre
  `QAbstractScrollArea` o a introducir stylesheets, además de validación visual por
  plataforma/estilo: demasiado coste para la ganancia marginal sobre el foco que el
  estilo ya pinta.
- ✅ **Auditar trampas de teclado** — *Revisado:* el `eventFilter` es un mero
  despachador que solo consume el evento si un sub-manejador lo hace, y ninguno
  bloquea navegación: `handleMathKeyPress` solo se queda teclas **imprimibles**
  (Tab=`\t` y Esc no son `isPrint()`, las flechas traen `text()` vacío) y
  Backspace/Delete en los bordes; la continuación de listas solo actúa con Enter sin
  modificadores; los shortcodes solo con `:`. El editor (Tab inserta tabulador, lo
  normal) se abandona con F10/Alt al menú o con los atajos, así que cumple WCAG
  2.1.2 (sin trampa). *Añadido:* `tst_formulacontroller` verifica que con el cursor
  dentro de una fórmula las flechas, Inicio/Fin, Tab y Esc **no** se consumen.

### Sistema y preferencias

- ✅ **Tema de alto contraste** — ya existe (`ThemeId::HighContrast`: blanco/negro
  21:1, bordes blancos explícitos; `themespec.cpp`).
- ✅ **Zoom de interfaz persistente** — `ChromeZoom` escala editor, menús, barras,
  estado, esquema e iconos, y se guarda en `AppSettings::zoomLevel`.
- 🚫 **Seguir el modo de alto contraste del SO** — *No viable de forma portable
  (no-objetivo):* Qt (`QStyleHints`) expone el esquema claro/oscuro (`colorScheme`,
  que el proyecto ya sigue) pero **no** un hint de alto contraste. Detectarlo
  obligaría a código por plataforma (`SystemParametersInfo`/registro en Windows,
  `NSWorkspace` en macOS, gsettings de GNOME en Linux), justo lo que el proyecto
  evita por diseño («Qt6 puro, sin `#ifdef Q_OS_*` ni APIs nativas»). Mitigación ya
  presente: el tema de **alto contraste** se selecciona a mano y es el de mayor
  contraste verificado.
- ✅ **Respetar el escalado de fuente del SO** — *Verificado:* no hay fuente global
  fija (`main.cpp` no llama a `setFont`) ni se desactiva el HiDPI (Qt6 lo escala
  solo). El editor WYSIWYG no fija fuente, así que arranca con la de la aplicación
  (que Qt deriva del sistema); el zoom se calcula como **delta** sobre
  `m_baseFontPointSize`, capturado de esa fuente, y lo mismo para fuente de código,
  menús, barras, estado y esquema. O sea, el tamaño base **ya** es el del SO (baja
  visión respetada). *Limitación menor:* los tamaños base se capturan al arrancar,
  así que cambiar la fuente del sistema en caliente requiere reiniciar la app.

### Documentación y verificación

- ✅ **Sección de accesibilidad en la ayuda (F1)** y en el README — *Hecho:* nueva
  sección «Accesibilidad» en el manual integrado (lectores de pantalla, manejo solo
  con teclado, alto contraste y zoom, foco), con su entrada en el índice, en los **9
  idiomas** (`help-app*.md`); y un punto de accesibilidad en la lista de
  características del README.
- 🚫 **Prueba con lector de pantalla real** — *Descartado (sin medios):* sería una
  pasada manual con Orca (Linux) / NVDA (Windows) / VoiceOver (macOS) comprobando que
  se anuncian los nombres de los controles (editor, fuente, esquema, búsqueda,
  contador), las descripciones y los mensajes de estado, y que el orden de foco es
  lógico. No puede automatizarse ni hacerse sin un lector real, y no se dispone de
  uno; todo lo demás está cableado contra la API de accesibilidad de Qt (que en Linux
  habla AT-SPI con Orca), que es lo máximo verificable sin AT en vivo.

## 📋 Proyecto / comunidad

16. ✅ **CHANGELOG.md** — *Hecho:* `CHANGELOG.md` con el historial por versión.
17. ✅ **Relicenciar a una licencia de software** — *Hecho:* el proyecto pasó de
    CC BY-ND 4.0 a **GPL-3.0** (copyleft fuerte: permite forks pero los obliga a
    seguir siendo libres). Desbloquea, entre otras cosas, la firma de código
    gratuita para OSS (p. ej. SignPath Foundation). La política del repositorio
    sigue siendo no aceptar PRs (independiente de la licencia).
18. **Analítica de descargas con privacidad** — GitHub no dice *quién* descarga;
    un redirector propio daría país/volumen sin rastrear personas.

---

> **Prioridad sugerida:** en distribución, el mayor desbloqueo pendiente es la #3
> (firma/notarización de binarios), que elimina la fricción de
> Gatekeeper/SmartScreen en la instalación, seguida de la #2 (packaging nativo).
> En funcionalidad, la **«Nueva auditoría (2026-06-30)» está completada**: sus 18
> mejoras salieron en 2.4.0 (primera tanda) y 2.5.0 (el resto). Lo siguiente, ya fuera
> de ese plan, serían las **reglas de entrada** y la **pasada de tipografía** de
> «Pulido de UX», la ventana de Preferencias, y las notas nuevas (fuente global,
> importar formatos).
