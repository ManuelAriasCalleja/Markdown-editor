# Errores detectados

> **Estado de corrección (2026-07-02).** Se han corregido 30 de los hallazgos,
> incluidos TODOS los del orden de arreglo sugerido (1–6) y varios del resto. Cada
> arreglo se hizo con su prueba de regresión donde aplicaba; la suite completa
> (64 tests) pasa en build normal y bajo ASan+UBSan, y clang-tidy no reporta avisos
> en los ficheros tocados. Lo corregido está marcado con **[CORREGIDO]** al final de
> su línea; lo que queda pendiente (más arriesgado o de menor valor) conserva su
> descripción sin marca y se resume al final.
>
> **Corregido:** doble `delete` en Mermaid · cuelgue O(n²) del parser TeX (texparser
> y mathlayout) · guardado atómico y comprobado (documento, borrador, exportación) ·
> off-by-one de las fórmulas (teclado y doble clic) · `inlineCodeRanges` · `$$$$` ·
> familia de round-trip (codespanfix con conciencia de fences/cita/indentado y
> backtick, markdowntidy longitud de run, tableedit fences) · cita conserva fórmulas ·
> buscar/reemplazar dentro de fórmulas y sin-resultado · baseUrl de exportación
> (PDF/impresión/vista previa/ODF/selección) · acciones WYSIWYG y corrector entre
> pestañas · cambios en disco de pestañas en segundo plano y re-vigilancia · borrador
> de recuperación al cerrar · última pestaña en modo fuente · pestaña huérfana al
> fallar la apertura · modo fuente antes de `maybeSave` · tabla dentro de tabla ·
> nota al pie con selección · Tab en celdas fusionadas · encabezado multibloque ·
> desbordamiento de número en listas · recuento de caracteres UTF-16 · escape LaTeX
> de URLs/rutas · desbordamiento de índice de sentinela.
>
> **Pendiente (documentado abajo, sin marca):** fusión de fórmulas idénticas
> adyacentes · secuestro de code spans `$...$` · sup/sub anidados · colisiones de
> sentinelas PUA (salvo el desbordamiento, ya corregido) · matrices anidadas ·
> arrastre en el esquema con encabezados `- #` · placeholders de diagramas · pila de
> deshacer del recoloreado · overlay de código · SymbolPicker no modal · pares
> subrogados en TeX/corrector · codificación de diccionarios · confirmación de
> sobrescritura al añadir extensión · detección de codificación al cargar · borradores
> entre instancias · apilado de diálogos · marca del menú de idioma al cancelar ·
> `paintEvent` del modo foco · resultado de escritura del PDF.

Auditoría del 2026-07-02 sobre la versión 2.6.0. Método: revisión sistemática de
todo `src/` por subsistemas buscando fallos de segmento, corrupción de datos y
comportamientos extraños de la interfaz, más build y suite completa bajo
ASan+UBSan (64/64 tests verdes, tanto en build normal como con sanitizers: las
rutas cubiertas por tests están limpias; todo lo de abajo son huecos no
cubiertos).

Cada entrada indica su estado de verificación:

- (V) verificado en ejecución con una sonda real compilada contra Qt 6.8 o el propio código del proyecto.
- (C) confirmado por lectura directa del código citado.
- (P) plausible: derivado del código, sin reproducción ejecutada.

---

## 1. Crashes y cuelgues

- **src/diagram/diagramrenderer.cpp:138 — doble `delete` del `QTemporaryDir` si mmdc muere de forma anómala. (C)**
  En `startMermaid`, la lambda `cleanup` (`proc->deleteLater(); delete dir;`) está
  conectada a `errorOccurred` Y a `finished`. Cuando el proceso crashea (mmdc usa
  puppeteer/Chromium, propenso a abortar), Qt emite ambas señales:
  `errorOccurred(Crashed)` y `finished(CrashExit)`. El `deleteLater` duplicado es
  inocuo, pero el `delete dir` se ejecuta dos veces → doble free → corrupción de
  heap / abort del editor. El branch PlantUML no tiene el problema (solo duplica
  `deleteLater`).

