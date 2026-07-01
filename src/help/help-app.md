# Manual de uso

**md-editor** es un editor visual (WYSIWYG) de Markdown: escribes y das formato
sobre el texto ya renderizado, sin ver el código. Al guardar, el documento se
serializa de vuelta a Markdown puro.

## Índice

- [Abrir y guardar](#abrir-y-guardar)
- [Dar formato al texto](#dar-formato-al-texto)
- [Encabezados, listas y bloques](#encabezados-listas-y-bloques)
- [Transformar texto y portapapeles](#transformar-texto-y-portapapeles)
- [Enlaces e imágenes](#enlaces-e-imagenes)
- [Notas al pie](#notas-al-pie)
- [Admoniciones, símbolos y atajos de texto](#admoniciones-simbolos-y-atajos-de-texto)
- [Snippets (fragmentos reutilizables)](#snippets-fragmentos-reutilizables)
- [Tablas](#tablas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Diagramas](#diagramas)
- [Corrección ortográfica](#correccion-ortografica)
- [Buscar y reemplazar](#buscar-y-reemplazar)
- [Esquema del documento](#esquema-del-documento)
- [Estadísticas del documento](#estadisticas-del-documento)
- [Modo sin distracciones](#modo-sin-distracciones)
- [Modo foco](#modo-foco)
- [Vista de código](#vista-de-codigo)
- [Exportar e imprimir](#exportar-e-imprimir)
- [Temas y apariencia](#temas-y-apariencia)
- [Recuperación automática](#recuperacion-automatica)
- [Accesibilidad](#accesibilidad)
- [Atajos](#atajos)

## Abrir y guardar

- **Archivo → Nuevo** (Ctrl+N) crea un documento vacío en una pestaña nueva.
- **Archivo → Nuevo desde plantilla** crea un documento a partir de un esqueleto
  (carta, acta, examen…) listo para rellenar.
- **Archivo → Abrir…** (Ctrl+O) abre un `.md` existente. La aplicación
  recuerda los últimos abiertos en **Archivo → Abrir recientes**.
- **Guardar** (Ctrl+S) y **Guardar como…** (Ctrl+Shift+S) escriben el
  documento en UTF-8. **Abrir carpeta contenedora** abre en el gestor de
  archivos la carpeta del documento actual.
- **Revertir a lo guardado** descarta los cambios sin guardar y recarga el
  archivo desde el disco (pide confirmación). Disponible solo si el documento
  tiene archivo y cambios pendientes.
- Si el archivo cambia fuera del editor, la aplicación lo detecta y, si no
  tienes cambios sin guardar, lo recarga; si los tienes, pregunta qué hacer.
- También puedes **arrastrar y soltar** un archivo sobre la ventana para
  abrirlo.

### Pestañas (varios documentos)

Puedes tener varios documentos abiertos a la vez, cada uno en su **pestaña**:

- **Nuevo** (Ctrl+N), **Nuevo desde plantilla** y **Abrir** (Ctrl+O) crean una
  pestaña (o reutilizan la pestaña vacía inicial). Arrastrar y soltar un archivo
  también lo abre en una pestaña; si ya está abierto, salta a la suya.
- Cambia de documento pulsando en su pestaña; arrástralas para reordenarlas. Con
  el teclado, **Ctrl+AvPág / Ctrl+RePág** (o **Ctrl+Tab / Ctrl+Shift+Tab**) saltan
  a la pestaña siguiente o anterior.
- **Cerrar pestaña** (Ctrl+W) cierra la actual y pregunta si tiene cambios sin
  guardar. La última no se cierra: queda como documento nuevo.
- **Reabrir pestaña cerrada** (Ctrl+Shift+R) vuelve a abrir la última pestaña que
  cerraste (solo las que tenían archivo en disco).
- La etiqueta muestra el nombre del archivo y un punto (•) si hay cambios sin
  guardar.
- Al cerrar la aplicación se recuerdan los documentos abiertos y se reabren todos
  al volver a arrancar.

### *Front matter*

Si el documento empieza con un bloque `---…---` (YAML) o `+++…+++` (TOML), se
conserva tal cual al guardar: no se ve en el editor, no se edita. Sirve para
metadatos como `title`, `lang`, etc., que se usan al exportar.

## Dar formato al texto

Selecciona un fragmento y aplica el formato con la barra de herramientas o el
menú **Formato**:

- **Negrita** (Ctrl+B), **Cursiva** (Ctrl+I), **Subrayado** (Ctrl+U),
  **Tachado**.
- **Código en línea** para fragmentos `monoespaciados`.
- **Enlace**: añade `[texto](url)` sobre la selección.

Los botones de la barra reflejan el formato activo bajo el cursor.

**Auto-emparejado.** Al teclear `(`, `[`, `{` o `` ` `` se cierra solo el par y el
cursor queda en medio; si hay texto seleccionado, lo envuelve. Si tecleas el cierre
justo delante de su pareja, el editor lo «salta» en vez de duplicarlo.

## Encabezados, listas y bloques

- **Encabezados** H1–H6 desde **Formato → Encabezado** o con
  Ctrl+1 … Ctrl+6.
- **Listas**: viñetas, numeradas y de tareas (con casilla). Pulsando Enter al
  final de un punto se crea el siguiente automáticamente; con Enter sobre un
  punto vacío se sale de la lista. Un **clic sobre la casilla** de una tarea la
  marca o desmarca.
- **Cita** (`>` al principio de un párrafo) y **bloque de código** se aplican
  desde la barra; ambos round-trip-ean a Markdown correctamente. Con **Formato →
  Lenguaje del bloque…** eliges el lenguaje de un bloque de código (estando el
  cursor dentro) para que se le resalte la sintaxis.
- **Sangría**: **Formato → Aumentar/Disminuir sangría** anida listas y citas.

## Transformar texto y portapapeles

- **Editar → Transformar texto** opera sobre la selección: **MAYÚSCULAS**,
  **minúsculas**, **Capitalizar** y **Ordenar líneas**.
- **Tipografía inteligente** (en el mismo menú) convierte sobre la selección
  los guiones `--`/`---` en `–`/`—`, `...` en `…` y las comillas rectas en
  tipográficas según el contexto.
- **Pegar como texto plano** (Ctrl+Shift+V) pega sin formato. **Pegar como
  Markdown** (Ctrl+Alt+V) convierte el contenido enriquecido del portapapeles
  (HTML) a Markdown en vez de incrustar el formato del origen.
- **Copiar como HTML** copia la selección (o el documento) como HTML, para
  pegarla en correo, un CMS, etc.
- **Copiar como Markdown** copia la selección (o el documento entero) como texto
  Markdown, para pegarlo en otro editor de Markdown o en un campo de código.
- Al pegar una **URL** sobre una selección de texto, el texto queda enlazado
  automáticamente.
- **Editar → Limpiar Markdown** normaliza todo el documento de una pasada:
  uniforma las viñetas a `-`, recorta los espacios al final de cada línea, colapsa
  las líneas en blanco de más y ajusta el espacio tras los `#` de los encabezados.
  Es conservador: no toca el interior de los bloques de código.

## Enlaces e imágenes

- **Insertar → Enlace…** abre un diálogo con texto y URL. Si tenías selección,
  pasa como texto.
- **Ctrl+clic** sobre un enlace lo abre en el navegador del sistema; al pasar
  el ratón por encima se muestra la URL en la barra de estado.
- **Imágenes**: arrastra un archivo, pega una imagen del portapapeles o usa
  **Insertar → Pegar imagen**. La imagen se guarda como PNG junto al `.md` y
  se inserta como `![alt](ruta-relativa)`; así sobrevive al round-trip a
  Markdown (las imágenes incrustadas, no).

## Notas al pie

- **Insertar → Nota al pie** (Ctrl+Shift+N) inserta una referencia numerada
  `[^n]` donde está el cursor y crea su definición `[^n]:` al final del
  documento, lista para que escribas el texto de la nota.
- Las referencias se muestran como **superíndice**; al hacer **clic** sobre una,
  el cursor salta a su definición.
- Se guardan como Markdown estándar (`texto[^1]` en el cuerpo y, abajo,
  `[^1]: la nota`), así que son compatibles con otros editores.

## Admoniciones, símbolos y atajos de texto

- **Insertar → Admonición** crea un *callout* estilo GitHub: una cita cuya
  primera línea es `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` o
  `[!CAUTION]`. Se muestra con fondo tintado y título en color, y se guarda como
  Markdown compatible con GitHub.
- **Insertar → Símbolos especiales…** abre un mapa de caracteres por categorías
  (matemáticos, griego, flechas, moneda, puntuación…); un clic inserta el
  símbolo y el diálogo se queda abierto para insertar varios.
- **Atajos `:nombre:`**: al teclear un código como `:alpha:` o `:euro:` se
  expande al símbolo correspondiente (α, €…).
- **Insertar → Fecha** y **Fecha y hora** insertan la fecha (y hora) actual en
  formato localizado.

## Snippets (fragmentos reutilizables)

Un **snippet** es un trozo de Markdown que guardas con un nombre para insertarlo
luego con un par de clics: una firma, una plantilla de tabla, un aviso que repites
a menudo…

- **Insertar → Snippet** despliega la lista de los que tienes; al elegir uno, su
  contenido se inserta donde está el cursor (funciona también en la vista de
  código).
- **Insertar → Snippet → Gestionar snippets…** abre un diálogo para crear, editar
  y borrar tus snippets. Cada uno tiene un **nombre** (el que ves en el menú) y un
  **cuerpo** en Markdown.
- Se guardan en los ajustes de la aplicación, así que están disponibles en todos
  tus documentos, no solo en el actual.

## Tablas

- **Tabla → Insertar tabla…** pide filas y columnas.
- **Insertar → Tabla desde el portapapeles** convierte en una tabla los datos
  TSV/CSV (columnas separadas por tabuladores o comas) copiados de una hoja de
  cálculo o un archivo CSV.
- Las acciones del menú **Tabla** (añadir/quitar fila o columna, alinear
  columna) sólo se activan cuando el cursor está dentro de una tabla.
- La alineación de columna (izquierda/centro/derecha) se conserva al guardar
  como `:--`/`:-:`/`--:`.

## Fórmulas matemáticas

md-editor admite **fórmulas TeX** en línea (`$...$`) y en bloque (`$$...$$`),
con la sintaxis habitual de LaTeX (Pandoc, Obsidian, Quarto…). No hace falta
ninguna dependencia externa.

- **Insertar → Fórmula…** (Ctrl+Shift+F) abre un diálogo con un campo para el
  TeX y una **previsualización en vivo**: a medida que escribes ves cómo
  quedará. Elige *En línea* o *Bloque* y acepta para insertarla.
- Las fórmulas se maquetan en **2D real**: las fracciones (`\frac`) se apilan
  con barra, los grandes operadores (`\sum`, `\int`, `\prod`…) muestran sus
  límites encima y debajo, las raíces (`\sqrt`) llevan su vínculo, y hay
  matrices (`\begin{pmatrix}`…), binomios (`\binom`) y acentos (`\hat`, `\vec`,
  `\bar`…). Las más simples (potencias, subíndices, griego) se componen en
  línea. El diagrama escala con el zoom.
- **Doble clic** sobre una fórmula reabre el diálogo con su TeX original
  precargado: editas y al aceptar se sustituye.
- Las fórmulas son **atómicas**: si tecleas dentro la app te recuerda que
  uses el doble clic; Backspace/Suprimir en su borde borran el grupo entero.
- Al **exportar** se conservan: a LaTeX se vuelcan tal cual (con `amsmath` y
  `amssymb` en el preámbulo); a HTML/PDF/ODF se vuelcan a su aproximación en
  línea.
- En la **vista de código** se ven como `$...$` / `$$...$$`, con todos los
  caracteres TeX (`\sum`, `\frac`, `_`, `*`) intactos al guardar.

Ejemplos:

```
La energía es $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> En la fuente, `$$...$$` puede cruzar varias líneas (estilo Obsidian/Pandoc);
> `$...$` debe abrir y cerrar en la misma línea.

## Diagramas

Un bloque de código con lenguaje `mermaid` o `plantuml` se **previsualiza como
imagen** justo debajo del bloque, sin tocar el código (que sigue editable) ni el
Markdown guardado.

- Requiere tener instalada la herramienta correspondiente: **`plantuml`** (con
  Java) para PlantUML, o **`mmdc`** (mermaid-cli, con Node) para Mermaid.
- Si la herramienta falta, bajo el bloque aparece un aviso con la orden de
  instalación de tu sistema operativo; el bloque se mantiene como código.
- La imagen es solo presentación: no se escribe en el Markdown ni cuenta como
  cambio sin guardar.

Por ejemplo, un bloque de código con la etiqueta `mermaid` que contenga
`flowchart LR  A --> B --> C` se previsualiza como el diagrama de flujo
correspondiente.

## Corrección ortográfica

- Subraya en rojo las palabras mal escritas según el **idioma del documento**
  (deducido del front matter `lang`, del ajuste de idioma o del sistema). No
  comprueba el código, las fórmulas ni los enlaces.
- **Clic derecho** sobre una palabra subrayada ofrece **sugerencias** (un clic
  la reemplaza), **Añadir al diccionario** (lista personal, permanente) e
  **Ignorar** (durante la sesión).
- Se activa/desactiva en **Ver → Corrección ortográfica**, y el idioma se fija
  en **Ver → Idioma de corrección** (o se deja en automático).
- Necesita diccionarios Hunspell: en Linux, los del sistema (`hunspell-es`,
  `hunspell-en-us`…); en Windows/macOS vienen con la aplicación.

## Buscar y reemplazar

- **Buscar** (Ctrl+F) abre una barra inferior con campos para buscar y
  reemplazar, además de opciones (caso, palabra completa).
- **Buscar siguiente** F3 / **Buscar anterior** Shift+F3.

## Esquema del documento

El panel lateral izquierdo muestra el índice de encabezados (TOC): se
actualiza al escribir y, al hacer clic en una entrada, el cursor salta a ese
encabezado. Se muestra/oculta con F9. Con **F6** llevas el foco del
teclado al esquema (lo muestra si estaba oculto); ahí, las flechas recorren los
encabezados y **Enter** salta al elegido y devuelve el foco al editor. Pulsar
**F6** de nuevo, sin más, devuelve el foco al editor.

Puedes **arrastrar** una entrada del esquema para **reordenar** esa sección
—su encabezado, su contenido y sus subsecciones— dentro del documento, sin
cambiar el nivel. Además, **Insertar → Índice (TOC)** vuelca en el documento una
lista anidada con los encabezados. **Ver → Ir a encabezado…** (Ctrl+G) salta a
un encabezado escribiendo parte de su texto, e **Ir a línea…** (Ctrl+L) lleva el
cursor a un número de línea (en la vista de fuente, a la línea del Markdown). La
**Paleta de comandos** (Ctrl+Shift+P) busca y ejecuta cualquier acción de los
menús escribiendo parte de su nombre.

## Estadísticas del documento

- **Ver → Estadísticas del documento…** muestra palabras, caracteres, párrafos,
  frases y tiempo estimado de lectura (del documento o de la selección).
- **Ver → Mostrar contador de palabras** activa un contador permanente en la
  barra de estado.
- **Ver → Mostrar línea y columna** muestra en la barra de estado la posición del
  cursor (línea y columna).

## Modo sin distracciones

**Ver → Sin distracciones** (F11) entra en pantalla completa con menú y
barras ocultas y el texto centrado en una columna de lectura. El esquema,
si está visible, queda pegado al bloque central. ESC o F11 salen.

## Modo foco

**Ver → Modo foco** (F12) te ayuda a concentrarte en lo que escribes sin salir de
la ventana normal. Un único interruptor activa dos cosas a la vez:

- **Máquina de escribir**: la línea del cursor se mantiene centrada en vertical.
  Según escribes, el texto se desplaza para que la línea activa quede a media
  altura, en lugar de irse pegando al borde inferior.
- **Atenuado**: todo el documento se ve apagado salvo el párrafo donde está el
  cursor, que destaca nítido.

Funciona en el editor visual y en la vista de código, y es **independiente** del
modo sin distracciones (F11): puedes usar los dos a la vez o cada uno por su lado.

## Vista de código

**Ver → Código fuente Markdown** (Ctrl+Shift+M) alterna entre el editor visual y
un editor de texto plano, a pantalla completa, con el Markdown crudo. Los cambios
en el modo fuente se vuelcan al documento al volver al modo visual.

**Ver → Vista dividida** (Ctrl+Shift+D) muestra ambos a la vez, lado a lado: el
editor visual y el código fuente, sincronizados (lo que escribes en uno se refleja
en el otro). Es excluyente con el modo fuente a pantalla completa.

## Exportar e imprimir

**Archivo → Exportar** ofrece **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** y **texto plano (.txt)**. En ODF, DOCX, LaTeX y
EPUB se incrusta el idioma del documento (tomado del front matter `lang`/`language`,
del ajuste de la aplicación o, en último caso, del idioma del sistema). En **PDF** se
incrustan el título y el autor cuando están en el front matter (`title`, `author`).

También puedes exportar **solo la selección a PDF** y usar la **Vista previa de
impresión**.

**Archivo → Imprimir** (Ctrl+P) abre el diálogo del sistema; **Imprimir
selección** imprime solo lo seleccionado.

## Temas y apariencia

- **Ver → Tema** ofrece Claro, Oscuro, GitHub Light, GitHub Dark, Monokai,
  Alto contraste, Solarized Light y Solarized Dark. **Seguir el sistema** ajusta
  el tema claro/oscuro al del SO.
- **Ver → Luz cálida nocturna** atenúa los azules del fondo según la hora.
- **Ver → Interlineado** ajusta la altura de línea del editor: Sencillo, 1,5
  líneas o Doble.
- **Ver → Resaltar la línea actual** marca con un fondo sutil la línea del cursor.
- **Zoom**: Ctrl+rueda del ratón, Ctrl++ / Ctrl+- y **Tamaño normal** (Ctrl+0)
  escalan toda la interfaz (no sólo el texto del editor).
- **Ver → Idioma** cambia el idioma de la interfaz; se aplica al instante (recrea la ventana).

## Recuperación automática

Mientras editas, el contenido se autoguarda cada pocos segundos en una copia
de borrador. Si la aplicación se cierra de forma anómala, al volver a abrirla
ofrece recuperar lo que estabas escribiendo.

## Accesibilidad

- **Lectores de pantalla**: el editor, el panel de esquema, los campos de búsqueda y los demás controles tienen nombre accesible; además, los mensajes de estado (guardado, «no encontrado», cambios en disco…) se anuncian por voz.
- **Solo con teclado**: todas las acciones tienen atajo o entrada de menú (F10 o Alt abre la barra de menús). Consulta la tabla de [Atajos](#atajos).
- **Contraste y tamaño**: el tema **Alto contraste** y el **zoom** de toda la interfaz ayudan con la baja visión; el tamaño de letra de partida es el del sistema.
- **Foco**: el elemento enfocado se resalta con el color de selección del tema.

## Atajos

| Acción                    | Atajo            |
|---------------------------|------------------|
| Nuevo                     | Ctrl+N           |
| Abrir                     | Ctrl+O           |
| Cerrar pestaña            | Ctrl+W           |
| Reabrir pestaña cerrada   | Ctrl+Shift+R     |
| Pestaña siguiente / anterior | Ctrl+AvPág / Ctrl+RePág (o Ctrl+Tab / Ctrl+Shift+Tab) |
| Guardar                   | Ctrl+S           |
| Guardar como              | Ctrl+Shift+S     |
| Imprimir                  | Ctrl+P           |
| Deshacer / Rehacer        | Ctrl+Z / Ctrl+Y  |
| Negrita / Cursiva         | Ctrl+B / Ctrl+I  |
| Subrayado                 | Ctrl+U           |
| Pegar como texto plano    | Ctrl+Shift+V     |
| Pegar como Markdown       | Ctrl+Alt+V       |
| Buscar                    | Ctrl+F           |
| Buscar siguiente/anterior | F3 / Shift+F3    |
| Encabezado H1 … H6        | Ctrl+1 … Ctrl+6  |
| Insertar fórmula          | Ctrl+Shift+F     |
| Insertar nota al pie      | Ctrl+Shift+N     |
| Ir a encabezado           | Ctrl+G           |
| Ir a línea                | Ctrl+L           |
| Paleta de comandos        | Ctrl+Shift+P     |
| Foco al esquema / volver al editor | F6 |
| Vista de código Markdown  | Ctrl+Shift+M     |
| Vista dividida            | Ctrl+Shift+D     |
| Esquema                   | F9               |
| Sin distracciones         | F11              |
| Modo foco                 | F12              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ayuda                     | F1               |
