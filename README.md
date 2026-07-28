# md-editor

A Markdown editor where you write on the rendered text and save clean Markdown.
TeX formulas, aligned tables, highlighted code, document templates, and export to
PDF/DOCX/ODT/LaTeX — lightweight, portable (Qt6/C++17, zero external
dependencies), in 9 languages.

![Version](https://img.shields.io/badge/version-2.8.1-blue)
![License](https://img.shields.io/badge/license-GPL--3.0-blue)
![Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20Windows%20%7C%20macOS-green)

![md-editor editing a document with TeX formulas, a code block, a chart, a table and a nested task list, with the outline panel open](docs/screenshot.png)

---

## Downloads

| System | File | Notes |
|--------|------|-------|
| **Linux** (x86_64) | [`md-editor-2.8.1-x86_64.AppImage`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Single-file executable. `chmod +x` and double-click. |
| **Windows** (x64) | [`md-editor-2.8.1-windows-x64.zip`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | Portable: unzip and run `md-editor.exe`. |
| **macOS** (Apple Silicon + Intel) | [`md-editor-2.8.1-macos-universal.dmg`](https://github.com/ManuelAriasCalleja/Markdown-editor/releases/latest) | First launch: Ctrl-click → *Open* (binary not signed). |

> All downloads, including previous versions, on the
> [releases page](https://github.com/ManuelAriasCalleja/Markdown-editor/releases).

---

## Installing on Windows

The Windows build is portable: **unzip and run `md-editor.exe`** — no installer.
The binary is not signed yet, so SmartScreen may show a blue *"Windows protected
your PC"* screen on first run. To proceed, click **More info → Run anyway**. This
only happens until the build earns SmartScreen reputation (or gets code-signed).

---

## Installing on macOS

The macOS build is **not signed or notarized** (Apple code signing requires a
paid Developer account). Gatekeeper will therefore block it on first launch with
a *"cannot be opened because it is from an unidentified developer"* message. This
is expected and the app is safe — to open it the first time:

1. Open the `.dmg` and drag **md-editor** into *Applications*.
2. In *Applications*, **Ctrl-click** (or right-click) the app and choose **Open**.
3. Confirm **Open** in the dialog that appears.

You only need to do this once; afterwards it launches normally with a
double-click. Alternatively, after the blocked attempt, go to *System Settings →
Privacy & Security* and click **Open anyway**.

---

## What it does

- **True WYSIWYG**: you never see the Markdown syntax, you see the rendered
  output.
- **Multiple documents in tabs**: open several files at once, each in its own
  tab; the open tabs reopen on the next launch.
- **Clean round-trip**: what you open is what you save. Aligned tables, quotes,
  nested lists, task lists, footnotes, code blocks with syntax highlighting.
- **TeX formulas** `$…$` and `$$…$$` with real super- and subscripts and a
  live preview — no external dependencies. Double-click to edit.
- **Diagrams**: `mermaid` and `plantuml` code blocks are previewed as an image
  below the block (needs the external `mmdc` / `plantuml` tool; degrades
  gracefully with an install hint if it is missing).
- **Admonitions / callouts** (`> [!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]`,
  `[!CAUTION]`) shown as coloured boxes, round-trip compatible with GitHub.
- **Spell checking** (Hunspell): misspellings underlined in the document's
  language, with suggestions and a personal dictionary.
- **Document templates** (*File → New from template*) and **reusable snippets**
  (*Insert → Snippet*) for content you write often.
- **Export** to PDF, HTML, ODF (`.odt`), LaTeX (`.tex`), DOCX (`.docx`), EPUB
  (`.epub`) and plain text (`.txt`), preserving the document language and the
  formula formatting; the PDF embeds the title and author from the front matter.
- **Import from HTML, EPUB and more** (*File → Import*): converts a web page or an
  EPUB book to Markdown natively, and DOCX/ODT/RTF/LaTeX/reStructuredText… via Pandoc
  (if installed); opens the result as a new untitled document.
- **YAML / TOML front matter** preserved verbatim on save.
- **Navigable outline panel** (F9), find and replace (Ctrl+F / Ctrl+H),
  autosave and crash recovery.
- **Distraction-free mode** (F11) and **focus mode** (typewriter scrolling +
  dimming), full-interface zoom (Ctrl+wheel), 6 light and dark themes including a
  warm night light, Markdown source view (Ctrl+Shift+M).
- **9 languages**: Spanish, English, German, French, Italian, Portuguese,
  Polish, Dutch and Romanian.
- **Paste / drop images** from the clipboard straight to disk as `![](path)`.
- **External file watching**: if the file changes on disk, the app detects it
  and offers to reload.
- **Accessibility**: accessible names on the editor, panels and controls; status
  messages announced to screen readers; full keyboard operation; a true
  high-contrast theme and whole-interface zoom for low vision.

## Common shortcuts

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` / `Ctrl+O` / `Ctrl+S` | New / Open / Save |
| `Ctrl+Shift+S` / `Ctrl+P` | Save as… / Print |
| `Ctrl+B` / `Ctrl+I` / `Ctrl+U` | Bold / Italic / Underline |
| `Ctrl+K` / `Ctrl+Shift+F` | Insert link / formula |
| `Ctrl++` / `Ctrl+-` / `Ctrl+0` | Zoom in / out / reset |
| `Ctrl+Shift+M` | View / edit the Markdown source |
| `Ctrl+Shift+P` | Command palette (find & run any action) |
| `F11` / `F9` / `F1` | Distraction-free / Outline / Help |

Full list under *Help → Manual* inside the app.

---

## Building from source

> **Legal note**: the code is **free software** under **GPL-3.0**. You may
> clone, build, modify and redistribute it, as long as derivatives stay under
> the GPL-3.0. See [License](#license).

### Dependencies

- CMake ≥ 3.16
- Qt 6.5 or higher (modules `Widgets`, `PrintSupport`, `LinguistTools`, `Test`)
  **plus its private headers**: the ODF export uses Qt's private QZip. On
  Debian/Ubuntu these ship in a separate package from `qt6-base-dev`, and
  without them CMake fails at configure time with *"Imported target
  `Qt6::GuiPrivate` includes non-existent path"*.
- A C++17 compiler (GCC 9+, Clang 10+, MSVC 19.20+)
- **Optional**: Hunspell, for spell checking. Without it everything else builds
  and the spell checker is simply inactive. On Linux the dictionaries are the
  system ones (`hunspell-en-us`, `hunspell-es`…).

```bash
# Debian / Ubuntu
sudo apt-get install cmake g++ qt6-base-dev qt6-base-private-dev libhunspell-dev
```

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

Tests use **Qt Test**. CMake picks the platform plugin per system: headless
(`offscreen`) on Linux and macOS, native on Windows — where `offscreen` has no
font database, which breaks Markdown serialisation and silences the test output.

### Installation (Linux, optional)

```bash
sudo ./install.sh                     # → /usr/local
PREFIX="$HOME/.local" ./install.sh    # → ~/.local (no sudo)
```

Installs the binary, the `.desktop` launcher and hicolor icons (PNG + SVG).

---

## License

This software is **free software**, distributed under the **GNU General Public
License v3.0** ([**GPL-3.0**](https://www.gnu.org/licenses/gpl-3.0.html)).

Copyright © 2026 Manuel Arias Calleja.

In short:

- ✅ You **may** use, study, and run the program for any purpose.
- ✅ You **may** redistribute copies, source or binary.
- ✅ You **may** modify it and distribute your modified versions — **provided**
  they are also released under the GPL-3.0 (same freedoms for everyone).
- ❌ You **may not** distribute a closed-source or proprietary derivative.

This is a strong copyleft licence: any fork must stay free and open under the
same terms. Full text in [`LICENSE`](LICENSE).

### Contributions

This project **does not accept pull requests**. Development is by the author
only. If you find a bug or have a suggestion, open an *issue* and the author
will consider it. (The licence lets you fork and modify your own copy; that is
independent of this repository's contribution policy.)

---

## Author

**Manuel Arias Calleja** — <manuelariascalleja@gmail.com>

If you find it useful, please ⭐ the repository — that is the simplest way to
let me know it is helping someone.
