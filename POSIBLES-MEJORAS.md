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
6. **Contador de palabras / tiempo de lectura / estadísticas** del documento.
7. **Pestañas o multi-documento** — `mainwindow` es de documento único.
8. **Export a DOCX** — ya hay PDF/HTML/ODT/LaTeX; `.docx` es el formato que más
   pide quien no usa Markdown.
9. **Diagramas** (Mermaid/PlantUML) — complementaría el soporte TeX existente.
10. **Insertar índice (TOC) automático** y **footnotes**, si no están cubiertos.
11. **Tema automático según el sistema** (seguir el modo claro/oscuro del SO).

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
