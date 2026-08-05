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
2. **Packaging para gestores nativos** — Flatpak/AppStream (Linux), Scoop
   (Windows), Homebrew cask (macOS). Multiplica la visibilidad frente al
   `.AppImage`/`.zip` suelto y mejora la confianza. *Hecho a medias:* los tres
   manifiestos están escritos en `packaging/` y **el job `packaging` de
   `release.yml` los pone al día al etiquetar** (versión, URL y SHA-256 de los
   artefactos, `tag`/`commit` del Flatpak y la entrada `<release>` del metainfo),
   vía `scripts/update-packaging.py`. Esa automatización nació de que se
   quedaron **seis versiones clavados en la 1.2.0** mientras el README de Scoop
   publicaba el comando de instalación que los usa: quien lo siguiera se
   instalaba la 1.2.0. *Flatpak validado (25-07-2026):* compila y exporta con
   `org.flatpak.Builder` sobre `org.kde.Platform//6.11` y pasa
   `flatpak-builder-lint`; hubo que corregir el app-id (el viejo derivaba en una
   URL de GitHub que daba 404) y el runtime EOL, y salió de ahí un fallo ajeno al
   empaquetado: el proyecto no configuraba con Qt 6.11 (ver el punto de
   `Qt6::GuiPrivate` en CMakeLists). *Pendiente:* enviarlos — el de Scoop a un
   *bucket* propio, el cask a un *tap*, y el Flatpak a Flathub, esto último
   **después de publicar una versión que lleve el arreglo de CMake** (el
   manifiesto compila desde el tag, y la 2.8.0 no lo tiene) y decidiendo antes
   qué hacer con `--filesystem=home`, el único error que el linter deja abierto
   (ver `packaging/flatpak/README.md`).
3. **Firma de binarios** — macOS y Windows no están firmados (Ctrl-clic → Abrir).
   Firmar (y notarizar en Mac) elimina la fricción de instalación, que es donde
   se pierden usuarios. *Windows, listo y a la espera:* `release.yml` ya tiene el
   paso de SignPath escrito y **se activa solo** en cuanto exista el secreto
   `SIGNPATH_API_TOKEN`; falta la aprobación del proyecto en signpath.org
   (gratuita para OSS), no trabajo de código. macOS sigue sin firmar.
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
- ✅ **Ventana de Preferencias única** — *Hecho:* diálogo `PreferencesDialog` con pestañas
  (Apariencia, Editor, Impresión) abierto desde *Editar → Preferencias…* (Ctrl+,,
  `PreferencesRole` para macOS). No persiste nada por su cuenta: **refleja y dispara** las
  acciones de ajuste ya existentes del menú Ver (tema, luz cálida, seguir el sistema,
  interlineado, resaltar línea, modo foco, números de página), que MainWindow le pasa como
  punteros; así reusa su lógica de aplicar+persistir sin duplicarla. Las casillas toman su
  rótulo del `text()` traducido de la acción. Riesgo bajo, confianza alta.

#### Affordances en el bloque

- ✅ **Barra flotante de tabla + navegación por celdas** — *Hecho:* widget `TableToolbar`
  (hijo del viewport, iconos monocromos nuevos en `formaticons::makeTableIcon`) que
  `EditorStack::updateTableToolbar` muestra y posiciona sobre la tabla del cursor
  (`cursorPositionChanged`); sus botones (insertar/eliminar fila y columna, alinear
  izq./centro/dcha.) enrutan a `TableController`. Y **Tab/Mayús+Tab** entre celdas con
  selección del contenido (`EditorStack::navigateTableCell`, enganchado en
  `handleEditorKeyPress`); Tab en la última celda añade fila. Presentación pura, sin
  stylesheet. *Nota:* «mover columna» y «borrar tabla» no están (no los expone la API).
- ✅ **Bloque de código: etiqueta de lenguaje + botón copiar** — *Hecho:* overlay
  `CodeBlockOverlay` (widget hijo del viewport) que aparece al pasar el ratón por un fence
  (detección en el `eventFilter` de la ventana → `EditorStack::updateCodeBlockOverlay`,
  posicionado con `cursorRect`): etiqueta de lenguaje (clic → reusa el diálogo
  `FormatController::setCodeLanguage`) + botón copiar (copia el texto del grupo). Módulo
  puro `mdcodeblock` (`groupAt`/`groupText` sobre el grupo de bloques `BlockCodeFence`
  contiguos) con `tst_codeblock`. Presentación pura, no toca el documento.

#### Toques menores de acabado

