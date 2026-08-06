# md-editor

Editor/visor WYSIWYG de Markdown en Qt6 + C++17. Por defecto editas sobre el texto
ya renderizado, sin lidiar con la sintaxis; pero opcionalmente puedes ver el código
Markdown, e incluso tener el código y su renderizado en paralelo (vista dividida) y
editar en cualquiera de los dos lados. Al guardar se serializa siempre a Markdown
limpio.

## Qué hace por ti

- **WYSIWYG real**: ves el resultado, no los símbolos.
- **Round-trip fiel**: lo que abres es lo que guardas, con tablas alineadas, listas
  de tareas, citas, bloques de código, notas al pie, admoniciones y fórmulas.
- **Edición por pestañas**: varios documentos abiertos a la vez, cada uno en su
  pestaña.
- **Tres formas de trabajar**: sólo renderizado (por defecto), sólo código, o ambos
  en paralelo (vista dividida sincronizada).
- **Modo sin distracciones**: columna de lectura centrada, sin barras (F11), con la
  tabla de contenidos opcional.
- **Cuidado ocular**: la *Luz cálida nocturna* atenúa el azul del fondo de forma
  gradual según la hora del día, para reducir la fatiga ocular por la noche.
- **Fórmulas TeX** con maquetación 2D real (fracciones apiladas, raíces, matrices,
  sumatorios con límites…) y vista previa en vivo, sin dependencias externas.
- **Corrector ortográfico** opcional (Hunspell) y **diagramas** Mermaid/PlantUML.
- **Exportación** a PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) y EPUB (.epub),
  conservando el idioma del documento y el formato de las fórmulas.
- **Visualización**: 1) 8 temas claros y oscuros, 2) zoom de toda la interfaz, 3)
  interfaz traducida a 10 idiomas.

## Empezar

- [Instalación](Instalacion)
- [Uso](Uso)
- [Características](Caracteristicas)
- [Atajos de teclado](Atajos)

---

*md-editor lo desarrolla Manuel Arias Calleja. Licencia GPL-3.0.*
