# Installation

md-editor runs on Linux, Windows and macOS.

## Downloads (binaries)

| Platform | Format | Notes |
|---|---|---|
| Linux x86_64 | **AppImage** | single executable file; make it executable and open it |
| Windows x64 | **Portable ZIP** | unzip and run; no installer |
| macOS (Apple Silicon + Intel) | **DMG** universal | unsigned: first launch is **Ctrl-click → Open** |

## Build from source

### Requirements

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 with its **private headers** (modules *Widgets*, *PrintSupport*,
  *LinguistTools*). The private headers (`Qt6::GuiPrivate`, Qt's QZip) are needed to
  export to ODF/DOCX/EPUB. On Debian/Ubuntu they are in `qt6-base-private-dev`, separate
  from `qt6-base-dev`.
- **C++17** compiler (GCC 9+, Clang 10+ or MSVC 19.20+)
- **Hunspell** *(optional)* for the spell checker (`libhunspell-dev`); without it,
  everything else compiles the same.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # optional (spell checker)
```

For **diagram** rendering (optional, at runtime) you need to have `mmdc` (Mermaid)
and/or `plantuml` (PlantUML) installed.

### Build

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [file.md]
```

On Windows you can use the *Visual Studio 17 2022* generator.

### Install on Linux

The `install.sh` script builds a minimum-size build and installs the binary, the
`.desktop` launcher and the desktop icons:

```bash
sudo ./install.sh                    # to /usr/local
PREFIX="$HOME/.local" ./install.sh   # user install, no sudo
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```
