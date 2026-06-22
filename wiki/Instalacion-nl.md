# Installatie

md-editor werkt op Linux, Windows en macOS.

## Downloads (binaries)

| Platform | Formaat | Opmerkingen |
|---|---|---|
| Linux x86_64 | **AppImage** | één enkel uitvoerbaar bestand; geef het uitvoeringsrechten en open het |
| Windows x64 | **draagbare ZIP** | uitpakken en uitvoeren; zonder installatieprogramma |
| macOS (Apple Silicon + Intel) | universele **DMG** | niet ondertekend: de eerste keer starten gaat met **Ctrl-klik → Openen** |

## Compileren vanuit de broncode

### Vereisten

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 met zijn **privé-headers** (modules *Widgets*, *PrintSupport*,
  *LinguistTools*). De privé-headers (`Qt6::GuiPrivate`, de QZip van Qt) zijn nodig om
  naar ODF/DOCX/EPUB te exporteren. Op Debian/Ubuntu zitten ze in
  `qt6-base-private-dev`, los van `qt6-base-dev`.
- **C++17**-compiler (GCC 9+, Clang 10+ of MSVC 19.20+)
- **Hunspell** *(optioneel)* voor de spellingcontrole (`libhunspell-dev`); zonder dit
  compileert de rest gewoon.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # optioneel (spellingcontrole)
```

Voor het renderen van **diagrammen** (optioneel, tijdens uitvoering) moet `mmdc`
(Mermaid) en/of `plantuml` (PlantUML) geïnstalleerd zijn.

### Compileren

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

Op Windows kun je de generator *Visual Studio 17 2022* gebruiken.

### Installeren op Linux

Het script `install.sh` compileert een build met minimale grootte en installeert het
binaire bestand, de `.desktop`-starter en de bureaubladpictogrammen:

```bash
sudo ./install.sh                    # in /usr/local
PREFIX="$HOME/.local" ./install.sh   # gebruikersinstallatie, zonder sudo
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```