- ✅ **Documento de bienvenida en el primer arranque** (reusa `doctemplates`) y
  **placeholder** sutil en el documento vacío. *Hecho:* `mdtemplate::welcomeDocument()`
  (traducido) cargado en `startSession` cuando no hay archivo/borrador/sesión y
  `AppSettings::welcomeShown()` es false (se marca tras mostrarlo); se carga con
  `loadFromString(..., asModified=false)` para no ensuciar el documento. Placeholder vía
  `QTextEdit::setPlaceholderText` en el editor visual. `tst_documentio` cubre el
  asModified.
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

- ✅ **Fase A — Agrupar (casi gratis).** *Hecho:* `mdtemplate::Category` (enum de 7) +
  `categoryName()` por `tr()` + `categoriesInOrder()` + campo `category` en `DocTemplate`;
  recategorizadas las 10 plantillas actuales (Personal, Programación, Docencia, Empresa,
  Escritura; Académico y Derecho quedan vacías hasta B/C y no se muestran); el menú monta
  un submenú por categoría no vacía (perezoso). Solo se tradujeron los 7 nombres de
  categoría. `tst_doctemplates` ampliado. Riesgo bajo.
- ✅ **Fase B — Plantillas de usuario (núcleo escalable).** *Hecho:* *Archivo → Guardar como
  plantilla…* (guarda el cuerpo actual + front matter; pide nombre y categoría) y *Gestionar
  plantillas…* (al final del submenú). Modelo puro `mdusertemplate` (name/body/category +
  (de)serialización a QSettings, categoría fuera de rango → Personal) con `tst_usertemplate`;
  persistidas en `AppSettings::userTemplates`; diálogo `UserTemplatesDialog` (lista + editor
  nombre/categoría/cuerpo, como `SnippetsDialog` + combo de categoría). El menú `Nuevo desde
  plantilla` se reconstruye (`rebuildTemplateMenu`) mezclando fábrica + usuario por categoría.
  Esquiva i18n y jurisdicción (las de usuario no se traducen). Es lo que hace útiles a
  Derecho/Sanidad sin traducir nada.
- ✅ **Fase C — Plantillas de fábrica curadas.** *Hecho:* 5 nuevas, cada una nombre + cuerpo
  por `tr()` traducidos a los 9 idiomas — Programación: **CHANGELOG**, **ADR** (decisión de
  arquitectura), **Informe de error**; Académico: **Artículo científico (IMRyD)**, **Informe
  de laboratorio**. Derecho/Sanidad siguen sin plantillas de fábrica (vía plantillas de
  usuario, Fase B). El catálogo pasa de 10 a 15 (`tst_doctemplates`).

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
- ✅ **Pandoc opcional** — *Hecho:* *Archivo → Importar → Otros formatos (Pandoc)…*
  (`MainWindow::importWithPandoc`) ejecuta `pandoc --to=gfm --wrap=none <archivo>` por
  `QProcess` síncrono; cubre DOCX/ODT/RTF/LaTeX/reST/… (Pandoc infiere el formato por la
  extensión). Módulo puro `mdimport` (`markdown/pandocimport.{h,cpp}`: `pandocAvailable`/
  `pandocArguments`/`pandocFilePattern`/`pandocInstallCommand`) con `tst_pandocimport`.
  Degradación elegante como los diagramas: si falta Pandoc, un aviso con la orden de
  instalación por plataforma. Sin dependencia enlazada. *Limitación:* las imágenes
  embebidas no se extraen (sin `--extract-media`).
- ⬜ **Importador nativo DOCX/ODT** (sin Pandoc) — ya cubierto por la vía Pandoc de
  arriba; quedaría solo como respaldo para quien no quiera instalar herramientas
  externas (más trabajo y lossy). Prioridad baja.

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
- ✅ **El fuzzer no corría bajo sanitizers, al contrario de lo que decía el
  código.** El comentario del punto anterior —y el del propio `CMakeLists`—
  afirmaba que `tst_roundtripfuzz` era «la red de verdad bajo ASan/UBSan, lo corre
  CI». No era cierto: **no está registrado en CTest** (a propósito) y el *job* de
  sanitizers ejecuta `ctest`, así que nunca lo tocaba. *Hecho (28-07-2026):* hay un
  *job* `fuzz` propio que lo invoca **directamente** —única vía, al no estar en
  ctest— bajo ASan/UBSan, con `continue-on-error` para que no pueda bloquear una
  release y `timeout-minutes` para que un cuelgue no queme el presupuesto de
  Actions. `continue-on-error` y no tragarse el código de salida con `exit 0`: así
  el fallo sigue **visible** en Actions, que un fuzzer que falla en silencio no
  vale para nada. Los comentarios mentirosos, corregidos.
