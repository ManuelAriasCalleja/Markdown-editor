# Installation

md-editor läuft unter Linux, Windows und macOS.

## Downloads (Binärdateien)

| Plattform | Format | Hinweise |
|---|---|---|
| Linux x86_64 | **AppImage** | eine einzige ausführbare Datei; gib ihr Ausführungsrechte und öffne sie |
| Windows x64 | **portables ZIP** | entpacken und ausführen; ohne Installer |
| macOS (Apple Silicon + Intel) | universelles **DMG** | unsigniert: der erste Start erfolgt mit **Ctrl-Klick → Öffnen** |

## Aus dem Quellcode kompilieren

### Voraussetzungen

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 mit seinen **privaten Headern** (Module *Widgets*, *PrintSupport*,
  *LinguistTools*). Die privaten Header (`Qt6::GuiPrivate`, das QZip von Qt) werden für
  den Export nach ODF/DOCX/EPUB benötigt. Unter Debian/Ubuntu sind sie in
  `qt6-base-private-dev`, getrennt von `qt6-base-dev`.
- **C++17**-Compiler (GCC 9+, Clang 10+ oder MSVC 19.20+)
- **Hunspell** *(optional)* für die Rechtschreibprüfung (`libhunspell-dev`); ohne es
  kompiliert der Rest genauso.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # optional (Rechtschreibprüfung)
```

Für das Rendern von **Diagrammen** (optional, zur Laufzeit) müssen `mmdc` (Mermaid)
und/oder `plantuml` (PlantUML) installiert sein.

### Kompilieren

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

Unter Windows kannst du den Generator *Visual Studio 17 2022* verwenden.

### Unter Linux installieren

Das Skript `install.sh` kompiliert einen Build minimaler Größe und installiert die
Binärdatei, den `.desktop`-Starter und die Desktop-Symbole:

```bash
sudo ./install.sh                    # nach /usr/local
PREFIX="$HOME/.local" ./install.sh   # Installation im Benutzerverzeichnis, ohne sudo
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```
