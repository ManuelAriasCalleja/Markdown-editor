# Características

Resumen de todo lo que ofrece md-editor. Para la referencia completa y técnica,
consulta `especificacion.md` en el repositorio.

## Edición WYSIWYG y round-trip

Editas sobre el texto renderizado y, al guardar, se serializa a Markdown limpio en
UTF-8. Lo que abres es lo que guardas: tablas con alineación, listas anidadas, listas
de tareas, citas, bloques de código y fórmulas se conservan fielmente.

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

Panel lateral (F9) con el índice de encabezados; un clic salta a la sección.

## Fórmulas TeX

Fórmulas en línea (`$...$`) y en bloque (`$$...$$`) con sintaxis LaTeX, sin
dependencias externas:

- Inserción con vista previa en vivo (Ctrl+Shift+F) y edición con doble clic.
- Super y subíndices reales, letras griegas, operadores, `\frac`, `\sqrt`, `\mathbb`…
- Son atómicas en el editor y sobreviven al round-trip y a la exportación.
- Limitaciones: `$...$` debe abrir y cerrar en la misma línea; no hay *layout* 2D
  (fracciones grandes como `(a)/(b)`).

## Resaltado de sintaxis

Los bloques de código se colorean según su lenguaje (familias C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… y un modo genérico).

## Imágenes

Pegar o soltar una imagen la guarda como PNG junto al documento y la inserta como
`![](ruta)` —no la incrusta—, de modo que el Markdown sigue siendo portable.

## Exportación e impresión

PDF, HTML, ODF (.odt) y LaTeX (.tex), más impresión (Ctrl+P). ODF y LaTeX incrustan
el idioma del documento (del front matter, del ajuste de la app o del sistema).

## Zoom de toda la interfaz

Ctrl++, Ctrl+- y Ctrl+0 (o Ctrl + rueda) escalan toda la interfaz, no solo el texto
del editor. El nivel se recuerda.

## Buscar y reemplazar

Ctrl+F / Ctrl+H, con anterior/siguiente, reemplazar todo y sensibilidad a mayúsculas.

## Archivos y seguridad de tus datos

- **Archivos recientes**, apertura por arrastre y confirmación de cambios sin guardar.
- **Front matter** YAML/TOML conservado verbatim.
- **Vigilancia del archivo en disco**: detecta cambios externos y ofrece recargar.
- **Autoguardado y recuperación** tras un cierre anómalo.

## Internacionalización

Interfaz en 9 idiomas: español, inglés, alemán, francés, italiano, portugués, polaco,
neerlandés y rumano (Ver → Idioma; se aplica al reiniciar).