- ✅ **Cuelgue del fuzzer en Windows: era un fallo de Qt, ya arreglado aguas
  arriba.** Apareció al añadir Windows a la matriz de CI (25-07-2026):
  `tst_roundtripfuzz.exe` moría allí con `0xC0000005` a los ~250 ms, en el caso
  281. *Resuelto (28-07-2026)* tras tres rondas de bisección en CI:
  - **No era del editor.** `tools/qtmdrepro`, que enlaza **solo `Qt6::Widgets`**,
    revienta igual: `QTextDocument::setMarkdown` mata el proceso en Windows con un
    documento que empieza y acaba por regla temática `---` sin salto de línea
    final. **El caso mínimo son seis caracteres: `---\n\n---`.**
  - **Dónde:** escritura fuera de rango en `QUtf8::convertFromUnicode`
    (`mov byte ptr [rdx],cl`), vía `QtPrivate::convertToUtf8` ←
    `QTextMarkdownImporter::import` ← `QTextDocument::setMarkdown`.
  - **Particularidades:** solo con el marcador `---` (con `***` o `- - -` no pasa,
    ni mezclándolos), solo con exactamente dos (con tres no pasa, ni con
    `---\n---` pegadas), el contenido intermedio es indiferente y las
    `MarkdownFeatures` también. Los acentos **no** pintan nada pese a que la traza
    pase por `convertToUtf8`: esa fue la primera hipótesis y se probó y descartó.
  - **Ya está arreglado en Qt.** El barrido de versiones (*job*
    `qt-version-sweep`, solo `workflow_dispatch`) lo confirma: cae en **6.8.2** y
    pasa en **6.9.3** y **6.10.3**. No hace falta reportarlo aguas arriba. (6.11
    quedó sin probar: `aqtinstall` no encuentra sus datos, cosa del instalador.)
  - **El rodeo se queda** mientras se soporte Qt 6.8 o anterior: `mdrender::protect()`
    garantiza un `\n` final, que es inocuo por construcción —no cambia la
    semántica del Markdown— y cubre el caso real, porque esto no era entrada
    sintética: **cualquier `.md` que cerrase con `---` sin salto final tumbaba el
    editor al abrirlo en Windows**. Para retirarlo hay que ver a `qtmdrepro`
    dejar de caer en el CI de Windows, que para eso está.
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

### Iconos de la barra flotante de tablas (2026-07-28)

- ⬜ **Rehacer los siete iconos de `makeTableIcon`** (insertar/eliminar fila y
  columna, alinear columna), que quedaron sin tocar cuando en la 2.8.2 se
  rediseñaron los de lista y los de formato de carácter. Dos motivos, por orden
  de importancia:
  - **La insignia `+`/`−` es diminuta y está pegada a la retícula.** «Insertar
    fila» y «eliminar fila» son el mismo dibujo salvo un trazo vertical de esa
    insignia (radio `0,13 × N`), así que a tamaño real se distinguen por unos dos
    píxeles; y como cae pegada al borde de las líneas, se lee como una línea más
    de la tabla en vez de como una insignia —en «eliminar columna» la raya de
    arriba parece el techo de la tabla, no un menos—. Es peor aquí que en la
    barra de formato porque esta barra pinta a `fontMetrics().height() × 0,85`
    ≈ 16 px (`EditorStack`, la llamada a `applyIcons`), aún más pequeño que los
    18-22 px de la otra. Arreglo: separar la insignia de la retícula y darle
    tamaño, o llevarla a una esquina como hacen los juegos de iconos que
    distinguen «añadir» de «quitar».
  - **Desajuste de familia.** Los de lista pasaron a dos renglones con trazo
    grueso; estos siguen con la retícula de tres renglones finos. Nunca se ven a
    la vez —una barra es de ventana y la otra flotante sobre la tabla—, así que
    no chirría, pero ya no son el mismo juego.

  Los **tres de alineación se salvan**: son el icono clásico de tres líneas con la
  del medio más corta y se leen bien a cualquier tamaño; el trabajo es en los
  cuatro de fila/columna. El banco de pruebas de `Auxiliar/icons` (variantes
  parametrizadas + láminas de contacto a 18/22/28/36 px en claro y oscuro) sirve
  para ensayarlo sin tocar `src/`: hoy solo cubre lista y formato, habría que
  añadirle los de tabla. Prioridad baja: hoy son **ambiguos, no ilegibles**.

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

