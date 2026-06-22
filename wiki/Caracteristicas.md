# Características

Resumen de todo lo que ofrece md-editor. Para la referencia completa y técnica,
consulta `especificacion.md` en el repositorio.

## Edición WYSIWYG y round-trip

Editas sobre el texto renderizado y, al guardar, se serializa a Markdown limpio en
UTF-8. Lo que abres es lo que guardas: tablas con alineación, listas anidadas, listas
de tareas, citas, bloques de código, notas al pie, admoniciones y fórmulas se
conservan fielmente.

## Edición por pestañas

Abre varios documentos a la vez, cada uno en su pestaña, y cambia entre ellos. Cerrar
pestaña con Ctrl+W. La sesión reabre las pestañas al volver a arrancar.

## Modos de vista

- WYSIWYG, Código fuente (Ctrl+Shift+M) y Vista dividida (Ctrl+Shift+D).
- En vista dividida, render y código se sincronizan: solo se actualiza el panel que no
  estás editando, sin saltos de cursor.

## Modo sin distracciones

F11 entra en pantalla completa con el texto centrado en una columna de lectura y sin
barras. ESC o F11 salen.

## Temas y luz cálida nocturna

- **Seis temas**: Claro, Oscuro, GitHub Light, GitHub Dark, Monokai y Alto contraste.
- **Luz cálida nocturna** (activada por defecto): atenúa el azul del fondo de forma
  automática y gradual según la hora, para reducir la fatiga visual de noche.
  Neutra de día (07–19 h), va calentándose por la tarde (19–23 h), máxima de noche
  (23–06 h) y se enfría al amanecer (06–07 h). Se reevalúa sola cada minuto y solo
  afecta al fondo (no a enlaces ni al resaltado).

## Esquema del documento

Panel lateral (F9) con el índice de encabezados; un clic salta a la sección. «Ir a
encabezado» (Ctrl+G) abre un buscador rápido de encabezados.

## Fórmulas TeX

Fórmulas en línea (`$...$`) y en bloque (`$$...$$`) con sintaxis LaTeX, sin
dependencias externas:

- Inserción con vista previa en vivo (Ctrl+Shift+F) y edición con doble clic.
- **Maquetación 2D real**: fracciones apiladas (`\frac`), raíces con vínculo
  (`\sqrt`), binomios (`\binom`), matrices y entornos (`matrix`, `pmatrix`, `cases`…),
  grandes operadores con límites encima y debajo (`\sum`, `\int`, `\prod`…), acentos
  (`\hat`, `\vec`…), super y subíndices reales, letras griegas y `\mathbb`.
- Son atómicas en el editor, escalan con el zoom y sobreviven al round-trip y a la
  exportación. Los bloques `$$...$$` pueden ocupar varias líneas.
- Limitaciones: `$...$` debe abrir y cerrar en la misma línea; las fórmulas 2D en
  línea quedan algo altas (las de bloque se ven bien).

## Corrección ortográfica (opcional)

Subraya las palabras mal escritas según el idioma del documento (Ver → Corrección
ortográfica). El idioma se elige solo (front matter, ajuste o sistema) o a mano (Ver
→ Idioma de corrección). Clic derecho ofrece sugerencias y añadir al diccionario
personal. Requiere Hunspell; sin él, el resto funciona igual.

## Diagramas (opcional)

Los bloques ```` ```mermaid ```` y ```` ```plantuml ```` se renderizan como imagen
bajo el bloque, ejecutando la herramienta externa (`mmdc` / `plantuml`) si está
instalada. Si falta, se muestra la orden de instalación para tu sistema. La imagen no
se guarda en el Markdown.

## Resaltado de sintaxis

Los bloques de código se colorean según su lenguaje (familias C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… y un modo genérico).

## Imágenes

Pegar o soltar una imagen la guarda como PNG junto al documento y la inserta como
`![](ruta)` —no la incrusta—, de modo que el Markdown sigue siendo portable.

## Insertar y transformar

- Insertar: enlace, imagen, tabla, regla, índice (TOC), fórmula, nota al pie,
  admonición (nota/aviso…), símbolos especiales y fecha/hora.
- Pegar como Markdown (Ctrl+Alt+V) convierte el HTML del portapapeles a Markdown.
- Transformar texto: MAYÚSCULAS/minúsculas, capitalizar, ordenar líneas y tipografía
  inteligente (—, –, …, comillas tipográficas).
- Estadísticas del documento: palabras, caracteres, párrafos, frases y tiempo de
  lectura.

## Exportación e impresión

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) y EPUB (.epub), más vista previa de
impresión e impresión (Ctrl+P). ODF, DOCX y LaTeX incrustan el idioma del documento
(del front matter, del ajuste de la app o del sistema).

## Zoom de toda la interfaz

Ctrl++, Ctrl+- y Ctrl+0 (o Ctrl + rueda) escalan toda la interfaz, no solo el texto
del editor. El nivel se recuerda.

## Buscar y reemplazar

Ctrl+F / Ctrl+H, con anterior/siguiente, reemplazar todo y sensibilidad a mayúsculas.

## Archivos y seguridad de tus datos

- **Archivos recientes**, apertura por arrastre y confirmación de cambios sin guardar.
- **Plantillas de documento** (Archivo → Nuevo desde plantilla).
- **Front matter** YAML/TOML conservado verbatim.
- **Vigilancia del archivo en disco**: detecta cambios externos y ofrece recargar.
- **Autoguardado y recuperación** tras un cierre anómalo.

## Internacionalización

Interfaz en 9 idiomas: español, inglés, alemán, francés, italiano, portugués, polaco,
neerlandés y rumano (Ver → Idioma; se aplica al reiniciar).
