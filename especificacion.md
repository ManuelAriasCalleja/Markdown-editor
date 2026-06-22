# Especificación de producto — md-editor

> Editor/visor **WYSIWYG** de Markdown en **Qt6 + C++17**. Por defecto editas sobre
> el texto ya renderizado, sin lidiar con la sintaxis; pero opcionalmente puedes ver
> el código Markdown e incluso tener código y renderizado en paralelo. Al guardar se
> serializa siempre a Markdown limpio.

- **Versión:** 2.0.0
- **Autor:** Manuel Arias Calleja
- **Licencia:** GPL-3.0 (software libre con copyleft fuerte: uso, estudio, modificación y redistribución, siempre que las obras derivadas se publiquen también bajo GPL-3.0)
- **Plataformas:** Linux, Windows, macOS
- **Idiomas de interfaz:** 9 (español + inglés, alemán, francés, italiano, portugués, polaco, neerlandés, rumano)

Este documento describe **todas** las características del producto. Es la fuente a
partir de la cual se redacta la wiki. El código fuente (`src/`) es la fuente de
verdad última; aquí se recoge el comportamiento observable de cara al usuario.

---

## 1. Filosofía y modelo de edición

- **Edición WYSIWYG real.** El editor principal muestra el documento ya renderizado;
  por defecto no se ve la sintaxis Markdown. El formato se aplica con formatos
  nativos de Qt que serializan limpiamente a Markdown.
- **Round-trip fiel.** «Lo que abres es lo que guardas»: el documento se serializa
  de vuelta a Markdown limpio en UTF-8, conservando tablas con alineación, citas,
  listas anidadas, listas de tareas, bloques de código, notas al pie, admoniciones y
  fórmulas.
- **El código es opcional, no obligatorio.** Quien quiera ver o editar el Markdown
  crudo puede hacerlo (vista de fuente a pantalla completa o vista dividida con
  render y código en paralelo, §4).
- **Marca «modificado».** El indicador `[*]` del título compara la serialización
  canónica actual con una línea base, no el estado interno de Qt (que se ensucia de
  forma espuria al trazar). Refleja cambios reales del contenido.

---

## 2. Gestión de archivos y sesión

### Edición multi-archivo por pestañas
- Cada documento abierto vive en su propia **pestaña**; se pueden tener varios
  abiertos a la vez y cambiar entre ellos. Cada pestaña conserva su propio estado
  (modo de vista, modificado, archivo en disco, etc.).
- **Cerrar pestaña** con `Ctrl+W` (pide confirmación si hay cambios sin guardar).
- La **sesión** reabre todas las pestañas que estaban abiertas al arrancar y tras un
  cambio de idioma.

### Operaciones de archivo
| Acción | Atajo |
|---|---|
| Nuevo | `Ctrl+N` |
| Nuevo desde plantilla | (submenú) |
| Abrir… | `Ctrl+O` |
| Guardar | `Ctrl+S` |
| Guardar como… | `Ctrl+Shift+S` |
| Cerrar pestaña | `Ctrl+W` |
| Salir | `Ctrl+Q` |

- Filtros de apertura: `*.md`, `*.markdown`, `*.mdown`, `*.mkd`.
- **Confirmación de cambios sin guardar** antes de cerrar o descartar un documento.
- **Abrir arrastrando y soltando** un archivo sobre la ventana.

### Plantillas de documento
- **Archivo → Nuevo desde plantilla** crea un documento a partir de un esqueleto
  Markdown ya preparado (carta, acta, artículo, etc.). El documento nace marcado como
  modificado para que no se pierda sin avisar.

### Archivos recientes
- Submenú **Abrir recientes** con los últimos documentos, ruta completa en el
  *tooltip*, descarte automático de los que ya no existen y opción **Borrar la lista**.

### Front matter
- Si el documento empieza por un bloque `---…---` (YAML) o `+++…+++` (TOML), se
  conserva **verbatim** al guardar: no se renderiza ni se edita en el editor visual.
- Sirve para metadatos como `title` y `lang`/`language`, que la exportación
  aprovecha (§9). Al abrir un documento con front matter se informa en la barra de estado.

