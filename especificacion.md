# Especificación de producto — md-editor

> Editor/visor **WYSIWYG** de Markdown en **Qt6 + C++17**. Por defecto editas sobre
> el texto ya renderizado, sin lidiar con la sintaxis; pero opcionalmente puedes ver
> el código Markdown e incluso tener código y renderizado en paralelo. Al guardar se
> serializa siempre a Markdown limpio.

- **Versión:** 1.2.0
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
  listas anidadas, listas de tareas, bloques de código y fórmulas.
- **El código es opcional, no obligatorio.** Quien quiera ver o editar el Markdown
  crudo puede hacerlo (vista de fuente a pantalla completa o vista dividida con
  render y código en paralelo, §4).
- **Marca «modificado».** El indicador `[*]` del título compara la serialización
  canónica actual con una línea base, no el estado interno de Qt (que se ensucia de
  forma espuria al trazar). Refleja cambios reales del contenido.

---

## 2. Gestión de archivos y sesión

### Operaciones de archivo
| Acción | Atajo |
|---|---|
| Nuevo | `Ctrl+N` |
| Abrir… | `Ctrl+O` |
| Guardar | `Ctrl+S` |
| Guardar como… | `Ctrl+Shift+S` |
| Salir | `Ctrl+Q` |

- Filtros de apertura: `*.md`, `*.markdown`, `*.mdown`, `*.mkd`.
- **Confirmación de cambios sin guardar** antes de cerrar o descartar un documento.
- **Abrir arrastrando y soltando** un archivo sobre la ventana.

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
borrador** (si hubo un cierre anómalo) › **reabrir el último documento**. La
apertura se difiere un instante para evitar diálogos espurios durante el trazado
inicial.

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
  vacío sale de la lista.
- Citas y bloques de código se gestionan reescribiendo el Markdown del bloque, de
  modo que round-trip-ean correctamente.

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

---

## 6. Modo sin distracciones

- Toggle con **F11**; se sale también con **ESC**.
- Pantalla completa que oculta menú, barras de herramientas, barra de búsqueda y
  barra de estado.
- El texto se centra en una **columna de lectura** (≈960 px), con los márgenes
  laterales en negro para minimizar la distracción.
- Si el esquema está visible, queda pegado a la columna y el conjunto se centra.
- Es de columna única: al entrar se abandona la vista dividida. Al cerrar la
  aplicación estando en este modo, se persiste el estado de ventana **previo**
  (normal, con barras), no el de pantalla completa.

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
- **Fórmula…** — `Ctrl+Shift+F` (§10).

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
- **Render**: super y subíndices **reales** (no caracteres Unicode planos), letras
  griegas, operadores matemáticos comunes (`\pm`, `\times`, `\div`, `\cdot`,
  `\oplus`…), `\frac{a}{b}`, `\sqrt{x}`, `\mathbb{R}`, y `^`/`_` con argumento. Las
  fórmulas se muestran en cursiva con el color de acento del tema.
- **Round-trip**: en la vista de código se ven como `$...$` / `$$...$$` con todos
  los caracteres TeX intactos.
- **Limitaciones conocidas**: `$...$` debe abrir y cerrar en la misma línea; no hay
  *layout* bidimensional (las fracciones grandes se muestran como `(a)/(b)`, y los
  sumatorios con límites a un lado, no encima y debajo).

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

## 13. Exportación e impresión

Desde **Archivo → Exportar** / **Imprimir**:

- **PDF** (vía `QPrinter`).
- **HTML**.
- **ODF (.odt)** — incrusta el idioma del documento.
- **LaTeX (.tex)** — serializador propio, preámbulo portable (`iftex` + `babel`),
  con las fórmulas emitidas verbatim (`amsmath`/`amssymb`).
- **Imprimir** — `Ctrl+P` (diálogo del sistema).

- **Idioma del documento** (ODF y LaTeX): se pregunta al exportar; el valor por
  defecto se toma del `lang`/`language` del front matter, en su defecto del ajuste de
  la aplicación y, por último, del idioma del sistema.
