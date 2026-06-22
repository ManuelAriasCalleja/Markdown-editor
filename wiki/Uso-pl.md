# Użytkowanie

## Otwieranie i zapisywanie

- Nowy (Ctrl+N), Otwórz (Ctrl+O), Zapisz (Ctrl+S), Zapisz jako (Ctrl+Shift+S).
  Wszystko w UTF-8.
- **Karty**: każdy otwarty dokument zajmuje własną kartę; zamknij ją skrótem
  Ctrl+W. Przy ponownym uruchomieniu karty z ostatniej sesji są otwierane na nowo.
- **Nowy z szablonu** (Plik → Nowy z szablonu) startuje od gotowego szkieletu
  Markdownu.
- **Otwórz ostatnie** wyświetla listę Twoich ostatnich dokumentów.
- Możesz też przeciągnąć i upuścić plik na okno, aby go otworzyć.
- Jeśli plik zmieni się poza md-editor, program Cię o tym powiadomi: przeładuje go
  automatycznie, jeśli nie miałeś żadnych zmian, lub zapyta, jeśli je miałeś.

### Front matter

Jeśli Twój dokument zaczyna się od bloku `---…---` (YAML) lub `+++…+++` (TOML),
zostaje on zachowany bez zmian przy zapisie (nie jest widoczny ani edytowalny). Służy
do metadanych takich jak `title` i `lang`, które są wykorzystywane przy eksporcie.

## Formatowanie

Użyj menu Format lub paska narzędzi. Nie musisz wpisywać symboli Markdownu: edytor
zastosuje je za Ciebie.

- Pogrubienie (Ctrl+B), Kursywa (Ctrl+I), Podkreślenie (Ctrl+U), Przekreślenie,
  Kod w wierszu, Odnośnik (Ctrl+K).
- Nagłówki H1–H6 (Ctrl+1 … Ctrl+6).
- Listy wypunktowane, numerowane i zadań, z automatyczną kontynuacją po naciśnięciu
  Enter (puste wypunktowanie kończy listę). Pola wyboru zadań zaznacza się
  kliknięciem.
- Cytaty i bloki kodu.

Wszystkie skróty znajdziesz w [Skróty klawiszowe](Atajos-pl).

## Edycja i przekształcanie tekstu

- **Wklej jako zwykły tekst** (Ctrl+Shift+V) lub **Wklej jako Markdown** (Ctrl+Alt+V),
  co konwertuje HTML ze schowka na Markdown. Wklejenie adresu URL na zaznaczenie
  automatycznie tworzy z niego odnośnik.
- **Edycja → Przekształć tekst**: WIELKIE litery, małe litery, kapitalizacja,
  sortowanie linii oraz inteligentna typografia (konwertuje `--`, `---`, `...` i
  proste cudzysłowy).

## Wstawianie

- Odnośnik i Obraz (ze ścieżką względną do dokumentu, aby był przenośny).
- **Wklej obraz**: obraz ze schowka jest zapisywany jako PNG obok Twojego pliku `.md`
  i wstawiany jako `![](ruta)`. Działa również przez przeciągnięcie lub wklejenie na
  edytor.
- Tabela, Linia pozioma, Spis treści (TOC) i Wzór (Ctrl+Shift+F).
- **Przypis dolny** (Ctrl+Shift+N): wstawia odwołanie `[^n]` i jego definicję.
- **Wyróżnienie**: wyróżniony blok (nota, wskazówka, ważne, ostrzeżenie, przestroga).
- **Symbole specjalne** oraz **Data / Data i godzina**.

## Tabele

Gdy kursor znajduje się wewnątrz tabeli, menu Tabela pozwala dodawać lub usuwać
wiersze i kolumny oraz wyrównywać każdą kolumnę (do lewej/do środka/do prawej).
Wyrównanie zostaje zachowane przy zapisie.

## Wzory

Wstawiaj wzory TeX w wierszu (`$...$`) lub w bloku (`$$...$$`) przez Wstaw → Wzór
(Ctrl+Shift+F), z podglądem na żywo. Dwukrotne kliknięcie wzoru otwiera go do edycji.
Są rysowane w prawdziwym układzie 2D (ułamki, pierwiastki, macierze, sumy z
granicami…). Więcej szczegółów w [Funkcje](Caracteristicas-pl#wzory-tex).

## Diagramy

Napisz blok kodu w języku `mermaid` lub `plantuml` i — jeśli masz zainstalowane
odpowiednie narzędzie (`mmdc` / `plantuml`) — zostanie on wyrenderowany jako obraz
pod blokiem. Jeśli go brak, zobaczysz polecenie jego instalacji.

## Sprawdzanie pisowni

Włącz je w Widok → Sprawdzanie pisowni (wymaga Hunspella). Język wybierany jest na
podstawie języka dokumentu lub ręcznie w Widok → Język sprawdzania pisowni.
Kliknięcie prawym przyciskiem na podkreślonym słowie oferuje sugestie oraz dodanie go
do osobistego słownika.

## Tryby widoku

- **WYSIWYG** (domyślnie): tylko wyrenderowany wynik.
- **Źródło Markdown** (Ctrl+Shift+M): surowy Markdown, na pełnym ekranie.
- **Widok podzielony** (Ctrl+Shift+D): wynik renderowania i kod obok siebie,
  zsynchronizowane.
- **Konspekt** (F9) oraz **Idź do nagłówka** (Ctrl+G) do nawigacji po dokumencie.

## Wyszukiwanie i zamiana

Ctrl+F, aby wyszukać, Ctrl+H, aby zamienić. Obejmuje poprzedni/następny, zamień
wszystko i rozróżnianie wielkości liter.

## Eksport i drukowanie

Plik → Eksportuj oferuje PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) i EPUB
(.epub); dostępne są też Podgląd wydruku i Drukowanie (Ctrl+P). W ODF, DOCX i LaTeX
osadzany jest język dokumentu.

## Automatyczne odzyskiwanie

md-editor zapisuje wersję roboczą co kilka sekund. Jeśli aplikacja zamknie się
nieoczekiwanie, przy ponownym otwarciu zaproponuje odzyskanie tego, co pisałeś.