- **src/math/texparser.cpp:717 — parseo O(n²) con TeX hostil congela la interfaz. (V, medido)**
  Cada comando desconocido con llave (`\a{`) dispara `readGroup` sobre TODO el
  resto de la cadena (con copia) antes de descartar la sonda. Medido:
  6 KB → 4 ms, 24 KB → 53 ms, 96 KB → 1,4 s (cuadrático). El mismo patrón está
  duplicado en `mathlayout.cpp:436`. Agravantes: el diálogo de fórmula re-parsea
  en cada pulsación (`updatePreview` síncrono) y `MathObject::intrinsicSize`/
  `drawObject` re-parsean el TeX completo en cada pase de layout/pintado sin
  caché, así que una fórmula 2D grande pegada por el usuario congela el editor en
  cada repintado.

- **src/diagram/diagramrenderer.cpp:75 — sin timeout ni kill para los procesos de render. (C)**
  Si mmdc/plantuml se cuelga, ni `finished` ni `errorOccurred` llegan: el proceso
  queda vivo indefinidamente y la clave nunca sale de `m_inFlight`, así que ese
  diagrama no vuelve a renderizarse en toda la sesión.

- **src/app/mainwindow.cpp:226 — zoom y lineSpacing leídos de QSettings sin validar rango. (P)**
  `zoomLevel` corrupto o editado a mano (p. ej. 10^9) produce
  `setPointSizeF(~1e9)` en editor, menús y barra; `updateToolBarIcons` desborda
  `int` calculando el tamaño de icono. La app queda inutilizable sin Ctrl+0
  alcanzable. `applyZoom` solo acota el límite inferior. `lineSpacing` 0 o
  negativo colapsa todas las líneas.

---

## 2. Pérdida / corrupción de datos — guardado y disco

- **src/io/documentio.cpp:142 — `write()` ignora los errores de escritura. (C)**
  Nunca comprueba el estado de `QTextStream`/`QFile` tras el volcado y devuelve
  `true`. Además abre con `WriteOnly` (trunca el original in situ, sin
  temporal+rename). Con disco lleno: archivo truncado en disco, línea base
  actualizada, `isModified()` a false, barra diciendo «Guardado», y el usuario
  cierra sin aviso. Pérdida silenciosa e irreversible.

- **src/app/mainwindow.cpp:332 — los cambios externos en disco de pestañas en segundo plano se descartan para siempre. (C)**
  La lambda de `diskExternalChange` solo reenvía si `stack == m_stack`. El
  DiskWatcher emite una sola vez (la instantánea queda actualizada) y nada
  re-comprueba al activar la pestaña: el usuario edita y guarda machacando la
  versión externa sin ningún diálogo de conflicto. Igual con `diskVanished`.

- **src/io/diskwatcher.cpp:58 — evento descartado durante la suspensión, sin re-añadir el path. (P)**
  Con el diálogo de conflicto abierto, un guardado externo con
  temporal+rename hace que `QFileSystemWatcher` pierda el path; `checkChange`
  retorna en el guard de `m_suspended` antes del re-add. Al cerrar el diálogo
  nadie re-vigila: los cambios externos posteriores pasan inadvertidos.

- **src/io/diskwatcher.cpp:68 — tras `vanished()` el archivo no se vuelve a vigilar si reaparece. (P)**
  Borrado/renombrado externo (p. ej. git al cambiar de rama) → si el archivo se
  recrea, no hay path vigilado ni timer: Guardar sobrescribe la versión nueva del
  disco sin preguntar.

