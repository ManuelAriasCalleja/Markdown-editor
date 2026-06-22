# Instalacja

md-editor działa w systemach Linux, Windows i macOS.

## Pobieranie (pliki binarne)

| Platforma | Format | Uwagi |
|---|---|---|
| Linux x86_64 | **AppImage** | pojedynczy plik wykonywalny; nadaj mu prawo do uruchamiania i otwórz |
| Windows x64 | **przenośny ZIP** | rozpakuj i uruchom; bez instalatora |
| macOS (Apple Silicon + Intel) | uniwersalny **DMG** | bez podpisu: pierwsze uruchomienie przez **Ctrl-klik → Otwórz** |

## Kompilacja ze źródeł

### Wymagania

- **CMake** ≥ 3.16
- **Qt 6** ≥ 6.5 z jego **nagłówkami prywatnymi** (moduły *Widgets*, *PrintSupport*,
  *LinguistTools*). Nagłówki prywatne (`Qt6::GuiPrivate`, QZip z Qt) są potrzebne do
  eksportu do ODF/DOCX/EPUB. W Debianie/Ubuntu znajdują się w pakiecie
  `qt6-base-private-dev`, osobno od `qt6-base-dev`.
- Kompilator **C++17** (GCC 9+, Clang 10+ lub MSVC 19.20+)
- **Hunspell** *(opcjonalnie)* do sprawdzania pisowni (`libhunspell-dev`); bez niego
  reszta kompiluje się tak samo.

```bash
# Debian/Ubuntu
sudo apt-get install qt6-base-dev qt6-base-private-dev cmake g++
sudo apt-get install libhunspell-dev          # opcjonalnie (sprawdzanie pisowni)
```

Do renderowania **diagramów** (opcjonalnie, w czasie działania) potrzebne jest
zainstalowanie `mmdc` (Mermaid) i/lub `plantuml` (PlantUML).

### Kompilacja

```bash
cmake -S . -B build && cmake --build build
./build/md-editor [archivo.md]
```

W systemie Windows możesz użyć generatora *Visual Studio 17 2022*.

### Instalacja w systemie Linux

Skrypt `install.sh` kompiluje build o minimalnym rozmiarze i instaluje plik binarny,
launcher `.desktop` oraz ikony pulpitu:

```bash
sudo ./install.sh                    # w /usr/local
PREFIX="$HOME/.local" ./install.sh   # instalacja użytkownika, bez sudo
```

### Testy

```bash
ctest --test-dir build --output-on-failure
```
