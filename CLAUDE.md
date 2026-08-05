# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Qué es

Editor/visor **WYSIWYG** de Markdown en **Qt6 + C++17**. Por defecto se edita sobre
el texto ya renderizado, sin ver la sintaxis; pero el código Markdown es visible
opcionalmente (vista de fuente a pantalla completa o vista dividida con render y
código en paralelo, ver «Modo fuente y vista dividida» abajo). Al guardar se
serializa con `QTextDocument::toMarkdown()` (con retoques para tablas, fórmulas y
notas al pie, ver abajo). La interfaz y todos los textos están en español (idioma
de origen) con traducciones a 8 idiomas más.

## Comandos

**Dependencias de compilación.** Qt6 (≥6.5) con sus cabeceras de desarrollo **y las
privadas**: la exportación ODF usa el QZip privado de Qt vía el target CMake
`Qt6::GuiPrivate`, que necesita las cabeceras privadas (`qzipwriter_p.h`, etc.). En
Debian/Ubuntu vienen en un paquete aparte de `qt6-base-dev`:

```bash
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
```

Sin `qt6-base-private-dev`, CMake falla en la configuración con «Imported target
"Qt6::GuiPrivate" includes non-existent path .../QtGui/<versión>» (el target existe
pero apunta a cabeceras que no están instaladas).

**Opcional:** el corrector ortográfico necesita **Hunspell** (`sudo apt-get
install libhunspell-dev`); sin él, el resto compila igual y el corrector queda
inactivo. Se enlaza estático por defecto (ver «Empaquetado»). Los diccionarios en
Linux son del sistema (`hunspell-es`, `hunspell-en-us`…).

```bash
# Compilar (configura + build en build/)
cmake -S . -B build && cmake --build build
./build.sh                 # equivalente; ./build.sh -x ejemplo.md compila y ejecuta

# Ejecutar
./build/md-editor [archivo.md]

# Tests (Qt Test, headless por CMake con QT_QPA_PLATFORM=offscreen)
ctest --test-dir build --output-on-failure
ctest --test-dir build -R tst_outline        # un solo test por nombre
./build/tst_outline                            # ejecutable de test directo

# Robustez (lo corre CI; en local, build aparte):
#   Sanitizers (ASan+UBSan): aborta ante errores de memoria / UB
cmake -S . -B build-san -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build build-san
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-san --output-on-failure
#   clang-tidy (config en .clang-tidy; CI falla ante cualquier aviso)
clang-tidy -p build src/*.cpp

# Instalar (compila y copia a $PREFIX el binario + .desktop + iconos hicolor
# PNG/SVG en Linux; sudo solo si $PREFIX no es escribible). Ayuda: ./install.sh -h
sudo ./install.sh                              # build normal -> /usr/local
sudo ./install.sh -m                           # build de tamaño mínimo (build-min/)
PREFIX="$HOME/.local" ./install.sh             # de usuario, sin sudo
```

Tras tocar el código siempre `cmake --build build && ctest --test-dir build`. El
binario instalado (`/usr/local/bin/md-editor`) **no** se actualiza al recompilar
`build/`: hay que reinstalar para probar cambios en el ejecutable real del usuario.

## Arquitectura

Toda la lógica vive en una **biblioteca estática `md-editor-core`** que enlazan
tanto el ejecutable (`main.cpp`, solo arranque + i18n) como las pruebas. Añadir un
`.cpp/.h` nuevo = colocarlo en el subdirectorio de `src/` que le corresponda por
componente (ver abajo) y añadirlo a la lista de `md-editor-core` en `CMakeLists.txt`
(y, si trae lógica pura, su `tst_*` a la lista de tests del mismo archivo). Todos
los subdirectorios de `src/` están en `target_include_directories(... PUBLIC)`, así
que los `#include "foo.h"` no llevan prefijo de carpeta.

**Distribución de `src/` por componente** (el código no es plano; cada fichero vive
en su carpeta):

- `app/` — el *shell* de ventana: `MainWindow` y sus 5 unidades de traducción
  (`mainwindow`/`menus`/`input`/`zoom`/`session`), `main`, `appsettings`
  (persistencia), `chromezoom`, `langtag` (normalización de códigos de idioma).
- `editor/` — núcleo de edición: `EditorStack`, `FocusEditor`, los controladores de
  formato/inserción/tabla y los gestos puros (`autopair`, `listcontinuation`,
  `blockconstructs`, `typewriter`).
- `view/` — apariencia y modos de vista: `SplitViewController`,
  `DistractionFreeController`, `ThemeController`, `themespec`/`mdtheme`.
- `io/` — E/S y persistencia en disco: `DocumentIo`, `FileController`,
  `DiskWatcher`, `RecoveryManager`, `RecentFilesManager`.
- `markdown/` — lógica Markdown pura compartida: `markdownrender` (pipeline de
  carga), `tableedit` (serialización canónica), y las extensiones ligeras
  (`footnotes`, `tasklist`, `shortcodes`, `admonitions`, `texttransform`,
  `markdowntidy`, `codespanfix`, `charformatfix`, `headingemphasis`, `urldetect`, `richpaste`,
  `doctemplates`, `docstats`, `symbolcatalog`, `snippets`).
- `highlight/` — resaltado: `CodeBlockHighlighter`, `languageregistry`.
- `math/` — motor TeX (`mathblocks`/`texparser`/`mathlayout`/`mathobject`) +
  `FormulaController`.
- `diagram/` — Mermaid/PlantUML: `diagram`, `diagramdoc`, `DiagramRenderer`,
  `DiagramController`.
- `spell/` — corrector: `spellscan`, `SpellChecker`, `SpellController`.
- `export/` — `ExportController` (la orquestación con diálogos) y el namespace
  `mdexport`, cuya **API entera vive en `exporters.h`** pero está repartida en una
  unidad de traducción por formato, igual que `MainWindow`: `exporters.cpp` (lo
  común: catálogo de idiomas, front matter, `cloneForExport`, paginación del PDF),
  `exportlatex`, `exportdocx`, `exportepub`, `exporthtml`, `exportodf`. Lo que
  comparten entre sí va en `exportutil.h`, cabecera **interna** que los
  consumidores no incluyen: `xmlEsc`, `localFileFor` y `imageData`, que es el único
  sitio donde se resuelve una imagen del documento a bytes (con la regla de
  conservar los originales o rasterizar a PNG, que antes estaba copiada en los
  cuatro formatos).
- `widgets/` — diálogos y widgets sueltos (`FindReplaceBar`, `GoToHeadingDialog`,
  `HelpDialog`, `SymbolPicker`, `SnippetsDialog`, `OutlinePanel`) + `formaticons`.
  Los dos diálogos de filtro rápido (la paleta de comandos y el salto a
  encabezado) comparten `FilterListDialog`, que pone el armazón «campo de filtro
  sobre lista» y **el comportamiento de teclado**: las flechas y AvPág/RePág mueven
  la selección sin sacar el foco del filtro. Esa parte estaba duplicada literalmente
  en ambos; la derivada solo llena la lista y responde a `filterChanged`.

`src/` conserva además `resources.qrc`, `icons/` y `help/` (recursos Qt, fuera de
las carpetas de código). El código está documentado con Doxygen (`Doxyfile` en la
raíz; `doxygen Doxyfile` → `docs/doxygen/html`).

**Edición por pestañas (`EditorStack`).** Cada documento abierto vive en un
`EditorStack` (un `QWidget`, una pestaña) que **posee** el editor WYSIWYG/fuente y
los ~14 colaboradores ligados a ESE documento (E/S, corrector, diagramas,
vista dividida, formato, tablas, fórmulas, inserción, exportación, autoguardado,
archivo, vigilancia de disco). `MainWindow` es el *shell*: un `QTabWidget` de
`EditorStack` más lo que es de ventana (menús, barra de formato, zoom, tema, panel de
esquema, barra de búsqueda, estado, modo sin distracciones, sesión). `m_stack`
apunta al documento **activo**; `setActiveStack()` re-vincula al activo la barra de
búsqueda, el esquema, el estado de las acciones, el título y el modo de vista al
cambiar de pestaña. Las acciones de menú/barra son **únicas** y compartidas: sus
*triggers* despachan a `m_stack` (el activo) con lambdas; su estado lo refresca el
documento activo (`configureStack()` les entrega las acciones a cada pestaña). El
esquema y la barra de búsqueda son de la ventana (compartidos): solo el documento
activo los alimenta. El modo sin distracciones se **traslada** a la pestaña activa
al cambiar de documento (`DistractionFreeController::retarget`), sin salir del modo.
El **control del tema** (`ThemeController`) es igualmente único de la ventana —la
paleta y el tinte cálido nocturno son globales de la aplicación—: `setActiveStack()`
lo reapunta al editor/resaltador activo (`ThemeController::retarget`) para recolorear
sus enlaces y fijar su resaltado, y un único `QTimer` lleva el tinte. Tenerlo por
pestaña era un error: varios temporizadores escribían la paleta global con estado
desfasado y el tinte parecía activarse/desactivarse solo.
El borrador de autoguardado/recuperación es **por pestaña**: cada `RecoveryManager`
usa un slot único (`recovery-draft-<uuid>.md`), así que un cierre anómalo conserva
TODOS los documentos con cambios; al arrancar, `RecoveryManager::leftoverDrafts()`
los enumera y se ofrecen para recuperar en sus pestañas. `addTab`/`closeTab`/
`openPathInTab` gestionan el ciclo de vida; la sesión (`AppSettings::openFiles`)
reabre todas las pestañas al arrancar y tras un cambio de idioma.

