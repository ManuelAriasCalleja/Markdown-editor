# Registro de cambios

Todos los cambios relevantes de **md-editor** se documentan en este archivo.

El formato sigue, a grandes rasgos, [Keep a Changelog](https://keepachangelog.com/es/),
y el proyecto usa [versionado semántico](https://semver.org/lang/es/).

## [2.8.2] — 2026-07-28

Versión de mantenimiento. Tres atajos que se anunciaban y no funcionaban, el
manual puesto al día con lo que hace de verdad el programa, y los iconos de lista
de la barra rediseñados para que se distingan al tamaño al que se ven.

### Corregido
- **Ctrl+W no cerraba la pestaña.** El atajo se registraba con la constante
  estándar de Qt, que en X11 tiene dos enlaces (Ctrl+F4 y Ctrl+W) de los que solo
  se aplicaba el primero. Ahora valen los dos.
- **F3 y Mayús+F3 no buscaban nada.** «Buscar siguiente» y «Buscar anterior»
  estaban documentados en el manual pero no existían: no había ni acción de menú
  ni atajo, solo los botones de la barra de búsqueda. Ahora repiten la última
  búsqueda aunque la barra esté cerrada, sin quitarle el foco al editor, y abren
  la barra si aún no se ha buscado nada.
- **Preferencias se quedaba sin atajo.** La constante estándar de Qt para
  «Preferencias» viene vacía fuera de macOS, así que Ctrl+, —anunciado en el
  manual— no hacía nada en Linux ni en Windows.
- **El manual describía cosas que no eran.** Decía que el diálogo de crear tabla
  está en el menú Tabla (está en Insertar), que «Ordenar líneas» vive dentro de
  «Transformar texto» (es una entrada aparte, y la única que exige selección) y
  que arrastrar un archivo de imagen la inserta en el documento —arrastrar un
  archivo lo abre en una pestaña, sea imagen o no; lo que guarda la imagen a
  disco es pegarla del portapapeles—.

### Cambiado
- **Iconos de lista de la barra: se distinguen de un vistazo.** Los tres
  compartían el 80 % del dibujo y se diferenciaban en un marcador de 5-6 px, con
  las cifras «1 2 3» dibujadas con la fuente a un tamaño al que el suavizado las
  deja en tres manchas grises. Pasan a dos renglones —que libera altura para el
  marcador—, con las cifras trazadas a mano con el mismo grosor que las líneas y
  las tareas como marcas de verificación sin recuadro. Los cuatro iconos de
  formato (N/C/S/T) se centran ahora por la mancha real de la letra, no por la
  caja de línea, que los dejaba altos dentro del botón.
- **Manual completo en los nueve idiomas.** Se documentan el menú contextual de
  las pestañas, «Insertar → Imagen…» y «Regla horizontal», Ctrl+H, los caracteres
  sin espacios de las estadísticas y una tabla de atajos con los que faltaban
  (Ctrl+E, Ctrl+K, Ctrl+Shift+X/Q/K/U/O/T, Ctrl+] y Ctrl+[, Ctrl+,). La guía de
  Markdown incorpora las cuatro extensiones que el editor admite y no explicaba
  —`==resaltado==`, `^super^`/`~sub~`, admoniciones y bloques mermaid/plantuml—
  y el front matter.
- **La página del proyecto (README) cuenta lo que hay**: reglas de entrada, barra
  flotante de tablas, overlay de los bloques de código, vista dividida, los ocho
  temas y una tabla de atajos de diecisiete filas en vez de ocho.

### Añadido
- Dos pruebas que vigilan lo que nadie compila: los atajos de la ventana
  (incluida la ausencia de atajos ambiguos, que es lo que rompe uno sin avisar) y
  los dieciocho archivos de ayuda —que cada enlace del índice caiga en un
  encabezado real y que ninguna traducción se quede corta de secciones—. La
  segunda encontró dos anclas rotas en el índice polaco.

## [2.8.1] — 2026-07-28

Versión de mantenimiento. Sin funciones nuevas: dos fallos que impedían usar el
programa —uno al abrir cierto documento en Windows, otro al compilarlo con un Qt
reciente— y el remate del portugués.

### Corregido
- **Windows: abrir un documento que cierre con `---` cerraba el programa.**
  Cualquier `.md` que empezara y acabara por una regla horizontal `---` y no
  terminara en salto de línea mataba el editor al abrirlo, sin mensaje. El fallo
  no era del editor sino de Qt (una escritura fuera de rango en el importador de
  Markdown, corregida por Qt en la 6.9); mientras se soporten las versiones
  afectadas, el editor lo rodea. El caso mínimo eran seis caracteres.
- **Portugués: una sola norma en toda la interfaz.** Se mezclaban vocabulario
  europeo y brasileño para los mismos conceptos, y se veía en la misma sesión: el
  menú decía «Salvar» mientras el resto hablaba de «ficheiro», y los diálogos de
  abrir y guardar ofrecían «Ficheiros Markdown» y «Arquivos Markdown». Se ha
  uniformado a la norma brasileña.

### Cambiado
- **Compila con Qt 6.11.** La configuración fallaba con las versiones recientes
  de Qt, que ya no definen solo el objetivo `Qt6::GuiPrivate` que usa la
  exportación a ODF; ahora se cubren las dos formas sin condicionar por número de
  versión. Quien tuviera un Qt reciente no podía compilar la 2.8.0.
- **Las pruebas se ejecutan también en Windows y macOS**, no solo en Linux: hasta
  ahora los binarios de esas dos plataformas se publicaban sin que se hubiera
  ejecutado ni una prueba sobre ellos. De ahí salió el fallo de Windows de arriba.

## [2.8.0] — 2026-07-25

Repaso a fondo de la exportación y la importación: cada corrección nació de un
documento que salía mal (un `.docx` que Word rechazaba, un `.tex` que no compilaba,
un `.html` que perdía las imágenes al moverlo, un EPUB con una sola entrada de
índice) y queda cubierta por su prueba. La reorganización interna que lo acompaña no
cambia la salida: los ficheros de referencia coinciden byte a byte.

### Añadido
- **Ver → Previsualizar diagramas**: interruptor para apagar la previsualización
  automática de los bloques ```` ```mermaid ````/```` ```plantuml ```` (activado por
  defecto, se recuerda). Útil cuando el documento ya trae debajo una imagen
  pre-renderizada a mano, que si no se veía por duplicado.

### Corregido
- **Exportación a DOCX**: un solo carácter que XML no admite (control, suplente
  suelto) dejaba el paquete mal formado y **Word lo rechazaba entero**; ahora se
  descartan. Las tablas llevan la anchura de cada columna y marcada la fila de
  encabezado —sin ellas, Pandoc las leía como si no tuvieran columnas y **perdía
  todas las celdas**—, y los enlaces van por relación, como los escribe Word, en vez
  del campo `HYPERLINK` que otros programas descartaban con rótulo incluido.
- **Importación con Pandoc**: las imágenes se extraen a una carpeta
  `<nombre>-media` junto al original (antes el Markdown apuntaba a rutas de dentro
  del paquete y **se perdían todas**); el título del documento llega como front
  matter en vez de descartarse; las imágenes con tamaño o sin texto alternativo ya
  no desaparecen al reabrir el documento; y las tablas que Markdown no sabe expresar
  (celdas combinadas, celdas de varios párrafos, tablas anidadas) se convierten a
  tabla de tuberías en vez de verse como texto literal.
- **Exportación a LaTeX**: un bloque de código que contuviera la línea
  `\end{verbatim}` —cualquier documento que hable de LaTeX— **tumbaba la
  compilación**; también las listas y citas muy anidadas («Too deeply nested»), un
  elemento de lista que empieza por `[` (una bibliografía) y el `~` de las URLs, que
  llegaba estropeado al enlace del PDF. Se conservan además cosas que se perdían: el
  arranque de una lista numerada, las citas anidadas, las listas y el código
  **dentro** de una cita, y el super/subíndice de la prosa (`H~2~O` salía «H2O»).
- **Exportación a HTML**: el `.html` **perdía todas las imágenes** al moverlo de
  carpeta o enviarlo, porque se referenciaban por ruta relativa; ahora se embeben en
  el propio archivo, con sus bytes originales cuando el navegador entiende el
  formato. Se añaden también el idioma y el título del documento. *Copiar como HTML*,
  igual.
- **Exportación a EPUB**: el libro llegaba al lector **con una sola entrada de
  índice**, sin manera de ir a un capítulo; ahora el índice se arma con los
  encabezados del documento. Las casillas de tarea hechas y pendientes se
  distinguen, y las imágenes conservan su formato en vez de rasterizarse todas.
- **PDF e impresión**: una imagen más ancha que la página salía **truncada** en vez
  de escalada; y al imprimir sin números de página desaparecían las imágenes
  referenciadas por ruta relativa.

### Cambiado
- **Los diálogos de abrir e importar** arrancan en la carpeta del documento activo
  (o del último abierto), en vez de siempre en la misma.

## [2.7.1] — 2026-07-24

### Corregido
- **Exportación a LaTeX**: la matemática Unicode escrita literalmente en la prosa
  (subíndices `₁ₙ`, superíndices `ⁿ`, griego `φΣ`, operadores `⊕∈≥`, conjuntos `ℝ`,
  alfabetos `𝒞`) rompía la compilación en pdfLaTeX, que con inputenc+T1 no compone
  esos glifos. Ahora se traducen a modo matemático (`$_{1}$`, `$\varphi$`,
  `$\mathcal{C}$`, `$\ldots$`) y el `.tex` compila.
- **Contador de palabras**: las cifras de cuatro dígitos no llevaban separador de
  miles cuando las de caracteres sí (`1234` frente a `12.345`). Palabras, caracteres
  y estadísticas agrupan ya de forma coherente.

### Cambiado
- **Exportación a LaTeX autocontenida**: el `.tex` se lleva consigo las imágenes
  —copiadas a su lado tal cual si pdfLaTeX admite el formato (pdf/png/jpg), o
  rasterizadas a PNG si no (`.svg`, `.gif`, `.bmp`…)—, así que compila esté donde
  esté y ya no depende de quedarse junto al `.md`.

## [2.7.0] — 2026-07-18

### Añadido
- **Menú contextual en las pestañas** (clic derecho sobre el nombre): abrir la
  carpeta contenedora en el gestor de archivos, copiar el nombre del archivo y
  copiar la ruta completa.
- **Los gráficos e imágenes escalan con el zoom**: los diagramas Mermaid/PlantUML
  y las imágenes `![]()` crecen y menguan con Ctrl++/Ctrl+− junto al texto.

### Cambiado
- **Contador y estadísticas**: las cifras se agrupan con el separador de miles del
  idioma; el tiempo de lectura se reparte en horas y minutos a partir de 60 min.

## [2.6.1] — 2026-07-02

Auditoría de robustez: 30 correcciones de estabilidad, round-trip y ciclo de vida
(ver `ERRORES-DETECTADOS.md`). Cada arreglo con su prueba de regresión donde
aplicaba; la suite pasa en build normal y bajo ASan+UBSan, y clang-tidy queda limpio.

### Corregido
- **Cuelgues y cierres inesperados**: doble `delete` del directorio temporal en el
  render de diagramas cuando el proceso falla; cuelgue O(n²) del parser TeX ante
  fórmulas hostiles muy anidadas (parser y motor 2D).
- **Guardado sin pérdida de datos**: escritura atómica y comprobada (`QSaveFile`) en
  el guardado de documentos, el borrador de recuperación y la exportación; un fallo
  de disco ya no se reporta como éxito ni deja el archivo truncado.
- **Fórmulas**: off-by-one al teclear en el borde de una fórmula y al editarla por
  doble clic; buscar/reemplazar ya no entra dentro de las fórmulas.
- **Round-trip Markdown**: código en línea con conciencia de fences, cita e
  indentado y del backtick escapado; cierre de fence por longitud de run al limpiar;
  las tablas de ejemplo dentro de un fence ya no reciben marcadores de alineación;
  fórmulas `$$…$$` multilínea y fórmulas dentro de una cita preservadas; corregido el
  desbordamiento del índice de sentinela.
- **Interfaz, pestañas y disco**: acciones WYSIWYG y corrector re-sincronizados al
  cambiar de pestaña; cambios en disco detectados también en pestañas de segundo
  plano y re-vigilancia tras guardar; el borrador de recuperación se conserva al
  cerrar; la última pestaña en modo fuente y la cancelación del guardado se respetan;
  ya no queda una pestaña huérfana si falla la apertura.
- **Otros**: `baseUrl` correcto en PDF, impresión, vista previa, ODF y exportación de
  la selección; tabla anidada dentro de tabla; nota al pie sobre una selección; Tab
  en celdas fusionadas; encabezado que abarca varios bloques; número de lista
  desbordado; recuento de caracteres UTF-16; escape LaTeX de URLs y rutas.

## [2.6.0] — 2026-07-02

### Añadido
- **Ventana de Preferencias** (*Editar → Preferencias…*, Ctrl+,): reúne en un diálogo
  con pestañas (Apariencia, Editor, Impresión) los ajustes más comunes —tema, luz
  cálida, seguir el sistema, interlineado, resaltar la línea actual, modo foco y
  números de página—, que hasta ahora solo estaban repartidos por el menú Ver.
- **Barra flotante de tabla y navegación por celdas**: cuando el cursor está dentro
  de una tabla aparece sobre ella una barra con botones para insertar/eliminar fila y
  columna y alinear la columna. Además, **Tab** salta a la celda siguiente y
  **Mayús+Tab** a la anterior; **Tab** en la última celda añade una fila.
- **Etiqueta de lenguaje y botón «Copiar» en los bloques de código**: al pasar el
  ratón por un bloque de código aparece, arriba a la derecha, su lenguaje (clic para
  cambiarlo) y un botón para copiar el código.
- **Documento de bienvenida** en el primer arranque (una sola vez), con una guía breve
  para empezar; se carga sin marcar como modificado. Y un **texto de ayuda sutil**
  (*placeholder*) en el documento vacío.
- **Nuevas plantillas de fábrica** en *Nuevo desde plantilla*: **Programación** —
  Registro de cambios (CHANGELOG), Decisión de arquitectura (ADR) e Informe de error;
  **Académico** — Artículo científico (IMRyD) e Informe de laboratorio. Traducidas a
  los 9 idiomas.
- **Plantillas de usuario**: *Archivo → Guardar como plantilla…* guarda el documento
  actual (con su front matter) como una plantilla propia, con nombre y categoría;
  reaparece en *Nuevo desde plantilla* junto a las de fábrica. *Gestionar plantillas…*
  (al final de ese submenú) permite editarlas o borrarlas. Se guardan en los ajustes.
- **Importar desde HTML, EPUB y otros formatos** (*Archivo → Importar*): convierte a
  Markdown y abre como documento nuevo sin título, sin tocar el original. **HTML**
  (respeta el juego de caracteres declarado: BOM › `<meta charset>` › UTF-8) y **EPUB**
  (lee sus capítulos en orden) son nativos; **Otros formatos (Pandoc)…** importa DOCX,
  ODT, RTF, LaTeX, reStructuredText… ejecutando Pandoc si está instalado (si no, indica
  cómo instalarlo).
- **Reglas de entrada** en el editor visual: al inicio de línea, teclear un marcador
  Markdown de bloque seguido de espacio lo transforma en el sitio (sin dejar el
  marcador): `#`…`######` → encabezado, `>` → cita, `-`/`*`/`+` → lista de viñetas,
  `1.`/`1)` → lista numerada.

### Cambiado
- **Aviso con la URL al pasar el ratón por un enlace**: además de la barra de estado,
  ahora aparece un *tooltip* con el destino junto al cursor.
- **Plantillas agrupadas por categoría** en *Archivo → Nuevo desde plantilla*: las
  plantillas se organizan ahora en submenús por categoría profesional (Personal,
  Programación, Docencia, Empresa, Escritura…) en vez de una lista plana. Una
  categoría sin plantillas no se muestra.
- **Tipografía del documento renderizado**: el editor ya no muestra el Markdown
  «plano». Los encabezados tienen ritmo vertical (más aire arriba cuanto mayor es el
  nivel), los párrafos se separan un poco, los bloques de código se ven como un panel
  con fondo tenue y sangría, y las citas llevan **barra lateral** (gris en una cita
  normal; del color del acento en una admonición, completando el aspecto de «callout»)
  y un fondo sutil. Es solo presentación (no cambia el Markdown ni el guardado) y se
  aplica automáticamente.
- **Botón H4 en la barra de formato** (antes solo H1–H3; H4–H6 seguían disponibles
  por el menú Formato y Ctrl+4–6).

### Corregido
- **Fórmulas con varios exponentes/subíndices seguidos** (p. ej.
  `$T^2 = \frac{4\pi^2}{GM}\,a^3$`): el super/subíndice de texto emparejaba por error
  dos `^`/`~` a través de la fórmula cuando no había espacio entre ellos, corrompiendo
  el TeX (el `a^3` se veía como `a` + un símbolo raro + `3`). Ahora el super/subíndice
  de texto respeta el código en línea y los bloques vallados, y por tanto las fórmulas.

## [2.5.0] — 2026-07-01

### Añadido
- **Paleta de comandos** (*Ver → Paleta de comandos*, Ctrl+Shift+P): busca y
  ejecuta cualquier acción de los menús escribiendo parte de su nombre, con
  filtrado difuso y navegación por teclado.
- **Resaltar la línea actual** (*Ver → Resaltar la línea actual*): marca con un
  fondo sutil la línea del cursor. Desactivado por defecto.
- **Promover/degradar encabezado** (*Formato*, Ctrl+Shift+[ / Ctrl+Shift+]): sube
  o baja un nivel el encabezado del cursor (acotado entre H1 y H6).
- **Ordenar filas de tabla por columna** (*Tabla → Ordenar filas por columna*,
  ascendente/descendente): reordena las filas por la columna del cursor dejando la
  cabecera fija; detecta si la columna es numérica o de texto.
- **Filtro y plegado del esquema**: el panel de esquema tiene un campo de filtro en
  vivo (muestra las coincidencias y sus ancestros) y botones «Expandir/Plegar
  todo»; el plegado se conserva ahora entre reconstrucciones (antes se reexpandía
  todo al editar).
- **Comandos de línea en la vista de código**: mover la línea arriba/abajo
  (Alt+↑/↓), duplicarla (Ctrl+D), borrarla (Ctrl+Shift+K) y unirla con la siguiente
  (Ctrl+J).
- **Resaltar texto** (*Formato → Resaltar*, Ctrl+Shift+H): marca el texto
  seleccionado con `==marca==`, mostrado con fondo de resaltado. Se guarda como
  texto literal (round-trip seguro).
- **Superíndice y subíndice** (*Formato*, Ctrl+Shift++ / Ctrl+Shift+-): eleva o baja
  el texto seleccionado, mostrándolo en su posición real; se guarda como `^texto^` y
  `~texto~` (estilo Pandoc), con round-trip fiel.
- **Números de página al imprimir** (*Ver*, activado por defecto): añade el número de
  página en el pie (`N / M`) al imprimir y al exportar a PDF.

### Cambiado
- **Buscar** resalta ahora **todas** las coincidencias en el documento y muestra
  un contador **«N de M»** en la barra. «Reemplazar todo» reusa el mismo motor de
  coincidencias.

### Corregido
- **El zoom de la interfaz ya no afecta al tamaño de letra al imprimir ni al
  exportar** (PDF, HTML, ODF, EPUB): la salida usa un cuerpo estándar,
  independiente del zoom de pantalla. Antes, con la interfaz ampliada, el texto
  impreso/exportado salía desproporcionadamente grande.

## [2.4.0] — 2026-07-01

### Añadido
- **Ir a línea** (*Ver → Ir a línea*, Ctrl+L): lleva el cursor a un número de
  línea del editor activo (en la vista de fuente, a la línea del Markdown).
- **Indicador de línea y columna** en la barra de estado (*Ver → Mostrar línea y
  columna*): muestra la posición del cursor; desactivado por defecto.
- **Insertar tabla desde el portapapeles** (*Insertar → Tabla desde el
  portapapeles*): convierte datos TSV/CSV en texto plano en una tabla Markdown.
- **Reabrir pestaña cerrada** (*Archivo → Reabrir pestaña cerrada*, Ctrl+Shift+R):
  vuelve a abrir la última pestaña cerrada que tenía archivo en disco.
- **Copiar como Markdown** (*Editar*): copia al portapapeles la selección o el
  documento entero como texto Markdown, por la serialización canónica.
- **Exportación a texto plano** (*Archivo → Exportar → A texto plano*).
- **Revertir a lo guardado** (*Archivo*): descarta los cambios sin guardar y
  recarga el archivo del disco, con confirmación.

### Cambiado
- **El PDF incrusta el título y el autor** del front matter (`title`, `author`)
  al exportar e imprimir a PDF.

## [2.3.0] — 2026-06-29

### Añadido
- **Interlineado configurable** (*Ver → Interlineado*): elige entre Sencillo, Medio
  y Amplio. Se aplica solo en pantalla (no se serializa, no afecta al round-trip) y
  se recuerda entre sesiones.
- **Temas Solarized**: dos temas nuevos, Solarized Light y Solarized Dark, que se
  suman al catálogo existente.

### Cambiado
- **El modo sin distracciones ya no se sale al cambiar de pestaña**: se traslada al
  documento activo en vez de desactivarse, y escala correctamente con el zoom de la
  interfaz.
- **La exportación conserva el resaltado de sintaxis** de los bloques de código:
  HTML, PDF, ODF y DOCX salen con el código coloreado como en pantalla.
- **Recuperación por pestaña**: el borrador de autoguardado pasa a indexarse por
  documento (un slot único por pestaña), así que un cierre inesperado conserva
  TODOS los documentos con cambios, no solo el último; al arrancar se ofrecen todos
  para recuperar en sus pestañas.
- **Atajos**: el modo foco pasa a **F12** y el foco al panel de esquema a **F6**
  (antes Ctrl+Shift+O, que era ambiguo). El manual integrado documenta los atajos
  de pestaña y de foco del esquema en los 8 idiomas, con anclas navegables.

## [2.2.0] — 2026-06-24

### Añadido
- **Modo foco** (*Ver → Modo foco*): un interruptor que (a) mantiene la línea del
  cursor centrada en vertical mientras escribes («máquina de escribir») y (b)
  atenúa todo el documento salvo el párrafo del cursor, para concentrar la vista.
  Independiente del modo sin distracciones; desactivado por defecto y recordado
  entre sesiones.
- **Snippets de usuario** (*Insertar → Snippet*): fragmentos de Markdown
  reutilizables que defines una vez (con *Gestionar snippets…*) e insertas por
  nombre desde el menú. A diferencia de las plantillas (de archivo entero), se
  insertan en el punto del cursor; en la vista WYSIWYG se renderizan y en la de
  fuente se pegan como Markdown. Se recuerdan entre sesiones.
- **Auto-emparejado** de paréntesis, corchetes, llaves y comillas invertidas: al
  teclear `(`, `[`, `{` o `` ` `` se inserta también su cierre con el cursor en
  medio; si hay texto seleccionado, se envuelve; y al teclear el cierre justo
  delante del automático, se salta en vez de duplicarlo.
- **Limpiar Markdown** (*Editar → Limpiar Markdown*): normaliza el documento de una
  pasada (viñetas a `-`, espacios finales, líneas en blanco de más, espacio tras
  los `#`), sin tocar el interior de los bloques de código.
- **Aviso de fallos de diagrama**: cuando un diagrama Mermaid/PlantUML no se puede
  renderizar (sintaxis inválida, error de la herramienta…), se avisa en la barra de
  estado con el error. El aviso espera a que el diagrama se asiente, para no
  molestar mientras se teclea.

### Cambiado
- **Menos parpadeo** al cargar un documento, cambiar de tema y sincronizar la vista
  dividida: las operaciones que reconstruían el documento ahora se agrupan en un
  único trazado.
- La carga y el guardado usan el dialecto Markdown con `MarkdownNoHTML`: un `<algo>`
  se trata como texto literal (lo correcto en un editor WYSIWYG) en vez de como HTML.

### Arreglado
- **Fórmulas TeX**: una fórmula con anidamiento extremo (`\frac{\frac{…}}`,
  `x^{y^{…}}`) ya no desborda la pila ni cierra la aplicación (tope de profundidad
  del parser).
- **Round-trip de Markdown**: un *code span* con `\` o `&` (p. ej. `` `C:\ruta` ``)
  ya no duplica esos caracteres en cada guardado; y un `<algo>` ya no se traga ese
  texto y el de alrededor al cargar (antes era pérdida de datos).

### Interno
- Reorganizado `src/` en subdirectorios por componente (app, editor, view, io,
  markdown, math, diagram, spell, export, widgets…), sin cambios de comportamiento.
- Documentado el código con **Doxygen** (`Doxyfile`), sin warnings de generación.
- `mainwindow.cpp` repartido en más unidades de traducción (zoom, sesión); eliminado
  código muerto; nuevas redes de pruebas: **fuzzing del round-trip** (bajo
  ASan/UBSan) y **golden tests** de los exportadores.

## [2.1.0] — 2026-06-23

### Añadido
- **Accesibilidad**: nombres y descripciones accesibles en los controles que no
  los derivaban solos (editor WYSIWYG, vista de fuente, panel de esquema, campos
  de Buscar/Reemplazar, contador de palabras y el diálogo «Ir a encabezado»); los
  mensajes de estado importantes y la fórmula bajo el cursor se **anuncian** a los
  lectores de pantalla (`QAccessibleAnnouncementEvent`); nueva sección
  «Accesibilidad» en la ayuda (F1) y el README, en los 9 idiomas; orden de
  tabulación y nombres revisados en los diálogos.
- **Más comandos en las fórmulas TeX**: delimitadores (`\langle`, `\lceil`,
  `\lfloor`, `\Vert`…), `\left`/`\right`, negación `\not`, subrayado `\underline`,
  espaciado `\quad`/`\qquad`, alfabetos `\mathcal`/`\mathscr`/`\mathfrak` y más
  operadores, relaciones, flechas y símbolos.

### Cambiado
- Los enlaces a archivos `.md` locales se abren en una pestaña nueva de la misma
  ventana, en vez de lanzar otra instancia.

### Arreglado
- Los iconos de la barra de formato (negrita, cursiva, listas…) recuperan el
  contraste al cambiar de tema y dejan de verse borrosos en pantallas HiDPI al
  arrancar: se regeneran a la densidad de pantalla y la paleta vigentes.

## [2.0.0] — 2026-06-22

### Añadido
- **Diagramas Mermaid y PlantUML** (opcional): un bloque de código ` ```mermaid `
  o ` ```plantuml ` se previsualiza como imagen justo debajo, sin tocar el código
  (que sigue editable) ni el Markdown guardado. Requiere tener instalada la
  herramienta correspondiente (`plantuml` con Java, o `mmdc` con Node); si falta,
  bajo el bloque aparece un aviso discreto con la orden de instalación de tu
  sistema operativo (y el código se mantiene como tal).
- **Corrección ortográfica** (opcional, basada en Hunspell): subraya las palabras
  mal escritas según el idioma del documento (deducido del front matter, el ajuste
  de idioma o el sistema). Clic derecho sobre una errata para ver sugerencias,
  añadirla al diccionario personal o ignorarla. Se activa/desactiva en
  *Ver → Corrección ortográfica*. En Linux usa los diccionarios del sistema; si no
  hay biblioteca Hunspell, el programa funciona igual sin corrector.
- **Fórmulas en 2D real («Nivel 2»)**: las fracciones se apilan con barra real,
  los grandes operadores (`\sum`, `\int`, `\prod`…) muestran sus límites encima y
  debajo, las raíces (`\sqrt`, `\sqrt[n]`) llevan vínculo sobre el radicando y las
  matrices (`pmatrix`, `bmatrix`…) se maquetan como rejilla con paréntesis o
  corchetes, en vez de aproximarse en línea. Se pintan vectorialmente, escalan con
  el zoom y siguen al tema. Las fórmulas más simples se siguen componiendo en
  línea. La exportación a HTML/ODF/PDF/DOCX y el round-trip Markdown no cambian.
  Además, los nombres de función (`\lim`, `\sin`, `\cos`, `\log`…) se componen
  como texto y los comandos de espaciado (`\,`, `\;`, `\!`) se respetan. También
  hay coeficientes binomiales (`\binom`), sistemas a trozos (`\begin{cases}`),
  acentos (`\hat`, `\bar`, `\vec`, `\tilde`, `\dot`…) y texto literal (`\text`).

### Interno
- El motor de fórmulas se divide en módulos: `texparser` (TeX→runs) y `mathlayout`
  (maquetación 2D), además del `mathblocks` existente; el `QTextObjectInterface`
  `MathObject` pinta las fórmulas 2D en el documento.

## [1.3.0] — 2026-06-21

### Añadido
- **Insertar → Símbolos especiales…**: diálogo con símbolos no habituales
  (matemáticos, griego, flechas, moneda, puntuación, astronomía, marcas y
  fracciones) organizados por categorías; un clic inserta el símbolo en el
  cursor y el diálogo permanece abierto para insertar varios.
- **Insertar → Fecha / Fecha y hora**: inserta la fecha (y hora) actual en
  formato localizado.
- **Exportar/Imprimir solo la selección**: *Archivo → Imprimir selección* y
  *Exportar → Selección a PDF*.
- **Auto-enlazar al pegar una URL**: pegar una URL sobre texto seleccionado
  inserta `[texto](url)`.
- **Ir a encabezado** (Ctrl+G): «quick open» con filtro sobre los encabezados
  del documento.
- **Tipografía inteligente** (en *Transformar texto*): `--`→—, `...`→…, y
  comillas rectas → tipográficas.
- **Shortcodes `:nombre:`**: al teclear, `:alpha:`→α, `:check:`→✓, etc.
- **Editar → Pegar como Markdown** (Ctrl+Alt+V): convierte el texto enriquecido
  del portapapeles (HTML) a Markdown en vez de incrustar su formato.
- **Archivo → Nuevo desde plantilla**: 10 plantillas de documento (acta de
  reunión, nota diaria, artículo de blog, README, carta, informe, lista de tareas,
  certificado, práctica de asignatura y examen), traducidas a los 9 idiomas.
- **Admoniciones / callouts** estilo GitHub (`> [!NOTE]`, `[!TIP]`, `[!IMPORTANT]`,
  `[!WARNING]`, `[!CAUTION]`): se muestran con fondo de color y título destacado;
  *Insertar → Admonición*. Round-trip compatible con GitHub.
- **Exportar a EPUB** (`.epub`): *Archivo → Exportar → A EPUB*, con idioma y título
  incrustados e imágenes embebidas. Sin dependencias externas.

## [1.2.0] — 2026-06-18

### Añadido
- **Cambio de idioma de la interfaz sin reiniciar**: *Ver → Idioma* aplica el
  idioma al instante recreando la ventana (antes pedía reiniciar).
- **Casillas de tarea interactivas**: un clic sobre la casilla de un ítem
  `- [ ]`/`- [x]` la marca o desmarca, con pista al pasar por encima.
- **Notas al pie**: *Insertar → Nota al pie* (Ctrl+Mayús+N) inserta una
  referencia `[^n]` autonumerada y su definición; las referencias se muestran
  como superíndice y un clic salta a su definición.
- **Reordenar secciones desde el esquema**: arrastrar un encabezado en el panel
  de índice mueve su sección entera (con su contenido y subsecciones).

### Corregido
- El panel de esquema (F9) podía aparecer ocupando casi toda la anchura al
  restaurar un estado guardado desproporcionado; ahora se normaliza.
- El tamaño de la fuente de los menús ya no se acumulaba al cambiar de idioma.
- Unificadas a portugués de Brasil unas cadenas que habían quedado en portugués
  europeo (mezcla de registros).

### Documentación
- Manual de la app y página de sintaxis Markdown actualizados en los 9 idiomas
  con las notas al pie, las casillas interactivas y la reordenación de secciones.
- Nuevo `CHANGELOG.md` y ejemplo completo `ejemplos/prueba-completa.md` que
  ejercita todas las construcciones.

## [1.1.1] — 2026-06-18

### Corregido
- La tabla de contenidos (panel de esquema) aparecía demasiado ancha.

### Cambios internos
- `install.sh`: modo `-m` (build de tamaño mínimo) / normal y ayuda `-h`.
- CI: GitHub Actions actualizadas a Node.js 24 (v5).

## [1.1.0] — 2026-06-16

### Añadido
- **Exportación a DOCX (Word)** con serializador OOXML propio (sin dependencias).
- **Estadísticas del documento**, inserción de **índice (TOC)** y **seguir el
  tema del sistema** (claro/oscuro).
- Funciones baratas de Qt puro: vista previa de impresión, búsqueda con regex y
  palabra completa, transformar texto y ordenar líneas, copiar como HTML, pegar
  como texto plano, recordar la posición del cursor por archivo y abrir la
  carpeta contenedora.
- Especificación del producto y wiki bilingüe (9 idiomas).

## [1.0.2] — 2026-05-30

### Añadido
- Traducción de la interfaz a 7 idiomas más (total: 9).

### Corregido
- Varios fallos menores reportados; refactor de arquitectura y mejora del
  sistema de ayuda.

## [1.0.1] — 2026-05-29

### Corregido
- Fallos reportados tras el lanzamiento inicial.

## [1.0.0] — 2026-05-28

### Añadido
- Primera versión pública: editor/visor **WYSIWYG** de Markdown en Qt6 + C++17,
  con round-trip por `QTextDocument`, fórmulas TeX, temas, modo sin
  distracciones, vista dividida, exportación a PDF/HTML/ODT/LaTeX e
  internacionalización a 9 idiomas.
- CI/CD multiplataforma (Linux AppImage, Windows ZIP, macOS DMG) y publicación
  de releases por tag.

[Sin publicar]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v2.8.2...HEAD
[2.8.2]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v2.8.1...v2.8.2
[2.8.1]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v2.8.0...v2.8.1
[1.2.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.2...v1.1.0
[1.0.2]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/releases/tag/v1.0.0
