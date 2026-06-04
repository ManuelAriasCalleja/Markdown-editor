# Installation

md-editor fonctionne sous Linux, Windows et macOS.

## Téléchargements (binaires)

| Plateforme | Format | Remarques |
|---|---|---|
| Linux x86_64 | **AppImage** | un seul fichier exécutable ; donnez-lui les droits d'exécution et ouvrez-le |
| Windows x64 | **ZIP portable** | décompressez et exécutez ; sans installateur |
| macOS (Apple Silicon + Intel) | **DMG** universel | non signé : le premier lancement se fait avec **Ctrl-clic → Ouvrir** |

## Compiler depuis le code source

### Prérequis

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 (modules *Widgets*, *PrintSupport*, *LinguistTools*)
- Compilateur **C++17** (GCC 9+, Clang 10+ ou MSVC 19.20+)

Il n'y a aucune dépendance tierce au-delà de Qt.

### Compiler

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

Sous Windows, vous pouvez utiliser le générateur *Visual Studio 17 2022*.

### Installer sous Linux

Le script `install.sh` compile une build de taille minimale et installe le binaire,
le lanceur `.desktop` et les icônes du bureau :

```bash
sudo ./install.sh                    # dans /usr/local
PREFIX="$HOME/.local" ./install.sh   # installation utilisateur, sans sudo
```

### Tests

```bash
ctest --test-dir build --output-on-failure
```