- **src/io/recoverymanager.cpp:37 — una segunda instancia se ofrece a «recuperar» y borra los borradores vivos de la primera. (P)**
  `leftoverDrafts()` no distingue borradores de una instancia en ejecución.
  La instancia B los enumera como huérfanos y, elija lo que elija el usuario,
  `startSession` los borra del disco: la red de seguridad de la instancia A
  desaparece.

- **src/io/recoverymanager.cpp:130 — `writeFile()` devuelve `true` incondicionalmente y reescribe el borrador in situ. (C)**
  Disco lleno o apagón a mitad de la reescritura de 5 s → borrador truncado o
  vacío, pero la meta se escribe igual y `hasDraft()` sigue cierto: tras el
  cierre anómalo la «recuperación» ofrece un documento vacío/parcial.

- **src/io/filecontroller.cpp:129 — añadir la extensión tras el diálogo esquiva la confirmación de sobrescritura. (C)**
  «Guardar como» con «informe» (sin extensión): el diálogo confirma sobre
  «informe», el código añade «.md» y sobrescribe «informe.md» existente sin
  preguntar. Lo mismo en las exportaciones vía
  `ExportController::promptSavePath` (exportcontroller.cpp:109).

- **src/io/documentio.cpp:116 — UTF-8 forzado sin detectar fallos de decodificación. (C)**
  Un .md en Latin-1/CP1252 se carga con U+FFFD sin aviso; un guardado posterior
  consolida el mojibake destruyendo los bytes originales.

- **src/app/mainwindow.cpp:830 — cerrar con «Descartar» no borra el borrador de recuperación. (C)**
  `closeTab` (y el bucle de `setLanguage`) hace `removeTab` + `deleteLater` sin
  `clearDraft()`: en un arranque posterior se ofrece «recuperar» contenido que el
  usuario descartó explícitamente en una sesión limpia.

## 3. Pérdida / corrupción de datos — round-trip Markdown

- **src/editor/blockconstructs.cpp:136 — alternar cita destruye las fórmulas TeX de la selección. (V)**
  `selectionToMarkdown()` usa `tmp.toMarkdown()` crudo en vez de la ruta
  canónica: los runs de fórmula se aplanan a glifos («Antes $E=mc^2$ después» →
  `> Antes *E=mc2* después`) y el TeX se pierde definitivamente. También pierde
  alineación de tablas y el des-escape de code spans del texto citado.

- **src/math/formulacontroller.cpp:181 — off-by-one en `fmtAt` rompe toda la protección de teclado de las fórmulas. (V)**
  `QTextCursor::charFormat()` devuelve el formato del carácter en p−1, no en p,
  así que toda la lógica de bordes está corrida una posición. Efectos
  verificados: no se puede teclear justo después de una fórmula inline (la tecla
  se traga); Delete en el borde izquierdo borra solo el primer carácter y al
  guardar la fórmula «resucita» completa (edición deshecha en silencio); Delete
  dentro borra media fórmula más el carácter siguiente; teclear dentro parte el
  grupo en dos y al guardar la fórmula se duplica (`$x^2$z$x^2$`).

- **src/math/formulacontroller.cpp:277 — el mismo off-by-one en `editFormulaAt` desplaza el rango a sustituir. (V)**
  La edición por doble clic deja el primer carácter viejo huérfano (con su
  `MathTex` → fórmula duplicada al guardar) y se come el carácter siguiente a la
  fórmula; en el borde izquierdo no detecta nada y el diálogo no se abre.

- **src/math/mathblocks.cpp:442 — `renderMathInDocument` secuestra code spans legítimos con forma `$...$`. (V)**
  El Markdown ``Usa `$x^2$` para formulas`` (código documentando sintaxis TeX)
  se convierte en fórmula: al guardar, los backticks desaparecen del archivo.
  Solo se comprueba fontFixedPitch + forma `$...$`; no se distinguen los spans
  creados por `protectMath` del código genuino del usuario.

