# Podręcznik użytkownika

**md-editor** to wizualny (WYSIWYG) edytor Markdown: piszesz i nadajesz
formatowanie na już wyrenderowanym tekście, bez oglądania kodu. Przy zapisie
dokument jest z powrotem serializowany do czystego Markdown.

## Spis treści

- [Otwieranie i zapisywanie](#otwieranie-i-zapisywanie)
- [Formatowanie tekstu](#formatowanie-tekstu)
- [Nagłówki, listy i bloki](#naglowki-listy-i-bloki)
- [Odnośniki i obrazy](#odnosniki-i-obrazy)
- [Przypisy](#przypisy)
- [Tabele](#tabele)
- [Wzory matematyczne](#wzory-matematyczne)
- [Znajdź i zamień](#znajdz-i-zamien)
- [Konspekt dokumentu](#konspekt-dokumentu)
- [Tryb bez rozproszeń](#tryb-bez-rozproszen)
- [Widok źródła](#widok-zrodla)
- [Eksport i drukowanie](#eksport-i-drukowanie)
- [Motywy i wygląd](#motywy-i-wyglad)
- [Automatyczne odzyskiwanie](#automatyczne-odzyskiwanie)
- [Skróty](#skroty)

## Otwieranie i zapisywanie

- **Plik → Nowy** (Ctrl+N) tworzy pusty dokument.
- **Plik → Otwórz…** (Ctrl+O) otwiera istniejący plik `.md`. Aplikacja
  zapamiętuje ostatnio używane pliki w **Plik → Otwórz ostatnie**.
- **Zapisz** (Ctrl+S) oraz **Zapisz jako…** (Ctrl+Shift+S) zapisują dokument
  w kodowaniu UTF-8.
- Jeśli plik zmieni się poza edytorem, aplikacja to wykryje i — jeśli nie masz
  niezapisanych zmian — przeładuje go; w przeciwnym razie zapyta, co zrobić.
- Możesz też **przeciągnąć i upuścić** plik na okno, aby go otworzyć.

### Front matter

Jeśli dokument zaczyna się od bloku `---…---` (YAML) lub `+++…+++` (TOML), jest
on zachowywany dosłownie przy zapisie: nie jest pokazywany w edytorze i nie
można go edytować. Służy do metadanych, takich jak `title`, `lang` itp., które
są wykorzystywane podczas eksportu.

## Formatowanie tekstu

Zaznacz fragment i nadaj formatowanie za pomocą paska narzędzi lub menu
**Format**:

- **Pogrubienie** (Ctrl+B), **Kursywa** (Ctrl+I), **Podkreślenie** (Ctrl+U),
  **Przekreślenie**.
- **Kod w wierszu** dla fragmentów o `stałej szerokości znaku`.
- **Odnośnik**: dodaje `[tekst](url)` na zaznaczeniu.

Przyciski na pasku odzwierciedlają formatowanie aktywne pod kursorem.

## Nagłówki, listy i bloki

- **Nagłówki** H1–H6 z menu **Format → Nagłówek** lub skrótami
  Ctrl+1 … Ctrl+6.
- **Listy**: punktowane, numerowane i listy zadań (z polem wyboru).
  Naciśnięcie Enter na końcu punktu automatycznie tworzy następny;
  naciśnięcie Enter na pustym punkcie kończy listę. **Kliknięcie pola wyboru
  zadania** zaznacza je lub odznacza.
- **Cytat** (`>` na początku akapitu) oraz **blok kodu** stosuje się z paska
  narzędzi; oba poprawnie zachowują się w obie strony (round-trip) w Markdown.

## Odnośniki i obrazy

- **Wstaw → Odnośnik…** otwiera okno z polami tekstu i adresu URL. Jeśli miałeś
  zaznaczenie, zostanie ono użyte jako tekst.
- **Ctrl+kliknięcie** na odnośniku otwiera go w przeglądarce systemowej;
  najechanie kursorem pokazuje adres URL na pasku stanu.
- **Obrazy**: przeciągnij plik, wklej obraz ze schowka lub użyj
  **Wstaw → Wklej obraz**. Obraz jest zapisywany jako PNG obok pliku `.md`
  i wstawiany jako `![alt](ścieżka-względna)`; dzięki temu przetrwa round-trip
  do Markdown (obrazy osadzone — nie).

## Przypisy

- **Wstaw → Przypis** (Ctrl+Shift+N) wstawia numerowane odwołanie `[^n]` w
  miejscu kursora i tworzy jego definicję `[^n]:` na końcu dokumentu, gotową na
  wpisanie treści przypisu.
- Odwołania są wyświetlane jako **indeks górny**; **kliknięcie** na jednym z nich
  przenosi kursor do jego definicji.
- Są zapisywane jako standardowy Markdown (`tekst[^1]` w treści, a niżej
  `[^1]: przypis`), więc są zgodne z innymi edytorami.

## Tabele

- **Tabela → Wstaw tabelę…** pyta o liczbę wierszy i kolumn.
- Polecenia menu **Tabela** (dodaj/usuń wiersz lub kolumnę, wyrównaj kolumnę)
  są aktywne tylko wtedy, gdy kursor znajduje się wewnątrz tabeli.
- Wyrównanie kolumny (do lewej/do środka/do prawej) jest zachowywane przy
  zapisie jako `:--`/`:-:`/`--:`.

## Wzory matematyczne

md-editor obsługuje **wzory TeX** w wierszu (`$...$`) i w bloku (`$$...$$`),
ze zwykłą składnią LaTeX (Pandoc, Obsidian, Quarto…). Nie wymaga żadnych
zewnętrznych zależności.

- **Wstaw → Wzór…** (Ctrl+Shift+F) otwiera okno z polem na kod TeX oraz
  **podglądem na żywo**: w miarę pisania widzisz, jak będzie wyglądać. Wybierz
  *W wierszu* lub *Blok* i zatwierdź, aby go wstawić.
- W edytorze wzory są wyświetlane kursywą w kolorze akcentu motywu, z
  **prawdziwymi indeksami górnymi i dolnymi** (nie płaskimi znakami Unicode):
  `x²`, `Hᵢ` itd. — wyrównanie pionowe Qt poprawnie skaluje dowolny znak.
- **Dwukrotne kliknięcie** wzoru ponownie otwiera okno z wczytanym oryginalnym
  kodem TeX: edytujesz i po zatwierdzeniu wzór zostaje zastąpiony.
- Wzory są **atomowe**: pisanie wewnątrz nich przypomina o dwukrotnym
  kliknięciu w celu edycji; Backspace/Delete na ich krawędzi usuwa całą grupę.
- Przy **eksporcie** wzory są zachowywane: do LaTeX trafiają dosłownie (z
  `amsmath` i `amssymb` w preambule); w HTML/PDF/ODF zachowywane jest pionowe
  wyrównanie indeksów górnych i dolnych Qt w formacie docelowym.
- W **widoku źródła** widać je jako `$...$` / `$$...$$`, ze wszystkimi znakami
  TeX (`\sum`, `\frac`, `_`, `*`) nienaruszonymi przy zapisie.

Przykłady:

```
Energia wynosi $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Ograniczenie: w źródle `$$...$$` może obejmować wiele wierszy (styl
> Obsidian/Pandoc); `$...$` musi otwierać się i zamykać w tym samym wierszu.

## Znajdź i zamień

- **Znajdź** (Ctrl+F) otwiera dolny pasek z polami wyszukiwania i zamiany
  oraz opcjami (wielkość liter, całe słowo).
- **Znajdź następne** F3 / **Znajdź poprzednie** Shift+F3.

## Konspekt dokumentu

Lewy panel boczny pokazuje spis nagłówków (TOC): aktualizuje się podczas
pisania, a kliknięcie pozycji przenosi kursor do danego nagłówka. Włącza się
i wyłącza klawiszem F9.

Możesz **przeciągnąć** pozycję konspektu, aby **zmienić kolejność** tej sekcji
— jej nagłówka, treści i podsekcji — w obrębie dokumentu, bez zmiany poziomu.
Ponadto **Wstaw → Spis treści (TOC)** umieszcza w dokumencie zagnieżdżoną listę
nagłówków.

## Tryb bez rozproszeń

**Widok → Tryb bez rozproszeń** (F11) przechodzi w tryb pełnoekranowy z
ukrytym menu i paskami narzędzi oraz tekstem wyśrodkowanym w kolumnie do
czytania. Konspekt, jeśli jest widoczny, pozostaje przyklejony do bloku
centralnego. ESC lub F11 kończy ten tryb.

## Widok źródła

**Widok → Źródło Markdown** (Ctrl+Shift+M) przełącza między edytorem
wizualnym a pełnoekranowym edytorem tekstu wyświetlającym surowy Markdown.
Zmiany wprowadzone w trybie źródła są przenoszone do dokumentu po powrocie
do trybu wizualnego.

**Widok → Widok podzielony** (Ctrl+Shift+D) pokazuje oba obok siebie: edytor
wizualny i źródło, utrzymywane w synchronizacji (to, co wpisujesz w jednym,
odzwierciedla się w drugim). Wyklucza się wzajemnie z pełnoekranowym trybem
źródła.

## Eksport i drukowanie

**Plik → Eksportuj** oferuje **PDF**, **HTML**, **ODF (.odt)** oraz
**LaTeX (.tex)**. W przypadku ODF i LaTeX osadzany jest język dokumentu
(pobrany z pola `lang`/`language` we front matter, z ustawienia aplikacji
lub, w ostateczności, z ustawień regionalnych systemu).

**Plik → Drukuj** (Ctrl+P) otwiera okno dialogowe systemu.

## Motywy i wygląd

- **Widok → Motyw** oferuje Jasny, Ciemny, GitHub Light, GitHub Dark, Monokai
  oraz Wysoki kontrast.
- **Widok → Ciepłe światło nocne** przyciemnia barwy niebieskie tła w
  zależności od pory dnia.
- **Powiększenie**: Ctrl+kółko myszy, Ctrl++ / Ctrl+- oraz **Rozmiar normalny**
  (Ctrl+0) skalują cały interfejs (nie tylko tekst w edytorze).
- **Widok → Język** zmienia język interfejsu; działa natychmiast (okno jest tworzone na nowo).

## Automatyczne odzyskiwanie

Podczas edycji zawartość jest co kilka sekund automatycznie zapisywana w
kopii roboczej. Jeśli aplikacja zamknie się nieoczekiwanie, przy następnym
uruchomieniu zaproponuje odzyskanie tego, co pisałeś.

## Skróty

| Czynność                  | Skrót            |
|---------------------------|------------------|
| Nowy                      | Ctrl+N           |
| Otwórz                    | Ctrl+O           |
| Zapisz                    | Ctrl+S           |
| Zapisz jako               | Ctrl+Shift+S     |
| Drukuj                    | Ctrl+P           |
| Cofnij / Ponów            | Ctrl+Z / Ctrl+Y  |
| Pogrubienie / Kursywa     | Ctrl+B / Ctrl+I  |
| Podkreślenie              | Ctrl+U           |
| Znajdź                    | Ctrl+F           |
| Znajdź następne / poprzednie | F3 / Shift+F3 |
| Nagłówek H1 … H6          | Ctrl+1 … Ctrl+6  |
| Wstaw wzór                | Ctrl+Shift+F     |
| Wstaw przypis             | Ctrl+Shift+N     |
| Widok źródła Markdown     | Ctrl+Shift+M     |
| Widok podzielony          | Ctrl+Shift+D     |
| Konspekt                  | F9               |
| Tryb bez rozproszeń       | F11              |
| Powiększenie + / − / Normalny | Ctrl++ / Ctrl+− / Ctrl+0 |
| Pomoc                     | F1               |
