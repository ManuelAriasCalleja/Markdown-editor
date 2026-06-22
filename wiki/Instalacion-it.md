# Installazione

md-editor funziona su Linux, Windows e macOS.

## Download (binari)

| Piattaforma | Formato | Note |
|---|---|---|
| Linux x86_64 | **AppImage** | un unico file eseguibile; concedigli il permesso di esecuzione e aprilo |
| Windows x64 | **ZIP portatile** | scompatta ed esegui; senza installer |
| macOS (Apple Silicon + Intel) | **DMG** universale | non firmato: il primo avvio si fa con **Ctrl-clic → Apri** |

## Compilare dal codice sorgente

### Requisiti

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 con le sue **intestazioni private** (moduli *Widgets*, *PrintSupport*,
  *LinguistTools*). Le intestazioni private (`Qt6::GuiPrivate`, il QZip di Qt) servono
  per esportare in ODF/DOCX/EPUB. Su Debian/Ubuntu sono in `qt6-base-private-dev`,
  separato da `qt6-base-dev`.
- Compilatore **C++17** (GCC 9+, Clang 10+ o MSVC 19.20+)
- **Hunspell** *(opzionale)* per il correttore ortografico (`libhunspell-dev`); senza di esso,
  il resto compila ugualmente.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opzionale (correttore)
```

Per il rendering dei **diagrammi** (opzionale, in fase di esecuzione) serve avere
installato `mmdc` (Mermaid) e/o `plantuml` (PlantUML).

### Compilare

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

Su Windows puoi usare il generatore *Visual Studio 17 2022*.

### Installare su Linux

Lo script `install.sh` compila una build di dimensioni minime e installa il binario, il
launcher `.desktop` e le icone del desktop:

```bash
sudo ./install.sh                    # in /usr/local
PREFIX="$HOME/.local" ./install.sh   # installazione utente, senza sudo
```

### Test

```bash
ctest --test-dir build --output-on-failure
```
