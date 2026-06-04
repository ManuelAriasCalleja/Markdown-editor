# Uso

## Abrir y guardar

- Nuevo (Ctrl+N), Abrir (Ctrl+O), Guardar (Ctrl+S), Guardar como (Ctrl+Shift+S).
  Todo en UTF-8.
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
  Enter (un punto vacío sale de la lista).
- Citas y bloques de código.

Consulta todos los atajos en [Atajos de teclado](Atajos).

## Insertar

- Enlace e Imagen (con ruta relativa al documento para que sea portable).
- **Pegar imagen**: la imagen del portapapeles se guarda como PNG junto a tu `.md` y
  se inserta como `![](ruta)`. También funciona arrastrando o pegando sobre el editor.
- Tabla, Regla horizontal y Fórmula (Ctrl+Shift+F).

## Tablas

Con el cursor dentro de una tabla, el menú Tabla permite añadir o eliminar filas y
columnas y alinear cada columna (izquierda/centro/derecha). La alineación se conserva
al guardar.

## Fórmulas

Inserta fórmulas TeX en línea (`$...$`) o en bloque (`$$...$$`) con Insertar → Fórmula
(Ctrl+Shift+F), con vista previa en vivo. Doble clic sobre una fórmula la edita. Más
detalle en [Características](Caracteristicas#fórmulas-tex).

## Modos de vista

- **WYSIWYG** (por defecto): solo el resultado renderizado.
- **Código fuente** (Ctrl+Shift+M): el Markdown crudo, a pantalla completa.
- **Vista dividida** (Ctrl+Shift+D): render y código lado a lado, sincronizados.

## Buscar y reemplazar

Ctrl+F para buscar, Ctrl+H para reemplazar. Incluye anterior/siguiente, reemplazar
todo y sensibilidad a mayúsculas.

## Exportar e imprimir

Archivo → Exportar ofrece PDF, HTML, ODF (.odt) y LaTeX (.tex); Imprimir es Ctrl+P.
En ODF y LaTeX se incrusta el idioma del documento.

## Recuperación automática

md-editor guarda un borrador cada pocos segundos. Si la aplicación se cierra de forma
anómala, al reabrir te ofrece recuperar lo que estabas escribiendo.
