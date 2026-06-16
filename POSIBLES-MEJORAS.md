# Posibles mejoras

Lista de mejoras propuestas para **md-editor**, agrupadas y priorizadas. El
proyecto es sólido (~9k LOC, 18 ficheros de test, arquitectura por controllers,
9 idiomas), así que las mejoras son sobre todo de alcance y distribución.

## 🎯 Alto impacto (poco esfuerzo / mucho valor)

1. **CI/CD con GitHub Actions** — automatizar el build de los 3 ejecutables
   (Linux/Windows/macOS) y la publicación de releases con tags. Hoy es manual;
   esto elimina errores y acelera versiones. *Probablemente la mejora número uno.*
2. **Packaging para gestores nativos** — Flatpak/AppStream o AUR (Linux),
   winget/Chocolatey (Windows), Homebrew cask (macOS). Multiplica la visibilidad
   frente al `.AppImage`/`.zip` suelto y mejora la confianza.
3. **Firma de binarios** — macOS y Windows no están firmados (Ctrl-clic → Abrir).
   Firmar (y notarizar en Mac) elimina la fricción de instalación, que es donde
   se pierden usuarios.
4. **Auto-actualización o aviso de nueva versión** — al ser descarga manual, los
   usuarios de una versión antigua nunca sabrán de la nueva. Un chequeo ligero
   contra la API de releases bastaría.

## ✨ Funcionalidad

5. **Corrector ortográfico** — casi imprescindible en un editor de texto; Qt +
   Hunspell encaja bien con los 9 idiomas que ya soporta.

   **Análisis (multiplataforma y dependencias).** Qt6 no trae corrector
   (`QSpellChecker` no existe), hay que aportar el motor. La opción que mantiene
   el carácter portable del proyecto (sin `#ifdef Q_OS_*`) es **Hunspell** (C++
   puro; lo usan Chrome, Firefox, LibreOffice). Alternativas descartadas: APIs
   nativas (NSSpellChecker / ISpellChecker / enchant) obligan a código por
   plataforma y Objective-C++ en Mac; KDE Sonnet arrastra KDE Frameworks.
   - *¿Funciona en Win/Linux/Mac?* Sí con Hunspell: mismo C++ en los tres y la
     integración (subrayado `QTextCharFormat::SpellCheckUnderline` + menú de
     sugerencias) es API pura de Qt. **Matiz**: `QTextDocument` admite un solo
     `QSyntaxHighlighter` y ya lo ocupa `CodeBlockHighlighter`; el subrayado hay
     que integrarlo ahí o aplicarlo por `QTextEdit::setExtraSelections`.
   - *¿Dependencias / todo en el ejecutable?* El **motor** sería la primera
     dependencia de terceros, pero se puede vendorizar y enlazar estático → viaja
     dentro del binario. Los **diccionarios** (.aff/.dic) son datos (~30–50 MB
     los 9 idiomas): o embebidos en `.qrc` (binario más grande + extraer a
     temporal, porque Hunspell lee de rutas de fichero), o como ficheros junto al
     binario (vía `install.sh`), o descarga bajo demanda.
   - *Avisos:* licencias de diccionarios variadas (GPL/LGPL/MPL/BSD según idioma,
     revisar dado lo sensible que es la licencia aquí); y es la mejora de
     funcionalidad más cara tras multi-documento (1ª dependencia externa +
     integración en el highlighter único + empaquetado en 3 SO).
6. ✅ **Contador de palabras / tiempo de lectura / estadísticas** del documento.
   *Hecho:* módulo puro `docstats`, contador con tiempo de lectura en la barra de
   estado y diálogo *Ver → Estadísticas del documento…*.
7. **Pestañas o multi-documento** — `mainwindow` es de documento único.
8. ✅ **Export a DOCX** — ya hay PDF/HTML/ODT/LaTeX; `.docx` es el formato que más
   pide quien no usa Markdown. *Hecho:* serializador OOXML propio
   (`mdexport::writeDocx`) empaquetado con el QZip privado de Qt (sin
   dependencias), con encabezados, formato de carácter, listas, tablas, citas,
   código, enlaces (campo HYPERLINK) e imágenes embebidas; idioma y título
   incrustados. *Archivo → Exportar → A DOCX (Word)*.
9. **Diagramas** (Mermaid/PlantUML) — complementaría el soporte TeX existente.
10. **Insertar índice (TOC) automático** y **footnotes**, si no están cubiertos.
    *Hecho el TOC:* `mdoutline::tableOfContentsMarkdown` + *Insertar → Índice
    (TOC)*. Pendientes las footnotes (revisar antes su round-trip con `toMarkdown`).
11. ✅ **Tema automático según el sistema** (seguir el modo claro/oscuro del SO).
    *Hecho:* opción *Ver → Tema → Seguir el sistema* vía `QStyleHints`.

## 🧹 Calidad de código

12. **Descomponer `mainwindow.cpp` (1507 líneas)** — es 3× el siguiente fichero.
    Mover lógica de menús/acciones a uno o dos controllers más reduciría ese
    "God object".
13. **`mathblocks.cpp` (982 líneas)** — segundo candidato a dividir (parser vs.
    render/edición).
14. **Static analysis en CI** — `clang-tidy` + compilar tests con ASAN/UBSAN.
    Para un parser con round-trip, los sanitizers cazan bugs de memoria que los
    tests funcionales no ven.
15. **Fuzzing del round-trip Markdown** — ya existe `tst_markdownroundtrip`; un
    fuzzer (libFuzzer) sobre "parse→serialize→parse == idempotente" da robustez
    con poco código.

## ✅ Mejoras baratas (hechas)

Lote de mejoras de bajo coste, **sin dependencias nuevas** y portables a los 3 SO
(Qt6 puro), ya implementadas:

- **Vista previa de impresión** — `ExportController::printPreview()`
  (`QPrintPreviewDialog`), *Archivo → Vista previa de impresión*.
- **Recordar la posición del cursor por archivo** — mapa acotado en `AppSettings`;
  `FileController` la guarda al cambiar de documento y al cerrar, y la restaura al
  abrir.
- **Abrir la carpeta contenedora** — `FileController::openContainingFolder()` vía
  `QDesktopServices`, *Archivo → Abrir carpeta contenedora*.
- **Pegar como texto plano** — *Editar* (Ctrl+Shift+V).
- **Buscar con regex y palabra completa** — casillas en `FindReplaceBar` (regex
  validada + `FindWholeWords`).
- **Copiar como HTML al portapapeles** — `ExportController::copyHtmlToClipboard()`,
  *Editar → Copiar como HTML*.
- **Transformar texto y ordenar líneas** — módulo puro `mdtext`
  (MAYÚSCULAS/minúsculas/Capitalizar/ordenar líneas), *Editar → Transformar texto*.

## 📋 Proyecto / comunidad

16. **CHANGELOG.md** — facilita saber qué cambió entre versiones.
17. **Repensar la licencia CC BY-ND** — el README dice que no se aceptan PRs de
    código por la licencia. Es legítimo, pero corta el crecimiento por
    contribuciones. Si algún día se quiere comunidad, una licencia de software
    (MIT/GPL) sería el mayor desbloqueo. *Solo si es un objetivo.*
18. **Analítica de descargas con privacidad** — GitHub no dice *quién* descarga;
    un redirector propio daría país/volumen sin rastrear personas.

---

> **Prioridad sugerida:** empezar por la #1 (workflow de GitHub Actions para
> build + release multiplataforma), que es la que más palanca tiene y deja todo
> lo demás más fácil de publicar.
