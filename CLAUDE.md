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
`.cpp/.h` nuevo = añadirlo a la lista de `md-editor-core` en `CMakeLists.txt` (y, si
trae lógica pura, su `tst_*` a la lista de tests del mismo archivo).

`MainWindow` es el orquestador y delega en **colaboradores autocontenidos**, cada
uno una clase pequeña con su propia responsabilidad. Tras el refactor de
arquitectura las acciones de usuario viven en controladores temáticos (la mayoría
miembros de `MainWindow`, declarados en `mainwindow.h`):

- **Entrada/salida y sesión**: `DocumentIo` (abrir/guardar, UTF-8, baseUrl, front
  matter, estado «modificado»), `FileController` (nuevo/abrir/guardar/recuperar +
  autoguardado), `RecoveryManager` (borrador de autoguardado), `RecentFilesManager`,
  `DiskWatcher` (vigila cambios externos del archivo).
- **Vista y apariencia**: `SplitViewController` (los tres modos de vista
  WYSIWYG/fuente/dividida y su sincronización), `DistractionFreeController`
  (pantalla completa + columna), `ThemeController` (tema, luz cálida nocturna y
  recoloreado de enlaces), `ThemeSpec`/`mdtheme` (catálogo declarativo de los 6
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
(`mdstats`), `blockconstructs` (`mdblock`), `outlinepanel`
(`mdoutline::headingsOf`), `spellscan` (`mdspell`; tokenización de palabras y
selección de diccionario para el corrector ortográfico, ver abajo), `diagram`
(`mddiagram`; clasifica el lenguaje de un bloque ```mermaid/plantuml para el
render de diagramas, ver abajo).

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
  es además la que reinyecta las fórmulas (ver «Fórmulas TeX») y deshace el escape
  `> \[!NOTE]` de las admoniciones (`mdadmonition::unescapeMarkers`, ver abajo).
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
- **Temas y luz cálida nocturna.** `ThemeController` aplica uno de los 6 temas del
  catálogo declarativo `mdtheme`/`ThemeSpec` (Claro, Oscuro, GitHub Light, GitHub
  Dark, Monokai, Alto contraste), persiste la clave `theme` (con migración del
  antiguo booleano `darkTheme`) y recolorea enlaces + resaltado al cambiar. Sobre
  cualquier tema se superpone, **ortogonalmente**, la **luz cálida nocturna**
  (toggle `warmLight`, **activo por defecto**): un tinte cálido **automático y
  gradual según la hora del reloj** del sistema. `warmthForTime` define la
  intensidad `w∈[0,1]` (día 07–19 → 0; rampa ascendente 19→23; noche 23–06 → 1;
  rampa descendente 06→07) y `applyWarmth` la aplica como filtro multiplicativo solo
  sobre `QPalette::Base`/`AlternateBase` (azul −16 %·w, verde −5 %·w, rojo intacto).
  Un `QTimer` refresca cada 60 s y solo repinta si `w` cambió ≥0.02. No afecta a
  enlaces ni resaltado.
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
  fuente de la barra), barras, estado, fuente, panel de esquema e iconos de la
  barra de formato (`updateToolBarIcons`).
- **`eventFilter` de `MainWindow`.** Es solo un despachador: delega en tres
  sub-manejadores según el objeto vigilado (cada uno devuelve `bool`, el primero
  que consume gana): `handleViewportEvent` (zoom con Ctrl+rueda; abrir enlaces con
  Ctrl+clic/hover; arrastrar-soltar un archivo; clic sobre la casilla de una tarea
  `mdtask` y sobre una referencia de nota al pie `mdfootnote`), `handleEditorKeyPress`
  (protección de fórmulas y shortcodes `:nombre:` en `m_editor`) y
  `handleSourceKeyPress` (**continuación de listas** con Enter en `m_sourceEditor`
  vía `mdlist::analyze`; en WYSIWYG la hace `QTextEdit` de serie).
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
  uno).
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
  selección.
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
  así que la imagen nunca llega al Markdown ni cuenta para «modificado». Avisa en
  la barra de estado solo si hay diagramas y falta la herramienta.
- **Corrección ortográfica (opcional, Hunspell).** Primera dependencia de
  terceros, **opcional** (`SPELL_CHECK`→`HAVE_HUNSPELL` en CMake): sin
  `libhunspell-dev` el build sigue verde. Piezas: `spellscan` (`mdspell`, puro:
  `tokenize` palabras + `pickDictionary`), `SpellChecker` (envuelve Hunspell tras
  un *pimpl*; **siempre se compila**, stub inerte sin soporte; carga perezosa de
  un idioma). El subrayado lo hace el **mismo** `CodeBlockHighlighter`
  (`highlightSpelling`, en la rama no-código, saltando código en línea, fórmulas
  y enlaces) con `SpellCheckUnderline` — presentación pura, no toca el Markdown.
  `MainWindow::applySpellLanguage` elige el diccionario por el idioma del
  documento (front matter › ajuste › locale) en cada `documentLoaded` y al
  arrancar, y rehace el resaltado; la lista personal vive en
  `AppSettings::personalDictionary`.

### Fórmulas TeX (`mdmath`)

El editor soporta `$...$` y `$$...$$` sin dependencias externas. El módulo `mdmath`
lo orquesta todo y es **puro** (lo prueban `tst_mathblocks`). Vive en un header
único `mathblocks.h` pero el `.cpp` está partido en dos: `mathblocks.cpp` (scanning
del Markdown fuente — `findMath`/`protectMath` — e integración con `QTextDocument`)
y `texparser.cpp` (el motor de parseo TeX→runs/Unicode: `renderTexAsRuns`,
`texToUnicode`, `wrapTex` y sus tablas). Mismo namespace `mdmath`; los consumidores
solo incluyen `mathblocks.h`. Piezas clave:

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
  letras griegas y operadores (tabla `singleCharCommands`), `^`/`_` con argumento
  de carácter / grupo / comando, `\frac{a}{b}` (fraction slash `⁄` si num y den son
  de un solo carácter; si no, `(num)/(den)`), `\sqrt{x}`, `\mathbb{R}`.
  `texToUnicode` es solo un thin-flatten encima para los exports sin formato rico.
- *Edición.* `Insertar → Fórmula…` (Ctrl+Shift+F) abre un diálogo con previsuali-
  zación en vivo e inserta los runs en el cursor. Doble clic sobre una fórmula
  reabre el diálogo precargado y la sustituye. Las fórmulas son **atómicas** frente
  al teclado: `MainWindow::handleMathKeyPress` (instalado como `eventFilter` en
  `m_editor`) descarta caracteres imprimibles dentro del grupo y convierte
  Backspace/Delete en el borde en borrado del grupo entero.
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
  que `$\hat{x}$` se queda en línea y exporta solo. `\text{…}`/`\mathrm{…}` emiten
  su argumento literal; `\binom` inline (y en export) se aproxima como `C(n, k)`
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
  un grupo de uno. **HTML/PDF/ODF/DOCX**: pasan por `mdexport::cloneForExport`, que
  clona el documento, limpia las propiedades custom de los runs inline y
  **expande** cada carácter objeto 2D a esos runs inline (cursiva + super/sub),
  dejando que Qt serialice el vertical-align a CSS/ODF/PDF (la maquetación 2D es
  solo de pantalla).
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
  raros, `\overbrace`, layout de límites de integral propios…) se aproxima inline
  y puede verse pobre. El alineado vertical de las fórmulas 2D
  **inline** queda alto: el `QTextObjectInterface` de Qt ancla el objeto por su
  borde inferior al baseline y su API (`intrinsicSize` da un `QSizeF`, sin
  separar ascenso/descenso) no permite descender bajo el baseline, así que no se
  puede centrar sobre el eje. Las de bloque (`$$`, solas en su línea) sí se ven
  bien. (`setBaselineOffset` no es fiable sobre objetos: se descartó.)

## Exportación e impresión

- **Formatos**: PDF (`QPrinter`), HTML (`toHtml`), **ODF (.odt)**, **LaTeX (.tex)**,
  **DOCX (.docx)** y **EPUB (.epub)** en `mdexport`, más **Imprimir**
  (`QPrintDialog`). Menú *Archivo → Exportar* / *Imprimir* (Ctrl+P).
- **Orquestación dirigida por datos.** Los cinco formatos basados en archivo
  (HTML/ODF/LaTeX/DOCX/EPUB) comparten `ExportController::runExport(FileExporter)`:
  un descriptor declara título/filtro/extensión, mensajes, si pide idioma, si usa el
  clon plano (`cloneForExport`) o el documento original (LaTeX, que necesita las
  propiedades de math), y la función `write`. Los textos del descriptor van como
  `QT_TRANSLATE_NOOP("MainWindow", …)` para que `lupdate` los extraiga sin
  traducirlos ahí. PDF/impresión van aparte (usan `QPrinter`, no un *writer*).
- **Idioma del documento**: ODF y LaTeX lo incrustan. Se pregunta al exportar
  (`QInputDialog`), por defecto el `lang`/`language` del front matter › ajuste de la
  app › locale del sistema. Tabla código→{babel, fo:language} en `mdexport`.
- **ODF**: Qt escribe el `.odt` (`QTextDocumentWriter "ODF"`) pero **no** el idioma;
  se reempaqueta el zip con el **QZip privado de Qt** (`Qt6::GuiPrivate`,
  `private/qzipreader_p.h`/`qzipwriter_p.h`) para añadir `styles.xml` (con
  `fo:language`) y `meta.xml` (`dc:language`/`dc:title`). API privada de Qt: revisar
  al actualizar Qt.
- **LaTeX**: serializador propio (`mdexport::toLatex`). Preámbulo portable con
  `iftex` (pdfLaTeX usa inputenc/T1; Lua/XeLaTeX usan fontspec) + `babel`. Los
  caracteres ≥ U+2190 (símbolos/emoji) se mapean a comandos LaTeX o se omiten para
  no romper pdfLaTeX; el código `verbatim` se sanea aparte.
- **DOCX**: serializador OOXML propio (`mdexport::toDocxDocumentXml`) empaquetado
  con el QZip privado; idioma/título incrustados, imágenes embebidas.
- **EPUB**: `mdexport::writeEpub` arma un EPUB 3 (`mimetype` sin comprimir primero,
  `META-INF/container.xml`, OPF, `nav.xhtml`, `toc.ncx`, CSS, un XHTML) con el QZip
  privado. **Reutiliza el HTML de Qt** (`toHtml`) como cuerpo, saneado a XHTML con
  `htmlBodyToXhtml` (`&nbsp;`→`&#160;`, elementos vacíos cerrados); las imágenes se
  recuperan con `doc->resource` y se embeben como PNG, reescribiendo su `src`. Las
  piezas XML son funciones puras (`epubContentOpf`, `epubNavXhtml`, etc.).

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

**Diccionarios del corrector.** Linux usa los del sistema (`/usr/share/hunspell`);
Windows/macOS no tienen, así que se empaquetan. La carpeta `dictionaries/` (con su
`README.md`; los `.aff/.dic` están en `.gitignore` por licencias y tamaño) es el
punto de empaquetado: si tiene ficheros, `CMakeLists.txt` los instala donde
`SpellChecker::searchPaths()` los busca — junto al `.exe` (Windows), en
`Contents/Resources/dictionaries` (macOS), o `<prefix>/share/hunspell` (Linux). El
script `scripts/bundle-dictionaries.sh` copia ahí los del sistema (los 9 idiomas
de la interfaz) para una build de Win/Mac. Es un bloque `install` **condicional**
(vacío = no-op), así que no afecta a la build de Linux.

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
