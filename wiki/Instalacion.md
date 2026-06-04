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
- **Qt 6** ≥ 6.5 (módulos *Widgets*, *PrintSupport*, *LinguistTools*)
- Compilador **C++17** (GCC 9+, Clang 10+ o MSVC 19.20+)

No hay dependencias de terceros más allá de Qt.

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
