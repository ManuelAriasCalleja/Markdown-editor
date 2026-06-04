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
- **Qt 6** ≥ 6.5 (module *Widgets*, *PrintSupport*, *LinguistTools*)
- Compilator **C++17** (GCC 9+, Clang 10+ sau MSVC 19.20+)

Nu există dependențe de la terți în afară de Qt.

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
