# Manual de uso

**md-editor** es un editor visual (WYSIWYG) de Markdown: escribes y das formato
sobre el texto ya renderizado, sin ver el código. Al guardar, el documento se
serializa de vuelta a Markdown puro.

## Índice

- [Abrir y guardar](#abrir-y-guardar)
- [Dar formato al texto](#dar-formato-al-texto)
- [Encabezados, listas y bloques](#encabezados-listas-y-bloques)
- [Enlaces e imágenes](#enlaces-e-imagenes)
- [Tablas](#tablas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Buscar y reemplazar](#buscar-y-reemplazar)
- [Esquema del documento](#esquema-del-documento)
- [Modo sin distracciones](#modo-sin-distracciones)
- [Vista de código](#vista-de-codigo)
- [Exportar e imprimir](#exportar-e-imprimir)
- [Temas y apariencia](#temas-y-apariencia)
- [Recuperación automática](#recuperacion-automatica)
- [Atajos](#atajos)

## Abrir y guardar

- **Archivo → Nuevo** (Ctrl+N) crea un documento vacío.
- **Archivo → Abrir…** (Ctrl+O) abre un `.md` existente. La aplicación
  recuerda los últimos abiertos en **Archivo → Abrir recientes**.
- **Guardar** (Ctrl+S) y **Guardar como…** (Ctrl+Shift+S) escriben el
  documento en UTF-8.
- Si el archivo cambia fuera del editor, la aplicación lo detecta y, si no
  tienes cambios sin guardar, lo recarga; si los tienes, pregunta qué hacer.
- También puedes **arrastrar y soltar** un archivo sobre la ventana para
  abrirlo.

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

## Encabezados, listas y bloques

- **Encabezados** H1–H6 desde **Formato → Encabezado** o con
  Ctrl+1 … Ctrl+6.
- **Listas**: viñetas, numeradas y de tareas (con casilla). Pulsando Enter al
  final de un punto se crea el siguiente automáticamente; con Enter sobre un
  punto vacío se sale de la lista.
- **Cita** (`>` al principio de un párrafo) y **bloque de código** se aplican
  desde la barra; ambos round-trip-ean a Markdown correctamente.

## Enlaces e imágenes

- **Insertar → Enlace…** abre un diálogo con texto y URL. Si tenías selección,
  pasa como texto.
- **Ctrl+clic** sobre un enlace lo abre en el navegador del sistema; al pasar
  el ratón por encima se muestra la URL en la barra de estado.
- **Imágenes**: arrastra un archivo, pega una imagen del portapapeles o usa
  **Insertar → Pegar imagen**. La imagen se guarda como PNG junto al `.md` y
  se inserta como `![alt](ruta-relativa)`; así sobrevive al round-trip a
  Markdown (las imágenes incrustadas, no).

## Tablas

- **Tabla → Insertar tabla…** pide filas y columnas.
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
- En el editor, las fórmulas se ven con cursiva y color de acento del tema,
  con **super y subíndices reales** (no caracteres Unicode planos): `x²`,
  `Hᵢ`, `∑ₙ₌₁` con el límite encima/debajo se aproximan mejor que con texto.
- **Doble clic** sobre una fórmula reabre el diálogo con su TeX original
  precargado: editas y al aceptar se sustituye.
- Las fórmulas son **atómicas**: si tecleas dentro la app te recuerda que
  uses el doble clic; Backspace/Suprimir en su borde borran el grupo entero.
- Al **exportar** se conservan: a LaTeX se vuelcan tal cual (con `amsmath` y
  `amssymb` en el preámbulo); a HTML/PDF/ODF se preserva el super/subíndice
  vertical-align de Qt en el formato destino.
- En la **vista de código** se ven como `$...$` / `$$...$$`, con todos los
  caracteres TeX (`\sum`, `\frac`, `_`, `*`) intactos al guardar.

Ejemplos:

```
La energía es $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitación: en la fuente, `$$...$$` puede cruzar varias líneas (estilo
> Obsidian/Pandoc); `$...$` debe abrir y cerrar en la misma línea.

## Buscar y reemplazar

- **Buscar** (Ctrl+F) abre una barra inferior con campos para buscar y
  reemplazar, además de opciones (caso, palabra completa).
- **Buscar siguiente** F3 / **Buscar anterior** Shift+F3.

## Esquema del documento

El panel lateral izquierdo muestra el índice de encabezados (TOC): se
actualiza al escribir y, al hacer clic en una entrada, el cursor salta a ese
encabezado. Se muestra/oculta con F9.

## Modo sin distracciones

**Ver → Sin distracciones** (F11) entra en pantalla completa con menú y
barras ocultas y el texto centrado en una columna de lectura. El esquema,
si está visible, queda pegado al bloque central. ESC o F11 salen.

## Vista de código

**Ver → Vista de código** alterna entre el editor visual y un editor de texto
plano con el Markdown crudo. Los cambios en el modo fuente se vuelcan al
documento al volver al modo visual.

## Exportar e imprimir

**Archivo → Exportar** ofrece **PDF**, **HTML**, **ODF (.odt)** y
**LaTeX (.tex)**. En ODF y LaTeX se incrusta el idioma del documento
(tomado del front matter `lang`/`language`, del ajuste de la aplicación o,
en último caso, del idioma del sistema).

**Archivo → Imprimir** (Ctrl+P) abre el diálogo del sistema.

## Temas y apariencia

- **Ver → Tema** ofrece Claro, Oscuro, GitHub Light, GitHub Dark, Monokai y
  Alto contraste.
- **Ver → Luz cálida nocturna** atenúa los azules del fondo según la hora.
- **Zoom**: Ctrl+rueda del ratón, Ctrl++ / Ctrl+- y **Tamaño normal** (Ctrl+0)
  escalan toda la interfaz (no sólo el texto del editor).
- **Ver → Idioma** cambia el idioma de la interfaz; se aplica al reiniciar.

## Recuperación automática

Mientras editas, el contenido se autoguarda cada pocos segundos en una copia
de borrador. Si la aplicación se cierra de forma anómala, al volver a abrirla
ofrece recuperar lo que estabas escribiendo.

## Atajos

| Acción                    | Atajo            |
|---------------------------|------------------|
| Nuevo                     | Ctrl+N           |
| Abrir                     | Ctrl+O           |
| Guardar                   | Ctrl+S           |
| Guardar como              | Ctrl+Shift+S     |
| Imprimir                  | Ctrl+P           |
| Deshacer / Rehacer        | Ctrl+Z / Ctrl+Y  |
| Negrita / Cursiva         | Ctrl+B / Ctrl+I  |
| Subrayado                 | Ctrl+U           |
| Buscar                    | Ctrl+F           |
| Buscar siguiente/anterior | F3 / Shift+F3    |
| Encabezado H1 … H6        | Ctrl+1 … Ctrl+6  |
| Insertar fórmula          | Ctrl+Shift+F     |
| Vista de código Markdown  | Ctrl+Shift+M     |
| Esquema                   | F9               |
| Sin distracciones         | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ayuda                     | F1               |