## 🌍 Internacionalización

### Revisión del juego de idiomas (2026-07-25)

Revisión de los 9 idiomas de la interfaz (es de origen + en, de, fr, it, pt, pl,
nl, ro). Conclusión: la selección está bien dimensionada y no conviene ampliarla;
lo que hay que arreglar es el remate del portugués.

- ✅ **Uniformar el portugués a una sola norma.** *Hecho:* `md-editor_pt.ts` mezclaba
  vocabulario europeo y brasileño para los mismos conceptos, y se ve en la misma
  sesión de uso: el menú traduce «Guardar» como `&Salvar` / `Salvar como...`
  (brasileño) mientras el resto de la interfaz dice `ficheiro` (europeo); y los dos
  filtros de diálogo, que el usuario ve casi seguidos, son `Ficheiros Markdown
  (…);;Todos os ficheiros (*)` al abrir y `Arquivos Markdown (…);;Todos os arquivos
  (*)` al guardar. No es un matiz de estilo: era la misma palabra traducida de dos
  maneras, y chirriaba para hablantes de cualquiera de las dos variantes. Se
  uniformó a la norma **brasileña** (por número de hablantes, ~200 M frente a ~10 M,
  y porque `QTranslator` carga igualmente el `pt` genérico en un sistema `pt_BR`):
  `ficheiro`→`arquivo`, `guardar`→`salvar`, `utilizador`→`usuário`, `ecrã`→`tela`,
  `rato`→`mouse`, `separador`→`aba` (con la concordancia de género rehecha),
  `ligação`→`link`, `registo`→`registro`, `gerir`→`gerenciar`, `definições`→
  `configurações`, `sistema operativo`→`sistema operacional`, `deteta`→`detecta`, y
  el único tuteo (`Queres recuperá-los?`) al tratamiento de usted del resto. Alcanzó
  también a `src/help/help-app_pt.md`, que además citaba nombres de menú que ya no
  coincidían con la interfaz (`Ligação` por `Link`, `Anular` por `Desfazer`) y dos
  anclas del índice. Sin tocar código.
- ✅ **No ampliar el juego de idiomas** (decisión, no tarea; **reabierta el 2026-08-05
  para el chino simplificado**, ver la sección siguiente). Los 9 actuales son
  justo aquellos a los que la aplicación **entera** puede dar servicio, no solo la
  interfaz: el corrector necesita un diccionario Hunspell y la exportación a LaTeX
  monta el preámbulo con `babel` sobre `inputenc`+`T1` (`exportlatex.cpp`), que cubre
  el latín-1 y poco más. De ahí que la ausencia más llamativa —el CJK— esté
  justificada: un usuario chino, japonés o coreano tendría la interfaz traducida y
  dos funciones importantes rotas (sin diccionario, y un `.tex` que pdfLaTeX no
  compila sin `ctex`/`xeCJK`). Contra la ampliación pesa además el coste recurrente:
  `tst_translations` falla ante cualquier cadena sin traducir, así que **cada `tr()`
  nuevo son 9 ficheros que tocar** antes de que la suite pase; el décimo idioma tiene
  un rendimiento claramente decreciente para un proyecto de una persona.
- ⬜ **Si algún día se amplía, la dirección es el este de Europa, no Asia.** Ruso,
  ucraniano, checo y turco tienen más usuarios potenciales de un editor Markdown que
  el neerlandés o el rumano, que sí están, y caen **dentro** de la frontera técnica
  de arriba: hay diccionario Hunspell y `babel` los soporta. Único trabajo de código:
  el ruso y el ucraniano piden `T2A` en el `fontenc` del preámbulo LaTeX (tres
  líneas) para que el cirílico de un documento no tumbe la compilación.

### Añadir el chino simplificado — `zh_CN` / 简体中文 (2026-08-05)

Reabre el punto anterior. La frontera que se puso entonces («solo idiomas a los que
la aplicación **entera** puede dar servicio») era **técnica, no lingüística**, así
que el plan la ataca de frente: **primero los cuatro arreglos de código**, que
valen por sí solos —hoy el editor ya maltrata un documento en chino aunque la
interfaz esté en español— y **después** la traducción, que es donde está el 90 % del
tiempo. Las tres fases son independientes: se puede parar tras la A y haber ganado
algo.

