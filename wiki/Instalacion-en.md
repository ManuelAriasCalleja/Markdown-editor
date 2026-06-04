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
- **Qt 6** ≥ 6.5 (modules *Widgets*, *PrintSupport*, *LinguistTools*)
- **C++17** compiler (GCC 9+, Clang 10+ or MSVC 19.20+)

There are no third-party dependencies beyond Qt.

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