`MainWindow` delega en **colaboradores autocontenidos**, cada uno una clase
pequeña con su propia responsabilidad. Su implementación está repartida en cinco
unidades de traducción para no inflar un único fichero: `mainwindow.cpp` (el
*shell*: constructor, ciclo de vida, pestañas, idioma, glue), `mainwindowmenus.cpp`
(construcción de menús y barra de formato), `mainwindowinput.cpp` (el filtro de
eventos `eventFilter` y sus sub-manejadores de entrada), `mainwindowzoom.cpp`
(zoom/escalado de toda la interfaz) y `mainwindowsession.cpp` (arranque de sesión,
recuperación de borrador y recarga del archivo en disco) — todos son métodos de
`MainWindow`, solo en `.cpp` distintos. Los iconos monocromos de la barra viven
en el módulo puro `formaticons`. Los colaboradores temáticos (la mayoría miembros
de `EditorStack` o `MainWindow`):

- **Entrada/salida y sesión**: `DocumentIo` (abrir/guardar, UTF-8, baseUrl, front
  matter, estado «modificado»), `FileController` (nuevo/abrir/guardar/recuperar +
  autoguardado), `RecoveryManager` (borrador de autoguardado), `RecentFilesManager`,
  `DiskWatcher` (vigila cambios externos del archivo).
- **Vista y apariencia**: `SplitViewController` (los tres modos de vista
  WYSIWYG/fuente/dividida y su sincronización), `DistractionFreeController`
  (pantalla completa + columna), `ThemeController` (único de la ventana: tema, luz
  cálida nocturna y recoloreado de enlaces), `ThemeSpec`/`mdtheme` (catálogo declarativo de los 6
  temas), `ChromeZoom` (zoom de toda la interfaz), `OutlinePanel` (índice TOC),
  `GoToHeadingDialog` (quick open sobre los encabezados, Ctrl+G), `FindReplaceBar`,
  `HelpDialog` (manual integrado, F1).
- **Edición e inserción**: `FormatController` (marcas de carácter, encabezados,
  listas, sangrías + estado de acciones), `InsertController` (enlaces, imágenes,
  tablas, regla, notas al pie, símbolos), `TableController` (edición contextual de
  tablas), `FormulaController` (fórmulas TeX: insertar/editar/proteger; registra
  el pintor 2D `MathObject`), `BlockConstructs` (citas y bloques de código),
  `CodeBlockHighlighter` +
  `LanguageRegistry` (resaltado), `SymbolPicker` (diálogo no modal «mapa de
  caracteres»), `FocusEditor` (QTextEdit con columna centrada para el modo sin
  distracciones y un *handler* de pegado/soltado, `setMimeInsertHandler`).
- **Exportación**: `ExportController` + `mdexport` (`exporters`).
- **Persistencia**: `AppSettings` (fachada tipada sobre `QSettings`; **todas** las
  claves de persistencia viven aquí, nadie más toca `QSettings`).

Módulos de **lógica pura** (sin clase, solo funciones en un namespace, con su
`tst_*` aislado): `listcontinuation` (`mdlist`), `tableedit` (`mdtable`), `exporters`
(`mdexport`), `mathblocks` + `texparser` + `mathlayout` (los tres en `mdmath`;
parser de fuente / motor TeX→runs / maquetación 2D), `footnotes` (`mdfootnote`), `tasklist`
(`mdtask`), `shortcodes` (`mdshortcode`), `symbolcatalog` (`mdsymbols`), `urldetect`
(`mdurl`), `richpaste` (`mdrichpaste`), `doctemplates` (`mdtemplate`),
`admonitions` (`mdadmonition`), `texttransform` (`mdtext`), `docstats`
(`mdstats`), `langtag` (`mdlang::canonicalTag`; etiqueta canónica de idioma, ver
«Internacionalización»), `blockconstructs` (`mdblock`), `outline`
(`mdoutline::headingsOf`; vive aparte de `OutlinePanel` para que quien solo quiere
el índice —la exportación a EPUB, por ejemplo— no arrastre un `QDockWidget`; lo
mismo con `mdcommands` en `app/commands.h`), `spellscan` (`mdspell`; tokenización de palabras y
selección de diccionario para el corrector ortográfico, ver abajo), `diagram`
(`mddiagram`; clasifica el lenguaje de un bloque ```mermaid/plantuml para el
render de diagramas, ver abajo), `typewriter` (`mdtypewriter`; lógica pura del
**modo foco** —*Ver → Modo foco*—: `centeredScrollValue` calcula el scroll que
centra la línea del cursor y `dimRanges` los tramos a atenuar fuera del párrafo del
cursor; la integración es `EditorStack::centerCursorLine` y `applyLineFocus` —esta
última pinta con `QTextEdit::extraSelections`, sin tocar el documento—), `snippets`
(`mdsnippet`; modelo y
(de)serialización de los snippets de usuario para `AppSettings`, con el diálogo
`SnippetsDialog` y `EditorStack::insertSnippet` aparte), `autopair`
(`mdautopair::apply`; auto-emparejado de `()[]{}` y `` ` `` sobre un `QTextCursor`,
enganchado por `MainWindow::applyAutoPair` en ambos editores), `markdowntidy`
(`mdtidy::tidy`; normalización conservadora del Markdown fuente para *Editar →
Limpiar Markdown* —recorta espacios finales preservando el salto duro, colapsa
líneas en blanco, uniforma viñetas a `- ` y el espacio tras los `#`, sin tocar el
interior de los fences ni las reglas temáticas—, integrado en
`EditorStack::cleanMarkdown`).

Patrón recurrente para lógica comprobable: separar las **funciones puras** (sin
GUI) de la integración (qué texto y dónde reinsertarlo, que vive en `MainWindow` o
el controlador), en un namespace, para testearlas aisladas. Sigue ese patrón al
añadir lógica nueva: hay un `tst_*` por módulo.

### Conceptos transversales que cruzan varios archivos

- **Round-trip Markdown.** El formato se aplica con formatos de Qt que serializan
  limpiamente a Markdown. Las citas y bloques de código se gestionan reescribiendo
  el Markdown del bloque (no con formatos de carácter) en `BlockConstructs`. El
  test `tst_markdownroundtrip` vigila la ida y vuelta.
