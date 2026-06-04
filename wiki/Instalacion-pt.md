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
- **Qt 6** ≥ 6.5 (módulos *Widgets*, *PrintSupport*, *LinguistTools*)
- Compilador **C++17** (GCC 9+, Clang 10+ ou MSVC 19.20+)

Não há dependências de terceiros para além do Qt.

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