### Arranque de sesión
Al iniciar, la prioridad es: **archivo de línea de comandos** › **recuperar
borrador** (si hubo un cierre anómalo) › **reabrir las pestañas de la última
sesión**. La apertura se difiere un instante para evitar diálogos espurios durante el
trazado inicial.

### Autoguardado y recuperación ante fallos
- **Borrador de recuperación** autoguardado cada ~5 s mientras hay cambios, en el
  directorio de datos de la aplicación.
- Tras un cierre anómalo, al reabrir la aplicación **ofrece recuperar** lo escrito,
  indicando nombre y fecha del borrador.

### Vigilancia del archivo en disco
- Se vigila el archivo abierto (con *debounce* e instantánea de bytes para
  distinguir el propio guardado de un cambio externo, incluidos guardados atómicos).
- Si el archivo cambia fuera de la aplicación: **si no hay cambios locales**, se
  recarga solo; **si los hay**, se pregunta entre *Recargar* y *Conservar los míos*.
- Si el archivo se elimina o se mueve, se avisa en la barra de estado. La recarga
  conserva aproximadamente la posición del cursor.

---

## 3. Formato de texto y estructura

Acciones disponibles desde el menú **Formato** y la barra de formato; los botones
reflejan el formato activo bajo el cursor.

### Marcas de carácter
| Acción | Atajo | Notas |
|---|---|---|
| Negrita | `Ctrl+B` | |
| Cursiva | `Ctrl+I` | |
| Subrayado | `Ctrl+U` | serializa como `_texto_` |
| Tachado | `Ctrl+Shift+X` | |
| Código en línea | `Ctrl+E` | |
| Enlace | `Ctrl+K` | |

- Sin selección, la marca se aplica a la palabra bajo el cursor y al texto que se
  escriba a continuación.
- Dentro de un **encabezado**, las marcas de carácter se deshabilitan (no
  round-trip-ean a Markdown).

### Encabezados, listas y bloques
| Acción | Atajo |
|---|---|
| Encabezados H1–H6 | `Ctrl+1` … `Ctrl+6` (toggle) |
| Lista de viñetas | `Ctrl+Shift+U` |
| Lista numerada | `Ctrl+Shift+O` |
| Lista de tareas (checkbox) | `Ctrl+Shift+T` |
| Aumentar sangría | `Ctrl+]` |
| Disminuir sangría | `Ctrl+[` |
| Cita / blockquote | `Ctrl+Shift+Q` |
| Bloque de código | `Ctrl+Shift+K` |
| Lenguaje del bloque… | (contextual, dentro de un bloque de código) |

- **Continuación inteligente de listas**: al pulsar Enter, la lista continúa sola
  (viñetas, numeración que se incrementa, tareas que nacen sin marcar). Un ítem
  vacío sale de la lista. Las listas de tareas se marcan/desmarcan con un clic sobre
  la casilla.
- Citas y bloques de código se gestionan reescribiendo el Markdown del bloque, de
  modo que round-trip-ean correctamente.

### Pegar inteligente
- **Pegar como texto plano** (`Ctrl+Shift+V`): inserta el portapapeles sin formato.
- **Pegar como Markdown** (`Ctrl+Alt+V`): convierte el HTML del portapapeles a
  Markdown en vez de incrustar el formato del origen.
- Al pegar una **URL** sobre una selección, se auto-enlaza el texto seleccionado.

### Transformar texto (sobre la selección)
Submenú **Editar → Transformar texto**:
- **MAYÚSCULAS**, **minúsculas**, **Capitalizar** (título).
- **Ordenar líneas**.
- **Tipografía inteligente**: convierte `--`, `---`, `...` y las comillas rectas en
  sus formas tipográficas (–, —, …, « » / " ").

### Edición general
- Deshacer `Ctrl+Z`, Rehacer `Ctrl+Y` / `Ctrl+Shift+Z`.
- **Contador de palabras y caracteres** en la barra de estado (con prefijo
  «Selección:» cuando hay texto seleccionado).

---

## 4. Modos de vista

