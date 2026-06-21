# Registro de cambios

Todos los cambios relevantes de **md-editor** se documentan en este archivo.

El formato sigue, a grandes rasgos, [Keep a Changelog](https://keepachangelog.com/es/),
y el proyecto usa [versionado semántico](https://semver.org/lang/es/).

## [Sin publicar]

### Añadido
- **Fórmulas en 2D real («Nivel 2»)**: las fracciones se apilan con barra real,
  los grandes operadores (`\sum`, `\int`, `\prod`…) muestran sus límites encima y
  debajo, las raíces (`\sqrt`, `\sqrt[n]`) llevan vínculo sobre el radicando y las
  matrices (`pmatrix`, `bmatrix`…) se maquetan como rejilla con paréntesis o
  corchetes, en vez de aproximarse en línea. Se pintan vectorialmente, escalan con
  el zoom y siguen al tema. Las fórmulas más simples se siguen componiendo en
  línea. La exportación a HTML/ODF/PDF/DOCX y el round-trip Markdown no cambian.

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
