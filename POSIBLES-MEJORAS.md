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
   actions están ya en Node.js 24 (`@v5`).
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
7. **Pestañas o multi-documento** — `mainwindow` es de documento único.
8. ✅ **Export a DOCX** — ya hay PDF/HTML/ODT/LaTeX; `.docx` es el formato que más
   pide quien no usa Markdown. *Hecho:* serializador OOXML propio
   (`mdexport::writeDocx`) empaquetado con el QZip privado de Qt (sin
   dependencias), con encabezados, formato de carácter, listas, tablas, citas,
   código, enlaces (campo HYPERLINK) e imágenes embebidas; idioma y título
   incrustados. *Archivo → Exportar → A DOCX (Word)*.
9. **Diagramas** (Mermaid/PlantUML) — complementaría el soporte TeX existente.

   **🚧 En curso.** Enfoque elegido: **CLI externo opcional** (degradación
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

12. 🚧 **Descomponer `mainwindow.cpp`** — *En curso:* de 2155 a **854 líneas**
    (−60 %). Pasos dados: (a) iconos de la barra a un módulo puro `formaticons`;
    (b) construcción de menús + barra a `mainwindowmenus.cpp`; (c) filtro de
    eventos/entrada a `mainwindowinput.cpp` (ambos, métodos de `MainWindow` en
    otra unidad de traducción, como `texparser`); (d) el corrector ortográfico a
    un `SpellController` real (posee motor + estado + menú contextual). Pendiente:
    el zoom/escalado de interfaz (entrelazado con el orden de construcción y lleno
    de *workarounds* de Qt/gtk3, el más delicado) y, si se quiere, la órbita de
    sesión/recarga.
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
  como limitación.
- ✅ **Export a EPUB** — *Hecho:* `mdexport::writeEpub` empaqueta un EPUB 3
  (mimetype + OPF + nav.xhtml + toc.ncx + XHTML) con el QZip privado, reutilizando
  el HTML de Qt saneado a XHTML (`htmlBodyToXhtml`) e incrustando las imágenes como
  PNG. Idioma y título del front matter. *Archivo → Exportar → A EPUB*.

### Robustez

- ✅ **ASAN/UBSAN + clang-tidy en CI** — *Hecho:* opción CMake `ENABLE_SANITIZERS`
  (ASan+UBSan, con `vptr` desactivado por el ruido de Qt) y `.clang-tidy` con
  checks de alta señal; dos *jobs* en `ci.yml` que corren la suite bajo sanitizers
  y clang-tidy (fail-on-warning) en cada push/PR. La primera pasada cazó un bug de
  un test (`QChar` de un carácter astral) y 3 avisos de clang-tidy, ya corregidos.
- ⬜ **Fuzzing del round-trip** Markdown.
- ⬜ **Golden tests de exportadores** — fijar HTML/LaTeX/ODF/DOCX de referencia
  para detectar regresiones de salida.
- ⬜ **Accesibilidad** — desarrollado en su propia sección, [♿ Accesibilidad](#-accesibilidad).

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
NSAccessibility en cada SO, así que encaja con la filosofía del proyecto. *Pendiente
transversal: validar todo lo ✅ con un lector real (Orca/NVDA/VoiceOver), ya que
hasta ahora solo está cableado contra la API de Qt, no probado en vivo.*

> Leyenda: ✅ ya existe · 🚧 parcial · ⬜ pendiente.

### Alto impacto / poco esfuerzo

- 🚧 **Nombres y descripciones accesibles** — *Hecho (nombres):* `setAccessibleName`
  en los widgets de los que un lector no podía derivar nombre: editor WYSIWYG y
  editor de fuente (que comparten clase y coexisten en vista dividida), los campos
  Buscar/Reemplazar de la barra de búsqueda, el árbol del esquema y el contador de
  palabras. Los `QAction` (las ~33 acciones de menús y los botones **solo-icono** de
  la barra) ya exponen su `text()` como nombre accesible de serie, así que no
  necesitaban añadido. *Pendiente:* las **descripciones** (`setAccessibleDescription`,
  el matiz que el lector lee tras una pausa) y los **diálogos** propios.
- ✅ **Anunciar los mensajes de estado efímeros** — *Hecho:* helper único
  `MainWindow::showStatusMessage` por el que pasa el feedback importante (guardado,
  exportado, «no encontrado», nº de reemplazos, regex inválida, diccionario ausente,
  cambios/recarga en disco, «sin encabezados», nota sin definición); además de
  mostrarlo en la barra, lo anuncia con `QAccessibleAnnouncementEvent` (Qt 6.8+,
  guardado con `QT_VERSION_CHECK`; degrada a solo-visual en 6.5–6.7) sin mover el
  foco y solo si hay un lector activo. Las pistas de *hover* y el banner de arranque
  se dejan en `showMessage` directo, sin anunciar (serían ruido). Los
  `QMessageBox`/`QInputDialog` ya eran accesibles de serie.
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

- 🚧 **Admoniciones** — hoy NOTE/TIP/IMPORTANT/WARNING/CAUTION se distinguen casi
  **solo por color** (azul/verde/violeta/ámbar/rojo + tinte de fondo;
  `admonitions.cpp`), que un usuario con deuteranopia/protanopia confunde. El título
  ya muestra el texto del tipo, pero añadir un **glifo distintivo por tipo**
  (ℹ/💡/❗/⚠/⛔ o un símbolo Unicode monocromo) lo vuelve inequívoco sin depender del
  color. (Las erratas del corrector ya llevan subrayado ondulado además del rojo
  —correcto—; las fórmulas se reconocen por su forma renderizada, no solo por el
  color.)
- ✅ **Verificador de contraste de los temas (función pura + `tst_`)** — *Ya existía:*
  `mdtheme::contrastRatio` (WCAG 2.x, linealización sRGB; `themespec.cpp`) y
  `tst_themespec` verifican el contraste de cada `ThemeSpec`, con un listón **más
  alto** que el AA propuesto: texto de cuerpo a **AAA (≥7:1)** sobre página y cromo,
  y enlaces, texto secundario, botones, selección, tooltips y los 5 colores de
  sintaxis a **AA (≥4.5:1)**, en los 6 temas. Blinda la paleta contra regresiones al
  editar el catálogo.

### Navegación por teclado y foco

- 🚧 **Orden de tabulación explícito** (`setTabOrder`) en los diálogos y en la barra
  de búsqueda: hoy se hereda el orden de creación de los widgets, que no siempre es
  el lógico. Revisar también que el editor de fuente y **ambos** paneles del modo
  dividido sean alcanzables solo con teclado.
- ⬜ **Indicador de foco visible** — garantizar un *focus ring* nítido en los
  widgets propios y, sobre todo, bajo el tema de alto contraste (borde de foco
  explícito), para quien navega sin ratón.
- ⬜ **Auditar trampas de teclado** — los filtros de evento propios (atomicidad de
  fórmulas en `handleMathKeyPress`, continuación de listas, protección de
  shortcodes) deben dejar pasar siempre Tab/Esc/flechas y no «secuestrar» el foco;
  conviene una pasada manual solo con teclado o un test de humo.

### Sistema y preferencias

- ✅ **Tema de alto contraste** — ya existe (`ThemeId::HighContrast`: blanco/negro
  21:1, bordes blancos explícitos; `themespec.cpp`).
- ✅ **Zoom de interfaz persistente** — `ChromeZoom` escala editor, menús, barras,
  estado, esquema e iconos, y se guarda en `AppSettings::zoomLevel`.
- ⬜ **Seguir el modo de alto contraste del SO** — igual que ya se sigue el modo
  claro/oscuro del sistema (`QStyleHints`), detectar el modo de alto contraste de
  Windows/Linux y proponer automáticamente el tema equivalente.
- ⬜ **Respetar el escalado de fuente del SO** — verificar que el tamaño base parte
  del que marca el sistema (baja visión) en lugar de un punto fijo.

### Documentación y verificación

- ⬜ **Sección de accesibilidad en la ayuda (F1)** y en el README — mapa de teclado
  completo y notas para usuarios de lector de pantalla, en los 9 idiomas.
- ⬜ **Prueba con lector de pantalla real** — pasada con Orca / NVDA / VoiceOver tras
  lo anterior, como checklist de validación de las etiquetas y el orden de foco.

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
> En funcionalidad, las nuevas ideas de la sección «pendiente» son las de mejor
> encaje con la filosofía; en calidad, #14 (ASAN/UBSAN + clang-tidy en CI).
