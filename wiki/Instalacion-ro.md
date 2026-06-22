# Instalare

md-editor funcționează pe Linux, Windows și macOS.

## Descărcări (binare)

| Platformă | Format | Note |
|---|---|---|
| Linux x86_64 | **AppImage** | un singur fișier executabil; dă-i permisiune de execuție și deschide-l |
| Windows x64 | **ZIP portabil** | dezarhivează și execută; fără program de instalare |
| macOS (Apple Silicon + Intel) | **DMG** universal | nesemnat: prima deschidere se face cu **Ctrl-clic → Deschide** |

## Compilare din cod

### Cerințe

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 cu **anteturile sale private** (module *Widgets*, *PrintSupport*,
  *LinguistTools*). Anteturile private (`Qt6::GuiPrivate`, QZip-ul din Qt) sunt
  necesare pentru export în ODF/DOCX/EPUB. În Debian/Ubuntu se află în
  `qt6-base-private-dev`, separat de `qt6-base-dev`.
- Compilator **C++17** (GCC 9+, Clang 10+ sau MSVC 19.20+)
- **Hunspell** *(opțional)* pentru corectorul ortografic (`libhunspell-dev`); fără el,
  restul se compilează la fel.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opțional (corector)
```

Pentru randarea **diagramelor** (opțional, în timpul execuției) trebuie să ai instalat
`mmdc` (Mermaid) și/sau `plantuml` (PlantUML).

### Compilare

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

Pe Windows poți folosi generatorul *Visual Studio 17 2022*.

### Instalare pe Linux

Scriptul `install.sh` compilează o versiune de dimensiune minimă și instalează binarul,
lansatorul `.desktop` și pictogramele de desktop:

```bash
sudo ./install.sh                    # în /usr/local
PREFIX="$HOME/.local" ./install.sh   # instalare la nivel de utilizator, fără sudo
```

### Teste

```bash
ctest --test-dir build --output-on-failure
```
