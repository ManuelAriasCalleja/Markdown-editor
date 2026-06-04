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
- **Qt 6** ≥ 6.5 (moduli *Widgets*, *PrintSupport*, *LinguistTools*)
- Compilatore **C++17** (GCC 9+, Clang 10+ o MSVC 19.20+)

Non ci sono dipendenze di terze parti oltre a Qt.

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
