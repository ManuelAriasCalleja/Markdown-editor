# Registro de cambios

Todos los cambios relevantes de **md-editor** se documentan en este archivo.

El formato sigue, a grandes rasgos, [Keep a Changelog](https://keepachangelog.com/es/),
y el proyecto usa [versionado semántico](https://semver.org/lang/es/).

## [Sin publicar]

### Añadido
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

[Sin publicar]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.2...v1.1.0
[1.0.2]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/ManuelAriasCalleja/Markdown-editor/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/ManuelAriasCalleja/Markdown-editor/releases/tag/v1.0.0
