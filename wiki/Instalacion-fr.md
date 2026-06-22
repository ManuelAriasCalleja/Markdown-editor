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
- **Qt 6** ≥ 6.5 avec ses **en-têtes privés** (modules *Widgets*, *PrintSupport*,
  *LinguistTools*). Les en-têtes privés (`Qt6::GuiPrivate`, le QZip de Qt) sont
  nécessaires pour exporter vers ODF/DOCX/EPUB. Sous Debian/Ubuntu, ils sont dans
  `qt6-base-private-dev`, en plus de `qt6-base-dev`.
- Compilateur **C++17** (GCC 9+, Clang 10+ ou MSVC 19.20+)
- **Hunspell** *(en option)* pour le correcteur orthographique (`libhunspell-dev`) ;
  sans lui, le reste compile normalement.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # en option (correcteur)
```

Pour le rendu des **diagrammes** (en option, à l'exécution), il faut avoir installé
`mmdc` (Mermaid) et/ou `plantuml` (PlantUML).

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
