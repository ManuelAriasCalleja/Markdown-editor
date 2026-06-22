# Instalación

md-editor funciona en Linux, Windows y macOS.

## Descargas (binarios)

| Plataforma | Formato | Notas |
|---|---|---|
| Linux x86_64 | **AppImage** | un solo archivo ejecutable; dale permiso de ejecución y ábrelo |
| Windows x64 | **ZIP portable** | descomprime y ejecuta; sin instalador |
| macOS (Apple Silicon + Intel) | **DMG** universal | sin firmar: el primer arranque es con **Ctrl-clic → Abrir** |

## Compilar desde el código

### Requisitos

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 con sus **cabeceras privadas** (módulos *Widgets*, *PrintSupport*,
  *LinguistTools*). Las cabeceras privadas (`Qt6::GuiPrivate`, el QZip de Qt) hacen
  falta para exportar a ODF/DOCX/EPUB. En Debian/Ubuntu van en `qt6-base-private-dev`,
  aparte de `qt6-base-dev`.
- Compilador **C++17** (GCC 9+, Clang 10+ o MSVC 19.20+)
- **Hunspell** *(opcional)* para el corrector ortográfico (`libhunspell-dev`); sin él,
  el resto compila igual.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opcional (corrector)
```

Para el render de **diagramas** (opcional, en tiempo de ejecución) hace falta tener
instalado `mmdc` (Mermaid) y/o `plantuml` (PlantUML).

### Compilar

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

En Windows puedes usar el generador *Visual Studio 17 2022*.

### Instalar en Linux

El script `install.sh` compila una build de tamaño mínimo e instala el binario, el
lanzador `.desktop` y los iconos del escritorio:

```bash
sudo ./install.sh                    # en /usr/local
PREFIX="$HOME/.local" ./install.sh   # instalación de usuario, sin sudo
```

### Pruebas

```bash
ctest --test-dir build --output-on-failure
```
