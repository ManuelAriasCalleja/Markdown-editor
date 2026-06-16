# md-editor

**WYSIWYG** Markdown editor written in Qt6 + C++17. You always edit on the
already-rendered text — never the syntax — and on save it is serialized back
to clean Markdown.

![Version](https://img.shields.io/badge/version-1.1.0-blue)
![License](https://img.shields.io/badge/license-CC%20BY--ND%204.0-lightgrey)
![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20Windows%20%7C%20macOS-green)

---

## Downloads

| System | File | Notes |
|--------|------|-------|
| **Linux** (x86_64) | [`md-editor-1.1.0-x86_64.AppImage`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Single-file executable. `chmod +x` and double-click. |
| **Windows** (x64) | [`md-editor-1.1.0-windows-x64.zip`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Portable: unzip and run `md-editor.exe`. |
| **macOS** (Apple Silicon + Intel) | [`md-editor-1.1.0-macos-universal.dmg`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | First launch: Ctrl-click → *Open* (binary not signed). |

> All downloads, including previous versions, on the
> [releases page](https://github.com/ManuelAriasCalleja/Markdown-editor/releases).

---

## What it does

- **True WYSIWYG**: you never see the Markdown syntax, you see the rendered
  output.
- **Clean round-trip**: what you open is what you save. Aligned tables, quotes,
  nested lists, task lists, code blocks with syntax highlighting.
- **TeX formulas** `$…$` and `$$…$$` with real super- and subscripts and a
  live preview — no external dependencies. Double-click to edit.
- **Export** to PDF, HTML, ODF (`.odt`) and LaTeX (`.tex`), preserving the
  document language and the formula formatting.
- **YAML / TOML front matter** preserved verbatim on save.
- **Navigable outline panel** (F9), find and replace (Ctrl+F / Ctrl+H),
  autosave and crash recovery.
- **Distraction-free mode** (F11), full-interface zoom (Ctrl+wheel), 6 light
  and dark themes including a warm night light, Markdown source view
  (Ctrl+Shift+M).
- **9 languages**: Spanish, English, German, French, Italian, Portuguese,
  Polish, Dutch and Romanian.
- **Paste / drop images** from the clipboard straight to disk as `![](path)`.
- **External file watching**: if the file changes on disk, the app detects it
  and offers to reload.

## Common shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` / `Ctrl+O` / `Ctrl+S` | New / Open / Save |
| `Ctrl+Shift+S` / `Ctrl+P` | Save as… / Print |
| `Ctrl+B` / `Ctrl+I` / `Ctrl+U` | Bold / Italic / Underline |
| `Ctrl+K` / `Ctrl+Shift+F` | Insert link / formula |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom in / out / reset |
| `Ctrl+Shift+M` | View / edit the Markdown source |
| `F11` / `F9` / `F1` | Distraction-free / Outline / Help |

Full list under *Help → Manual* inside the app.

---

## Building from source

> **Legal note**: the code is released under **CC BY-ND 4.0**. You may clone
> and build it for your own use; **you may not distribute modified versions**.
> See [License](#license).

### Dependencies

- CMake ≥ 3.16
- Qt 6.5 or higher (modules `Widgets`, `PrintSupport`, `LinguistTools`, `Test`)
- A C++17 compiler (GCC 9+, Clang 10+, MSVC 19.20+)

### Linux / macOS

```bash
cmake -S . -B build
cmake --build build
./build/md-editor [file.md]
```

Shortcut: `./build.sh -x example.md` configures, builds and runs.

### Windows (PowerShell)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\Release\md-editor.exe
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```

Tests use **Qt Test** and run headless (`QT_QPA_PLATFORM=offscreen`, set by
CMake).

### Installation (Linux, optional)

```bash
sudo ./install.sh                     # → /usr/local
PREFIX="$HOME/.local" ./install.sh    # → ~/.local (no sudo)
```

Installs the binary, the `.desktop` launcher and hicolor icons (PNG + SVG).

---

## License

This software is distributed under the **Creative Commons
Attribution-NoDerivatives 4.0 International** ([**CC BY-ND 4.0**](https://creativecommons.org/licenses/by-nd/4.0/)) license.

In short:

- ✅ You **may** read the source code, download it and build it for your own use.
- ✅ You **may** use the binary for any purpose (including commercial).
- ✅ You **may** redistribute the binary or the unmodified source code, with
  attribution.
- ❌ You **may not** modify the source code and distribute that modified version.
- ❌ You **may not** create derivative works from this code.

Full text in [`LICENSE`](LICENSE).

### Contributions

Because of the license, **this project does not accept pull requests with
code changes**. If you find a bug or have a suggestion, open an *issue* and
the author will consider it.

---

## Author

**Manuel Arias Calleja** — <manuelariascalleja@gmail.com>

If you find it useful, please ⭐ the repository — that is the simplest way to
let me know it is helping someone.