- **Serialización canónica (tablas).** `QTextDocument::toMarkdown()` de Qt **no
  emite la alineación de columnas** de las tablas (`:--`/`:-:`/`--:`): la lee al
  abrir pero la descarta al guardar. Por eso la serialización «de verdad» pasa por
  `mdtable::documentMarkdown()`, que reinyecta esos marcadores a partir de la
  alineación de las celdas. **Todo** lo que serializa el documento usa esa función
  (no `toMarkdown` directo): `DocumentIo` (línea base, `isModified`, `write`) y
  `MainWindow` (vista de fuente, `currentBody` de recuperación, recarga). Si añades
  otra ruta de serialización, usa `mdtable::documentMarkdown()`. Esta misma función
  es además la que reinyecta las fórmulas (ver «Fórmulas TeX»), deshace el escape
  `> \[!NOTE]` de las admoniciones (`mdadmonition::unescapeMarkers`, ver abajo) y
  revierte el sobre-escapado de Qt dentro de los code spans en línea
  (`mdcodespan::unescapeInlineCode`: Qt antepone `\` a `\ & < * [ !` dentro del
  código, que `setMarkdown` re-lee literal y se duplicaría en cada guardado), y
  reinyecta el énfasis de los encabezados (`mdheademph`, abajo).
- **Énfasis dentro de un encabezado (`mdheademph`).** Otro «lo lee al abrir y lo
  descarta al guardar» de Qt, este con **pérdida de contenido**: `## uno **negrita**
  y *cursiva* fin` volvía a disco como `## uno negrita y cursiva fin` (negrita,
  cursiva y tachado; el código en línea y los enlaces sí los emite). Solo pasa en
  encabezados: en párrafos, citas, listas, tareas y celdas de tabla Qt emite las
  marcas bien. El arreglo va en dos mitades porque la información se destruye entre
  una y otra:
  - Al **cargar**, `markExplicitBold` anota con `ExplicitBoldProperty` los runs que
    venían en negrita del fuente. Hay que hacerlo **antes** de que
    `mdcharfix::repairHeadingRuns` ponga la negrita ESTRUCTURAL en todo el bloque
    (arriba), porque después las dos negritas son la misma; por eso lo llama esa
    función y no el pipeline, para que no dependa de que nadie reordene las pasadas.
    Se distinguen por el tamaño: Qt le pone `FontSizeAdjustment` al run del
    encabezado y no al span de énfasis (y lo pone en los seis niveles, también el
    paso 0 del h4). La cursiva y el tachado no necesitan anotarse: un encabezado no
    los lleva de suyo.
  - Al **guardar**, `replaceWithSentinels`/`restoreFromSentinels` en
    `documentMarkdown`, con la misma técnica de centinelas PUA que las fórmulas.
    Corre **después** de las fórmulas: sus runs son cursiva, y antes saldrían
    envueltos en `*…*`. Los centinelas van en formato plano (dentro de un span de
    código Qt escaparía las marcas), los runs contiguos con el mismo énfasis
    comparten un par (`**x****y**` no es negrita para ningún lector) y no abarcan los
    espacios de los extremos (`*x *` tampoco lo es).

  *Limitaciones:* la negrita que el usuario aplique a mano dentro de un encabezado no
  se guarda (el encabezado ya está en negrita, así que no hay nada que la distinga de
  la estructural; la cursiva y el tachado sí). Y `**` sobre PARTE de un enlace
  (`[a **b**](u)`) sale como dos enlaces: eso lo parte Qt por su cuenta, también en
  un párrafo y ya antes de este arreglo.
- **Front matter.** Si el archivo empieza por `---…---`/`+++…+++`, `DocumentIo` lo
  separa antes de `setMarkdown` (para que no se tome por una regla horizontal), lo
  conserva verbatim y lo reescribe al guardar. No se renderiza ni se edita. Se
  expone con `frontMatter()`; la exportación lee de ahí el idioma (`lang`/`language`)
  y el `title` con `mdexport::frontMatterValue()`.
- **Modo fuente y vista dividida (doble editor).** Esta lógica vive en
  `SplitViewController` (`m_split`), no en `MainWindow`. El widget central es un
  `QSplitter` horizontal con el editor WYSIWYG (`m_editor`, izquierda) y un editor
  de texto plano (`m_sourceEditor`, derecha). Hay tres modos de vista, gestionados
  por `updateEditorVisibility()` mostrando/ocultando paneles: **WYSIWYG** (solo
  `m_editor`), **dividido** (`m_splitMode`, ambos visibles y editables) y **fuente
  a pantalla completa** (`m_sourceMode`, solo `m_sourceEditor`). Split y fuente-
  completo son excluyentes. El contenido del fuente vive como texto plano hasta
  `commitSourceToDocument()`; por eso `DocumentIo::isModified()` no ve sus cambios
  y hay que mirar también `m_sourceDirty` (las rutas de lectura/guardado keyean por
  `m_sourceDirty`, no por el modo). Las acciones WYSIWYG se deshabilitan cuando el
  fuente es el panel activo (lista `m_wysiwygActions`); las de tabla, por contexto
  (`updateTableActions`). En split, el panel activo lo decide el **foco**
  (`updateActionsForFocus`, vía `QApplication::focusChanged`).
- **Sincronización de la vista dividida.** Regla: *solo se actualiza el panel SIN
  foco*, nunca el que el usuario está editando (evita saltos de cursor y que se le
  reescriba el texto). Dos `QTimer` de debounce (~250 ms): `m_syncToSourceTimer`
  (WYSIWYG→fuente, `syncSourceFromDocument`) y `m_syncToDocTimer` (fuente→WYSIWYG,
  `syncDocumentFromSource`/`commitSourceToDocument`). El flag `m_syncing` envuelve
  toda actualización programática para que los `contentsChanged`/`textChanged` que
  provoca no realimenten el bucle. `flushPendingSync()` (en `focusChanged`) vacía
  el timer pendiente al cambiar de panel para que el destino llegue al día. Se
  preserva el scroll del panel refrescado.
- **Temas y luz cálida nocturna.** `ThemeController` aplica uno de los 8 temas del
  catálogo declarativo `mdtheme`/`ThemeSpec` (Claro, Oscuro, GitHub Light, GitHub
  Dark, Monokai, Alto contraste, Solarized Light, Solarized Dark), persiste la
  clave `theme` (con migración del
  antiguo booleano `darkTheme`) y recolorea enlaces + resaltado al cambiar. Sobre
  cualquier tema se superpone, **ortogonalmente**, la **luz cálida nocturna**
  (toggle `warmLight`, **activo por defecto**): un tinte cálido **automático y
  gradual según la hora del reloj** del sistema. `warmthForTime` define la
  intensidad `w∈[0,1]` (día 07–19 → 0; rampa ascendente 19→23; noche 23–06 → 1;
  rampa descendente 06→07) y `applyWarmth` la aplica como filtro multiplicativo solo
  sobre `QPalette::Base`/`AlternateBase` (azul −16 %·w, verde −5 %·w, rojo intacto).
  Un `QTimer` refresca cada 60 s y solo repinta si `w` cambió ≥0.02. No afecta a
  enlaces ni resaltado. El controlador es **único de la ventana** (no uno por
  pestaña, que se pisarían sobre la paleta global): `setActiveStack` lo reapunta al
  editor activo (`retarget`); reaplicar el tema al activar una pestaña es barato
  porque `recolorLinks` no edita si los enlaces ya están al día y
  `CodeBlockHighlighter::setSyntaxColors` es idempotente (no rehace el resaltado si
  los colores no cambian). El menú es *Ver → Tema → «Luz cálida nocturna»*.
- **«Modificado».** `DocumentIo::isModified()` compara la serialización canónica
  con una línea base, no usa `QTextDocument::isModified()` (que `QTextEdit` ensucia
  de forma espuria al trazar la primera vez).
- **Plantillas de documento.** `doctemplates` (`mdtemplate::all()`) es el catálogo
  de esqueletos Markdown de *Archivo → Nuevo desde plantilla*. Sus textos pasan por
  `tr()` (contexto "MainWindow") para traducirse con el resto; **no** van en `.qrc`.
  `FileController::newFromTemplate` los carga vía `DocumentIo::loadFromString`, que
  es como `load()` pero sin archivo y con línea base vacía (cuenta como modificado,
  para que no se pierdan sin avisar). El tamaño de fuente no es expresable en
  Markdown: lo «grande» (p. ej. `CERTIFICO`) se consigue con un encabezado.
- **Arranque de sesión.** `main.cpp` difiere con `QTimer::singleShot(0, ...)` la
  llamada a `MainWindow::startSession()` (abrir en mitad del trazado inicial de
  `QTextEdit` provoca un diálogo espurio). Prioridad: archivo de línea de comandos
  › recuperar borrador › reabrir último documento. `lastFile` solo se persiste con
  rutas **no vacías** (el documento nuevo inicial no debe pisarlo).
- **Zoom de toda la interfaz.** `applyChromeZoom()` escala, partiendo de tamaños
  base, no solo el editor: menú **y cada `QMenu`** (los desplegables no heredan la
  fuente de la barra), barras, estado, fuente, panel de esquema, **rótulos de las
  pestañas** e iconos de la barra de formato (`updateToolBarIcons`). Las pestañas se
  escalan en la `QTabBar`, **no** en el `QTabWidget`: la fuente del contenedor se
  propagaría a los editores de cada pestaña, que llevan su propio tamaño.
- **Zoom de los diálogos.** Van aparte del resto del chrome porque son **ventanas
  propias**: Qt no les propaga la fuente del padre (solo con `WA_WindowPropagation`),
  así que tomaban la de `QApplication` y se quedaban al tamaño base. `applyDialogZoom`
  se la fija **explícitamente**; la base es siempre `QApplication::font()`, que el
  zoom no toca, así que reaplicarlo no acumula. Dos decisiones que parecen de más y
  no lo son:
  - **No sirve la fuente por clase** (`QApplication::setFont(f, "QDialog")`, como sí
    se hace con los menús): escala el marco del diálogo, pero **sus hijos no la
    heredan** —Qt solo hereda del padre si este la tiene puesta a mano o si el hijo
    no resuelve ninguna fuente de clase, y el tema de la plataforma pone unas
    cuantas—, así que los rótulos y botones de dentro se quedaban pequeños.
  - **Quién se la aplica a cada diálogo**: el `eventFilter`, instalado también en
    `QApplication`, al ver su `QEvent::Polish` (justo antes de mostrarse, y el primer
    momento en que Qt sabe de qué clase es el widget). Es lo único que los alcanza a
    todos sin escalarlos uno a uno: los abren `MainWindow`, `EditorStack` y sus
    controladores, y unos cuantos son de Qt (`QMessageBox`, `QInputDialog`). Los ya
    abiertos —el manual y el mapa de caracteres son no modales— los re-escala
    `applyChromeZoom` recorriendo `findChildren<QDialog *>()`. Los diálogos nativos
    del sistema (`QFileDialog` en Linux) los pinta el escritorio: van a su aire.

  Un diálogo que derive tamaños de su propia fuente **al construirse** no se entera
  (el Polish llega después): el mapa de caracteres (`SymbolPicker`, símbolos a ×1.4 y
  su celda) y el ancho del índice del manual lo recalculan en `changeEvent`
  (`QEvent::FontChange`), y el diálogo de fórmulas y «Acerca de» se adelantan con
  `ensurePolished()` antes de medir con esa fuente. Nada de esto puede ser un tamaño
  fijo en píxeles: con el zoom subido, el texto no cabe en la casilla.
- **Tamaño de las ventanas de texto con el zoom.** Escalar solo la fuente no basta en
  las ventanas que son texto corrido: en el mismo ancho en píxeles cabe menos y las
  líneas se parten de forma poco natural. La ventana tiene que crecer con la fuente:
  - **Manual** (`HelpDialog::updateWindowSize`, en cada `FontChange`): parte del
    tamaño cómodo con la fuente base (820×620) y lo escala por lo que haya crecido
    esta, sin pasar del 90 % de la pantalla. El cálculo puro es
    `chromezoom::scaledWindowSize`.
  - **«Acerca de»** es un `QMessageBox`, que decide él el ancho del texto y a la
    medida de la fuente base. Quitarle el ajuste de línea al rótulo **no vale** (se lo
    vuelve a poner al mostrarse); lo que sí respeta es el mínimo de su rejilla, así
    que se le mete un `QSpacerItem` del ancho que piden las frases medidas con la
    fuente real. Depende de cómo Qt maqueta el `QMessageBox`: lo vigila
    `aboutBoxWidensWithZoom` en `tst_chromezoom`.
- **`eventFilter` de `MainWindow`.** Está instalado en los editores de la pestaña
  activa **y en `QApplication`** (esto último solo para el `QEvent::Polish` de los
  diálogos, ver «Zoom de los diálogos»); por lo demás es un despachador: delega en
  tres sub-manejadores según el objeto vigilado (cada uno devuelve `bool`, el
  primero que consume gana): `handleViewportEvent` (zoom con Ctrl+rueda; abrir enlaces con
  Ctrl+clic/hover; arrastrar-soltar un archivo; clic sobre la casilla de una tarea
  `mdtask` y sobre una referencia de nota al pie `mdfootnote`), `handleEditorKeyPress`
  (protección de fórmulas, shortcodes `:nombre:` y **auto-emparejado** en `m_editor`)
  y `handleSourceKeyPress` (**continuación de listas** con Enter en `m_sourceEditor`
  vía `mdlist::analyze` —en WYSIWYG la hace `QTextEdit` de serie— y auto-emparejado).
  El auto-emparejado (`applyAutoPair` → `mdautopair`) corre el último, tras la
  protección de fórmulas, así que no interfiere dentro de una math.
- **Pegar/soltar imágenes y URLs.** El *handler* de `FocusEditor` desvía las
  imágenes del portapapeles a disco (PNG junto al `.md`, ruta relativa) e inserta
  `![](ruta)`, en vez de incrustarlas (que no round-trip-ean). También en *Insertar
  → Pegar imagen*; pregunta el texto alternativo. Al pegar una URL (`mdurl`) sobre
  una selección, se auto-enlaza el texto seleccionado. *Editar → Pegar como
  Markdown* (Ctrl+Alt+V) convierte el HTML del portapapeles a Markdown con
  `mdrichpaste::htmlToMarkdown` (`QTextDocument` auxiliar + `documentMarkdown`) en
  vez de incrustar el formato del origen.
- **Vigilancia del archivo en disco.** `QFileSystemWatcher` sobre el archivo
  abierto, con debounce (`QTimer`) e instantánea de bytes para distinguir el propio
  guardado de un cambio externo: si no hay cambios locales recarga solo; si los
  hay, pregunta.
- **Pipeline de carga (`mdrender`).** La secuencia «proteger el fuente →
  `setMarkdown` → pasadas de render (fórmulas, notas al pie, admoniciones)» vive en
  un único sitio: `mdrender::setMarkdownWithExtensions` (con `protect` y
  `renderPasses` separables). La usan `DocumentIo::load`, `DocumentIo::loadFromString`
  y `MainWindow::setBodyMarkdown`. **Añadir una extensión ligera nueva = tocar solo
  `mdrender`**, no esos tres sitios (antes estaba duplicado y era fácil olvidarse de
  uno). El dialecto Markdown único de carga **y** guardado es
  `mdrender::kMarkdownFeatures` = GitHub + **`MarkdownNoHTML`**: sin NoHTML, Qt trata
  `<algo>` como HTML en línea y se traga ese texto y el de alrededor al cargar
  (pérdida de datos); con NoHTML son texto literal y el round-trip converge. Lo usan
  `setMarkdownWithExtensions` (carga) y `mdtable::documentMarkdown` (`toMarkdown`).
- **Reparación del formato que deja el importador de Qt (`mdcharfix`).** Primera
  pasada de `renderPasses` (antes que las extensiones, que heredan de ella).
  `setMarkdown` deja dos defectos de PRESENTACIÓN —no tocan el Markdown, y el test
  `roundTripIsUnchanged` lo vigila— que se veían en pantalla y se arrastraban a las
  exportaciones:
  - **Un encabezado se corta a media línea.** Qt solo pone el tamaño y la negrita
    del encabezado al PRIMER fragmento del bloque: en cuanto aparece un span en
    línea (código, negrita, cursiva, un enlace), ese span **y todo lo que va
    detrás** se quedan con el formato del cuerpo. `## … los `module-info` las
    vigilan` salía con «module-info» pequeño y «las vigilan» ni grande ni en
    negrita. `repairHeadingRuns` repone en todos los fragmentos el paso relativo
    del nivel (`FontSizeAdjustment` = `4 - nivel`, el mismo que usa Qt) y la
    negrita. Se deriva del **nivel del bloque**, no de otro fragmento, porque un
    encabezado puede EMPEZAR por un span y entonces no queda ninguno bien.
  - **El código no sigue al zoom.** Qt clava en los runs de código un
    `FontPointSize` ABSOLUTO = el tamaño por defecto al importar; como el zoom
    cambia la fuente del documento (`editor->setFont`), el código se quedaba al
    tamaño de carga mientras la prosa crecía. `unpinCodeFontSize` lo retira para que
    herede. Ojo al reconocerlos: el span en línea lleva `fontFixedPitch`, pero los
    BLOQUES de código (vallados **e** indentados) no —solo la familia monospace—,
    así que se miran por `BlockCodeLanguage` del bloque. `cloneForExport` ya hacía
    esto mismo para la exportación; ahora sale bien desde la carga.
- **Tareas, notas al pie, shortcodes, tipografía y admoniciones (extensiones
  ligeras).** Módulos puros que Qt no entiende pero **tampoco estorba** al
  round-trip (sus pasadas de render las orquesta `mdrender`, arriba):
  - `mdtask` — casillas `- [ ]`/`- [x]`. Qt las renderiza y serializa solo
    (`QTextBlockFormat::marker()`); el módulo solo aporta el gesto de marcar/
    desmarcar con clic sobre la casilla.
  - `mdfootnote` — referencias `[^id]` y definiciones `[^id]:`. **Sí** toca la
    carga: `protectFootnotes` sustituye el `:` del rótulo por un centinela de la
    PUA antes de `setMarkdown` (si no, md4c se comería `[^1]: Ibíd.` por una
    definición de enlace de referencia), y `renderFootnotesInDocument` lo restaura
    y da estilo de superíndice a las referencias. No toca el guardado (el `[^id]`
    sobrevive como texto literal). Clic en una referencia salta a su definición.
  - `mdshortcode` — expande `:nombre:` a símbolos (`:alpha:`→α) al teclear.
  - `mdadmonition` — «callouts» estilo GitHub: una cita cuya primera línea es
    `[!NOTE]`/`[!TIP]`/`[!IMPORTANT]`/`[!WARNING]`/`[!CAUTION]`.
    `renderAdmonitionsInDocument` les da fondo tintado y título en color (solo
    color: negrita/cursiva sí serializan y romperían el marcador). El round-trip
    es casi transparente, salvo que `toMarkdown` escapa el corchete (`> \[!NOTE]`);
    `unescapeMarkers` lo deshace dentro de `mdtable::documentMarkdown`. Inserción
    desde *Insertar → Admonición*.
  - `mdtext` — transformaciones sobre la selección (mayúsculas/minúsculas, *title
    case*, ordenar líneas) y **tipografía inteligente** (`---`→—, `--`→–, `...`→…,
    comillas tipográficas según el contexto).
- **Símbolos especiales.** `mdsymbols` es el catálogo por categorías (datos puros)
  y `SymbolPicker` el diálogo no modal que los presenta en pestañas + rejilla y
  emite `symbolChosen()` para insertarlos sin cerrarse.
- **Estadísticas del documento.** `mdstats::analyze` (palabras, caracteres,
  párrafos, frases, tiempo de lectura) alimenta el contador de la barra de estado
  y el diálogo de estadísticas, sobre el texto plano del editor activo o la
  selección. **No parte por `\s+`**: el chino, el japonés y el coreano no separan
  las palabras con espacios, así que hacerlo contaba un párrafo entero como UNA
  palabra y daba un tiempo de lectura absurdo. Cada ideograma, kana o sílaba hangul
  cuenta como una palabra (y corta la palabra latina en la que aparezca: `混合
  español` son tres), su puntuación no cuenta, y el tiempo suma los dos ritmos
  porque no se leen igual de rápido (`wordsPerMinute * 2` para los ideogramas). Para
  un texto latino el resultado es idéntico al de antes.
- **Diagramas (opcional, Mermaid/PlantUML).** Como Mermaid es JS y PlantUML es
  Java, no hay motor C++: se renderizan ejecutando la herramienta externa
  (`plantuml` / `mmdc`) si está instalada — degradación elegante, **sin
  dependencia enlazada** (solo `QProcess`). Piezas: `diagram` (`mddiagram`, puro:
  `kindForLanguage`), `DiagramRenderer` (async vía `QProcess`, cachea por fuente,
  emite `rendered`/`failed`), `diagramdoc` (bloque de preview marcado +
  `removePreviewBlocks`) y `DiagramController` (escanea los grupos de bloques de
  código ```mermaid/plantuml, pide el render con debounce y coloca la imagen en un
  bloque de presentación **bajo** el bloque, opción «imagen debajo»). El round-trip
  es transparente: `documentMarkdown` llama a `removePreviewBlocks` sobre el clon,
  así que la imagen nunca llega al Markdown ni cuenta para «modificado». Si la
  herramienta falta, en vez de la imagen pone un **marcador inline** (mismo bloque
  de preview, `PreviewPlaceholderProperty`) con la orden de instalación de la
  plataforma (detectada con `QSysInfo::kernelType`, sin `#ifdef`): contextual,
  seleccionable para copiar la orden, y se reemplaza por la imagen en cuanto la
  herramienta aparece. La previsualización se puede apagar (*Ver → Previsualizar
  diagramas*, ajuste global `AppSettings::diagramPreview`, activado por defecto):
  `DiagramController::setEnabled(false)` retira todas las previews con
  `removeOrphanPreviews({})` (conjunto de regiones vacío = todas huérfanas) y
  `scheduleRefresh` corta en seco; el menú lo aplica a **todas** las pestañas. Útil
  cuando el autor ya inserta a mano una imagen pre-renderizada bajo el bloque (que
  si no, se vería por duplicado con la preview automática).
- **Corrección ortográfica (opcional, Hunspell).** Primera dependencia de
  terceros, **opcional** (`SPELL_CHECK`→`HAVE_HUNSPELL` en CMake): sin
  `libhunspell-dev` el build sigue verde. Piezas: `spellscan` (`mdspell`, puro:
  `tokenize` palabras + `pickDictionary`), `SpellChecker` (envuelve Hunspell tras
  un *pimpl*; **siempre se compila**, stub inerte sin soporte; carga perezosa de
  un idioma). El subrayado lo hace el **mismo** `CodeBlockHighlighter`
  (`highlightSpelling`, en la rama no-código, saltando código en línea, fórmulas
  y enlaces) con `SpellCheckUnderline` — presentación pura, no toca el Markdown.
  `SpellController` es el colaborador de `MainWindow` que **posee** el motor
  `SpellChecker` (se lo enchufa al highlighter), el interruptor activado/
  desactivado y el menú contextual de sugerencias. `applyLanguage()` elige el
  diccionario por el idioma del documento (front matter › ajuste › locale) en cada
  `documentLoaded` y al arrancar, y rehace el resaltado. La lista personal vive en
  `AppSettings::personalDictionary`.
  **Cuando falta el diccionario** no basta con degradar en silencio (el usuario no
  sabe por qué no se subraya nada): además del mensaje de la barra de estado, sale
  un aviso —**una vez por idioma y sesión**, con «No volver a avisar» persistido en
  `AppSettings::spellMissingWarning`— que dice qué falta y **cómo instalarlo**: en
  Linux la orden del gestor de paquetes de SU distribución
  (`mdspell::dictionaryInstallCommand`, por `QSysInfo::productType()`), y en
  Windows/macOS la carpeta donde copiarlo. El aviso distingue «falta el
  diccionario» de «esta build no trae corrector»
  (`SpellChecker::isEngineAvailable()`): mandar a instalar un diccionario cuando no
  hay motor es un callejón sin salida. El botón **«Descargar e instalar»**
  (`DictionaryInstaller`) lo baja del repositorio de LibreOffice a
  `SpellChecker::userDictionaryDir()` —la primera ruta de búsqueda y la única
  escribible: dentro de un AppImage o un `.app` no se puede escribir— y recarga el
  idioma. Es **el único sitio del programa que usa la red**, y solo al pulsar el
  botón; las URLs (`mdspell::dictionaryUrls`) son la misma tabla que
  `scripts/fetch-dictionaries.sh`, así que una ruta que cambie upstream hay que
  tocarla en los dos sitios.
  Hay un **tercer** caso, que es el mismo callejón sin salida por otra puerta:
  idiomas para los que **no existe** diccionario Hunspell en ninguna parte (chino,
  japonés y coreano: Hunspell corrige palabras separadas por espacios, y esas
  escrituras no lo están). `mdspell::hasHunspellDictionary` los distingue de «no
  está instalado», y con ellos `applyLanguage` se queda en el mensaje de la barra de
  estado: ni aviso, ni orden de instalación de un paquete que no existe
  (`hunspell-zh`), ni botón de descarga sin nada que descargar. Por lo mismo,
  `mdspell::tokenize` **no tokeniza** esas escrituras (para `isLetterOrNumber` son
  letras, así que una frase china entera salía como UNA palabra y el párrafo
  aparecía subrayado de punta a punta); las palabras latinas que haya entremedias sí
  se siguen corrigiendo. El menú *Ver → Idioma de corrección* no necesita nada de
  esto: lo llena `SpellChecker::availableLanguages()` con lo que hay en disco.

### Fórmulas TeX (`mdmath`)

El editor soporta `$...$` y `$$...$$` sin dependencias externas. El módulo `mdmath`
lo orquesta todo y es **puro** (lo prueban `tst_mathblocks`). Vive en un header
único `mathblocks.h` pero el `.cpp` está partido en dos: `mathblocks.cpp` (scanning
del Markdown fuente — `findMath`/`protectMath` — e integración con `QTextDocument`)
y `texparser.cpp` (el motor de parseo TeX→runs/Unicode: `renderTexAsRuns`,
`texToUnicode`, `wrapTex` y sus tablas). Mismo namespace `mdmath`; los consumidores
solo incluyen `mathblocks.h`. **El parseo es recursivo por nivel de anidamiento**
(`renderTexAsRuns`/`parseFrac`/`parseScript`/`texToUnicode` se llaman entre sí, y
`mathlayout::buildHList` consigo mismo), así que ambos *hubs* llevan un **tope de
profundidad (256, guard RAII)**: como el TeX lo escribe/pega el usuario, sin tope
una fórmula muy anidada (`\frac{\frac{…}}`, `x^{y^{…}}`) desbordaba la pila
(SIGSEGV); al excederlo se devuelve el TeX restante como texto literal. Piezas clave:

- *Carga.* `DocumentIo::load` aplica `mdmath::protectMath` al texto fuente antes de
  `setMarkdown`: envuelve cada `$tex$`/`$$tex$$` en inline-code ``` ``$tex$`` ```
  para que Qt no reinterprete `_`/`*`/`\` dentro como cursiva o escape. Después
  `mdmath::renderMathInDocument` sustituye cada inline-code con forma `$tex$` por
  una **secuencia de fragmentos** del `QTextDocument`: cursiva + super/subíndice
  real de Qt (`QTextCharFormat::AlignSuperScript`/`AlignSubScript`, no solo el
  repertorio Unicode). Todos los fragmentos de una misma fórmula comparten tres
  propiedades custom — `IsMathProperty`, `MathTexProperty`, `MathBlockProperty` —
  que permiten reconocerlos como grupo.
- *Render TeX → runs.* `mdmath::renderTexAsRuns(tex, baseFmt)` es el parser que
  produce esa lista de `MathRun = {QString text, QTextCharFormat fmt}`. Maneja:
  letras griegas y operadores (tabla `singleCharCommands`: griego, relaciones,
  conjuntos, lógica, flechas, grandes operadores, delimitadores como `\langle`/
  `\lceil`, símbolos varios), `^`/`_` con argumento de carácter / grupo / comando,
  `\frac{a}{b}` (fraction slash `⁄` si num y den son de un solo carácter; si no,
  `(num)/(den)`), `\sqrt{x}`, `\mathbb{R}`. Además: `\left`/`\right` emiten su
  delimitador (sin escalarlo; `.` = delimitador nulo), `\not X` lo niega con el
  combinante U+0338, `\quad`/`\qquad` dan espacios em y `\mathcal`/`\mathscr`/
  `\mathfrak{X}` mapean cada letra a su variante matemática Unicode
  (`styledMathAlphabet`, con las sustituciones de «Letterlike Symbols» del bloque
  astral). `\left`/`\right`/`\not`/`\mathcal`/… se manejan igual en el motor 2D
  (`mathlayout`), reusando los helpers `readTokenAsUnicode`/`styledMathAlphabet` de
  `mathblocks.h`. `texToUnicode` es solo un thin-flatten encima para los exports
  sin formato rico.
- *Edición.* `Insertar → Fórmula…` (Ctrl+Shift+F) abre un diálogo con previsuali-
  zación en vivo e inserta los runs en el cursor. Doble clic sobre una fórmula
  reabre el diálogo precargado y la sustituye. Las fórmulas son **atómicas** frente
  al teclado: `MainWindow::handleMathKeyPress` (instalado como `eventFilter` en
  `m_editor`) descarta caracteres imprimibles dentro del grupo y convierte
  Backspace/Delete en el borde en borrado del grupo entero.
- *Accesibilidad.* Una fórmula 2D es un `ObjectReplacementCharacter` opaco para los
  lectores de pantalla (y las inline, glifos Unicode sin estructura), así que
  `EditorStack::announceFormulaUnderCursor` (en `cursorPositionChanged` del WYSIWYG)
  detecta la fórmula bajo el cursor por `IsMathProperty`/`MathTexProperty` y anuncia
  su TeX con `showStatusMessage` → `QAccessibleAnnouncementEvent`. Deduplica por el
  inicio del grupo (`formulaAtCursor`) para sonar una vez al entrar, no al moverse
  dentro.
- *Serialización fiel.* `mdtable::documentMarkdown` clona el documento, reemplaza
  cada grupo de fórmula por una **sentinela** en la PUA de Unicode
  (`U+F8FE…U+F8FF` envolviendo el índice en `MathSentinelTable`) — texto opaco que
  `QTextDocument::toMarkdown()` no escapa — y reinyecta `$tex$`/`$$tex$$` con
  `restoreMathFromSentinels`. Resultado: los `\sum`, `\frac`, `_`, `*` del TeX
  sobreviven íntegros al round-trip. `unprotectMath` sigue existiendo pero no se usa
  en producción (queda como inversa explícita de `protectMath` para los tests).
- *Maquetación 2D (Nivel 2).* Las fórmulas con `\frac`, `\sqrt`, `\binom`, una
  matriz (`\begin{matrix}`/`pmatrix`/`bmatrix`/`cases`…) o un gran operador
  (`\sum`/`\int`/`\prod`…) con límites se pintan en 2D real (fracciones y binomios
  apilados, límites encima/debajo, radical con vínculo, rejillas con
  delimitadores) en vez de aplanarse a runs. El motor puro es `mdmath` en
  `mathlayout.{h,cpp}`: parsea el TeX a un árbol de cajas (`HList`/`Glyph`/`Frac`/
  `Script`/`BigOp`/`Sqrt`/`Matrix`/`Binom`) y lo mide/pinta (`needsTwoDLayout`/
  `measureFormula`/`paintFormula`, reutilizando la tabla de glifos de texparser,
  `commandToUnicode`). Los **acentos** (`\hat`/`\bar`/`\vec`/`\tilde`/`\dot`/
  `\ddot`…) NO fuerzan 2D: se resuelven con caracteres combinantes Unicode
  (`accentCombiningChar`, p. ej. `x̂`) tanto en los runs inline como en el 2D, así
  que `$\hat{x}$` se queda en línea y exporta solo. `\underline` es un acento más
  (combinante U+0332). `\text{…}`/`\mathrm{…}` emiten su argumento literal;
  `\binom` inline (y en export) se aproxima como `C(n, k)`
  (en LaTeX se emite nativo). Una de esas fórmulas vive en
  el documento como **un carácter** `ObjectReplacementCharacter` con
  `setObjectType(MathObjectType)` + las propiedades de math; lo dibuja el
  `QTextObjectInterface` `MathObject` (`mathobject.{h,cpp}`), que lo mide con la
  fuente por defecto del documento (así **escala con el zoom**). `renderFormulaRuns`
  es el despachador único (objeto 2D vs runs inline según `needsTwoDLayout`) que
  usan la carga (`renderMathInDocument`), la inserción y el preview. Las demás
  fórmulas siguen como runs. Como el carácter objeto comparte `IsMath`/`MathTex`,
  la serialización (sentinelas), los bounds y la edición atómica lo tratan como un
  grupo de un fragmento, sin cambios. *Limitación:* el objeto se ancla por su borde
  inferior al baseline (modelo de Qt), así que una fórmula 2D **inline** queda algo
  alta; las de bloque (`$$`, solas en su línea) se ven centradas.
- *Resaltado.* El color vive en `SyntaxColors::math` y lo aplica
  `CodeBlockHighlighter::highlightMathFragments` recorriendo los fragmentos del
  bloque con `IsMathProperty` y haciendo `setFormat(...)` solo con el foreground.
  Se reaplica al cambiar de tema (`setSyntaxColors` invalida el resaltado). El
  carácter objeto 2D toma su color del lápiz que Qt fija antes de `drawObject`
  (que ya incluye ese overlay), así que también sigue al tema.
- *Exportación.* **LaTeX**: `inlineLatex` detecta fragmentos por `IsMathProperty`,
  agrupa los consecutivos con el mismo `MathTex` y emite **una** `$tex$`/`$$tex$$`
  por grupo (preámbulo con `amsmath`+`amssymb`); el carácter objeto 2D cuenta como
  un grupo de uno. Ojo con no confundir esos runs con el super/subíndice **que no
  es fórmula** (`x^2^`/`H~2~O` de `mdsupsub`, las referencias de nota al pie): ese
  solo lleva `verticalAlignment`, y hay que emitirlo como
  `\textsuperscript`/`\textsubscript` —mirar únicamente las propiedades de math lo
  dejaba caer a ras de línea («H2O»), y LaTeX era el único formato que lo perdía
  (DOCX lo emite como `w:vertAlign`, y HTML/ODF/PDF los serializa Qt).
  **HTML/PDF/ODF/DOCX**: pasan por `mdexport::cloneForExport`, que
  clona el documento, limpia las propiedades custom de los runs inline y
  **expande** cada carácter objeto 2D a esos runs inline (cursiva + super/sub),
  dejando que Qt serialice el vertical-align a CSS/ODF/PDF (la maquetación 2D es
  solo de pantalla). `cloneForExport` además **hornea el resaltado de sintaxis**
  (`bakeCodeHighlighting`): el color del código lo pinta el `QSyntaxHighlighter`
  como overlay de la maqueta (`block.layout()->formats()`), que `clone()` no copia;
  se copia como formato de carácter real **solo en los bloques de código**
  (`BlockCodeFence`), para no arrastrar el subrayado ortográfico de la prosa. Así el
  código exporta con color (no-op si no hay resaltador, p. ej. en tests).
- *Multilínea.* `$$...$$` de bloque puede cruzar varias líneas en la fuente
  (estilo Pandoc/Obsidian): `findMath` rastrea la apertura entre líneas y
  `protectMath` codifica los saltos internos en un placeholder PUA
  (`kNewlinePlaceholder`) para que el inline-code quepa en una sola línea de
  Markdown; `renderMathInDocument` los restaura. Las inline (`$...$`) no cruzan
  líneas (regla habitual). Lo verifican `findFindsMultilineBlockMath` y
  `roundTripPreservesMultilineMath` (+ casos límite: contenido en las líneas
  delimitadoras, descarte si no cierra, ignorado dentro de un fence).
- *Limitaciones.* El motor 2D cubre fracciones, raíces, binomios, matrices,
  `cases`, acentos y grandes operadores con límites; lo no soportado (entornos
  multilínea con `&` como `align`/`aligned`, `\overbrace`/`\underbrace`,
  `\xrightarrow`, layout de límites de integral propios…) se aproxima inline o se
  ve pobre. Los delimitadores de `\left`/`\right` se emiten pero **no se escalan**
  al contenido (limitación asumida). `\mathcal`/`\mathfrak` dependen de que la
  fuente tenga los glifos matemáticos del plano astral. El alineado vertical de las fórmulas 2D
  **inline** queda alto: el `QTextObjectInterface` de Qt ancla el objeto por su
  borde inferior al baseline y su API (`intrinsicSize` da un `QSizeF`, sin
  separar ascenso/descenso) no permite descender bajo el baseline, así que no se
  puede centrar sobre el eje. Las de bloque (`$$`, solas en su línea) sí se ven
  bien. (`setBaselineOffset` no es fiable sobre objetos: se descartó.)

## Exportación e impresión

- **Formatos**: PDF (`QPrinter`), HTML (`toHtml`), **ODF (.odt)**, **LaTeX (.tex)**,
  **DOCX (.docx)** y **EPUB (.epub)** en `mdexport`, más **Imprimir**
  (`QPrintDialog`). Menú *Archivo → Exportar* / *Imprimir* (Ctrl+P).
- **PDF e impresión: dos trampas de imágenes** (ambas en `renderToPrinter`, que es
  el embudo de PDF/imprimir/selección/vista previa, y con test en `tst_exporters`):
  (1) una imagen más ancha que la página salía TRUNCADA — la maqueta recorta en el
  borde en vez de escalar—; `clampImagesToWidth` la encoge antes de maquetar, y el
  tope va en **unidades de formato** (`maxWidth/dpiScale`), porque la maqueta
  multiplica por el factor de dpi también los tamaños explícitos (fijar píxeles de
  dispositivo re-escala otra vez y la imagen sale gigante). (2) `doc->print()`
  clona por dentro y el clon copia los recursos explícitos pero NO la baseUrl ni
  la caché: las imágenes relativas desaparecían de la rama sin números de página;
  `bakeImageResources` fija lo resuelto como recurso explícito antes de imprimir.
- **PDF e impresión: la fuente del chino, el japonés y el coreano.** Aquí el texto se
  pinta con las fuentes del SISTEMA (a diferencia de HTML/DOCX/ODF/EPUB, donde las
  pone quien abre el archivo), y lo que ninguna sepa dibujar **no sale, y no deja
  hueco**: comprobado exportando sin ninguna fuente CJK instalada, del PDF
  desaparecen el encabezado, la prosa, las celdas de la tabla y el comentario del
  bloque de código, y el resto del documento sale como si tal cosa —la misma pérdida
  silenciosa que el LaTeX de antes—. Lo que **no** hace falta es un respaldo de
  familia al exportar: si hay alguna fuente CJK en el sistema, Qt recurre a ella sola
  aunque el documento use una latina, y el PDF sale bien y con esa fuente incrustada.
  Así que el arreglo es avisar: `mdexport::cjkScriptsIn` (pura) dice qué escrituras
  usa el documento —`han`/`kana`/`hangul` por separado, porque tener fuente china no
  salva al hangul de un documento coreano— y `ExportController::reportMissingCjkFont`
  se lo pregunta a `QFontDatabase::families(...)` y avisa **sin bloquear** la
  exportación, como `reportLatexIssues`. Los rangos que definen «esto es escritura
  CJK» viven en un solo sitio (`isScriptChar`, en `exportutil.h`): los comparten esta
  comprobación y el escape de LaTeX, que tienen que estar de acuerdo.
- **Orquestación dirigida por datos.** Los cinco formatos basados en archivo
  (HTML/ODF/LaTeX/DOCX/EPUB) comparten `ExportController::runExport(FileExporter)`:
  un descriptor declara título/filtro/extensión, mensajes, si pide idioma, si usa el
  clon plano (`cloneForExport`) o el documento original (LaTeX, que necesita las
  propiedades de math), y la función `write`. Los textos del descriptor van como
  `QT_TRANSLATE_NOOP("MainWindow", …)` para que `lupdate` los extraiga sin
  traducirlos ahí. PDF/impresión van aparte (usan `QPrinter`, no un *writer*).
- **Idioma del documento**: ODF y LaTeX lo incrustan. Se pregunta al exportar
  (`QInputDialog`), por defecto el `lang`/`language` del front matter › ajuste de la
  app › locale del sistema. Tabla código→{babel, fo:language} en `mdexport`. El
  código de cada fila es la **etiqueta canónica** (`mdlang::canonicalTag`), por eso
  el chino es `zh_CN` y no `zh`: simplificado y tradicional son filas distintas. Y su
  campo babel son **dos opciones**, `provide=*,chinese`: no existe `chinese.ldf`, así
  que `[chinese]{babel}` aborta con «Unknown option» y `provide=*` —lo que el propio
  error sugiere— carga el idioma de su `.ini`.
- **HTML**: el cuerpo lo escribe Qt (`toHtml`), pero `mdexport::toHtmlDocument` le
  añade lo que Qt no pone y el documento sí sabe: `lang` en `<html>` (sin él, ni el
  lector de pantalla ni la separación silábica saben el idioma), `<title>` (sin él
  la pestaña del navegador muestra el nombre del fichero) y, sobre todo, las
  **imágenes embebidas** como `data:` URI. Qt referencia la ruta relativa tal cual
  (`src="imagen.png"`), así que el .html se veía bien donde se exportó y perdía
  TODAS las imágenes al moverlo o enviarlo. Se embeben con los **bytes originales**
  cuando el navegador entiende el formato (png/jpg/gif/webp/svg): reencodear a PNG
  conserva el aspecto pero infla una foto JPEG y convierte un SVG vectorial en un
  mapa de bits. Lo que no se puede cargar (una URL remota) se deja como está. Lo
  mismo vale para *Copiar como HTML*, que si no pegaba el texto sin las imágenes.
- **ODF**: Qt escribe el `.odt` (`QTextDocumentWriter "ODF"`) pero **no** el idioma;
  se reempaqueta el zip con el **QZip privado de Qt** (`Qt6::GuiPrivate`,
  `private/qzipreader_p.h`/`qzipwriter_p.h`) para añadir `styles.xml` (con
  `fo:language`) y `meta.xml` (`dc:language`/`dc:title`). API privada de Qt: revisar
  al actualizar Qt.
- **LaTeX**: serializador propio (`mdexport::toLatex`). Preámbulo portable con
  `iftex` (pdfLaTeX usa inputenc/T1; Lua/XeLaTeX usan fontspec) + `babel`. La
  matemática Unicode escrita **literalmente en la prosa** (subíndices `₁ₙ`,
  superíndices `ⁿ`, griego `φΣ`, operadores `⊕∈≥`, `…`, conjuntos `ℝ`, alfabetos
  `𝒞`) rompía la compilación de pdfLaTeX (que con inputenc+T1 no compone esos
  glifos): `mdmath::unicodeToLatex` la traduce a modo matemático (`₁`→`$_{1}$`,
  `φ`→`$\varphi$`, `𝒞`→`$\mathcal{C}$`, `…`→`$\ldots$`) reutilizando las tablas
  TeX→Unicode del parser **invertidas** (`toSuperscript`/`toSubscript`/
  `singleCharCommands`), con las preferencias de alias donde varias órdenes comparten
  glifo; el latín-1 y la puntuación corriente (`× · — …`) se dejan pasar (T1 sí los
  compone). El resto de símbolos/emoji ≥ U+2190 se mapean con la tabla suelta del
  exportador o se omiten.
  **Un `.tex` que no compila no sirve de nada**, y esa es la clase de fallo que
  domina aquí: se descubren exportando y pasando `pdflatex`, no leyendo el código.
  Los que ya costaron un documento entero, todos con su caso en `tst_exporters`:
  - Los bloques de código van en **`alltt`, no en `verbatim`**: verbatim es
    interrumpible, así que bastaba que el código contuviera la línea
    `\end{verbatim}` —cualquier documento que hable de LaTeX— para cerrar el
    entorno ahí y tumbar la compilación entera. En alltt solo `\ { }` conservan su
    significado y `codeBlockSanitize` los escapa: el resto es literal pase lo que pase.
  - **Listas y citas comparten un tope de anidamiento** (`kMaxNesting`): LaTeX
    cuenta `quote` como entorno de lista y aborta con «Too deeply nested» pasados
    cuatro. Se limita la profundidad; se pierde la sangría de los niveles de más,
    nunca el contenido.
  - Un `\item` cuyo contenido empieza por `[` (`- [1] Knuth`, una bibliografía) se
    lo come LaTeX como argumento opcional: se antepone un grupo vacío.
  - El destino de `\href` va por **`latexUrl`, no por `latexEscape`**: el
    `~` → `\textasciitilde{}` de este último dejaba las llaves DENTRO de la URL
    (`http://e.com/~{}manuel`). `latexUrl` deja `~` literal, pone barra a
    `# % & _ $` y codifica en porcentaje lo que no es válido en una URL.
  - Se conservan cosas que el documento sí sabe y el serializador tiraba: el
    arranque de una lista numerada (`5.` → `\setcounter{enumi}{4}`), el nivel de
    las citas anidadas, y que una lista o un bloque de código **dentro** de una
    cita siguen dentro de ella (`BlockQuoteLevel` los marca).
  - **Una escritura no es un símbolo suelto.** El descarte de todo lo que pasa de
    `kHighSymbolStart` (0x2190) está bien para un emoji, pero los ideogramas chinos
    empiezan en U+4E00: un documento en chino, japonés o coreano exportaba **a un
    `.tex` sin su texto, y en silencio**. `isScriptChar` separa ambos casos: lo que
    es escritura (Han, kana, hangul, bopomofo, su puntuación y las formas de ancho
    completo) **se emite siempre**; lo que es símbolo se sigue descartando, pero
    ahora **se cuenta**. Las dos cosas salen por `LatexIssues` (parámetro de salida
    de `toLatex`), y quien traduce y avisa es `ExportController::reportLatexIssues`
    —el módulo es puro y no traduce—. **No se bloquea la exportación**: el chino en
    LaTeX funciona, solo que con otro motor, y bloquear castigaría a quien cita tres
    ideogramas en un documento en español. Con escritura presente, el preámbulo
    cambia: `ctex` en la rama Unicode de `iftex` (elige fuente del sistema él solo,
    y va **antes** que `babel` para que los rótulos los siga fijando el idioma del
    documento) y un `\errmessage` en la de pdfTeX, que no puede componerlas de
    ninguna manera —un fallo con motivo es mejor que el «Unicode character not set
    up for use with LaTeX»—. Ese `.tex` **ya solo compila con `xelatex`/`lualatex`**;
    lo dicen un comentario en su cabecera y el aviso al exportar.
  *Limitación:* una tabla dentro de un elemento de lista sale fuera de la lista,
  porque Qt tampoco la anida en el documento: no hay información que usar.
  **Imágenes: export autocontenido.** `toLatex` recibe la ruta del `.tex` y **trae
  toda imagen junto a él** (`<stem>-imgN.ext`), de modo que el `.tex` compila esté
  donde esté (no depende de exportarse junto al `.md`). Las de formato incluible por
  pdfLaTeX (pdf/png/jpg/jpeg) se **copian byte a byte** (conservan formato y calidad);
  las que no soporta (`.svg`, `.gif`, `.bmp`, `.tiff`…) —o una incluible ilegible/
  remota— se **rasterizan a PNG** vía `doc->resource()` (que resuelve la ruta relativa
  por `baseUrl` y rasteriza el SVG con el mismo plugin `qsvg` que ya lo muestra en el
  editor). Se referencia el fichero traído (ruta escapada). Si no se puede traer, un
  marcador `\texttt{[imagen: …]}` inocuo (nunca un `%`, que comentaría la línea). Sin
  ruta de salida (tests) se mantiene la conducta previa: referencia directa de las
  incluibles, marcador para el resto.
- **DOCX**: serializador OOXML propio (`mdexport::toDocxDocumentXml`) empaquetado
  con el QZip privado; idioma/título incrustados, imágenes embebidas. Tres
  detalles que no se pueden relajar (los tres nacieron de un fallo real, ver
  `tst_docx`): (1) `xmlEsc` **descarta** los caracteres que XML 1.0 no admite
  (control C0, U+FFFE/U+FFFF, suplentes sueltos) —uno solo deja el
  `word/document.xml` mal formado y Word rechaza el paquete entero—; (2) cada
  `<w:gridCol>` lleva su `w:w`: sin anchura hay consumidores (Pandoc) que leen la
  tabla como si no tuviera columnas y **pierden todas las celdas**; (3) los
  enlaces van por **relación** (`<w:hyperlink r:id>` + `TargetMode="External"`,
  como Word), no por campo `w:fldSimple HYPERLINK`, que otros consumidores
  descartan entero, rótulo visible incluido. Las imágenes y los enlaces comparten
  el contador de rId, que empieza en 3 (1 = styles.xml, 2 = numbering.xml).
- **EPUB**: `mdexport::writeEpub` arma un EPUB 3 (`mimetype` sin comprimir primero,
  `META-INF/container.xml`, OPF, `nav.xhtml`, `toc.ncx`, CSS, un XHTML) con el QZip
  privado. **Reutiliza el HTML de Qt** (`toHtml`) como cuerpo, saneado a XHTML con
  `htmlBodyToXhtml` (`&nbsp;`→`&#160;`, elementos vacíos cerrados). Las piezas XML
  son funciones puras (`epubContentOpf`, `epubNavXhtml`, etc.). Tres cosas que hay
  que mantener, las tres nacidas de un libro que salía mal:
  - **El índice se arma con los encabezados** (`mdoutline::headingsOf`), anidado por
    niveles, y `epubAnchorHeadings` pone un `id` a cada `<hN>` del cuerpo para que
    los enlaces salten a alguna parte. Antes el libro llegaba al lector con una sola
    entrada, sin manera de ir a un capítulo. Ojo con el anidamiento: un `<ol>` sin
    `<li>` dentro no es válido y **un nav mal formado invalida el libro entero**,
    así que `epubNavList` lleva una pila de niveles (un salto de h1 a h3 abre UN
    nivel; un h1 tras un h3 no sube por encima de la raíz).
  - Las **casillas de tarea** las marca Qt con `li.unchecked`/`li.checked` y deja la
    regla que las pinta en el `<style>` de su `<head>`… que es justo lo que
    `htmlBodyToXhtml` descarta. `epubStyleCss` las redefine con `::before` (mejor
    soportado que el `::marker` de Qt); sin eso, hecha y pendiente son dos viñetas
    iguales.
  - Las **imágenes** conservan sus bytes originales cuando el formato es de los que
    entienden los lectores (png/jpg/gif/svg) y solo se rasterizan si no; el
    `media-type` del manifiesto se deriva de la extensión, no se da por hecho que
    todo es PNG. Reencodearlo todo inflaba las fotos y convertía los SVG en mapas de
    bits.

**Importación de otros formatos (`mdimport`).** *Archivo → Importar → Otros formatos*
convierte DOCX/ODT/RTF/LaTeX/reST… ejecutando **Pandoc** por `QProcess` (mismo
enfoque que los diagramas: sin dependencia enlazada, con degradación elegante si
falta). La lógica pura vive en `pandocimport`; el proceso, en
`MainWindow::importWithPandoc`. Lo que hace usable el resultado —cada pieza nació de
una pérdida real de contenido, toda ella cubierta por `tst_docx` y `tst_pandocimport`:

- **Metadatos**: `--standalone`. Sin él Pandoc parsea el título del documento (el
  estilo «Title» de Word) y lo tira, porque solo emite el cuerpo. Con él sale como
  front matter YAML, que es justo lo que el editor conserva verbatim (`DocumentIo`)
  y de donde la exportación lee `title`/`lang`. Un documento sin metadatos no gana
  ningún bloque, y uno vacío sigue produciendo salida vacía (de lo que depende el
  aviso «no produjo ningún contenido»).
- **Imágenes**: `--extract-media`. Van dentro del paquete, así que sin extraerlas
  Pandoc emite una ruta que no existe fuera de él. Se sacan a `<nombre>-media`,
  junto al documento de origen (`mediaDirFor`; si esa carpeta no es escribible se
  cae a `AppLocalDataLocation` en vez de abortar la importación), y el Markdown las
  referencia por ruta **absoluta**: el documento importado aún no tiene ubicación en
  disco, así que una relativa no resolvería.
- **`repairImages`**: convierte a `![alt](ruta)` el `<img>` crudo que Pandoc emite
  cuando la imagen lleva tamaño (GFM no lo expresa) y rellena el texto alternativo
  vacío. Ambas cosas las arregla `mdrender::imageMarkdown`, que es donde vive esa
  regla porque **también** la necesita *Insertar → Imagen*: `setMarkdown` **descarta**
  `![](ruta)` sin insertar nada —ni la imagen ni un hueco—, así que un rótulo vacío
  hace desaparecer la imagen al reabrir el documento.
- **`htmlTablesToMarkdown`**: Pandoc cae a HTML crudo cuando GFM no expresa la
  tabla (celdas combinadas, celdas con varios párrafos, tablas anidadas), y con
  `MarkdownNoHTML` eso se vería como texto literal. Se convierte a tabla de tuberías
  aplanando lo que Markdown no tiene (los bloques de una celda quedan en una línea,
  el `colspan` se reparte en celdas vacías), conservando el formato en línea. Los
  bloques de código se saltan (un ` ``` ` con un `<table>` dentro es texto del
  usuario) y, si el fragmento no se puede parsear, se deja intacto.

## Empaquetado multiplataforma

El código es Qt6 puro y portable (sin `#ifdef Q_OS_*` ni APIs POSIX). En
`CMakeLists.txt`: `add_executable(... WIN32 MACOSX_BUNDLE ...)` (cada flag lo ignora
la plataforma que no toca, Linux igual que antes), reglas `install`, despliegue de
Qt (`qt_generate_deploy_app_script` → windeployqt/macdeployqt) solo en Win/macOS, e
**iconos de SO**: `src/icons/md-editor.ico` (Windows, vía recurso `md-editor.rc`) y
`md-editor.icns` (macOS, copiado al bundle + `MACOSX_BUNDLE_ICON_FILE`). El icono de
ventana en runtime ya lo fija `main.cpp` con `app.setWindowIcon`.

**Corrector: motor estático.** Hunspell se enlaza **estático** por defecto
(`SPELL_CHECK_STATIC`, busca el `.a`/`.lib` forzando el sufijo): el motor viaja
DENTRO del ejecutable, así que el paquete no depende de ninguna `.so/.dll/.dylib`
de Hunspell (`ldd` no lo muestra). Esto es lo que cierra el empaquetado en
Windows/macOS, donde `windeployqt`/`macdeployqt` solo despliegan Qt: al ir
estático **no hay biblioteca de terceros que desplegar**. Para esas builds se
necesita un Hunspell con su `.a`/`.lib` estático: Homebrew (`brew install
hunspell`) o vcpkg (`hunspell:x64-windows-static`). Con `-DSPELL_CHECK_STATIC=OFF`
vuelve al enlace dinámico.

**El motor tiene que ir en el paquete, y eso no se ve.** Hasta la 2.8.3 inclusive,
los tres binarios descargables salieron **sin corrector**: `release.yml` no
instalaba Hunspell en ningún sistema, CMake dejaba `HAVE_HUNSPELL` sin definir y
`SpellChecker` compilaba como stub inerte. Nada lo avisaba —«no encontrado» era un
simple `message(STATUS)`— y los tests del corrector pasan igual (se saltan solos
sin motor). Dos defensas contra que se repita: los jobs de release configuran con
**`SPELL_CHECK_REQUIRED=ON`**, que convierte «Hunspell no encontrado» en error de
configuración, y la detección tiene **respaldo sin `pkg-config`**
(`find_path`/`find_library`), que es lo que dejaba fuera a Windows. Cómo se obtiene
el motor en cada job: `libhunspell-dev` (Linux), vcpkg
`hunspell:x64-windows-static-md` —estático pero con CRT dinámico, como Qt— y, en
macOS, **compilado de fuente universal** (`-arch arm64 -arch x86_64`): `brew
install hunspell` solo da la arquitectura del runner y la mitad x86_64 no
enlazaría.

**Diccionarios del corrector.** Linux usa los del sistema (`/usr/share/hunspell`) y
**no los empaqueta** (duplicarían 5 MB de lo que el escritorio ya suele traer);
Windows y macOS no tienen ninguno, así que viajan los **9 idiomas de la interfaz**
dentro del paquete. No están en el repositorio (`.gitignore`: son de terceros,
pesan 24 MB sin comprimir y cambian por su cuenta): los trae al empaquetar
`scripts/fetch-dictionaries.sh <destino>` desde el repositorio de diccionarios de
LibreOffice, con las licencias de cada idioma en `licenses/<idioma>/` y un
`THIRDPARTY-DICTIONARIES.txt`. Ojo con los nombres: el destino se normaliza a
`<idioma>_<REGIÓN>` porque upstream el alemán es `de_DE_frami` y el francés
`fr_FR/dictionaries/fr`, y con esos nombres `mdspell::pickDictionary` no los
encontraría. El script no usa `python`/`jq` a propósito: corre también en el runner
de Windows (Git Bash). Sigue existiendo `scripts/bundle-dictionaries.sh`, que copia
los del sistema para una build local de Win/Mac, y el bloque `install` condicional
de `dictionaries/` para quien prefiera esa vía.

## Internacionalización (importante y con trampas)

Todos los textos visibles pasan por `tr()`. El idioma de origen es el **español**;
hay `.ts` para en, de, fr, it, pt, pl, nl, ro en `translations/`. El test
`tst_translations` (script `tests/check-translations.sh`) **falla** si algún `.ts`
tiene cadenas sin traducir (`type="unfinished"`) o se desincroniza del código.

Flujo al añadir/cambiar texto con `tr()`:

```bash
cmake --build build --target update_translations   # lupdate: refresca los .ts
# traducir las nuevas entradas en los 8 .ts objetivo, luego recompilar
```

- **`md-editor_es.ts` es parcial a propósito** (solo las formas de plural, que el
  texto de origen no puede expresar). En `CMakeLists.txt` se pasa como
  `PLURALS_TS_FILE`, no en `TS_FILES`, para que `update_translations` lo mantenga
  filtrado a `-pluralonly` y no lo infle con todas las cadenas.
- **Atajos en tooltips**: no se traducen a mano; se derivan del propio atajo con
  `QKeySequence::NativeText` para que se localicen solos y no se desincronicen.
- **Plurales**: usar `%n` (`tr(...)`/numerus). Polaco y rumano necesitan **3**
  formas; el resto, 2.
- Al editar `.ts` a mano, respetar exactamente la estructura
  `<source>…</source>` + `<translation>…</translation>` y no borrar el `<source>`.
- Cadenas que **no** deben traducirse: nombres de fichero generados (p. ej.
  `imagen-<fecha>.png`) usan `QStringLiteral`, no `tr()`. El test `tst_translations`
  solo corre con `bash`, así que en Windows ese test se omite (guard en CMake).
- **Etiqueta canónica de idioma (`mdlang::canonicalTag`).** Los códigos llegan de
  sitios muy distintos y escritos de todas las maneras —el ajuste del menú, el
  locale del sistema, el `lang:` del front matter de un documento ajeno (`es`,
  `es-ES`, `es_ES`, `zh-Hans`)— y todos tienen que acabar en el **mismo nombre** para
  encontrar el mismo recurso. Antes cada consumidor normalizaba por su cuenta
  (`code.left(2)`, `section('_', 0, 0)`, un `switch` sobre `QLocale::Language`), y
  las tres formas colapsan el chino simplificado y el tradicional en un solo `zh`
  aunque sean **dos recursos distintos** (manual y traducción propios). El módulo
  puro `langtag` lo centraliza: recorta la región (`es_ES` → `es`) y decide el chino
  por su **sistema de escritura**, no por el territorio —`zh_SG` es simplificado y
  `zh_HK` tradicional, y ninguno se llama `CN` ni `TW`—, así que `zh`/`zh_CN`/
  `zh-Hans`/`zh_SG` → `zh_CN` y `zh-Hant`/`zh_TW`/`zh_HK` → `zh_TW`. **Solo el chino
  se consulta a `QLocale`**: un código que Qt no reconoce devuelve el locale «C», y
  de ahí saldría un idioma que nadie pidió, así que para el resto vale el prefijo de
  la cadena. Lo usan `mdhelp::helpSuffixForLanguage` y `mdexport::languageForCode`.
  No lo usa `mdspell::pickDictionary`, y es deliberado: ahí el código no nombra un
  recurso del programa sino el **archivo de diccionario** que se busca en disco, donde
  colapsar la región (`es_MX` → `es`) es lo correcto.
- **Qué manual abre cada idioma** (`mdhelp::helpSuffixForLanguage`,
  `helpdialog.cpp`): el sufijo se compone con la etiqueta canónica (`de` → `_de`; el
  español es la base, sin sufijo) y **se comprueba que el recurso existe**; si no,
  cae al inglés. Así un idioma de la interfaz cuyo manual aún no esté traducido no
  abre un visor en blanco, y **estrenar un manual es soltar sus dos `.md` en el
  `.qrc`**, sin lista de `case` que ampliar. Lo vigila
  `suffixPicksTheManualOfTheLanguage` en `tst_help`, que además exige que el sufijo
  devuelto lleve a un archivo legible.
