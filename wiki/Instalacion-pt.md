# Instalação

O md-editor funciona em Linux, Windows e macOS.

## Transferências (binários)

| Plataforma | Formato | Notas |
|---|---|---|
| Linux x86_64 | **AppImage** | um único ficheiro executável; dê-lhe permissão de execução e abra-o |
| Windows x64 | **ZIP portátil** | descomprima e execute; sem instalador |
| macOS (Apple Silicon + Intel) | **DMG** universal | sem assinatura: o primeiro arranque é com **Ctrl-clique → Abrir** |

## Compilar a partir do código

### Requisitos

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 com os seus **cabeçalhos privados** (módulos *Widgets*, *PrintSupport*,
  *LinguistTools*). Os cabeçalhos privados (`Qt6::GuiPrivate`, o QZip do Qt) são
  necessários para exportar para ODF/DOCX/EPUB. No Debian/Ubuntu vêm em `qt6-base-private-dev`,
  à parte de `qt6-base-dev`.
- Compilador **C++17** (GCC 9+, Clang 10+ ou MSVC 19.20+)
- **Hunspell** *(opcional)* para o corretor ortográfico (`libhunspell-dev`); sem ele,
  o resto compila na mesma.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opcional (corretor)
```

Para a renderização de **diagramas** (opcional, em tempo de execução) é necessário ter
instalado `mmdc` (Mermaid) e/ou `plantuml` (PlantUML).

### Compilar

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

No Windows pode usar o gerador *Visual Studio 17 2022*.

### Instalar no Linux

O script `install.sh` compila uma build de tamanho mínimo e instala o binário, o
lançador `.desktop` e os ícones do ambiente de trabalho:

```bash
sudo ./install.sh                    # em /usr/local
PREFIX="$HOME/.local" ./install.sh   # instalação de utilizador, sem sudo
```

### Testes

```bash
ctest --test-dir build --output-on-failure
```