Tres modos, mutuamente compatibles según las reglas siguientes:

- **WYSIWYG** (por defecto): solo el editor renderizado.
- **Código fuente Markdown** — `Ctrl+Shift+M`: editor de texto plano a pantalla
  completa con el Markdown crudo en fuente monoespaciada. Las acciones de formato,
  inserción y tabla se deshabilitan mientras el panel de fuente está activo.
- **Vista dividida** — `Ctrl+Shift+D`: render y código **lado a lado**, ambos
  editables y **sincronizados** (lo que escribes en uno se refleja en el otro).

Reglas y comportamiento:
- La vista dividida y el modo fuente a pantalla completa son **excluyentes**.
- La sincronización tiene *debounce* (~250 ms) y actualiza **solo el panel sin
  foco**, nunca el que estás editando, preservando scroll y cursor.
- En vista dividida, el panel «activo» (al que se aplican las acciones, la búsqueda,
  etc.) lo determina el **foco**.

---

## 5. Esquema del documento (índice / TOC)

- Panel lateral izquierdo acoplable con el árbol de encabezados (H1 ▸ H2 ▸ H3…),
  tolerante a saltos de nivel.
- Toggle con **F9**.
- Se reconstruye al editar (con *debounce*) y al cargar. Un clic navega al
  encabezado y devuelve el foco al editor. Muestra «Sin encabezados» cuando procede.
  La sección se puede reordenar arrastrándola en el árbol.
- **Ir a encabezado** — `Ctrl+G`: apertura rápida (*quick open*) que filtra los
  encabezados al teclear y salta al elegido.

---

## 6. Modo sin distracciones

- Toggle con **F11**; se sale también con **ESC**.
- Pantalla completa que oculta menú, barras de herramientas, barra de pestañas, barra
  de búsqueda y barra de estado.
- El texto se centra en una **columna de lectura** (≈960 px), con los márgenes
  laterales en negro para minimizar la distracción.
- Si el esquema está visible, queda pegado a la columna y el conjunto se centra.
- Es de documento único y columna única: al entrar se abandona la vista dividida y se
  oculta la barra de pestañas (no se cambia de pestaña mientras dura el modo). Al
  cerrar la aplicación estando en este modo, se persiste el estado de ventana
  **previo** (normal, con barras), no el de pantalla completa.

---

## 7. Temas y cuidado visual

### Temas
Seis temas en un catálogo declarativo, mutuamente exclusivos, con paleta completa,
colores de resaltado de sintaxis y color de enlaces, y verificación de contraste:

- **Claro**
- **Oscuro**
- **GitHub Light**
- **GitHub Dark**
- **Monokai**
- **Alto contraste**

El tema se persiste y, al cambiarlo, se recolorean los enlaces del documento, se
reajusta el resaltado de sintaxis y se regeneran los iconos monocromos de la barra.

### Luz cálida nocturna (salud ocular)
Ajuste **conmutable** e independiente del tema (se superpone a cualquiera de los 6),
**activado por defecto**. Reduce la luz azul del fondo de forma **automática y
gradual según la hora del reloj del sistema**, para reducir la fatiga visual y la
alteración del sueño al trabajar de noche:

| Franja horaria | Intensidad del tinte |
|---|---|
| 07:00 – 19:00 (día) | nula (fondo neutro) |
| 19:00 – 23:00 (atardecer) | rampa ascendente de 0 a máximo |
| 23:00 – 06:00 (noche) | máxima |
| 06:00 – 07:00 (amanecer) | rampa descendente de máximo a 0 |

- La transición es **gradual**, no a saltos. El fondo se reevalúa cada ~60 s.
- El filtro es multiplicativo y se aplica **solo al fondo de la página**: recorta el
  azul hasta ~16 % y el verde ~5 %, dejando el rojo intacto (un blanco puro pasa a
  crema suave; un gris oscuro, a gris cálido). No altera enlaces ni resaltado.

---

## 8. Inserción de contenido (menú Insertar)

- **Enlace…**: diálogo con texto y URL.
- **Imagen…**: texto alternativo y ruta/URL; las imágenes locales se referencian con
  **ruta relativa** al `.md` cuando es posible, por portabilidad.