Se elige **simplificado (`zh_CN`)** y no tradicional: es el que cubre más hablantes
y el que Qt resuelve por defecto en `zh_Hans`. El tradicional (`zh_TW`) sería una
segunda tanda con los mismos ficheros, no un `left(2)` del mismo.

#### Fase A — arreglos de código (previos, independientes del idioma de la interfaz)

**Completa (2026-08-05), los cuatro puntos.** El editor ya no maltrata un documento
en chino: exporta su texto a LaTeX (A1), no lo subraya entero ni manda a instalar un
paquete inexistente (A2), lo cuenta bien (A3) y distingue simplificado de tradicional
donde eso decide qué recurso se abre (A4). Nada de esto necesita a B ni a C.

- ✅ **A1. La exportación a LaTeX borra el texto chino, sin avisar.** `latexEscape`
  (`src/export/exportlatex.cpp:90`) descarta todo punto de código ≥
  `kHighSymbolStart` (0x2190) que no esté en `highSymbolMap()`, y los ideogramas Han
  empiezan en U+4E00; `codeBlockSanitize` (línea 106) hace lo mismo dentro de
  `alltt`. Un documento en chino exporta **a un `.tex` sin texto**. El descarte se
  puso para que pdfLaTeX+T1 no abortara ante un emoji, y es correcto para eso; lo
  que falta es distinguir «símbolo suelto que no se compone» de «la escritura del
  documento».

  **Decidido (2026-08-05): detectar, pero no bloquear.** «No se puede exportar»
  sería falso: el chino en LaTeX funciona bien, solo que **con otro compilador**
  (`xelatex`/`lualatex` con `ctex`/`xeCJK`; `pdflatex` no puede, y `babel` no tiene
  opción `chinese`). Bloquear castigaría además el caso pequeño: quien cita tres
  ideogramas en un documento en español se quedaría sin exportar nada. Y lo caro de
  las tres salidas posibles —callar, bloquear, o emitir el preámbulo correcto— es
  **detectar**, que hace falta en las tres; emitir el preámbulo son unas líneas más.
  Plan:

  1. **La detección va dentro del escapado, no en una pasada aparte**: `latexEscape`
     y `codeBlockSanitize` ya recorren el texto carácter a carácter, así que basta
     con **apuntar lo que descartan** en vez de tirarlo en silencio.
  2. **Distinguir escritura de símbolo suelto.** Si el punto de código pertenece a
     una escritura de verdad (ideogramas Han, kana japonés, hangul coreano y su
     puntuación) → **se emite** y se marca el documento como «pide un motor
     Unicode». Si es un símbolo o un emoji sin equivalente → se descarta como hasta
     ahora, pero **contándolo**.
  3. **Preámbulo condicional**: con ideogramas, la rama no-pdfTeX de `iftex`
     (`exportlatex.cpp:345`) carga `ctex`; la de pdfTeX, un `\errmessage` que dice
     en una línea qué pasa y con qué compilar. Un fallo claro es mejor que el
     «Unicode character not set up for use with LaTeX» que saldría si no.
     Y encabeza el `.tex` un comentario con la orden correcta.
  4. **Aviso al exportar, sin bloquear**: *el documento lleva texto chino, japonés o
     coreano; el archivo ya lleva la configuración necesaria, pero hay que
     compilarlo con `xelatex`*. Y, en su caso, *se han omitido N caracteres sin
     equivalente* — que **hoy pasa constantemente y nadie se entera** (emoji,
     dingbats): es el mismo aviso, y es la parte que aprovecha a todo el mundo, no
     solo a quien escriba chino.
  5. **Vía de retorno**: `toLatex` es una función pura que devuelve el texto, así
     que el aviso sale por un parámetro de salida (`LatexIssues *`, como el
     `QString *error` que ya usa el descriptor de exportación); quien traduce y
     muestra el mensaje es `ExportController`, no el módulo puro. Casos en
     `tst_exporters`.

  *Hecho (2026-08-05), los cinco puntos:* `isScriptChar` separa escritura de
  símbolo en `latexEscape` y `codeBlockSanitize`; `mdexport::LatexIssues`
  (`needsUnicodeEngine`, `droppedSymbols`) sale por parámetro de `toLatex`; el
  preámbulo se decide **después** del cuerpo (es lo que permite que dependa de lo
  aparecido en él) y el título se escapa el primero, porque puede traer ideogramas
  aunque el cuerpo no; `ExportController::reportLatexIssues` avisa sin bloquear.
  Tests `latexKeepsCjkAndAsksForUnicodeEngine` y `latexReportsDroppedSymbols`.
  *Verificado compilando de verdad* (`texlive-lang-chinese` + `texlive-xetex`), que
  es como se descubren los fallos de este exportador: un documento con ideogramas
  compila con **`xelatex` sin un solo error**, y el PDF lleva el texto —encabezado,
  prosa mezclada con español, celdas de tabla y el comentario de dentro del bloque
  de código—, que es justo lo que antes desaparecía; el mismo documento se detiene
  en `pdflatex` con nuestro mensaje; y un documento latino sigue compilando con
  `pdflatex` como antes. `lualatex` no se pudo probar, pero por una avería ajena:
  falla igual con un documento latino sin una línea de chino (le falta el script de
  `luaotfload` que genera la caché de fuentes).