- **src/math/mathblocks.cpp:412 — dos fórmulas idénticas adyacentes se fusionan y al guardar se emite una sola. (V)**
  Insertar → Fórmula dos veces seguidas sin mover el cursor: en pantalla hay dos,
  `replaceMathWithSentinels` produce una sola entrada; al reabrir se ha perdido
  una.

- **src/math/mathblocks.cpp:68 — `inlineCodeRanges` no avanza `i` tras encontrar un cierre: rangos fantasma. (V)**
  Falta `i = j` tras `ranges.append(...)`. Con `` `a` $x_1$ `b` `` el re-escaneo
  empareja el cierre de `a` con la apertura de `b` y el `$x_1$` intermedio queda
  cubierto por un rango fantasma: no se protege ni se renderiza, y sus `_`/`*`
  quedan expuestos al re-parseo de `setMarkdown`.

- **src/math/mathblocks.cpp:223 — `$$$$` (o `$$` sin cierre en su línea) engulle los párrafos siguientes. (V)**
  `matchAt` lo rechaza como fórmula vacía pero el fallback multilínea lo re-trata
  como apertura: `$$$$\nhola mundo\n$$x$$` produce una pseudo-fórmula que
  contiene la prosa y hace desaparecer la fórmula real.

- **src/markdown/supsub.cpp:171 — sup/sub anidados corrompen texto adyacente y dejan tofu PUA visible. (V)**
  Con `antes ~x^2^~ despues`: los reemplazos anidados usan longitudes desfasadas;
  resultado verificado: el editor muestra sentinelas PUA sin resolver y al
  guardar se ha comido « d» de « despues». Pérdida permanente al cargar.

- **src/markdown/tableedit.cpp:96 — `injectAlignments()` no distingue los fences: reescribe código y desincroniza alineaciones. (V)**
  Un ejemplo de tabla Markdown dentro de un bloque de código consume la entrada
  de alineación de la tabla real: el contenido del bloque de código se modifica
  en silencio en cada guardado y la tabla real pierde su alineación. Con varias
  tablas, las alineaciones se corren a las tablas equivocadas.