- **Pegar imagen**: la imagen del portapapeles se **guarda a disco como PNG** junto
  al `.md` (o se pide ubicación si el documento no se ha guardado todavía) y se
  inserta como `![](ruta)`. No se incrusta —así sobrevive al round-trip—. Pregunta
  el texto alternativo. También funciona pegando o **soltando imágenes**
  directamente sobre el editor.
- **Tabla…**: diálogo con número de columnas y filas; la tabla se crea con borde
  visible.
- **Regla horizontal**.
- **Índice (TOC)**: inserta una lista de enlaces con los encabezados del documento.
- **Fórmula…** — `Ctrl+Shift+F` (§10).
- **Nota al pie** — `Ctrl+Shift+N`: inserta una referencia `[^n]` y su definición al
  final del documento (§16).
- **Admonición**: bloque destacado estilo GitHub —**Nota**, **Consejo**,
  **Importante**, **Advertencia**, **Precaución**— (§16).
- **Símbolos especiales…**: diálogo no modal de «mapa de caracteres» por categorías
  para insertar símbolos poco habituales sin cerrar el diálogo.
- **Fecha** / **Fecha y hora**: inserta la fecha (y hora) actuales en formato local.

### Enlaces
- **Ctrl+clic** abre el enlace en la aplicación externa correspondiente.
- Al pasar el ratón por encima, el cursor cambia a mano y se muestra una pista en la
  barra de estado.

---

## 9. Tablas

Edición desde el menú **Tabla**, contextual: las acciones se habilitan solo con el
cursor dentro de una tabla en el editor visual.

- Insertar fila encima / debajo.
- Insertar columna a la izquierda / derecha.
- Eliminar fila / columna (manteniendo al menos la cabecera y una columna).
- **Alinear columna**: izquierda / centro / derecha. La alineación se serializa al
  Markdown como `:--` / `:-:` / `--:` y se conserva al guardar.

---

## 10. Fórmulas TeX

Soporte de fórmulas matemáticas en línea (`$...$`) y en bloque (`$$...$$`) con la
sintaxis habitual de LaTeX (estilo Pandoc / Obsidian / Quarto), **sin dependencias
externas**.

- **Insertar** — `Ctrl+Shift+F`: diálogo con campo TeX, selector En línea / Bloque y
  **previsualización en vivo** según se teclea.
- **Editar**: **doble clic** sobre una fórmula reabre el diálogo precargado y la
  sustituye.
- **Fórmulas atómicas**: en el editor se comportan como una unidad. Teclear dentro
  recuerda usar el doble clic; Backspace/Suprimir en el borde borran el grupo
  entero; pegar sobre una fórmula la reemplaza completa.
- **Render con maquetación 2D real.** Las fórmulas se pintan en dos dimensiones, no
  aplanadas: super y subíndices **reales**, letras griegas, operadores matemáticos
  (`\pm`, `\times`, `\div`, `\cdot`, `\oplus`…), **fracciones apiladas** (`\frac`),
  **raíces con vínculo** (`\sqrt`), **binomios** (`\binom`), **matrices** y entornos
  (`matrix`, `pmatrix`, `bmatrix`, `cases`…), **grandes operadores con límites**
  encima y debajo (`\sum`, `\int`, `\prod`…), **acentos** (`\hat`, `\bar`, `\vec`,
  `\tilde`, `\dot`…), `\mathbb{R}`, `\text{…}`/`\mathrm{…}`. Las fórmulas en línea se
  muestran con el color de acento del tema y **escalan con el zoom**.
- **Multilínea**: los bloques `$$...$$` pueden abarcar varias líneas en la fuente
  (estilo Pandoc/Obsidian).
- **Round-trip**: en la vista de código se ven como `$...$` / `$$...$$` con todos
  los caracteres TeX intactos.
- **Limitaciones conocidas**: `$...$` debe abrir y cerrar en la misma línea. El
  alineado vertical de las fórmulas 2D **en línea** queda algo alto (las de bloque,
  solas en su línea, se ven bien). Entornos poco comunes o construcciones no
  soportadas (`\overbrace`, etc.) se aproximan en línea y pueden verse pobres.

