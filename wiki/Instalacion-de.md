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
- **Qt 6** ≥ 6.5 (Module *Widgets*, *PrintSupport*, *LinguistTools*)
- **C++17**-Compiler (GCC 9+, Clang 10+ oder MSVC 19.20+)

Außer Qt gibt es keine Abhängigkeiten von Drittanbietern.

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