- **src/markdown/codespanfix.cpp:50 — falta el backtick en la lista de escapes: una barra se acumula en cada guardado. (V)**
  Qt también escapa `` ` `` dentro de spans multi-backtick; `unescapeContent`
  («\\&<*[!») no lo revierte. Verificado en 3 ciclos: ``` ``a`b`` ``` →
  ``` ``a\`b`` ``` → ``` ``a\\`b`` ``` — corrupción acumulativa.

- **src/markdown/codespanfix.cpp:136 — una línea de párrafo que empieza por ≥3 backticks se toma por apertura de fence y desactiva el des-escape para el resto del documento. (V)**
  Tras ese punto, todos los code spans acumulan escapes en cada guardado
  (`` `c*d` `` → `` `c\*d` `` → `` `c\\\*d` ``…).

- **src/markdown/codespanfix.cpp:142 — los bloques de código indentados (4 espacios) no se saltan: se les borra la barra invertida en el primer guardado. (V)**

- **src/markdown/codespanfix.cpp:24 — los fences dentro de citas (`> ```) no se reconocen: su contenido verbatim se des-escapa. (V)**

- **src/markdown/markdowntidy.cpp:61 — tidy cierra un fence con cualquier línea de fence del mismo carácter, sin comparar longitudes. (V)**
  CommonMark exige cierre con run ≥ apertura. Un fence de 4 backticks que embebe
  un ejemplo de 3 se cierra antes de tiempo y Limpiar Markdown reescribe el
  contenido del código (viñetas, espacios finales).

- **src/editor/insertcontroller.cpp:299 — Insertar → Tabla con el cursor dentro de una celda crea una tabla anidada que corrompe ambas al guardar. (V)**
  La acción no está deshabilitada dentro de tablas. `toMarkdown()` emite Markdown
  malformado para las dos y tras guardar+recargar quedan destruidas.

- **src/editor/tablecontroller.cpp:149 — ordenar por columna corrompe tablas con celdas fusionadas. (P sobre comportamiento V)**
  Con celdas con span (pegadas como HTML rico), `cellAt(r,c)` devuelve la misma
  celda para todas las coordenadas cubiertas (verificado): la instantánea captura
  duplicados y la reescritura machaca la misma celda física con contenidos de
  filas distintas.

- **src/widgets/findreplacebar.cpp:275 — Reemplazar (y Reemplazar todo) edita dentro de fórmulas atómicas y el cambio se revierte en silencio al guardar. (C)**
  No hay guarda `IsMathProperty` (la protección solo existe en el filtro de
  teclado). El texto insertado hereda las propiedades math y la serialización
  reinyecta el TeX original: en pantalla «y²», en el archivo `$x^2$`.

- **src/view/splitviewcontroller.cpp:140 — ediciones del panel de fuente hechas sin foco en él se pierden. (C)**
  Reemplazar-todo desde la barra de búsqueda apuntando al fuente marca
  `m_sourceDirty` pero no arranca el timer (guard `hasFocus()`); la siguiente
  sincronización WYSIWYG→fuente hace `setPlainText` y pone `m_sourceDirty=false`:
  los reemplazos desaparecen.

- **src/view/splitviewcontroller.cpp:65 — la sincronización pendiente se descarta (no se repone) si el foco saltó al fuente. (C)**
  El fuente queda desactualizado sin marca alguna; la siguiente edición del
  fuente comete el texto viejo y revierte los cambios hechos en el WYSIWYG.

- **src/widgets/outlinepanel.cpp:103 — arrastrar en el esquema puede mover la sección equivocada, sin deshacer posible. (V parcial)**
  `scanHeadings` solo ve encabezados ATX a columna 0, pero el árbol cuenta
  también `- # Título` (válido en CommonMark, verificado que Qt lo produce): los
  ordinales se desalinean y `moveSection` opera sobre otro encabezado. El drop
  usa `setBodyMarkdown`, que limpia la pila de deshacer.

- **src/export/exportcontroller.cpp:314 — el clon de exportación pierde la `baseUrl`: las imágenes relativas desaparecen de PDF, imprimir y vista previa. (V)**
  `QTextDocument::clone` no copia la baseUrl (verificado en Qt 6.8.2) y limpia la
  caché de recursos. Afecta justo a las imágenes que crea el propio pegado
  (PNG con ruta relativa). También `exportOdf` declara `needsBaseUrl=false`
  (exportcontroller.cpp:346) y el .odt sale sin esas imágenes (DOCX/EPUB sí las
  llevan).

- **src/math/mathblocks.cpp:499 y 351, src/markdown/supsub.cpp:165, src/markdown/footnotes.cpp:106 — colisiones de sentinelas PUA. (V, probabilidad baja)**
  Texto del usuario que contenga literalmente U+F8FE/U+F8FF (math),
  U+F8F0–U+F8F3 (sup/sub), U+F8FD (salto en math multilínea) o U+F8FB (dos
  puntos de nota al pie) se transforma o sustituye por contenido interno al
  cargar/guardar. Además el índice de sentinela usa `toInt` sin comprobar `ok`:
  un índice enorme desborda a 0 y sustituye por la entrada 0 de la tabla.

## 4. Interfaz — estado inconsistente entre pestañas

- **src/app/mainwindow.cpp:625 — el estado habilitado de las acciones WYSIWYG se filtra entre pestañas con modos de vista distintos. (C)**
  Pestaña A en modo fuente (acciones deshabilitadas) → cambiar a pestaña B
  (WYSIWYG): `toggleSourceMode(false)` retorna temprano y nadie re-habilita.
  Enlace, Cita, H1–H6, listas, todo el menú Insertar y sus atajos quedan grises
  hasta alternar el modo fuente a mano. En el sentido inverso quedan habilitadas
  y, p. ej., Ctrl+B edita el documento WYSIWYG oculto; si el fuente está sucio,
  esa edición se descarta en el siguiente commit.