- ✅ **A2. El corrector no tiene salida limpia para un idioma sin diccionario.** No
  existe diccionario Hunspell de chino en el repositorio de LibreOffice, así que ni
  `scripts/fetch-dictionaries.sh` ni `DictionaryInstaller` pueden darlo. Hoy
  `SpellController::applyLanguage` (`src/spell/spellcontroller.cpp:68`) degrada a «no
  disponible» y saca el aviso de *falta el diccionario, instálalo así*, con el botón
  «Descargar e instalar» —**un callejón sin salida**, exactamente el caso que el
  código ya se cuida de evitar cuando no hay motor
  (`SpellChecker::isEngineAvailable`). Hace falta una **tercera rama**: idioma sin
  diccionario que exista en ninguna parte → ni aviso ni botón, solo el mensaje de
  barra de estado. Aparte, `mdspell::tokenize` (`spellscan.cpp:22`) usa
  `isLetterOrNumber()`, así que una frase china sin espacios sale como **una sola
  palabra** de 40 caracteres: aunque apareciera un diccionario, subrayaría el
  párrafo entero. Lo barato es **no tokenizar** los rangos Han (saltarlos como ya se
  salta el código en línea).

  *Hecho (2026-08-05):* `mdspell::hasHunspellDictionary` separa «no está instalado»
  de «no existe» —lista corta y cerrada: chino, japonés y coreano, las escrituras
  que no separan palabras con espacios, que es lo que Hunspell necesita— y con ellas
  `applyLanguage` se queda en el mensaje de barra de estado, sin diálogo, sin orden
  de instalación de un paquete inexistente y sin botón de descarga. `tokenize` trata
  esas escrituras como separador, así que ya no subraya el párrafo entero y las
  palabras latinas de en medio (`混合español`) se siguen corrigiendo. El menú *Ver →
  Idioma de corrección* no hizo falta tocarlo: lo llena
  `SpellChecker::availableLanguages()` con los diccionarios que hay en disco, y ahí
  el chino no va a aparecer nunca. Tests `tokenizeSkipsUnsegmentedScripts` y
  `languagesWithoutAnyDictionary`.
- ✅ **A3. El contador de palabras cuenta párrafos.** `mdstats::analyze`
  (`src/markdown/docstats.cpp:24`) parte por `\s+`; en chino no hay espacios entre
  palabras, así que un párrafo entero cuenta como **1 palabra** y el tiempo de
  lectura queda absurdo. Convención habitual: cada ideograma cuenta como una palabra
  y la velocidad de lectura sube (~300–500 caracteres/minuto frente a las 200
  palabras/minuto de ahora). Módulo puro, con `tst_docstats`: es el arreglo más
  barato de los cuatro.

  *Hecho (2026-08-05):* cada ideograma, kana o sílaba hangul cuenta como una palabra
  y **corta** la palabra latina en la que aparezca (`混合español` son tres, no un
  token de cinco caracteres que nadie sabría explicar); su puntuación no cuenta,
  igual que no cuenta la latina. El tiempo de lectura **suma los dos ritmos** en vez
  de dividir el total por uno solo: los ideogramas van a `wordsPerMinute * 2`,
  derivado del parámetro y no una segunda constante, para que quien suba la
  velocidad las suba a la vez. `DocStats::cjkChars` deja ver de qué se compone el
  total. Para un texto latino el recuento sale idéntico al de antes (los tramos
  entre blancos son los mismos), y eso lo fija un caso del test. Tests
  `countsCjkCharactersAsWords` y `cjkReadingTimeUsesItsOwnPace`.
