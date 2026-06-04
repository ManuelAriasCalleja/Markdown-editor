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
- **Qt 6** ≥ 6.5 (modules *Widgets*, *PrintSupport*, *LinguistTools*)
- **C++17**-compiler (GCC 9+, Clang 10+ of MSVC 19.20+)

Er zijn geen externe afhankelijkheden buiten Qt.

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