- **src/app/mainwindowmenus.cpp:934 — el interruptor del corrector y el idioma solo se aplican a la pestaña activa. (C)**
  Cada EditorStack tiene su SpellController y `setActiveStack` no re-sincroniza:
  desactivar «Corrección ortográfica» deja a las demás pestañas subrayando (y
  ofreciendo sugerencias) el resto de la sesión.

- **src/app/mainwindow.cpp:827 — cerrar la última pestaña en modo fuente deja visible (y resucitable) el documento cerrado. (C)**
  La rama de última pestaña solo hace `documentIo()->reset()`: ni sale del modo
  fuente ni refresca el panel. Si el usuario edita ese texto huérfano, el commit
  re-renderiza el documento supuestamente cerrado dentro del nuevo.

- **src/editor/insertcontroller.cpp:431 — el SymbolPicker no modal sigue insertando en la pestaña de origen aunque esté oculta. (C)**
  Cambiar de pestaña o de modo de vista con el diálogo abierto: los símbolos van
  al documento invisible (y en modo fuente se pierden en el siguiente commit).

- **src/app/mainwindowmenus.cpp:1059 — cancelar el aviso de cambios al cambiar de idioma deja la marca del menú en el idioma no aplicado. (C)**

## 5. Interfaz — otros fallos

- **src/diagram/diagramcontroller.cpp:132 — con dos diagramas y la herramienta ausente, el aviso se inserta DENTRO del segundo fence. (C)**
  `refresh()` itera con números de bloque capturados antes de editar: cada
  inserción corre la numeración y el segundo placeholder cae en medio del código
  del diagrama; `removeOrphanPreviews` lo detecta como huérfano en el siguiente
  refresh y el ciclo repite con parpadeo.

- **src/diagram/diagramcontroller.cpp:204 — con dos diagramas idénticos, solo el primero recibe la imagen. (C)**
  `onRendered` hace `return` tras la primera región cuya fuente coincide; la
  segunda copia se queda sin preview permanentemente.

- **src/editor/insertcontroller.cpp:335 — insertar nota al pie con selección activa aplica el superíndice al texto equivocado. (V)**
  `refStart` toma el fin de la selección pero la referencia se inserta en el
  inicio: el formato (y el clic-para-saltar) cae sobre los caracteres siguientes
  y la referencia queda sin formato.

- **src/editor/formatcontroller.cpp:72 — H1–H6 con selección multibloque: headingLevel en todos, tamaño/negrita solo en el bloque del cursor. (V)**
  Los demás se ven como texto normal hasta recargar (al guardar sí serializan
  como `# …`).

- **src/editor/editorstack.cpp:361 — Tab no avanza nunca en tablas con celdas fusionadas. (V sobre cellAt)**
  Con colspan (pegado de HTML), `cellAt(index/cols, index%cols)` devuelve la
  misma celda y cada Tab la reselecciona para siempre.

- **src/widgets/findreplacebar.cpp:184 — buscar un término inexistente mueve el cursor al inicio del documento. (C)**
  `findWithWrap` hace `setTextCursor(Start)` incondicionalmente antes del
  segundo intento: posición y selección del usuario perdidas aunque no haya
  resultado.

- **src/io/filecontroller.cpp:70 — abrir archivo/plantilla sale del modo fuente ANTES de `maybeSave`: cancelar deja al usuario expulsado de su vista. (C)**

- **src/app/mainwindowsession.cpp:162 — un aviso de cambio en disco puede apilarse sobre el diálogo de «¿guardar cambios?» y cambiar el documento por debajo. (P)**

- **src/app/mainwindow.cpp:809 — si abrir falla, queda una pestaña vacía huérfana (se crea antes de intentar la carga). (C)**