---

## 11. Buscar y reemplazar

- Barra inferior. **Buscar** `Ctrl+F`, **Reemplazar** `Ctrl+H`.
- Anterior / Siguiente (con vuelta al principio o al final), Reemplazar uno,
  Reemplazar todo (un único paso de deshacer), casilla de sensibilidad a mayúsculas.
- Enter busca/reemplaza, ESC cierra. La selección actual se usa como término inicial.
- En vista dividida actúa sobre el panel con foco.

---

## 12. Resaltado de sintaxis en bloques de código

Colorea palabras clave, cadenas, números y comentarios según el lenguaje declarado
en el bloque. Familias reconocidas (por alias):

- **Estilo C** (`//` y `/* */`): C, C++, Java, C#, Go, Rust, Swift, Kotlin, PHP,
  Scala, Dart…
- **JS/TS/JSON**: JavaScript, TypeScript, JSX/TSX, JSON.
- **Python**.
- **Estilo almohadilla** (`#`): Bash/Shell, Ruby, YAML, TOML, R, Perl, Makefile,
  INI/conf.
- Lenguaje desconocido: resalta cadenas, números y los tres estilos de comentario.

El lenguaje del bloque se fija desde *Formato → Lenguaje del bloque…* con una lista
editable. Los colores siguen al tema.

---

## 13. Diagramas (Mermaid / PlantUML)

Los bloques de código ```` ```mermaid ```` y ```` ```plantuml ```` se pueden
**renderizar como imagen** dentro del editor, en un bloque de previsualización
**bajo** el bloque de código.

- **Opcional y sin dependencia enlazada**: el render se hace ejecutando la
  herramienta externa (`mmdc` para Mermaid, `plantuml` para PlantUML) si está
  instalada. El round-trip es transparente: la imagen nunca llega al Markdown.
- **Degradación elegante**: si la herramienta no está instalada, en lugar de la
  imagen se muestra un marcador con la **orden de instalación** para tu sistema
  operativo, y se sustituye por la imagen en cuanto la herramienta aparece.

---

## 14. Corrección ortográfica (opcional)

Subraya las palabras mal escritas según el idioma del documento. Es una
característica **opcional**: requiere **Hunspell** y sus diccionarios; sin ellos, el
resto de la aplicación funciona igual.

- **Activar/desactivar**: *Ver → Corrección ortográfica* (conmutable, se recuerda).
- **Idioma de corrección**: *Ver → Idioma de corrección*; por defecto se elige por el
  idioma del documento (front matter › ajuste › idioma del sistema). Si falta el
  diccionario, se avisa en la barra de estado.
- **Sugerencias**: clic derecho sobre una palabra subrayada ofrece correcciones y la
  opción de añadirla al **diccionario personal**.

---

## 15. Exportación e impresión

Desde **Archivo → Exportar** / **Imprimir**:

- **PDF** (vía `QPrinter`), y **Selección a PDF** (solo el texto seleccionado).
- **HTML**.
- **ODF (.odt)** — incrusta el idioma del documento.
- **DOCX (.docx)** — serializador OOXML propio; idioma/título incrustados, imágenes
  embebidas.
- **LaTeX (.tex)** — serializador propio, preámbulo portable (`iftex` + `babel`),
  con las fórmulas emitidas verbatim (`amsmath`/`amssymb`).
- **EPUB (.epub)** — libro electrónico EPUB 3.
- **Vista previa de impresión** e **Imprimir** — `Ctrl+P` (diálogo del sistema).

- **Idioma del documento** (ODF, DOCX y LaTeX): se pregunta al exportar; el valor por
  defecto se toma del `lang`/`language` del front matter, en su defecto del ajuste de
  la aplicación y, por último, del idioma del sistema.
- Las fórmulas se conservan: en PDF/HTML/ODF/DOCX/EPUB como super/subíndices reales;
  en LaTeX, verbatim.

---

## 16. Extensiones de Markdown

Construcciones que enriquecen el documento conservando un Markdown portable:

