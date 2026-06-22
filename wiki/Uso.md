# Uso

## Abrir y guardar

- Nuevo (Ctrl+N), Abrir (Ctrl+O), Guardar (Ctrl+S), Guardar como (Ctrl+Shift+S).
  Todo en UTF-8.
- **Pestañas**: cada documento abierto ocupa su propia pestaña; cierra una con
  Ctrl+W. Al volver a arrancar se reabren las pestañas de la última sesión.
- **Nuevo desde plantilla** (Archivo → Nuevo desde plantilla) parte de un esqueleto
  Markdown ya preparado.
- **Abrir recientes** lista tus últimos documentos.
- También puedes arrastrar y soltar un archivo sobre la ventana para abrirlo.
- Si el archivo cambia fuera de md-editor, te avisa: lo recarga solo si no tenías
  cambios, o te pregunta si los tenías.

### Front matter

Si tu documento empieza con un bloque `---…---` (YAML) o `+++…+++` (TOML), se
conserva tal cual al guardar (no se ve ni se edita). Sirve para metadatos como
`title` y `lang`, que se usan al exportar.

## Dar formato

Usa el menú Formato o la barra de herramientas. No necesitas teclear símbolos
Markdown: los aplica el editor por ti.

- Negrita (Ctrl+B), Cursiva (Ctrl+I), Subrayado (Ctrl+U), Tachado, Código en línea,
  Enlace (Ctrl+K).
- Encabezados H1–H6 (Ctrl+1 … Ctrl+6).
- Listas de viñetas, numeradas y de tareas, con continuación automática al pulsar
  Enter (un punto vacío sale de la lista). Las casillas de tarea se marcan con un clic.
- Citas y bloques de código.

Consulta todos los atajos en [Atajos de teclado](Atajos).

## Editar y transformar texto

- **Pegar como texto plano** (Ctrl+Shift+V) o **Pegar como Markdown** (Ctrl+Alt+V),
  que convierte el HTML del portapapeles a Markdown. Pegar una URL sobre una
  selección la auto-enlaza.
- **Editar → Transformar texto**: MAYÚSCULAS, minúsculas, capitalizar, ordenar líneas
  y tipografía inteligente (convierte `--`, `---`, `...` y las comillas rectas).

## Insertar

- Enlace e Imagen (con ruta relativa al documento para que sea portable).
- **Pegar imagen**: la imagen del portapapeles se guarda como PNG junto a tu `.md` y
  se inserta como `![](ruta)`. También funciona arrastrando o pegando sobre el editor.
- Tabla, Regla horizontal, Índice (TOC) y Fórmula (Ctrl+Shift+F).
- **Nota al pie** (Ctrl+Shift+N): inserta una referencia `[^n]` y su definición.
- **Admonición**: bloque destacado (nota, consejo, importante, advertencia, precaución).
- **Símbolos especiales** y **Fecha / Fecha y hora**.

## Tablas

Con el cursor dentro de una tabla, el menú Tabla permite añadir o eliminar filas y
columnas y alinear cada columna (izquierda/centro/derecha). La alineación se conserva
al guardar.

## Fórmulas

Inserta fórmulas TeX en línea (`$...$`) o en bloque (`$$...$$`) con Insertar → Fórmula
(Ctrl+Shift+F), con vista previa en vivo. Doble clic sobre una fórmula la edita. Se
pintan en 2D real (fracciones, raíces, matrices, sumatorios con límites…). Más detalle
en [Características](Caracteristicas#fórmulas-tex).

## Diagramas

Escribe un bloque de código con lenguaje `mermaid` o `plantuml` y, si tienes
instalada la herramienta correspondiente (`mmdc` / `plantuml`), se renderiza como
imagen bajo el bloque. Si falta, verás la orden para instalarla.

## Corrección ortográfica

Actívala en Ver → Corrección ortográfica (requiere Hunspell). El idioma se elige por
el del documento o a mano en Ver → Idioma de corrección. Clic derecho sobre una
palabra subrayada ofrece sugerencias y añadirla al diccionario personal.

## Modos de vista

- **WYSIWYG** (por defecto): solo el resultado renderizado.
- **Código fuente** (Ctrl+Shift+M): el Markdown crudo, a pantalla completa.
- **Vista dividida** (Ctrl+Shift+D): render y código lado a lado, sincronizados.
- **Esquema** (F9) e **Ir a encabezado** (Ctrl+G) para navegar el documento.

## Buscar y reemplazar

Ctrl+F para buscar, Ctrl+H para reemplazar. Incluye anterior/siguiente, reemplazar
todo y sensibilidad a mayúsculas.

## Exportar e imprimir

Archivo → Exportar ofrece PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) y EPUB
(.epub); también Vista previa de impresión e Imprimir (Ctrl+P). En ODF, DOCX y LaTeX
se incrusta el idioma del documento.

## Recuperación automática

md-editor guarda un borrador cada pocos segundos. Si la aplicación se cierra de forma
anómala, al reabrir te ofrece recuperar lo que estabas escribiendo.