- Las fórmulas se conservan: en PDF/HTML/ODF como super/subíndices reales; en LaTeX,
  verbatim.

---

## 14. Zoom de toda la interfaz

- **Aumentar** `Ctrl++` / `Ctrl+=`, **Reducir** `Ctrl+-`, **Tamaño normal** `Ctrl+0`.
  También con **Ctrl + rueda del ratón** sobre el editor.
- Escala no solo el texto del editor, sino toda la interfaz: menús (y cada
  desplegable), barras de herramientas e iconos, barra de búsqueda, barra de estado,
  editor de fuente y panel de esquema.
- El nivel de zoom se persiste.

---

## 15. Internacionalización

- Idioma de la interfaz desde **Ver → Idioma**: Automático (sistema), Español,
  English, Deutsch, Français, Italiano, Português, Polski, Nederlands, Română.
- El cambio se aplica al reiniciar (se avisa). Idioma de origen: español; respaldo a
  inglés si el sistema no tiene traducción.
- Todos los textos visibles están traducidos; los plurales son correctos por idioma
  (polaco y rumano usan tres formas). Los atajos en *tooltips* se localizan solos.

---

## 16. Ayuda

- **Manual** — **F1**: ventana de ayuda no modal con dos secciones, «Uso de la
  aplicación» y «Markdown», renderizadas con el mismo motor del editor y localizadas
  a los 9 idiomas.
- **Acerca de**: datos del autor y de la versión.

---

## 17. Persistencia de ajustes

Se recuerdan entre sesiones: tema, luz cálida nocturna, nivel de zoom, idioma de la
interfaz, geometría y estado de la ventana, posición del divisor de la vista
dividida, lista de archivos recientes y último archivo abierto.

---

## 18. Plataformas, requisitos y empaquetado

### Requisitos de compilación
- **CMake** ≥ 3.16.
- **Qt 6** ≥ 6.5 (módulos `Widgets`, `PrintSupport`, `LinguistTools`; `Test` para
  las pruebas). Usa además la API privada `Qt6::GuiPrivate` (QZip) para incrustar el
  idioma en los `.odt`. Las builds de CI/release usan Qt 6.8.2.
- **C++17** (GCC 9+, Clang 10+, MSVC 19.20+).
- Sin dependencias de terceros: solo Qt6, código portable.

### Compilar, probar, instalar
```bash
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
asociación MIME `text/markdown`, iconos hicolor).

---

## Apéndice A — Atajos de teclado

| Atajo | Acción |
|---|---|
| `Ctrl+N` | Nuevo |
| `Ctrl+O` | Abrir |
| `Ctrl+S` | Guardar |
| `Ctrl+Shift+S` | Guardar como |
| `Ctrl+P` | Imprimir |
| `Ctrl+Q` | Salir |
| `Ctrl+Z` | Deshacer |
| `Ctrl+Y` / `Ctrl+Shift+Z` | Rehacer |
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
| `Ctrl+Shift+M` | Vista de código fuente |
| `Ctrl+Shift+D` | Vista dividida |
| `F11` | Modo sin distracciones (ESC para salir) |
| `F9` | Esquema |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom (también Ctrl + rueda) |
| `F1` | Manual |

## Apéndice B — Estructura de menús

- **Archivo**: Nuevo · Abrir · Abrir recientes · Guardar · Guardar como · Exportar
  (PDF / HTML / ODF / LaTeX) · Imprimir · Salir
- **Editar**: Deshacer · Rehacer · Buscar · Reemplazar
- **Formato**: marcas de carácter · Enlace · Encabezados H1–H6 · listas · sangrías ·
  Cita · Bloque de código · Lenguaje del bloque
- **Insertar**: Enlace · Imagen · Pegar imagen · Tabla · Regla horizontal · Fórmula
- **Tabla** (contextual): filas · columnas · alinear columna
- **Ver**: Código fuente · Vista dividida · Sin distracciones · Esquema · zoom · Tema
  (6 temas + Luz cálida nocturna) · Idioma
- **Ayuda**: Manual · Acerca de