- **Listas de tareas**: `- [ ]` / `- [x]`, con casilla que se marca con un clic.
- **Notas al pie**: referencias `[^id]` y definiciones `[^id]:`. La referencia se
  muestra en superíndice; un clic salta a su definición. Se insertan con
  *Insertar → Nota al pie* (`Ctrl+Shift+N`).
- **Admoniciones** («callouts» estilo GitHub): una cita cuya primera línea es
  `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` o `[!CAUTION]`, con fondo tintado
  y título en color. Se insertan desde *Insertar → Admonición*.
- **Shortcodes**: al teclear `:nombre:` se expande a su símbolo (`:alpha:` → α).
- **Front matter** YAML/TOML conservado verbatim (§2).

---

## 17. Estadísticas del documento

- *Ver → Estadísticas del documento…* muestra palabras, caracteres, párrafos, frases
  y **tiempo estimado de lectura**, sobre todo el documento o sobre la selección.
- El contador de palabras/caracteres también vive en la barra de estado.

---

## 18. Zoom de toda la interfaz

- **Aumentar** `Ctrl++` / `Ctrl+=`, **Reducir** `Ctrl+-`, **Tamaño normal** `Ctrl+0`.
  También con **Ctrl + rueda del ratón** sobre el editor.
- Escala no solo el texto del editor, sino toda la interfaz: menús (y cada
  desplegable), barras de herramientas e iconos, barra de búsqueda, barra de estado,
  editor de fuente y panel de esquema.
- El nivel de zoom se persiste.

---

## 19. Internacionalización

- Idioma de la interfaz desde **Ver → Idioma**: Automático (sistema), Español,
  English, Deutsch, Français, Italiano, Português, Polski, Nederlands, Română.
- El cambio se aplica al reiniciar (se avisa). Idioma de origen: español; respaldo a
  inglés si el sistema no tiene traducción.
- Todos los textos visibles están traducidos; los plurales son correctos por idioma
  (polaco y rumano usan tres formas). Los atajos en *tooltips* se localizan solos.

---

## 20. Ayuda

- **Manual** — **F1**: ventana de ayuda no modal con dos secciones, «Uso de la
  aplicación» y «Markdown», renderizadas con el mismo motor del editor y localizadas
  a los 9 idiomas.
- **Acerca de**: datos del autor y de la versión.

---

## 21. Persistencia de ajustes

Se recuerdan entre sesiones: tema, luz cálida nocturna, nivel de zoom, idioma de la
interfaz, geometría y estado de la ventana, posición del divisor de la vista
dividida, lista de archivos recientes, pestañas/archivos abiertos, estado y idioma
del corrector ortográfico y diccionario personal.

---

## 22. Plataformas, requisitos y empaquetado

### Requisitos de compilación
- **CMake** ≥ 3.16.
- **Qt 6** ≥ 6.5 con sus **cabeceras privadas** (módulos `Widgets`, `PrintSupport`,
  `LinguistTools`; `Test` para las pruebas). Usa la API privada `Qt6::GuiPrivate`
  (QZip) para incrustar el idioma en los `.odt`/`.docx` y para empaquetar el `.epub`;
  en Debian/Ubuntu vienen en `qt6-base-private-dev`, aparte de `qt6-base-dev`. Las
  builds de CI/release usan Qt 6.8.2.
- **C++17** (GCC 9+, Clang 10+, MSVC 19.20+).
- **Hunspell** *(opcional)* para el corrector ortográfico (`libhunspell-dev`); sin él
  el resto compila igual. Se enlaza estático por defecto en las builds de
  distribución.
- Para el render de **diagramas** *(opcional, en tiempo de ejecución)*: `mmdc`
  (Mermaid) y/o `plantuml` (PlantUML) instalados en el sistema.

### Compilar, probar, instalar
```bash
# Dependencias (Debian/Ubuntu)
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opcional (corrector)

# Compilar
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]

# Tests (headless)
ctest --test-dir build --output-on-failure

# Instalar en Linux (binario + .desktop + iconos hicolor)
sudo ./install.sh                    # -> /usr/local
PREFIX="$HOME/.local" ./install.sh   # de usuario, sin sudo
```