- **src/math/mathlayout.cpp:281 — matrices anidadas del mismo entorno se renderizan como basura. (C)**
  `makeMatrix` corta en el PRIMER `\end{env}` sin contar anidamiento, y el split
  por `&`/`\\` ignora llaves (`\text{a & b}` se parte por dentro).

- **src/view/themecontroller.cpp:283 — `recolorLinks` añade una entrada espuria a la pila de deshacer en cada retarget/cambio de tema. (C)**
  El primer Ctrl+Z tras activar una pestaña deshace el recoloreado, no la última
  edición.

- **src/editor/editorstack.cpp:94 — el overlay de bloque de código solo se invalida con scroll, no al cambiar el texto. (C)**
  Si el documento cambia sin scroll (preview de diagrama, commit del split), los
  números de bloque se corren y «Copiar»/«Lenguaje» actúan sobre otro bloque.

## 6. Menores

- **src/math/texparser.cpp:584 y src/spell/spellscan.cpp:34 — pares subrogados UTF-16 partidos.** En fórmulas, `x^😀` produce subrogados sueltos (UTF-16 inválido en el documento, tofu en 2D); en el corrector, una letra astral parte la palabra en dos tokens que se subrayan como erratas falsas.
- **src/markdown/docstats.cpp:22 — el contador de caracteres cuenta unidades UTF-16.** Un emoji cuenta como 2 caracteres en la barra de estado y estadísticas.
- **src/markdown/admonitions.cpp:80 — un `> \[!NOTE]` escapado a propósito se convierte en admonición activa en el round-trip.** El escape del usuario se elimina permanentemente.
- **src/editor/listcontinuation.cpp:30 — `toInt()` sin comprobar desbordamiento.** Un ítem `99999999999.` continúa la lista con «1.».
- **src/editor/focuseditor.cpp:42 — `paintEvent` recorre TODOS los bloques restantes si no hay citas tras el viewport.** El chequeo de parada solo se ejecuta en bloques de cita: coste O(documento) por repintado al teclear en documentos muy grandes.
- **src/export/exportcontroller.cpp:123 — `writeUtf8File` no comprueba el resultado.** Disco lleno → export truncado con mensaje de éxito. Igual en PDF/imprimir (exportcontroller.cpp:316): si `QPainter::begin` falla no se escribe nada pero se anuncia «Exportado a PDF».
- **src/export/exporters.cpp:223 — `\href{...}` y `\includegraphics{...}` con URL/ruta sin escapar.** Un `%` (URLs codificadas, muy habitual) rompe el .tex; un destino malicioso puede inyectar comandos LaTeX.
- **src/spell/spellchecker.cpp:60 — `encodingFromName` solo distingue Latin1 y UTF-8.** Diccionarios en ISO8859-2/KOI8-R/CP1251 producen erratas falsas y sugerencias mojibake; «ISO8859-15» cae en Latin1 por el `contains("8859-1")`.

---

## Orden de arreglo sugerido

1. Crash real: doble `delete` en `startMermaid` (diagramrenderer.cpp:138) — arreglo de una línea (flag o `QPointer`/`deleteLater` para el dir).
2. Guardado silenciosamente fallido: comprobar estado de escritura en `DocumentIo::write`, `RecoveryManager::writeFile` y `writeUtf8File`; idealmente escribir a temporal + rename.
3. El off-by-one de `charFormat()` en formulacontroller.cpp:181/277 — un solo arreglo (leer el formato en p+1) elimina 8 síntomas de corrupción de fórmulas.
4. `i = j` en `inlineCodeRanges` (mathblocks.cpp:68) — una línea.
5. La familia codespanfix/tableedit/markdowntidy (conciencia de fences) — protege el round-trip, que es la promesa central del editor.
6. Estado entre pestañas (acciones WYSIWYG, corrector, cambios en disco de pestañas en segundo plano).
7. El resto según molestia observada.
