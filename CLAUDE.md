# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Qué es

Editor/visor **WYSIWYG** de Markdown en **Qt6 + C++17**. Por defecto se edita sobre
el texto ya renderizado, sin ver la sintaxis; pero el código Markdown es visible
opcionalmente (vista de fuente a pantalla completa o vista dividida con render y
código en paralelo, ver «Modo fuente y vista dividida» abajo). Al guardar se
serializa con `QTextDocument::toMarkdown()` (con un retoque para las tablas, ver
abajo). La interfaz y todos los textos están en español (idioma de origen) con
traducciones a 8 idiomas más.

## Comandos

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

# Instalar (compila build-min/ a tamaño mínimo y copia a $PREFIX, requiere sudo).
# Instala binario + .desktop + iconos hicolor (PNG/SVG) en Linux.
sudo ./install.sh                              # -> /usr/local
PREFIX="$HOME/.local" ./install.sh             # de usuario, sin sudo
```

Tras tocar el código siempre `cmake --build build && ctest --test-dir build`. El
binario instalado (`/usr/local/bin/md-editor`) **no** se actualiza al recompilar
`build/`: hay que reinstalar para probar cambios en el ejecutable real del usuario.

## Arquitectura

Toda la lógica vive en una **biblioteca estática `md-editor-core`** que enlazan
tanto el ejecutable (`main.cpp`, solo arranque + i18n) como las pruebas. Añadir un
`.cpp/.h` nuevo = añadirlo a la lista de `md-editor-core` en `CMakeLists.txt`.

`MainWindow` es el orquestador y delega en **colaboradores autocontenidos**, cada
uno una clase pequeña con su propia responsabilidad. Tras el refactor de
arquitectura las acciones de usuario viven en controladores temáticos (la mayoría
miembros de `MainWindow`, declarados en `mainwindow.h`):

- **Entrada/salida y sesión**: `DocumentIo` (abrir/guardar, UTF-8, baseUrl, front
  matter, estado «modificado»), `FileController` (nuevo/abrir/guardar/recuperar +
  autoguardado), `RecoveryManager` (borrador de autoguardado), `RecentFilesManager`,
  `DiskWatcher` (vigila cambios externos del archivo).
- **Vista y apariencia**: `SplitViewController` (los tres modos de vista
  WYSIWYG/fuente/dividida y su sincronización — antes en `MainWindow`),
  `DistractionFreeController` (pantalla completa + columna), `ThemeController`
  (tema, luz cálida nocturna y recoloreado de enlaces), `ThemeSpec`/`mdtheme`
  (catálogo declarativo de los 6 temas), `ChromeZoom` (zoom de toda la interfaz),
  `OutlinePanel` (índice TOC), `FindReplaceBar`.
- **Edición e inserción**: `FormatController` (marcas de carácter, encabezados,
  listas, sangrías + estado de acciones), `InsertController` (enlaces, imágenes,
  tablas, regla), `TableController` (edición contextual de tablas),
  `FormulaController` (fórmulas TeX: insertar/editar/proteger), `BlockConstructs`
  (citas y bloques de código), `CodeBlockHighlighter` + `LanguageRegistry`
  (resaltado), `FocusEditor` (QTextEdit con columna centrada para el modo sin
  distracciones y un *handler* de pegado/soltado, `setMimeInsertHandler`).
- **Exportación**: `ExportController` + `mdexport` (`exporters`).
- **Persistencia**: `AppSettings` (fachada tipada sobre `QSettings`; **todas** las
  claves de persistencia viven aquí, nadie más toca `QSettings`).

Módulos de lógica pura (sin clase, solo funciones en un namespace):
`listcontinuation` (`mdlist`), `tableedit` (`mdtable`), `exporters` (`mdexport`).

Patrón recurrente para lógica comprobable: separar las **funciones puras** (sin
GUI) de la integración, en un namespace, para testearlas aisladas — `mdblock`
(`blockconstructs`), `mdoutline::headingsOf` (`outlinepanel`), `mdlist::analyze`
(`listcontinuation`), `mdtable::injectAlignments` (`tableedit`), `mdexport::toLatex`
(`exporters`). Sigue ese patrón al añadir lógica nueva: hay un `tst_*` por módulo.

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
  otra ruta de serialización, usa `mdtable::documentMarkdown()`.
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
- **Sincronización de la vista dividida.** Regla: *solo se actualiza el panel SIN
  foco*, nunca el que el usuario está editando (evita saltos de cursor y que se le
  reescriba el texto). Dos `QTimer` de debounce (~250 ms): `m_syncToSourceTimer`
  (WYSIWYG→fuente, `syncSourceFromDocument`) y `m_syncToDocTimer` (fuente→WYSIWYG,
  `syncDocumentFromSource`/`commitSourceToDocument`). El flag `m_syncing` envuelve
  toda actualización programática para que los `contentsChanged`/`textChanged` que
  provoca no realimenten el bucle. `flushPendingSync()` (en `focusChanged`) vacía
  el timer pendiente al cambiar de panel para que el destino llegue al día. Se
  preserva el scroll del panel refrescado.
- **«Modificado».** `DocumentIo::isModified()` compara la serialización canónica
  con una línea base, no usa `QTextDocument::isModified()` (que `QTextEdit` ensucia
  de forma espuria al trazar la primera vez).
- **Arranque de sesión.** `main.cpp` difiere con `QTimer::singleShot(0, ...)` la
  llamada a `MainWindow::startSession()` (abrir en mitad del trazado inicial de
  `QTextEdit` provoca un diálogo espurio). Prioridad: archivo de línea de comandos
  › recuperar borrador › reabrir último documento. `lastFile` solo se persiste con
  rutas **no vacías** (el documento nuevo inicial no debe pisarlo).
- **Zoom de toda la interfaz.** `applyChromeZoom()` escala, partiendo de tamaños
  base, no solo el editor: menú **y cada `QMenu`** (los desplegables no heredan la
  fuente de la barra), barras, estado, fuente, panel de esquema e iconos de la
  barra de formato (`updateToolBarIcons`).
- **`eventFilter` de `MainWindow`.** Centraliza: zoom con Ctrl+rueda y abrir
  enlaces (Ctrl+clic, hover) sobre `m_editor->viewport()`; arrastrar-soltar un
  archivo para abrirlo; y **continuación de listas** con Enter en `m_sourceEditor`
  (en WYSIWYG la hace `QTextEdit` de serie; en el editor de fuente la añade
  `mdlist::analyze`).
- **Pegar/soltar imágenes.** El *handler* de `FocusEditor` desvía las imágenes del
  portapapeles a disco (PNG junto al `.md`, ruta relativa) e inserta `![](ruta)`,
  en vez de incrustarlas (que no round-trip-ean). También en *Insertar → Pegar
  imagen*; pregunta el texto alternativo.
- **Vigilancia del archivo en disco.** `QFileSystemWatcher` sobre el archivo
  abierto, con debounce (`QTimer`) e instantánea de bytes para distinguir el propio
  guardado de un cambio externo: si no hay cambios locales recarga solo; si los
  hay, pregunta.

- **Fórmulas TeX (`mdmath`).** El editor soporta `$...$` y `$$...$$` sin
  dependencias externas. El módulo `mdmath` (`mathblocks.{h,cpp}`) lo orquesta
  todo y es **puro** (lo prueban `tst_mathblocks`). Piezas clave:

  - *Carga.* `DocumentIo::load` aplica `mdmath::protectMath` al texto fuente
    antes de `setMarkdown`: envuelve cada `$tex$`/`$$tex$$` en inline-code
    ``` ``$tex$`` ``` para que Qt no reinterprete `_`/`*`/`\` dentro como
    cursiva o escape. Después llama a `mdmath::renderMathInDocument`, que
    sustituye cada inline-code con forma `$tex$` por una **secuencia de
    fragmentos** del `QTextDocument`: cursiva + super/subíndice real de Qt
    (`QTextCharFormat::AlignSuperScript`/`AlignSubScript`, no solo el
    repertorio Unicode de scripts). Todos los fragmentos de una misma fórmula
    comparten tres propiedades custom — `IsMathProperty`, `MathTexProperty`,
    `MathBlockProperty` — que permiten reconocerlos como grupo.

  - *Render TeX → runs.* `mdmath::renderTexAsRuns(tex, baseFmt)` es el parser
    que produce esa lista de `MathRun = {QString text, QTextCharFormat fmt}`.
    Maneja: letras griegas y operadores (tabla `singleCharCommands`), `^`/`_`
    con argumento de carácter / grupo / comando (`^\infty` → run `∞` con
    AlignSuperScript), `\frac{a}{b}` (fraction slash `⁄` si num y den son de un
    solo carácter; si no, `(num)/(den)` con num y den como sub-runs),
    `\sqrt{x}`, `\mathbb{R}`. `texToUnicode` es solo un thin-flatten encima
    para los exports que no llevan formato rico.

  - *Edición.* `Insertar → Fórmula…` (Ctrl+Shift+F) abre un diálogo con
    previsualización en vivo (`texToUnicode` sobre el TeX según se teclea) e
    inserta los runs en el cursor. Doble clic sobre una fórmula reabre el
    mismo diálogo precargado y la sustituye. Las fórmulas son **atómicas**
    frente al teclado: `MainWindow::handleMathKeyPress` (instalado como
    `eventFilter` en `m_editor`) descarta caracteres imprimibles dentro del
    grupo y convierte Backspace/Delete en el borde en borrado del grupo entero.

  - *Serialización fiel.* `mdtable::documentMarkdown` clona el documento,
    reemplaza cada grupo de fórmula por una **sentinela** en la PUA de
    Unicode (`U+F8FE…U+F8FF` envolviendo el índice en la tabla
    `MathSentinelTable`) — texto opaco que `QTextDocument::toMarkdown()` no
    escapa, a diferencia del `\` dentro de inline-code — y reinyecta
    `$tex$`/`$$tex$$` con `restoreMathFromSentinels`. Resultado: los `\sum`,
    `\frac`, `_`, `*` del TeX sobreviven íntegros al round-trip. La función
    `unprotectMath` sigue existiendo pero no se usa en producción (queda como
    inversa explícita de `protectMath` para los tests).

  - *Resaltado.* El color de las fórmulas vive en `SyntaxColors::math` y lo
    aplica `CodeBlockHighlighter::highlightMathFragments` recorriendo los
    fragmentos del bloque actual con `IsMathProperty` y haciendo
    `setFormat(...)` solo con el foreground (la cursiva y vertical-align del
    fragmento se conservan). Se actualiza al cambiar de tema porque
    `setSyntaxColors` ya invalida y vuelve a aplicar el resaltado.

  - *Exportación.*
    - **LaTeX**: `inlineLatex` detecta los fragmentos por `IsMathProperty`,
      agrupa los consecutivos con el mismo `MathTex` (los runs de super/sub)
      y emite **una** `$tex$`/`$$tex$$` por grupo a partir de la propiedad,
      no del texto visible. Preámbulo añade `amsmath` + `amssymb`.
    - **HTML/PDF/ODF**: pasan por `mdexport::cloneForExport`, que **clona el
      documento tal cual** y solo limpia las propiedades custom de math. Así
      Qt serializa el vertical-align de los super/subíndices a CSS (`toHtml`),
      al atributo equivalente del ODF y los pinta directamente en PDF. No se
      aplana a Unicode plano.

  - *Limitaciones conocidas.* (1) `$$...$$` que cruza varias líneas en la
    fuente no se detecta (`findMath` trabaja línea a línea). (2) No hay
    layout 2D: las fracciones grandes son `(a)/(b)`, los `\sum` con límites
    los muestran con AlignSuperScript/SubScript a la derecha, no encima y
    debajo (eso requeriría un `QTextObjectInterface` propio — Nivel 2).

## Exportación e impresión

- **Formatos**: PDF (`QPrinter`), HTML (`toHtml`), **ODF (.odt)** y **LaTeX (.tex)**
  en `mdexport`, más **Imprimir** (`QPrintDialog`). Menú *Archivo → Exportar* /
  *Imprimir* (Ctrl+P).
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

## Empaquetado multiplataforma

El código es Qt6 puro y portable (sin `#ifdef Q_OS_*` ni APIs POSIX). En
`CMakeLists.txt`: `add_executable(... WIN32 MACOSX_BUNDLE ...)` (cada flag lo ignora
la plataforma que no toca, Linux igual que antes), reglas `install`, despliegue de
Qt (`qt_generate_deploy_app_script` → windeployqt/macdeployqt) solo en Win/macOS, e
**iconos de SO**: `src/icons/md-editor.ico` (Windows, vía recurso `md-editor.rc`) y
`md-editor.icns` (macOS, copiado al bundle + `MACOSX_BUNDLE_ICON_FILE`). El icono de
ventana en runtime ya lo fija `main.cpp` con `app.setWindowIcon`.

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