### Distribución
| Plataforma | Formato | Notas |
|---|---|---|
| Linux x86_64 | **AppImage** | ejecutable de un solo archivo |
| Windows x64 | **ZIP portable** | sin instalador, sin firma |
| macOS (universal arm64 + x86_64) | **DMG** | sin firmar: primer arranque con Ctrl-clic → Abrir |

En Linux se integra con el escritorio (`.desktop`, categorías `Utility;TextEditor;`,
asociación MIME `text/markdown`, iconos hicolor). El corrector se enlaza estático en
las builds de distribución, de modo que el paquete no depende de ninguna biblioteca
de Hunspell.

---

## Apéndice A — Atajos de teclado

| Atajo | Acción |
|---|---|
| `Ctrl+N` | Nuevo |
| `Ctrl+O` | Abrir |
| `Ctrl+S` | Guardar |
| `Ctrl+Shift+S` | Guardar como |
| `Ctrl+W` | Cerrar pestaña |
| `Ctrl+P` | Imprimir |
| `Ctrl+Q` | Salir |
| `Ctrl+Z` | Deshacer |
| `Ctrl+Y` / `Ctrl+Shift+Z` | Rehacer |
| `Ctrl+Shift+V` | Pegar como texto plano |
| `Ctrl+Alt+V` | Pegar como Markdown |
| `Ctrl+F` | Buscar |
| `Ctrl+H` | Reemplazar |
| `Ctrl+B` | Negrita |
| `Ctrl+I` | Cursiva |
| `Ctrl+U` | Subrayado |
| `Ctrl+Shift+X` | Tachado |
| `Ctrl+E` | Código en línea |
| `Ctrl+K` | Enlace |
| `Ctrl+1` … `Ctrl+6` | Encabezados H1–H6 |
| `Ctrl+Shift+U` | Lista de viñetas |
| `Ctrl+Shift+O` | Lista numerada |
| `Ctrl+Shift+T` | Lista de tareas |
| `Ctrl+]` / `Ctrl+[` | Aumentar / disminuir sangría |
| `Ctrl+Shift+Q` | Cita |
| `Ctrl+Shift+K` | Bloque de código |
| `Ctrl+Shift+F` | Insertar fórmula |
| `Ctrl+Shift+N` | Insertar nota al pie |
| `Ctrl+Shift+M` | Vista de código fuente |
| `Ctrl+Shift+D` | Vista dividida |
| `F11` | Modo sin distracciones (ESC para salir) |
| `F9` | Esquema |
| `Ctrl+G` | Ir a encabezado |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom (también Ctrl + rueda) |
| `F1` | Manual |

## Apéndice B — Estructura de menús

- **Archivo**: Nuevo · Nuevo desde plantilla · Abrir · Abrir recientes · Guardar ·
  Guardar como · Cerrar pestaña · Exportar (PDF / HTML / ODF / DOCX / LaTeX / EPUB /
  Selección a PDF) · Vista previa de impresión · Imprimir · Salir
- **Editar**: Deshacer · Rehacer · Pegar como texto plano · Pegar como Markdown ·
  Transformar texto (MAYÚSCULAS / minúsculas / Capitalizar / Tipografía inteligente /
  Ordenar líneas) · Buscar · Reemplazar
- **Formato**: marcas de carácter · Enlace · Encabezados H1–H6 · listas · sangrías ·
  Cita · Bloque de código · Lenguaje del bloque
- **Insertar**: Enlace · Imagen · Pegar imagen · Tabla · Regla horizontal ·
  Índice (TOC) · Fórmula · Nota al pie · Admonición · Símbolos especiales · Fecha ·
  Fecha y hora
- **Tabla** (contextual): filas · columnas · alinear columna
- **Ver**: Código fuente · Vista dividida · Sin distracciones · Esquema · Ir a
  encabezado · Estadísticas del documento · Corrección ortográfica · Idioma de
  corrección · zoom · Tema (6 temas + Luz cálida nocturna) · Idioma
- **Ayuda**: Manual · Acerca de
