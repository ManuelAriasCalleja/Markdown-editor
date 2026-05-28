# md-editor

Editor **WYSIWYG** de Markdown en Qt6 + C++17. Se edita siempre sobre el texto
ya renderizado, y al guardar se serializa de vuelta a Markdown limpio.

![Versión](https://img.shields.io/badge/versi%C3%B3n-1.0.0-blue)
![Licencia](https://img.shields.io/badge/licencia-CC%20BY--ND%204.0-lightgrey)
![Plataformas](https://img.shields.io/badge/plataformas-Linux%20%7C%20Windows%20%7C%20macOS-green)

---

## Descargas

| Sistema | Archivo | Notas |
|---------|---------|-------|
| **Linux** (x86_64) | [`md-editor-1.0.0-x86_64.AppImage`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Ejecutable único. `chmod +x` y doble clic. |
| **Windows** (x64) | [`md-editor-1.0.0-windows-x64.zip`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Portable: descomprime y ejecuta `md-editor.exe`. |
| **macOS** (Apple Silicon + Intel) | [`md-editor-1.0.0-macos.dmg`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | La primera vez: Ctrl+clic → *Abrir* (binario sin firmar). |

> Todas las descargas, incluidas versiones anteriores, en la
> [página de releases](https://github.com/ManuelAriasCalleja/Markdown-editor/releases).

---

## Qué hace

- **WYSIWYG real**: nunca ves la sintaxis Markdown, ves el resultado renderizado.
- **Round-trip limpio**: lo que abres es lo que guardas. Tablas con alineación,
  citas, listas anidadas, listas de tareas, bloques de código con resaltado.
- **Fórmulas TeX** `$…$` y `$$…$$` con super/subíndices reales y previsualización
  Unicode — sin dependencias externas. Doble clic para editar.
- **Exportación** a PDF, HTML, ODF (`.odt`) y LaTeX (`.tex`), conservando el
  idioma del documento y el formato de las fórmulas.
- **Front matter** YAML/TOML preservado verbatim al guardar.
- **Esquema** lateral navegable (F9), búsqueda y reemplazo (Ctrl+F / Ctrl+H),
  autoguardado y recuperación tras cierre inesperado.
- **Modo sin distracciones** (F11), zoom de toda la interfaz (Ctrl+rueda),
  6 temas claros/oscuros con luz cálida nocturna, modo fuente Markdown
  (Ctrl+Shift+M).
- **9 idiomas**: español, inglés, alemán, francés, italiano, portugués, polaco,
  neerlandés y rumano.
- **Pegar/soltar imágenes** del portapapeles directo a disco como `![](ruta)`.
- **Vigilancia del archivo en disco**: si cambia desde fuera, avisa y propone
  recargar.

## Atajos más usados

| Atajo | Acción |
|-------|--------|
| `Ctrl+N` / `Ctrl+O` / `Ctrl+S` | Nuevo / Abrir / Guardar |
| `Ctrl+Shift+S` / `Ctrl+P` | Guardar como… / Imprimir |
| `Ctrl+B` / `Ctrl+I` / `Ctrl+U` | Negrita / Cursiva / Subrayado |
| `Ctrl+K` / `Ctrl+Shift+F` | Insertar enlace / fórmula |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom de toda la interfaz |
| `Ctrl+Shift+M` | Ver / editar el Markdown fuente |
| `F11` / `F9` / `F1` | Sin distracciones / Esquema / Manual |

Lista completa en *Ayuda → Manual* dentro de la aplicación.

---

## Compilación desde el código

> **Nota legal**: el código se publica con licencia **CC BY-ND 4.0**. Puedes
> clonarlo y compilarlo para uso propio; **no puedes distribuir versiones
> modificadas**. Ver [Licencia](#licencia).

### Dependencias

- CMake ≥ 3.16
- Qt 6.5 o superior (módulos `Widgets`, `PrintSupport`, `LinguistTools`, `Test`)
- Compilador C++17 (GCC 9+, Clang 10+, MSVC 19.20+)

### Linux / macOS

```bash
cmake -S . -B build
cmake --build build
./build/md-editor [archivo.md]
```

Atajo: `./build.sh -x ejemplo.md` configura, compila y ejecuta.

### Windows (PowerShell)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\Release\md-editor.exe
```

### Pruebas

```bash
ctest --test-dir build --output-on-failure
```

Las pruebas usan **Qt Test** y se ejecutan sin pantalla
(`QT_QPA_PLATFORM=offscreen`, fijado por CMake).

### Instalación (Linux, opcional)

```bash
sudo ./install.sh                     # → /usr/local
PREFIX="$HOME/.local" ./install.sh    # → ~/.local (sin sudo)
```

Instala binario, lanzador `.desktop` e iconos hicolor (PNG + SVG).

---

## Licencia

Este software se distribuye bajo la **Creative Commons
Atribución-SinObraDerivada 4.0 Internacional** ([**CC BY-ND 4.0**](https://creativecommons.org/licenses/by-nd/4.0/deed.es)).

En resumen:

- ✅ **Puedes** ver el código fuente, descargarlo y compilarlo para uso propio.
- ✅ **Puedes** usar el binario para cualquier propósito (incluido comercial).
- ✅ **Puedes** redistribuir el binario o el código sin cambios, citando al autor.
- ❌ **No puedes** modificar el código y distribuir esa versión modificada.
- ❌ **No puedes** crear obras derivadas a partir de este código.

Texto íntegro en [`LICENSE`](LICENSE).

### Contribuciones

Por las condiciones de la licencia, **este proyecto no acepta pull requests
con cambios al código**. Si encuentras un fallo o tienes una sugerencia, abre
un *issue* y será valorado por el autor.

---

## Autor

**Manuel Arias Calleja** — <manuelariascalleja@gmail.com>

Si te resulta útil, ⭐ el repositorio: es la forma más sencilla de saber que
está ayudando a alguien.