- ✅ **A4. Simplificado y tradicional no se distinguen.** `helpSuffix()`
  (`src/widgets/helpdialog.cpp:36`) elige el manual con un `switch` sobre
  `locale.language()`, y `QLocale::Chinese` vale lo mismo para `zh_CN` que para
  `zh_TW`: hay que mirar `locale.script()` (`QLocale::SimplifiedHanScript`). Lo
  mismo en `mdexport::languageForCode` (`src/export/exporters.cpp:48`), que
  normaliza con `code.left(2)` y colapsa ambos en `zh`, y en
  `mdspell::pickDictionary`, que hace `section('_', 0, 0)`. Si no se arregla ahora,
  añadir el tradicional más adelante obliga a rehacerlo.

  *Hecho (2026-08-05):* la normalización deja de estar copiada en cada sitio y pasa
  a un módulo puro, `mdlang::canonicalTag` (`src/app/langtag.{h,cpp}`, con
  `tst_langtag`): `es`/`es_ES`/`es-ES` → `es`, y el chino a `zh_CN` o `zh_TW` según
  el **sistema de escritura**, no el territorio, porque el territorio se queda
  corto —`zh_SG` es simplificado y `zh_HK` tradicional, y ninguno se llama `CN` ni
  `TW`—; `QLocale` deduce el script aunque solo se le dé el territorio. Solo el
  chino se consulta a `QLocale`: para los demás vale el prefijo de la cadena,
  porque un código que Qt no reconoce devuelve el locale «C» y de ahí saldría un
  idioma que nadie pidió (caso `emptyAndUnknownStayHarmless`).
  El `switch` de `helpSuffix()` desaparece: `mdhelp::helpSuffixForLanguage` compone
  el sufijo con la etiqueta canónica y **comprueba que el recurso existe**, así que
  un idioma sin manual traducido cae al inglés en vez de abrir un visor vacío y
  estrenar un manual nuevo es soltar sus dos `.md` en el `.qrc` —sin tocar código—.
  Eso convierte B3 en menos trabajo del que decía y hace que este arreglo se note
  antes de que exista el chino: hoy ya no hay lista de idiomas que ampliar a mano.
  Lo cubre `suffixPicksTheManualOfTheLanguage` en `tst_help`, con las filas de
  `zh_CN`/`zh_TW` esperando `_en` y una nota de que ese día pasarán a fallar, que es
  el aviso que se quiere.
  `mdspell::pickDictionary` **no se toca, a propósito**: ahí `zh` no es una etiqueta
  de recurso sino el nombre del archivo de diccionario que se busca en disco, y
  colapsar la región es justo lo que tiene que hacer (`es_MX` → `es`). Para el chino
  da igual: A2 ya establece que no existe diccionario Hunspell de ninguna de sus dos
  escrituras, y `hasHunspellDictionary` responde que no para `zh_CN` y `zh_TW` por
  igual (lo fija `languagesWithoutAnyDictionary`).

#### Fase B — la interfaz

- ⬜ **B1. `translations/md-editor_zh_CN.ts`** + su línea en `TS_FILES`
  (`CMakeLists.txt:435`). Son **478 mensajes**. `tst_translations` falla ante
  cualquier `type="unfinished"`, así que no admite entrega parcial. El chino tiene
  **una sola forma de plural**, así que los `%n` no dan trabajo extra (a diferencia
  del polaco y el rumano, que piden tres). Verificar que `QTranslator::load`
  (`src/app/main.cpp:55`) resuelve `QLocale("zh_CN")` al `.qm` con ese nombre: Qt
  prueba antes `zh_Hans_CN`/`zh_Hans`.
- ⬜ **B2. Una línea en el menú** `Ver → Idioma`: `{ "zh_CN", "简体中文" }` en
  `src/app/mainwindowmenus.cpp:1053` (autónimo, no se traduce).
- ⬜ **B3. El manual**: `src/help/help-app_zh_CN.md` y `help-markdown_zh_CN.md`
  (**~700 líneas**), sus dos entradas en `src/resources.qrc:30` y el sufijo en las
  dos listas de `tests/tst_help.cpp` (la de idiomas con manual y la fila `zh_CN` de
  `suffixPicksTheManualOfTheLanguage`, que hoy espera `_en`). **Código, ninguno**:
  desde A4 el sufijo lo decide el recurso. Ojo: `tst_help`
  valida que **cada `](#ancla)` del índice caiga en un encabezado real**, y
  `mdhelp::headingSlug` normaliza en NFD y filtra: hay que comprobar qué produce con
  encabezados en Han (probablemente anclas vacías o colisiones), y quizá dejar los
  títulos del índice en el original con el chino al lado.
