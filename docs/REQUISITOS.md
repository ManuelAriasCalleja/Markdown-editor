# Requisitos del producto — md-editor

> Documento de requisitos (funcionales y no funcionales) recopilados a partir del
> código fuente (`src/`), el manual integrado (`src/help/`), el `CHANGELOG.md`, el
> `CMakeLists.txt` y la CI (`.github/workflows/`). Consolida y reemplaza a la antigua
> `especificacion.md`, y es la referencia completa del producto desde la que se
> redacta la wiki.
>
> - **Producto:** md-editor — editor/visor **WYSIWYG** de Markdown.
> - **Versión cubierta:** 2.8.0.
> - **Tecnología:** Qt6 (≥ 6.5) + C++17.
> - **Plataformas:** Linux, Windows, macOS.
> - **Fuente de verdad última:** el código fuente. Aquí se recoge el comportamiento
>   observable de cara al usuario y las restricciones de construcción/operación.

Cada requisito tiene un identificador estable (`RF-*` funcional, `RNF-*` no
funcional) para poder trazarlo desde la **lista de comprobaciones manuales** (al
final de este documento).

---

## Índice

- [Requisitos funcionales](#requisitos-funcionales)
  - [1. Modelo de edición y round-trip](#1-modelo-de-edición-y-round-trip)
  - [2. Gestión de archivos y sesión](#2-gestión-de-archivos-y-sesión)
  - [3. Edición por pestañas (multi-documento)](#3-edición-por-pestañas-multi-documento)
  - [4. Formato de texto y estructura](#4-formato-de-texto-y-estructura)
  - [5. Inserción de contenido](#5-inserción-de-contenido)
  - [6. Tablas](#6-tablas)
  - [7. Fórmulas TeX](#7-fórmulas-tex)
  - [8. Modos de vista (fuente y dividida)](#8-modos-de-vista-fuente-y-dividida)
  - [9. Navegación y esquema](#9-navegación-y-esquema)
  - [10. Buscar y reemplazar](#10-buscar-y-reemplazar)
  - [11. Resaltado de código](#11-resaltado-de-código)
  - [12. Diagramas (Mermaid / PlantUML)](#12-diagramas-mermaid--plantuml)
  - [13. Corrección ortográfica](#13-corrección-ortográfica)
  - [14. Extensiones de Markdown](#14-extensiones-de-markdown)
  - [15. Exportación e impresión](#15-exportación-e-impresión)
  - [16. Temas y apariencia](#16-temas-y-apariencia)
  - [17. Modos de concentración](#17-modos-de-concentración)
  - [18. Estadísticas del documento](#18-estadísticas-del-documento)
  - [19. Zoom de toda la interfaz](#19-zoom-de-toda-la-interfaz)
  - [20. Internacionalización (función)](#20-internacionalización-función)
  - [21. Ayuda](#21-ayuda)
  - [22. Plantillas y snippets](#22-plantillas-y-snippets)
  - [23. Persistencia de ajustes](#23-persistencia-de-ajustes)
- [Requisitos no funcionales](#requisitos-no-funcionales)
- [Comprobaciones manuales](#comprobaciones-manuales)
- [Apéndice A — Atajos de teclado](#apéndice-a--atajos-de-teclado)

---

# Requisitos funcionales

## 1. Modelo de edición y round-trip

- **RF-EDI-01 — Edición WYSIWYG real.** El editor principal muestra el documento ya
  renderizado; por defecto el usuario no ve la sintaxis Markdown. El formato se
  aplica con formatos nativos de Qt que serializan limpiamente a Markdown.
- **RF-EDI-02 — Round-trip fiel.** «Lo que abres es lo que guardas»: al guardar, el
  documento se serializa de vuelta a Markdown limpio en UTF-8, conservando tablas
  con alineación, citas, listas anidadas, listas de tareas, bloques de código,
  notas al pie, admoniciones y fórmulas.
- **RF-EDI-03 — Serialización canónica.** Toda ruta que serializa el documento usa
  la serialización canónica (`mdtable::documentMarkdown`), que reinyecta la
  alineación de columnas (`:--`/`:-:`/`--:`) que `QTextDocument::toMarkdown()`
  descarta, las fórmulas TeX y deshace el sobre-escapado de Qt en code spans y
  admoniciones.
- **RF-EDI-04 — Indicador «modificado».** El indicador del título/pestaña refleja
  cambios reales del contenido comparando la serialización canónica actual con una
  línea base, no el estado interno de Qt.
- **RF-EDI-05 — Deshacer/Rehacer.** El editor admite deshacer (`Ctrl+Z`) y rehacer
  (`Ctrl+Y` / `Ctrl+Shift+Z`); «Reemplazar todo» cuenta como un único paso de
  deshacer.
- **RF-EDI-06 — El código fuente es opcional.** Ver/editar el Markdown crudo es
  posible pero no obligatorio (ver [§8](#8-modos-de-vista-fuente-y-dividida)).
- **RF-EDI-07 — Auto-emparejado.** Al teclear `(`, `[`, `{` o `` ` `` se inserta su
  cierre con el cursor en medio; con selección, la envuelve; al teclear el cierre
  justo delante del automático, se «salta» en vez de duplicarlo. Funciona en el
  editor visual y en el de fuente, y no interfiere dentro de una fórmula.

## 2. Gestión de archivos y sesión

- **RF-ARC-01 — Nuevo / Abrir / Guardar / Guardar como.** `Ctrl+N`, `Ctrl+O`,
  `Ctrl+S`, `Ctrl+Shift+S`. Se guarda en UTF-8.
- **RF-ARC-02 — Filtros de apertura.** `*.md`, `*.markdown`, `*.mdown`, `*.mkd`.
- **RF-ARC-03 — Confirmación de cambios sin guardar** antes de cerrar o descartar un
  documento.
- **RF-ARC-04 — Abrir arrastrando y soltando** un archivo sobre la ventana. Si ya
  está abierto, salta a su pestaña.
- **RF-ARC-05 — Archivos recientes.** Submenú *Abrir recientes* con ruta completa en
  el tooltip, descarte automático de los que ya no existen y opción *Borrar la lista*.
- **RF-ARC-06 — Abrir carpeta contenedora** del documento actual en el gestor de
  archivos del sistema.
- **RF-ARC-07 — Front matter.** Si el documento empieza por `---…---` (YAML) o
  `+++…+++` (TOML), se conserva **verbatim** al guardar; no se renderiza ni se
  edita. Al abrir un documento con front matter se informa en la barra de estado.
  Se usa para metadatos (`title`, `lang`/`language`) que aprovecha la exportación.
- **RF-ARC-08 — Arranque de sesión con prioridad.** Al iniciar: **archivo de línea
  de comandos** › **recuperar borrador** (tras cierre anómalo) › **reabrir las
  pestañas de la última sesión**. La apertura se difiere para evitar diálogos
  espurios durante el trazado inicial.
- **RF-ARC-09 — Autoguardado de borrador.** Mientras hay cambios, se autoguarda un
  borrador de recuperación periódicamente (~5 s) en el directorio de datos de la
  aplicación.
- **RF-ARC-10 — Recuperación ante fallos por pestaña.** Cada documento tiene su
  propio borrador (slot único por pestaña). Tras un cierre anómalo, al reabrir se
  ofrece recuperar **todos** los documentos con cambios (indicando nombre y fecha),
  cada uno en su pestaña.
- **RF-ARC-11 — Vigilancia del archivo en disco.** Se vigila el archivo abierto con
  *debounce* e instantánea de bytes (para distinguir el propio guardado, incluido
  el atómico, de un cambio externo): si **no** hay cambios locales, se recarga solo;
  si los hay, se pregunta entre *Recargar* y *Conservar los míos*. La recarga
  preserva aproximadamente la posición del cursor.
- **RF-ARC-12 — Aviso de archivo eliminado/movido** en la barra de estado.
- **RF-ARC-13 — Posición del cursor por archivo.** Se recuerda la posición del
  cursor por documento para reabrir cada uno donde se dejó.
- **RF-ARC-14 — Salir.** `Ctrl+Q`, con confirmación de cambios sin guardar.

## 3. Edición por pestañas (multi-documento)

- **RF-PES-01 — Varios documentos en pestañas.** Cada documento vive en su propia
  pestaña con su estado independiente (modo de vista, modificado, archivo en disco).
- **RF-PES-02 — Crear/Reutilizar pestaña.** Nuevo, Nuevo desde plantilla y Abrir
  crean una pestaña (o reutilizan la pestaña vacía inicial).
- **RF-PES-03 — Cerrar pestaña** (`Ctrl+W`) con confirmación si hay cambios. La
  última pestaña no se cierra: queda como documento nuevo.
- **RF-PES-04 — Navegación entre pestañas.** `Ctrl+AvPág`/`Ctrl+RePág` o
  `Ctrl+Tab`/`Ctrl+Shift+Tab`; reordenar arrastrando.
- **RF-PES-05 — Etiqueta de pestaña** con el nombre del archivo y marca (•) de
  cambios sin guardar.
- **RF-PES-06 — Reapertura de sesión.** Al cerrar se recuerdan los documentos
  abiertos y se reabren todos al volver a arrancar y tras un cambio de idioma.
- **RF-PES-07 — Re-vinculación al activar pestaña.** Al cambiar de pestaña se
  re-vinculan al documento activo: barra de búsqueda, esquema, estado de acciones,
  título, modo de vista, tema/luz cálida y modo sin distracciones (sin salir de él).

## 4. Formato de texto y estructura

- **RF-FMT-01 — Marcas de carácter.** Negrita (`Ctrl+B`), Cursiva (`Ctrl+I`),
  Subrayado (`Ctrl+U`, serializa `_texto_`), Tachado (`Ctrl+Shift+X`), Código en
  línea (`Ctrl+E`), Enlace (`Ctrl+K`).
- **RF-FMT-02 — Aplicación sin selección.** Sin selección, la marca se aplica a la
  palabra bajo el cursor y al texto que se escriba a continuación.
- **RF-FMT-03 — Inhibición en encabezados.** Dentro de un encabezado, las marcas de
  carácter se deshabilitan (no round-trip-ean a Markdown).
- **RF-FMT-04 — Estado reflejado.** Los botones de la barra reflejan el formato
  activo bajo el cursor.
- **RF-FMT-05 — Encabezados H1–H6** (`Ctrl+1`…`Ctrl+6`, toggle).
- **RF-FMT-06 — Listas.** Viñetas (`Ctrl+Shift+U`), numerada (`Ctrl+Shift+O`),
  tareas (`Ctrl+Shift+T`).
- **RF-FMT-07 — Sangría.** Aumentar (`Ctrl+]`) / disminuir (`Ctrl+[`); anida listas
  y citas.
- **RF-FMT-08 — Cita / blockquote** (`Ctrl+Shift+Q`) y **bloque de código**
  (`Ctrl+Shift+K`), gestionados reescribiendo el Markdown del bloque para que
  round-trip-een.
- **RF-FMT-09 — Lenguaje del bloque de código.** *Formato → Lenguaje del bloque…*
  (contextual, con el cursor dentro de un bloque) fija el lenguaje para el resaltado.
- **RF-FMT-10 — Continuación inteligente de listas.** Al pulsar Enter, la lista
  continúa sola (viñetas; numeración que se incrementa; tareas que nacen sin
  marcar). Un ítem vacío sale de la lista.
- **RF-FMT-11 — Casillas de tarea interactivas.** Un clic sobre la casilla marca/
  desmarca el ítem.
- **RF-FMT-12 — Transformar texto** (sobre la selección): MAYÚSCULAS, minúsculas,
  Capitalizar (título), Ordenar líneas.
- **RF-FMT-13 — Tipografía inteligente** (sobre la selección): `--`→–, `---`→—,
  `...`→…, comillas rectas → tipográficas según el contexto.
- **RF-FMT-14 — Pegar como texto plano** (`Ctrl+Shift+V`).
- **RF-FMT-15 — Pegar como Markdown** (`Ctrl+Alt+V`): convierte el HTML del
  portapapeles a Markdown en vez de incrustar el formato del origen.
- **RF-FMT-16 — Copiar como HTML** de la selección (o el documento).
- **RF-FMT-17 — Auto-enlazar al pegar URL** sobre una selección de texto.
- **RF-FMT-18 — Limpiar Markdown** (*Editar → Limpiar Markdown*): normaliza el
  documento de una pasada (viñetas a `-`, recorta espacios finales preservando el
  salto duro, colapsa líneas en blanco de más, uniforma el espacio tras los `#`),
  sin tocar el interior de los bloques de código.
- **RF-FMT-19 — Contador de palabras/caracteres** en la barra de estado (con prefijo
  «Selección:» cuando hay texto seleccionado); conmutable con *Ver → Mostrar
  contador de palabras*.

## 5. Inserción de contenido

- **RF-INS-01 — Enlace…** Diálogo con texto y URL (la selección pasa como texto).
- **RF-INS-02 — Imagen…** Texto alternativo y ruta/URL; las imágenes locales se
  referencian con **ruta relativa** al `.md` cuando es posible.
- **RF-INS-03 — Pegar/soltar imagen.** La imagen del portapapeles (o soltada sobre
  el editor) se **guarda a disco como PNG** junto al `.md` (o se pide ubicación si el
  documento aún no se ha guardado) y se inserta como `![alt](ruta)`; no se incrusta,
  para que sobreviva al round-trip. Pregunta el texto alternativo.
- **RF-INS-04 — Tabla…** Diálogo con nº de columnas y filas; se crea con borde.
- **RF-INS-05 — Regla horizontal.**
- **RF-INS-06 — Índice (TOC).** Inserta una lista anidada de enlaces a los
  encabezados del documento.
- **RF-INS-07 — Fórmula…** (`Ctrl+Shift+F`, ver [§7](#7-fórmulas-tex)).
- **RF-INS-08 — Nota al pie** (`Ctrl+Shift+N`): referencia `[^n]` y su definición.
- **RF-INS-09 — Admonición** (Nota/Consejo/Importante/Advertencia/Precaución).
- **RF-INS-10 — Símbolos especiales…** Diálogo no modal de «mapa de caracteres» por
  categorías; un clic inserta y el diálogo permanece abierto.
- **RF-INS-11 — Fecha / Fecha y hora** en formato local.
- **RF-INS-12 — Apertura de enlaces.** `Ctrl+clic` abre el enlace en la aplicación
  externa correspondiente; al pasar el ratón, el cursor cambia a mano y se muestra
  la URL en la barra de estado. Los enlaces a `.md` locales se abren en una pestaña
  nueva de la misma ventana.

## 6. Tablas

- **RF-TAB-01 — Menú contextual de tabla.** Las acciones se habilitan solo con el
  cursor dentro de una tabla en el editor visual.
- **RF-TAB-02 — Filas.** Insertar fila encima/debajo; eliminar fila (manteniendo al
  menos la cabecera).
- **RF-TAB-03 — Columnas.** Insertar columna a la izquierda/derecha; eliminar columna
  (manteniendo al menos una).
- **RF-TAB-04 — Alinear columna** (izquierda/centro/derecha); la alineación se
  serializa como `:--`/`:-:`/`--:` y se conserva al guardar.

## 7. Fórmulas TeX

- **RF-MAT-01 — Sintaxis.** Soporta fórmulas en línea (`$...$`) y en bloque
  (`$$...$$`) con sintaxis LaTeX (estilo Pandoc/Obsidian/Quarto), **sin dependencias
  externas**.
- **RF-MAT-02 — Insertar** (`Ctrl+Shift+F`): diálogo con campo TeX, selector En
  línea / Bloque y **previsualización en vivo**.
- **RF-MAT-03 — Editar.** Doble clic sobre una fórmula reabre el diálogo precargado y
  la sustituye.
- **RF-MAT-04 — Atomicidad.** En el editor la fórmula es una unidad: teclear dentro
  no la corrompe (recuerda usar doble clic), Backspace/Suprimir en el borde borran el
  grupo entero, pegar sobre ella la reemplaza.
- **RF-MAT-05 — Maquetación 2D real.** Super/subíndices reales, letras griegas,
  operadores, **fracciones apiladas** (`\frac`), **raíces con vínculo** (`\sqrt`,
  `\sqrt[n]`), **binomios** (`\binom`), **matrices/entornos** (`matrix`, `pmatrix`,
  `bmatrix`, `cases`…), **grandes operadores con límites** (`\sum`, `\int`,
  `\prod`…), **acentos** (`\hat`, `\bar`, `\vec`, `\tilde`, `\dot`…), `\mathbb{R}`,
  `\text{…}`/`\mathrm{…}`, nombres de función (`\lim`, `\sin`…), espaciados,
  `\left`/`\right`, `\not`, alfabetos `\mathcal`/`\mathscr`/`\mathfrak`.
- **RF-MAT-06 — Escalado y color.** Las fórmulas en línea usan el color de acento del
  tema y escalan con el zoom.
- **RF-MAT-07 — Multilínea.** Los bloques `$$...$$` pueden abarcar varias líneas en
  la fuente; las `$...$` deben abrir y cerrar en la misma línea.
- **RF-MAT-08 — Round-trip.** En la vista de código se ven como `$...$`/`$$...$$` con
  todos los caracteres TeX intactos.
- **RF-MAT-09 — Robustez del parser.** Un anidamiento extremo no desborda la pila
  (tope de profundidad); el TeX restante se devuelve como texto literal.
- **RF-MAT-10 — Accesibilidad de fórmulas.** La fórmula bajo el cursor se anuncia
  (su TeX) a los lectores de pantalla.

## 8. Modos de vista (fuente y dividida)

- **RF-VIS-01 — WYSIWYG** (por defecto): solo el editor renderizado.
- **RF-VIS-02 — Código fuente Markdown** (`Ctrl+Shift+M`): editor de texto plano a
  pantalla completa, monoespaciado. Las acciones de formato, inserción y tabla se
  deshabilitan mientras el panel de fuente está activo.
- **RF-VIS-03 — Vista dividida** (`Ctrl+Shift+D`): render y código lado a lado, ambos
  editables y sincronizados.
- **RF-VIS-04 — Exclusión mutua.** Vista dividida y fuente a pantalla completa son
  excluyentes.
- **RF-VIS-05 — Sincronización con debounce.** ~250 ms; actualiza **solo el panel sin
  foco**, nunca el que se está editando, preservando scroll y cursor.
- **RF-VIS-06 — Panel activo por foco.** En vista dividida, las acciones/búsqueda
  actúan sobre el panel con foco.

## 9. Navegación y esquema

- **RF-NAV-01 — Panel de esquema (TOC).** Árbol de encabezados acoplable a la
  izquierda, tolerante a saltos de nivel; toggle con **F9**.
- **RF-NAV-02 — Reconstrucción.** Se reconstruye al editar (con debounce) y al
  cargar; muestra «Sin encabezados» cuando procede.
- **RF-NAV-03 — Navegar al encabezado.** Un clic salta al encabezado y devuelve el
  foco al editor.
- **RF-NAV-04 — Reordenar sección.** Arrastrar una entrada del esquema mueve su
  sección entera (encabezado, contenido y subsecciones) sin cambiar el nivel.
- **RF-NAV-05 — Foco al esquema** (**F6**): lleva el foco de teclado al esquema (lo
  muestra si estaba oculto); flechas para recorrer, Enter salta y devuelve el foco al
  editor; F6 de nuevo devuelve el foco al editor.
- **RF-NAV-06 — Ir a encabezado** (`Ctrl+G`): apertura rápida que filtra los
  encabezados al teclear y salta al elegido.

## 10. Buscar y reemplazar

- **RF-BUS-01 — Barra inferior.** Buscar (`Ctrl+F`) y Reemplazar (`Ctrl+H`).
- **RF-BUS-02 — Navegación.** Anterior/Siguiente con vuelta al principio/final;
  `F3` / `Shift+F3` para siguiente/anterior.
- **RF-BUS-03 — Reemplazo.** Reemplazar uno y Reemplazar todo (un único paso de
  deshacer).
- **RF-BUS-04 — Opciones.** Sensibilidad a mayúsculas y palabra completa.
- **RF-BUS-05 — Interacción.** Enter busca/reemplaza, ESC cierra; la selección actual
  se usa como término inicial.
- **RF-BUS-06 — En vista dividida** actúa sobre el panel con foco.

## 11. Resaltado de código

- **RF-COD-01 — Resaltado por lenguaje.** Colorea palabras clave, cadenas, números y
  comentarios según el lenguaje declarado en el bloque.
- **RF-COD-02 — Familias reconocidas (por alias).** Estilo C (C/C++/Java/C#/Go/Rust/
  Swift/Kotlin/PHP/Scala/Dart…), JS/TS/JSX/TSX/JSON, Python, estilo almohadilla
  (Bash/Shell, Ruby, YAML, TOML, R, Perl, Makefile, INI/conf). Lenguaje desconocido:
  resalta cadenas, números y los tres estilos de comentario.
- **RF-COD-03 — Colores según el tema.** El resaltado sigue al tema activo.

## 12. Diagramas (Mermaid / PlantUML)

- **RF-DIA-01 — Previsualización como imagen.** Los bloques ```` ```mermaid ```` y
  ```` ```plantuml ```` se renderizan como imagen en un bloque de previsualización
  **bajo** el bloque de código.
- **RF-DIA-02 — Opcional, sin dependencia enlazada.** El render ejecuta la
  herramienta externa (`mmdc` / `plantuml`) si está instalada.
- **RF-DIA-03 — Round-trip transparente.** La imagen nunca llega al Markdown ni
  cuenta como «modificado».
- **RF-DIA-04 — Degradación elegante.** Si falta la herramienta, en lugar de la
  imagen se muestra un marcador con la **orden de instalación** del sistema
  operativo, sustituido por la imagen en cuanto la herramienta aparece.
- **RF-DIA-05 — Aviso de error.** Si un diagrama no se puede renderizar (sintaxis
  inválida, error de la herramienta), se avisa en la barra de estado tras un margen
  para no molestar al teclear.
- **RF-DIA-06 — Previsualización conmutable.** *Ver → Previsualizar diagramas*
  activa o desactiva el render automático (global, activado por defecto, se
  recuerda). Al desactivarlo se retiran las previsualizaciones de todas las
  pestañas; útil si el documento ya trae una imagen puesta a mano bajo el bloque.

## 13. Corrección ortográfica

- **RF-ORT-01 — Subrayado de erratas** según el idioma del documento, saltando
  código, fórmulas y enlaces. Es **opcional** (requiere Hunspell y diccionarios);
  sin ellos el resto funciona igual.
- **RF-ORT-02 — Activar/desactivar** en *Ver → Corrección ortográfica* (se recuerda).
- **RF-ORT-03 — Idioma de corrección** en *Ver → Idioma de corrección*; por defecto
  por el idioma del documento (front matter › ajuste › locale). Si falta el
  diccionario, se avisa en la barra de estado.
- **RF-ORT-04 — Sugerencias.** Clic derecho sobre una palabra subrayada ofrece
  correcciones, *Añadir al diccionario* (lista personal permanente) e *Ignorar*
  (durante la sesión).

## 14. Extensiones de Markdown

- **RF-EXT-01 — Listas de tareas** `- [ ]` / `- [x]` (ver RF-FMT-11).
- **RF-EXT-02 — Notas al pie** `[^id]` / `[^id]:`: referencia en superíndice, clic
  salta a la definición; Markdown estándar al guardar.
- **RF-EXT-03 — Admoniciones** estilo GitHub (`[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`,
  `[!WARNING]`, `[!CAUTION]`): fondo tintado y título en color; round-trip compatible
  con GitHub.
- **RF-EXT-04 — Shortcodes `:nombre:`** que se expanden al símbolo (`:alpha:`→α).
- **RF-EXT-05 — Front matter** YAML/TOML conservado verbatim (ver RF-ARC-07).

## 15. Exportación e impresión

- **RF-EXP-01 — PDF** (vía `QPrinter`) y **Selección a PDF** (solo el texto
  seleccionado).
- **RF-EXP-02 — HTML.**
- **RF-EXP-03 — ODF (.odt)** con idioma del documento incrustado.
- **RF-EXP-04 — DOCX (.docx)** con serializador OOXML propio; idioma/título
  incrustados, imágenes embebidas.
- **RF-EXP-05 — LaTeX (.tex)** con serializador propio, preámbulo portable (`iftex`
  + `babel`) y fórmulas verbatim (`amsmath`/`amssymb`).
- **RF-EXP-06 — EPUB (.epub)** EPUB 3, con idioma/título e imágenes embebidas.
- **RF-EXP-07 — Idioma del documento** (ODF/DOCX/LaTeX/EPUB): se pregunta al exportar
  (por defecto front matter › ajuste de la app › idioma del sistema).
- **RF-EXP-08 — Fidelidad de fórmulas y código.** Las fórmulas se conservan
  (super/subíndices reales en PDF/HTML/ODF/DOCX/EPUB; verbatim en LaTeX); la
  exportación conserva el **resaltado de sintaxis** de los bloques de código.
- **RF-EXP-09 — Imprimir** (`Ctrl+P`, diálogo del sistema), **Vista previa de
  impresión** e **Imprimir selección**.

## 16. Temas y apariencia

- **RF-TEM-01 — Catálogo de 8 temas** mutuamente exclusivos: Claro, Oscuro, GitHub
  Light, GitHub Dark, Monokai, Alto contraste, Solarized Light, Solarized Dark. Cada
  tema lleva paleta completa, colores de resaltado y color de enlaces.
- **RF-TEM-02 — Persistencia y recoloreado.** El tema se persiste; al cambiarlo se
  recolorean los enlaces, se reajusta el resaltado y se regeneran los iconos
  monocromos de la barra.
- **RF-TEM-03 — Seguir el sistema.** *Ver → Tema → Seguir el sistema* deriva el tema
  claro/oscuro del esquema de color del SO y cambia solo si el SO cambia.
- **RF-TEM-04 — Luz cálida nocturna** (conmutable, **activa por defecto**),
  ortogonal al tema: tinte cálido automático y gradual según la hora del reloj del
  sistema (día 07–19 → nulo; rampa 19→23; noche 23–06 → máximo; rampa 06→07). Se
  reevalúa cada ~60 s; filtro multiplicativo solo sobre el fondo (azul −16 %·w,
  verde −5 %·w, rojo intacto); no altera enlaces ni resaltado. Es global de la
  aplicación (controlador único de ventana).
- **RF-TEM-05 — Interlineado.** *Ver → Interlineado*: Sencillo (100 %), 1,5 líneas
  (150 %), Doble (200 %). Es presentación pura (no se serializa) y se recuerda.

## 17. Modos de concentración

- **RF-CON-01 — Modo sin distracciones** (`F11`; sale también con `ESC`): pantalla
  completa que oculta menú, barras, pestañas, búsqueda y estado; el texto se centra
  en una columna de lectura (~960 px) con márgenes laterales en negro.
- **RF-CON-02 — Esquema en modo sin distracciones.** Si el esquema está visible,
  queda pegado a la columna y el conjunto se centra.
- **RF-CON-03 — Documento único.** Al entrar se abandona la vista dividida y se
  oculta la barra de pestañas; no se cambia de pestaña mientras dura el modo. El
  modo se traslada a la pestaña activa al cambiar de documento (no se desactiva).
- **RF-CON-04 — Persistencia del estado de ventana previo.** Al cerrar la app estando
  en este modo, se persiste el estado de ventana **previo** (normal), no el de
  pantalla completa.
- **RF-CON-05 — Modo foco** (`F12`): un interruptor que activa (a) **máquina de
  escribir** (la línea del cursor se mantiene centrada en vertical) y (b)
  **atenuado** (todo se ve apagado salvo el párrafo del cursor). Funciona en el
  editor visual y en la vista de código, e es independiente del modo sin
  distracciones. Desactivado por defecto y recordado.

## 18. Estadísticas del documento

- **RF-EST-01 — Diálogo de estadísticas.** *Ver → Estadísticas del documento…*
  muestra palabras, caracteres, párrafos, frases y tiempo estimado de lectura, sobre
  todo el documento o sobre la selección.
- **RF-EST-02 — Contador en barra de estado** (ver RF-FMT-19).

## 19. Zoom de toda la interfaz

- **RF-ZOO-01 — Aumentar/Reducir/Normal.** `Ctrl++`/`Ctrl+=`, `Ctrl+-`, `Ctrl+0`;
  también `Ctrl + rueda` sobre el editor.
- **RF-ZOO-02 — Escala toda la interfaz.** No solo el texto del editor: menús (y cada
  desplegable), barras e iconos, barra de búsqueda, barra de estado, editor de fuente
  y panel de esquema.
- **RF-ZOO-03 — Persistencia** del nivel de zoom.

## 20. Internacionalización (función)

- **RF-I18N-01 — Idioma de la interfaz.** *Ver → Idioma*: Automático (sistema),
  Español, English, Deutsch, Français, Italiano, Português, Polski, Nederlands,
  Română (9 idiomas).
- **RF-I18N-02 — Cambio en caliente.** El cambio de idioma se aplica recreando la
  ventana (reabriendo las pestañas); idioma de origen español, respaldo a inglés.
- **RF-I18N-03 — Cobertura total.** Todos los textos visibles están traducidos; los
  plurales son correctos por idioma (polaco y rumano: 3 formas); los atajos en
  tooltips se localizan solos.

## 21. Ayuda

- **RF-AYU-01 — Manual** (`F1`): ventana no modal con dos secciones («Uso de la
  aplicación» y «Markdown»), renderizadas con el motor del editor y localizadas a los
  9 idiomas.
- **RF-AYU-02 — Acerca de** con datos de autor y versión.

## 22. Plantillas y snippets

- **RF-PLA-01 — Nuevo desde plantilla.** *Archivo → Nuevo desde plantilla* crea un
  documento a partir de un esqueleto Markdown (acta, nota diaria, blog, README,
  carta, informe, lista de tareas, certificado, práctica, examen). El documento nace
  marcado como modificado.
- **RF-PLA-02 — Snippets de usuario.** *Insertar → Snippet*: fragmentos de Markdown
  reutilizables que se insertan en el punto del cursor (en WYSIWYG se renderizan, en
  fuente se pegan como Markdown).
- **RF-PLA-03 — Gestionar snippets.** *Insertar → Snippet → Gestionar snippets…*
  permite crear/editar/borrar snippets (nombre + cuerpo); se recuerdan entre
  sesiones y están disponibles en todos los documentos.

## 23. Persistencia de ajustes

- **RF-PER-01 — Ajustes recordados entre sesiones.** Tema, seguir-el-sistema, luz
  cálida nocturna, nivel de zoom, contador de palabras, modo foco, interlineado,
  idioma de la interfaz, geometría y estado de la ventana, posición del divisor de la
  vista dividida, archivos recientes, pestañas/archivos abiertos, último archivo,
  posición del cursor por archivo, estado e idioma del corrector, diccionario
  personal y snippets.
- **RF-PER-02 — Punto único de persistencia.** Todas las claves viven en
  `AppSettings` (fachada sobre `QSettings`); migración del antiguo booleano
  `darkTheme` a la clave de tema.

---

# Requisitos no funcionales

## Portabilidad y plataformas

- **RNF-POR-01 — Multiplataforma.** Funciona en Linux, Windows y macOS desde una base
  de código Qt6 portable, sin `#ifdef Q_OS_*` ni APIs POSIX directas.
- **RNF-POR-02 — Detección de SO sin `#ifdef`.** Las diferencias de plataforma
  (p. ej. orden de instalación de herramientas de diagramas) se resuelven en runtime
  (`QSysInfo`).
- **RNF-POR-03 — Integración de escritorio (Linux).** `.desktop` (categorías
  `Utility;TextEditor;`), asociación MIME `text/markdown`, iconos hicolor PNG/SVG.

## Tecnología y dependencias

- **RNF-TEC-01 — Stack.** Qt 6 ≥ 6.5 (módulos `Widgets`, `PrintSupport`,
  `LinguistTools`, `Test`) y C++17 (GCC 9+, Clang 10+, MSVC 19.20+). CMake ≥ 3.16.
  Las builds de CI/release usan Qt 6.8.2.
- **RNF-TEC-02 — Cabeceras privadas de Qt.** Usa `Qt6::GuiPrivate` (QZip privado)
  para incrustar el idioma en `.odt`/`.docx` y empaquetar `.epub`; requiere las
  cabeceras privadas (`qt6-base-private-dev` en Debian/Ubuntu). API privada: revisar
  al actualizar Qt.
- **RNF-TEC-03 — Sin dependencias externas para el núcleo.** WYSIWYG, round-trip,
  fórmulas TeX, exportación y temas no dependen de librerías de terceros.
- **RNF-TEC-04 — Dependencias opcionales con degradación elegante.** Hunspell
  (corrector) y `mmdc`/`plantuml` (diagramas) son opcionales: sin ellos, el resto
  compila/funciona igual.
- **RNF-TEC-05 — Arquitectura en biblioteca estática.** Toda la lógica vive en
  `md-editor-core`, enlazada por el ejecutable y los tests; el código se organiza por
  componente en subdirectorios de `src/`.
- **RNF-TEC-06 — Lógica pura testeable.** Las funciones puras (sin GUI) se separan de
  la integración para poder probarse aisladas (un `tst_*` por módulo).

## Rendimiento y responsividad

- **RNF-REN-01 — Operaciones costosas con debounce.** Esquema, sincronización de
  vista dividida, vigilancia de disco y refresco de la luz cálida usan debounce/
  temporizadores para no penalizar la escritura.
- **RNF-REN-02 — Menos parpadeo.** Las operaciones que reconstruyen el documento
  (carga, cambio de tema, sincronización) se agrupan en un único trazado.
- **RNF-REN-03 — Render de diagramas asíncrono** (vía `QProcess`, con caché por
  fuente), sin bloquear la interfaz.
- **RNF-REN-04 — Estructuras acotadas.** El mapa de posiciones de cursor por archivo
  está acotado para no crecer sin límite.

## Fiabilidad y robustez

- **RNF-FIA-01 — Round-trip sin pérdida.** El dialecto único de carga y guardado es
  GitHub + `MarkdownNoHTML` (un `<algo>` se trata como texto literal, no como HTML),
  para evitar pérdida de datos.
- **RNF-FIA-02 — Recuperación ante cierre anómalo** (ver RF-ARC-09/10).
- **RNF-FIA-03 — Distinción guardado propio vs. cambio externo** mediante instantánea
  de bytes y debounce, incluidos guardados atómicos.
- **RNF-FIA-04 — Sin desbordamiento de pila** en el parser de fórmulas (tope de
  profundidad RAII).
- **RNF-FIA-05 — Suite de pruebas por módulo** (Qt Test), ejecutada *headless*
  (`QT_QPA_PLATFORM=offscreen`), incluyendo round-trip, golden tests de exportadores
  y **fuzzing** del round-trip.
- **RNF-FIA-06 — Sanitizers en CI.** Build con AddressSanitizer + UndefinedBehavior
  Sanitizer que aborta ante errores de memoria / UB.

## Accesibilidad

- **RNF-ACC-01 — Nombres accesibles** en editor, panel de esquema, campos de
  búsqueda, contador y diálogos.
- **RNF-ACC-02 — Anuncios a lectores de pantalla** de mensajes de estado (guardado,
  «no encontrado», cambios en disco…) y de la fórmula bajo el cursor
  (`QAccessibleAnnouncementEvent`).
- **RNF-ACC-03 — Operación solo con teclado.** Todas las acciones tienen atajo o
  entrada de menú; F10 o Alt abren la barra de menús.
- **RNF-ACC-04 — Baja visión.** Tema de **Alto contraste** real y **zoom** de toda la
  interfaz; el tamaño de letra de partida es el del sistema.
- **RNF-ACC-05 — Foco visible** con el color de selección del tema; orden de
  tabulación revisado en los diálogos.

## Usabilidad

- **RNF-USA-01 — Atajos derivados y localizados.** Los atajos en tooltips se derivan
  del propio atajo (`QKeySequence::NativeText`) para localizarse solos y no
  desincronizarse.
- **RNF-USA-02 — Sin atajos duplicados.** Hay una guardia sistemática (test) contra
  atajos duplicados.
- **RNF-USA-03 — Acciones contextuales.** Las acciones de tabla y las WYSIWYG se
  habilitan/inhiben según el contexto (cursor en tabla, panel de fuente activo,
  encabezado).
- **RNF-USA-04 — Idioma de la interfaz en español por origen**, con interfaz y textos
  íntegramente traducidos a 8 idiomas más.

## Mantenibilidad y calidad

- **RNF-MAN-01 — Análisis estático.** `clang-tidy` (config en `.clang-tidy`) en CI con
  `--warnings-as-errors='*'`: cualquier aviso falla la build.
- **RNF-MAN-02 — Pruebas verdes obligatorias.** CI ejecuta la suite completa en
  Linux/Windows/macOS; el test de traducciones falla si algún `.ts` tiene cadenas sin
  traducir o se desincroniza del código.
- **RNF-MAN-03 — Documentación del código.** Doxygen (`Doxyfile`) sin warnings de
  generación.
- **RNF-MAN-04 — Punto único por responsabilidad.** Persistencia solo en
  `AppSettings`; serialización solo por `mdtable::documentMarkdown`; pipeline de
  carga solo en `mdrender`; añadir una extensión ligera toca un único sitio.
- **RNF-MAN-05 — Flujo de i18n.** `lupdate` (`update_translations`) refresca los `.ts`;
  `md-editor_es.ts` se mantiene `-pluralonly`.

## Seguridad y privacidad

- **RNF-SEG-01 — Local-first.** La aplicación opera sobre archivos locales; no hay
  telemetría ni envío de datos a servicios externos. (La apertura de enlaces y el
  render de diagramas invocan herramientas/aplicaciones externas del sistema a
  petición del usuario.)
- **RNF-SEG-02 — Portabilidad de imágenes.** Las imágenes pegadas se guardan a disco
  con ruta relativa, evitando incrustación opaca.

## Empaquetado y distribución

- **RNF-EMP-01 — Formatos de distribución.** Linux x86_64 **AppImage**, Windows x64
  **ZIP portable** (sin instalador, sin firma), macOS **DMG universal** (arm64 +
  x86_64, sin firmar: primer arranque con Ctrl-clic → Abrir).
- **RNF-EMP-02 — Corrector estático.** Hunspell se enlaza **estático** por defecto en
  las builds de distribución, de modo que el paquete no depende de ninguna
  `.so/.dll/.dylib` de Hunspell.
- **RNF-EMP-03 — Diccionarios empaquetados** en Windows/macOS (Linux usa los del
  sistema); bloque de instalación condicional que no afecta a la build de Linux.
- **RNF-EMP-04 — Instalación en Linux.** `install.sh` copia binario + `.desktop` +
  iconos hicolor a `$PREFIX` (sin sudo si `$PREFIX` es escribible).
- **RNF-EMP-05 — CI/CD por tag.** Releases multiplataforma publicados por tag.

## Legal

- **RNF-LEG-01 — Licencia GPL-3.0** (copyleft fuerte): uso, estudio, modificación y
  redistribución, siempre que las obras derivadas se publiquen también bajo GPL-3.0.
- **RNF-LEG-02 — Política de contribución.** El repositorio no acepta *pull
  requests*; las incidencias se gestionan por *issues*.

---

# Comprobaciones manuales

> Lista de verificación manual derivada de los requisitos anteriores. Sirve como
> guion de prueba de aceptación / *smoke test* antes de una release. Cada
> comprobación referencia el/los requisito(s) que valida.
>
> **Cómo usarla.** Marca `[x]` lo que pasa. Salvo que se indique, todo se prueba en
> el **ejecutable instalado** (no solo en `build/`, que no se actualiza al
> reinstalar). Útil empezar con un documento de ejemplo rico (p. ej.
> `ejemplos/prueba-completa.md`).
>
> **Versión objetivo:** 2.8.0 · **Plataforma probada:** ____________ · **Fecha:** __________

## 0. Preparación

- [ ] Compila limpio: `cmake -S . -B build && cmake --build build` sin errores.
- [ ] Tests verdes: `ctest --test-dir build --output-on-failure`.
- [ ] (Opcional, robustez) Build con sanitizers y tests verdes:
      `cmake -S . -B build-san -DENABLE_SANITIZERS=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build build-san`,
      `ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-san --output-on-failure`.
- [ ] (Opcional) `clang-tidy -p build src/*.cpp` sin avisos.
- [ ] Arranca el ejecutable: `./build/md-editor`.

## C1. Edición WYSIWYG y round-trip · _RF-EDI-\*_

- [ ] Al abrir un `.md`, se ve **renderizado** (sin sintaxis Markdown a la vista). _(RF-EDI-01)_
- [ ] Abrir un documento rico y **Guardar** sin tocar nada **no cambia** el archivo
      (round-trip estable): tablas con alineación, citas, listas anidadas, tareas,
      código, notas al pie, admoniciones y fórmulas intactas. _(RF-EDI-02, RF-EDI-03)_
- [ ] El indicador de **modificado** (•/`[*]`) aparece solo con cambios reales y
      desaparece al guardar; abrir-y-guardar sin tocar no lo activa. _(RF-EDI-04)_
- [ ] Deshacer/Rehacer funcionan (`Ctrl+Z` / `Ctrl+Y` / `Ctrl+Shift+Z`). _(RF-EDI-05)_
- [ ] Auto-emparejado: teclear `(`, `[`, `{`, `` ` `` cierra el par; con selección la
      envuelve; teclear el cierre delante del automático lo «salta». _(RF-EDI-07)_

## C2. Archivos y sesión · _RF-ARC-\*_

- [ ] Nuevo / Abrir / Guardar / Guardar como (atajos) funcionan; se guarda en UTF-8. _(RF-ARC-01)_
- [ ] El diálogo de apertura filtra `*.md`, `*.markdown`, `*.mdown`, `*.mkd`. _(RF-ARC-02)_
- [ ] Cerrar/descartar con cambios sin guardar **pide confirmación**. _(RF-ARC-03)_
- [ ] Arrastrar y soltar un `.md` sobre la ventana lo abre; si ya estaba abierto,
      salta a su pestaña. _(RF-ARC-04)_
- [ ] *Abrir recientes* lista los últimos, con ruta en el tooltip; descarta los
      inexistentes; *Borrar la lista* la vacía. _(RF-ARC-05)_
- [ ] *Abrir carpeta contenedora* abre el gestor de archivos en la carpeta del doc. _(RF-ARC-06)_
- [ ] Un documento con front matter `---…---`/`+++…+++`: no se ve/edita, se avisa en
      estado y se **conserva verbatim** al guardar. _(RF-ARC-07)_
- [ ] Pasar un archivo por línea de comandos lo abre con prioridad sobre la sesión. _(RF-ARC-08)_
- [ ] **Recuperación:** con cambios sin guardar, matar el proceso (p. ej. `kill -9`)
      y reabrir → ofrece recuperar, con nombre y fecha; con varias pestañas sucias,
      ofrece **todas**. _(RF-ARC-09, RF-ARC-10)_
- [ ] **Vigilancia de disco:** editar el archivo desde fuera. Sin cambios locales →
      se recarga solo; con cambios locales → pregunta *Recargar / Conservar los míos*. _(RF-ARC-11)_
- [ ] Borrar/mover el archivo abierto → aviso en la barra de estado. _(RF-ARC-12)_
- [ ] Reabrir un documento deja el cursor donde se quedó. _(RF-ARC-13)_

## C3. Pestañas · _RF-PES-\*_

- [ ] Abrir varios documentos: cada uno en su pestaña, con estado propio. _(RF-PES-01)_
- [ ] Cerrar pestaña (`Ctrl+W`) pide confirmación si hay cambios; la **última** no se
      cierra (queda como documento nuevo). _(RF-PES-03)_
- [ ] `Ctrl+AvPág`/`Ctrl+RePág` y `Ctrl+Tab`/`Ctrl+Shift+Tab` cambian de pestaña;
      arrastrar reordena. _(RF-PES-04)_
- [ ] La etiqueta muestra el nombre y • si hay cambios. _(RF-PES-05)_
- [ ] Cerrar la app con varias pestañas y reabrir → se reabren todas. _(RF-PES-06)_
- [ ] Al cambiar de pestaña, búsqueda/esquema/estado de acciones/título/modo de vista
      se re-vinculan al documento activo. _(RF-PES-07)_

## C4. Formato y estructura · _RF-FMT-\*_

- [ ] Negrita/Cursiva/Subrayado/Tachado/Código/Enlace (atajos); subrayado guarda
      `_texto_`. _(RF-FMT-01)_
- [ ] Sin selección, la marca afecta a la palabra bajo el cursor y a lo que sigue. _(RF-FMT-02)_
- [ ] Dentro de un encabezado, las marcas de carácter están **deshabilitadas**. _(RF-FMT-03)_
- [ ] Los botones de la barra reflejan el formato bajo el cursor. _(RF-FMT-04)_
- [ ] Encabezados H1–H6 (`Ctrl+1…6`) como toggle. _(RF-FMT-05)_
- [ ] Listas viñetas/numerada/tareas (`Ctrl+Shift+U/O/T`); sangrar/desangrar
      (`Ctrl+]`/`Ctrl+[`); cita (`Ctrl+Shift+Q`) y bloque de código (`Ctrl+Shift+K`)
      round-trip-ean. _(RF-FMT-06, RF-FMT-07, RF-FMT-08)_
- [ ] Con el cursor en un bloque de código, *Formato → Lenguaje del bloque…* fija el
      lenguaje. _(RF-FMT-09)_
- [ ] Continuación de listas con Enter (viñetas; números que incrementan; tareas sin
      marcar); ítem vacío sale de la lista. _(RF-FMT-10)_
- [ ] Clic sobre la casilla de una tarea la marca/desmarca. _(RF-FMT-11)_
- [ ] Transformar texto: MAYÚSCULAS/minúsculas/Capitalizar/Ordenar líneas. _(RF-FMT-12)_
- [ ] Tipografía inteligente: `--`→–, `---`→—, `...`→…, comillas rectas → tipográficas. _(RF-FMT-13)_
- [ ] Pegar como texto plano (`Ctrl+Shift+V`); Pegar como Markdown (`Ctrl+Alt+V`)
      convierte HTML→Markdown; Copiar como HTML. _(RF-FMT-14, RF-FMT-15, RF-FMT-16)_
- [ ] Pegar una URL sobre texto seleccionado lo auto-enlaza. _(RF-FMT-17)_
- [ ] *Editar → Limpiar Markdown* normaliza (viñetas a `-`, espacios finales, líneas
      en blanco, espacio tras `#`) **sin** tocar el interior de los fences. _(RF-FMT-18)_
- [ ] Contador de palabras/caracteres en estado; prefijo «Selección:» con selección;
      conmutable con *Ver → Mostrar contador*. _(RF-FMT-19)_

## C5. Inserción · _RF-INS-\*_

- [ ] Insertar Enlace / Imagen (imagen local con **ruta relativa**). _(RF-INS-01, RF-INS-02)_
- [ ] Pegar/soltar imagen → se guarda **PNG** junto al `.md` (o pide ubicación si no
      está guardado) e inserta `![alt](ruta)`; pregunta el alt. _(RF-INS-03)_
- [ ] Insertar Tabla (filas/columnas) con borde; Regla horizontal; Índice (TOC). _(RF-INS-04, RF-INS-05, RF-INS-06)_
- [ ] Insertar Nota al pie (`Ctrl+Shift+N`); Admonición; Símbolos especiales (no
      modal, inserta sin cerrar); Fecha / Fecha y hora. _(RF-INS-08…11)_
- [ ] `Ctrl+clic` abre el enlace; hover muestra la URL en estado y cursor en mano; un
      enlace a `.md` local abre **una pestaña nueva** en la misma ventana. _(RF-INS-12)_

## C6. Tablas · _RF-TAB-\*_

- [ ] Las acciones del menú **Tabla** solo se activan con el cursor dentro de una tabla. _(RF-TAB-01)_
- [ ] Insertar/eliminar fila y columna (no se baja de cabecera + 1 columna). _(RF-TAB-02, RF-TAB-03)_
- [ ] Alinear columna izq./centro/der. → al guardar aparece `:--`/`:-:`/`--:` y se
      conserva al reabrir. _(RF-TAB-04)_

## C7. Fórmulas TeX · _RF-MAT-\*_

- [ ] Insertar fórmula (`Ctrl+Shift+F`) con **previsualización en vivo**; En línea / Bloque. _(RF-MAT-02)_
- [ ] Doble clic sobre una fórmula reabre el diálogo precargado y la sustituye. _(RF-MAT-03)_
- [ ] Atomicidad: teclear dentro no la corrompe; Backspace/Suprimir en el borde borra
      el grupo; pegar encima la reemplaza. _(RF-MAT-04)_
- [ ] Render 2D: `\frac`, `\sqrt`, `\binom`, matrices/`cases`, `\sum`/`\int` con
      límites, acentos, griego, super/subíndices. _(RF-MAT-05)_
- [ ] Las fórmulas en línea toman el color de acento del tema y escalan con el zoom. _(RF-MAT-06)_
- [ ] Un `$$...$$` multilínea en la fuente se renderiza bien y round-trip-ea. _(RF-MAT-07, RF-MAT-08)_
- [ ] Una fórmula muy anidada (`\frac{\frac{…}}`) **no** cierra la app. _(RF-MAT-09)_
- [ ] Con lector de pantalla, al situar el cursor en una fórmula se anuncia su TeX. _(RF-MAT-10)_

## C8. Modos de vista · _RF-VIS-\*_

- [ ] Vista de código (`Ctrl+Shift+M`): texto plano monoespaciado a pantalla completa;
      formato/inserción/tabla deshabilitados. _(RF-VIS-02)_
- [ ] Vista dividida (`Ctrl+Shift+D`): render y código a la vez, sincronizados. _(RF-VIS-03)_
- [ ] Split y fuente-completo son **excluyentes**. _(RF-VIS-04)_
- [ ] Al escribir en un panel, el otro se actualiza tras ~250 ms **sin** mover el
      cursor del panel que editas. _(RF-VIS-05)_
- [ ] En split, las acciones/búsqueda actúan sobre el panel con foco. _(RF-VIS-06)_

## C9. Navegación y esquema · _RF-NAV-\*_

- [ ] F9 muestra/oculta el esquema; se actualiza al editar; «Sin encabezados» cuando
      procede. _(RF-NAV-01, RF-NAV-02)_
- [ ] Clic en una entrada salta al encabezado y devuelve el foco al editor. _(RF-NAV-03)_
- [ ] Arrastrar una entrada **reordena** su sección entera sin cambiar el nivel. _(RF-NAV-04)_
- [ ] F6 lleva el foco al esquema (lo muestra si estaba oculto); flechas + Enter
      navegan; F6 de nuevo vuelve al editor. _(RF-NAV-05)_
- [ ] `Ctrl+G` filtra encabezados al teclear y salta al elegido. _(RF-NAV-06)_

## C10. Buscar y reemplazar · _RF-BUS-\*_

- [ ] `Ctrl+F` / `Ctrl+H` abren la barra; `F3`/`Shift+F3` siguiente/anterior con
      vuelta al principio/final. _(RF-BUS-01, RF-BUS-02)_
- [ ] Reemplazar uno y Reemplazar todo (un solo deshacer). _(RF-BUS-03)_
- [ ] Opciones de caso y palabra completa funcionan. _(RF-BUS-04)_
- [ ] Enter busca/reemplaza, ESC cierra; la selección se usa como término inicial. _(RF-BUS-05)_

## C11. Resaltado de código · _RF-COD-\*_

- [ ] Bloques con lenguaje (`c++`, `python`, `bash`, `json`, `yaml`…) se resaltan
      (palabras clave, cadenas, números, comentarios). _(RF-COD-01, RF-COD-02)_
- [ ] Al cambiar de tema, los colores del código se actualizan. _(RF-COD-03)_

## C12. Diagramas · _RF-DIA-\*_

- [ ] Con `mmdc`/`plantuml` instalado: un bloque ```` ```mermaid ````/```` ```plantuml ````
      muestra la imagen **debajo**; el código sigue editable. _(RF-DIA-01, RF-DIA-02)_
- [ ] La imagen **no** llega al Markdown guardado ni marca «modificado». _(RF-DIA-03)_
- [ ] Sin la herramienta: aparece el marcador con la **orden de instalación** del SO;
      al instalarla, se sustituye por la imagen. _(RF-DIA-04)_
- [ ] Un diagrama con sintaxis inválida avisa en la barra de estado. _(RF-DIA-05)_
- [ ] *Ver → Previsualizar diagramas* desactivado retira las imágenes de todas las
      pestañas y no vuelve a renderizar; al reactivarlo, reaparecen. _(RF-DIA-06)_

## C13. Corrección ortográfica · _RF-ORT-\*_ (requiere Hunspell)

- [ ] Las erratas se subrayan según el idioma del documento; no marca código,
      fórmulas ni enlaces. _(RF-ORT-01)_
- [ ] *Ver → Corrección ortográfica* activa/desactiva (se recuerda). _(RF-ORT-02)_
- [ ] *Ver → Idioma de corrección* fija el idioma; si falta el diccionario, avisa. _(RF-ORT-03)_
- [ ] Clic derecho ofrece sugerencias, *Añadir al diccionario* e *Ignorar*. _(RF-ORT-04)_

## C14. Extensiones de Markdown · _RF-EXT-\*_

- [ ] Notas al pie: referencia en superíndice, clic salta a la definición; Markdown
      estándar al guardar. _(RF-EXT-02)_
- [ ] Admoniciones (`[!NOTE]`…) con fondo y título en color; round-trip GitHub. _(RF-EXT-03)_
- [ ] Shortcode `:alpha:` se expande a α al teclear. _(RF-EXT-04)_

## C15. Exportación e impresión · _RF-EXP-\*_

- [ ] Exportar a **PDF**, **HTML**, **ODF**, **DOCX**, **LaTeX**, **EPUB** produce un
      archivo abrible. _(RF-EXP-01…06)_
- [ ] **Selección a PDF** e **Imprimir selección** usan solo lo seleccionado. _(RF-EXP-01, RF-EXP-09)_
- [ ] ODF/DOCX/LaTeX/EPUB: el diálogo de idioma propone el valor correcto (front
      matter › ajuste › sistema) y queda incrustado. _(RF-EXP-07)_
- [ ] En el exportado, las **fórmulas** se conservan y los **bloques de código** salen
      coloreados. _(RF-EXP-08)_
- [ ] `Ctrl+P` abre el diálogo del sistema; Vista previa de impresión funciona. _(RF-EXP-09)_

## C16. Temas y apariencia · _RF-TEM-\*_

- [ ] Los 8 temas (Claro, Oscuro, GitHub Light/Dark, Monokai, Alto contraste,
      Solarized Light/Dark) se aplican; al cambiar, enlaces, resaltado e iconos de la
      barra se actualizan. _(RF-TEM-01, RF-TEM-02)_
- [ ] *Seguir el sistema* ajusta claro/oscuro al del SO. _(RF-TEM-03)_
- [ ] **Luz cálida nocturna** (activa por defecto): de noche el fondo se ve cálido;
      conmutar la apaga/enciende; persiste; no altera enlaces ni resaltado.
      _(Para forzarlo, cambiar la hora del sistema a, p. ej., 02:00.)_ _(RF-TEM-04)_
- [ ] *Ver → Interlineado* (Sencillo / 1,5 líneas / Doble) cambia el alto de línea
      en pantalla, no el Markdown; se recuerda. _(RF-TEM-05)_

## C17. Concentración · _RF-CON-\*_

- [ ] F11 entra en modo sin distracciones (oculta menú/barras/pestañas/estado, columna
      centrada, márgenes negros); ESC/F11 salen. _(RF-CON-01)_
- [ ] Con el esquema visible, queda pegado a la columna centrada. _(RF-CON-02)_
- [ ] En modo sin distracciones, al cambiar de documento el modo **se traslada** (no se
      desactiva); cerrar la app en este modo persiste el estado de ventana **normal**. _(RF-CON-03, RF-CON-04)_
- [ ] F12 (modo foco): la línea del cursor se mantiene centrada y el resto se atenúa
      salvo el párrafo activo; funciona en visual y en código; independiente de F11;
      se recuerda. _(RF-CON-05)_

## C18. Estadísticas · _RF-EST-\*_

- [ ] *Ver → Estadísticas del documento…* muestra palabras, caracteres, párrafos,
      frases y tiempo de lectura, del documento o de la selección. _(RF-EST-01)_

## C19. Zoom · _RF-ZOO-\*_

- [ ] `Ctrl++`/`Ctrl+-`/`Ctrl+0` y `Ctrl + rueda` escalan **toda la interfaz** (menús
      y desplegables, barras, iconos, búsqueda, estado, fuente, esquema). _(RF-ZOO-01, RF-ZOO-02)_
- [ ] El nivel de zoom se conserva al reiniciar. _(RF-ZOO-03)_

## C20. Internacionalización · _RF-I18N-\*_

- [ ] *Ver → Idioma* ofrece Automático + 9 idiomas; al cambiar, la interfaz se recrea
      y reabre las pestañas, ya traducida. _(RF-I18N-01, RF-I18N-02)_
- [ ] Revisar que no quedan textos sin traducir en el idioma elegido (los plurales y
      tooltips de atajos correctos). _(RF-I18N-03)_

## C21. Ayuda · _RF-AYU-\*_

- [ ] F1 abre el manual no modal con «Uso de la aplicación» y «Markdown», en el idioma
      de la interfaz; los enlaces internos navegan. _(RF-AYU-01)_
- [ ] *Ayuda → Acerca de* muestra autor y versión. _(RF-AYU-02)_

## C22. Plantillas y snippets · _RF-PLA-\*_

- [ ] *Archivo → Nuevo desde plantilla* crea el esqueleto elegido, marcado como
      modificado. _(RF-PLA-01)_
- [ ] *Insertar → Snippet* inserta en el cursor (renderizado en WYSIWYG, Markdown en
      fuente); *Gestionar snippets…* crea/edita/borra; se recuerdan entre sesiones. _(RF-PLA-02, RF-PLA-03)_

## C23. Persistencia · _RF-PER-\*_

- [ ] Tras reiniciar se recuerdan: tema, seguir-sistema, luz cálida, zoom, contador,
      modo foco, interlineado, idioma, geometría/estado de ventana, divisor de split,
      recientes, pestañas abiertas, posición de cursor por archivo, estado/idioma del
      corrector, diccionario personal y snippets. _(RF-PER-01)_

## C24. Requisitos no funcionales · _RNF-\*_

- [ ] **Portabilidad:** la misma versión arranca y opera en las tres plataformas
      objetivo del entorno disponible. _(RNF-POR-01)_
- [ ] **Integración Linux:** `.desktop`, asociación MIME `text/markdown` e iconos
      hicolor presentes tras `install.sh`. _(RNF-POR-03, RNF-EMP-04)_
- [ ] **Dependencias opcionales:** compilar **sin** Hunspell y **sin** `mmdc`/
      `plantuml` → el resto funciona; el corrector/diagramas degradan con elegancia. _(RNF-TEC-04, RNF-FIA-01)_
- [ ] **Rendimiento:** escribir en un documento grande es fluido; el esquema y la
      sincronización de split no causan saltos de cursor; sin parpadeo notable al
      cargar/cambiar de tema. _(RNF-REN-01, RNF-REN-02)_
- [ ] **Robustez round-trip:** `tst_markdownroundtrip`, `tst_roundtripfuzz` y los
      golden tests pasan; un `<algo>` y un code span con `\`/`&` no se corrompen al
      guardar. _(RNF-FIA-01, RNF-FIA-05)_
- [ ] **Sanitizers:** la suite pasa bajo ASan+UBSan. _(RNF-FIA-06)_
- [ ] **Accesibilidad:** con un lector de pantalla, editor/esquema/búsqueda anuncian
      nombre; los mensajes de estado se anuncian; toda acción es alcanzable por
      teclado (F10/Alt abren menús); el tema Alto contraste y el zoom ayudan a baja
      visión. _(RNF-ACC-01…04)_
- [ ] **Calidad:** CI verde en las tres plataformas; `clang-tidy` sin avisos;
      `tst_translations` sin cadenas `unfinished`; guardia de atajos duplicados pasa. _(RNF-MAN-01, RNF-MAN-02, RNF-USA-02)_
- [ ] **Empaquetado:** AppImage (Linux), ZIP (Windows), DMG universal (macOS); en los
      paquetes, `ldd`/equivalente **no** muestra Hunspell (enlace estático); los
      diccionarios viajan en Windows/macOS. _(RNF-EMP-01, RNF-EMP-02, RNF-EMP-03)_
- [ ] **Privacidad:** sin telemetría ni conexiones de red no solicitadas (solo abrir
      enlaces / lanzar herramientas a petición del usuario). _(RNF-SEG-01)_
- [ ] **Legal:** `LICENSE` GPL-3.0 presente; README documenta la política de
      contribución. _(RNF-LEG-01, RNF-LEG-02)_

### Resultado

- Comprobaciones superadas: ____ / ____
- Incidencias abiertas (id de requisito + descripción):
  - …

---

# Apéndice A — Atajos de teclado

| Atajo | Acción |
|---|---|
| `Ctrl+N` / `Ctrl+O` / `Ctrl+S` | Nuevo / Abrir / Guardar |
| `Ctrl+Shift+S` | Guardar como |
| `Ctrl+W` | Cerrar pestaña |
| `Ctrl+AvPág` / `Ctrl+RePág` (o `Ctrl+Tab` / `Ctrl+Shift+Tab`) | Pestaña siguiente / anterior |
| `Ctrl+P` | Imprimir |
| `Ctrl+Q` | Salir |
| `Ctrl+Z` / `Ctrl+Y` (o `Ctrl+Shift+Z`) | Deshacer / Rehacer |
| `Ctrl+Shift+V` / `Ctrl+Alt+V` | Pegar como texto plano / como Markdown |
| `Ctrl+F` / `Ctrl+H` | Buscar / Reemplazar |
| `F3` / `Shift+F3` | Buscar siguiente / anterior |
| `Ctrl+B` / `Ctrl+I` / `Ctrl+U` | Negrita / Cursiva / Subrayado |
| `Ctrl+Shift+X` / `Ctrl+E` / `Ctrl+K` | Tachado / Código en línea / Enlace |
| `Ctrl+1` … `Ctrl+6` | Encabezados H1–H6 |
| `Ctrl+Shift+U` / `Ctrl+Shift+O` / `Ctrl+Shift+T` | Lista viñetas / numerada / tareas |
| `Ctrl+]` / `Ctrl+[` | Aumentar / disminuir sangría |
| `Ctrl+Shift+Q` / `Ctrl+Shift+K` | Cita / Bloque de código |
| `Ctrl+Shift+F` / `Ctrl+Shift+N` | Insertar fórmula / nota al pie |
| `Ctrl+Shift+M` / `Ctrl+Shift+D` | Vista de código fuente / dividida |
| `F11` | Modo sin distracciones (ESC para salir) |
| `F12` | Modo foco |
| `F9` / `F6` | Esquema (mostrar/ocultar) / Foco al esquema |
| `Ctrl+G` | Ir a encabezado |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom (también `Ctrl + rueda`) |
| `F1` | Manual |
</content>
</invoke>