- ⬜ **B4. Fila en `mdexport::languages()`** (`src/export/exporters.cpp:45`), que es
  lo que alimenta el `lang` de ODF/HTML/EPUB y el `babel` de LaTeX. El campo babel
  queda pendiente de lo que se decida en A1; `fo:language`/`fo:country` son
  `zh`/`CN`.
- ⬜ **B5. Excepción explícita en `tst_spellscan`.** El test
  `dictionaryUrlsCoverInterfaceLanguages` (`tests/tst_spellscan.cpp:158`) afirma que
  **todo idioma de la interfaz se puede descargar desde el programa**; el chino sería
  el primero que no. No basta con quitarlo de la lista: hay que dejar escrito por qué
  (y que A2 se encarga de que eso no se note como un fallo).
- ⬜ **B6. Metadatos y cifras**: `Comment[zh_CN]` en `md-editor.desktop:22`,
  «9 languages» en `packaging/flatpak/…metainfo.xml:23`, «9 idiomas» en
  `packaging/scoop/md-editor.json:3` y «8 idiomas más» en `CLAUDE.md:13`.
- ⬜ **B7. Comprobar la fuente en el PDF.** El editor usa la fuente del documento;
  en un sistema cuya fuente no tenga dibujados los ideogramas chinos, el PDF
  exportado saldría con cajas vacías en lugar del texto. Verificar (Linux con y sin
  el paquete de fuentes `fonts-noto-cjk`, Windows, macOS) y, si hace falta, un respaldo
  de familia al exportar. Es lo único de esta fase que no se puede dar por hecho
  desde el código.

#### Fase C — la wiki

La wiki (`wiki/`) son **5 páginas por idioma** más dos parciales compartidos; el
rumano ocupa 381 líneas. No la toca ningún test ni CI: se publica a mano copiando
la carpeta al repositorio `.wiki.git` (`wiki/README.md`), así que aquí el riesgo no
es romper el build sino **dejarla a medias y que el selector de idioma lleve a un
404**.

- ⬜ **C1. Las cinco páginas**: `Home-zh.md`, `Instalacion-zh.md`, `Uso-zh.md`,
  `Caracteristicas-zh.md`, `Atajos-zh.md` (~380 líneas). Mantener el sufijo de dos
  letras del resto (`-zh`), no `-zh_CN`: es el esquema de nombres que ya usa la wiki
  y GitHub no los interpreta, son solo nombres de página.
- ⬜ **C2. `wiki/_Sidebar.md`**: un bloque `<details><summary>简体中文</summary>` con
  los cinco enlaces, al final, como los demás.
- ⬜ **C3. `wiki/_Footer.md`**: añadir `· [ZH](Home-zh)` al selector de idioma. Es
  **una línea y aparece en todas las páginas**: si se hace antes que C1, cada página
  de la wiki gana un enlace roto.
- ⬜ **C4. `wiki/README.md`**: la tabla de páginas (solo documenta el es y el en
  explícitamente; conviene que el chino no quede fuera del inventario).
- ⬜ **C5. La cifra «9 idiomas» dentro del texto de la wiki**: aparece en
  `Caracteristicas*.md` y `Home*.md` **de los nueve idiomas**, cada una en su lengua
  («9 Sprachen», «9 languages»…). Es la parte más fácil de olvidar.

#### Coste y orden

| Fase | Trabajo | Grueso |
|---|---|---|
| A (código) | 4 arreglos, todos con test | ~1 día; **valen sin B ni C** |
| B (interfaz) | 478 cadenas + ~700 líneas de manual | días, casi todo traducción |
| C (wiki) | ~380 líneas + 4 ficheros de índice | medio día |

Y el coste **recurrente** que ya señalaba la revisión de 2026-07-25 y que este
cambio empeora: `tst_translations` no deja pasar una cadena sin traducir, así que a
partir de aquí **cada `tr()` nuevo son 10 ficheros que tocar**, y cada cambio en el
manual, 10 `.md` × 2. Ese es el argumento real contra la ampliación; el chino lo
compensa por número de usuarios potenciales, pero no lo elimina.

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
> mejoras salieron en 2.4.0 (primera tanda) y 2.5.0 (el resto). Ya fuera de ese plan,
> están hechas (acumuladas para la próxima versión) las **reglas de entrada** y la
> **pasada de tipografía** de «Pulido de UX», las **plantillas por profesión** y la
> **importación de HTML/EPUB**. Lo siguiente sería la **ventana de Preferencias** y
> **Pandoc opcional** para importar más formatos. (La *fuente global* se descartó.)
