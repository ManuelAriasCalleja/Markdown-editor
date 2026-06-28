# Podręcznik użytkownika

**md-editor** to wizualny (WYSIWYG) edytor Markdown: piszesz i formatujesz na już
wyrenderowanym tekście, bez oglądania kodu. Przy zapisie dokument jest z powrotem
serializowany do czystego Markdown.

## Spis treści

- [Otwieranie i zapisywanie](#otwieranie-i-zapisywanie)
- [Formatowanie tekstu](#formatowanie-tekstu)
- [Nagłówki, listy i bloki](#nagłowki-listy-i-bloki)
- [Przekształcanie tekstu i schowek](#przekształcanie-tekstu-i-schowek)
- [Odnośniki i obrazy](#odnosniki-i-obrazy)
- [Przypisy](#przypisy)
- [Wyróżnienia, symbole i skróty tekstowe](#wyroznienia-symbole-i-skroty-tekstowe)
- [Snippety (fragmenty wielokrotnego użytku)](#snippety-fragmenty-wielokrotnego-uzytku)
- [Tabele](#tabele)
- [Wzory matematyczne](#wzory-matematyczne)
- [Diagramy](#diagramy)
- [Sprawdzanie pisowni](#sprawdzanie-pisowni)
- [Znajdź i zamień](#znajdz-i-zamien)
- [Struktura dokumentu](#struktura-dokumentu)
- [Statystyki dokumentu](#statystyki-dokumentu)
- [Tryb bez rozpraszania](#tryb-bez-rozpraszania)
- [Tryb skupienia](#tryb-skupienia)
- [Widok kodu](#widok-kodu)
- [Eksport i drukowanie](#eksport-i-drukowanie)
- [Motywy i wygląd](#motywy-i-wyglad)
- [Automatyczne odzyskiwanie](#automatyczne-odzyskiwanie)
- [Dostępność](#dostepnosc)
- [Skróty](#skroty)

## Otwieranie i zapisywanie

- **Plik → Nowy** (Ctrl+N) tworzy pusty dokument w nowej karcie.
- **Plik → Nowy z szablonu** tworzy dokument na podstawie szkieletu (list,
  protokół, egzamin…) gotowego do wypełnienia.
- **Plik → Otwórz…** (Ctrl+O) otwiera istniejący `.md`. Aplikacja zapamiętuje
  ostatnio otwarte w **Plik → Otwórz ostatnie**.
- **Zapisz** (Ctrl+S) i **Zapisz jako…** (Ctrl+Shift+S) zapisują dokument w UTF-8.
  **Otwórz folder dokumentu** otwiera folder dokumentu w menedżerze plików.
- Jeśli plik zmieni się poza edytorem, aplikacja to wykryje i — jeśli nie masz
  niezapisanych zmian — przeładuje go; jeśli masz, zapyta, co zrobić.
- Plik możesz też **przeciągnąć i upuścić** na okno, aby go otworzyć.

### Karty (wiele dokumentów)

Możesz mieć otwartych kilka dokumentów naraz, każdy w osobnej **karcie**:

- **Nowy** (Ctrl+N), **Nowy z szablonu** i **Otwórz** (Ctrl+O) tworzą kartę (lub
  używają początkowej pustej karty). Upuszczenie pliku również otwiera go w karcie;
  jeśli jest już otwarty, następuje przejście do jego karty.
- Zmieniaj dokument, klikając jego kartę; przeciągaj karty, aby zmienić ich
  kolejność. Z klawiatury **Ctrl+Page Down / Ctrl+Page Up** (lub **Ctrl+Tab /
  Ctrl+Shift+Tab**) przechodzą do następnej lub poprzedniej karty.
- **Zamknij kartę** (Ctrl+W) zamyka bieżącą, pytając o niezapisane zmiany. Ostatnia
  karta nie jest zamykana: staje się nowym dokumentem.
- Etykieta pokazuje nazwę pliku i kropkę (•), gdy są niezapisane zmiany.
- Przy zamykaniu aplikacji otwarte dokumenty są zapamiętywane i wszystkie otwierane
  ponownie przy następnym uruchomieniu.

### *Front matter*

Jeśli dokument zaczyna się blokiem `---…---` (YAML) lub `+++…+++` (TOML), jest on
zachowywany bez zmian przy zapisie: nie jest widoczny w edytorze ani edytowany.
Służy do metadanych takich jak `title`, `lang` itp., używanych przy eksporcie.

## Formatowanie tekstu

Zaznacz fragment i nadaj formatowanie paskiem narzędzi lub menu **Format**:

- **Pogrubienie** (Ctrl+B), **Kursywa** (Ctrl+I), **Podkreślenie** (Ctrl+U),
  **Przekreślenie**.
- **Kod w wierszu** dla fragmentów o `stałej szerokości`.
- **Odnośnik**: dodaje `[tekst](url)` na zaznaczeniu.

Przyciski paska odzwierciedlają formatowanie aktywne pod kursorem.

**Automatyczne parowanie.** Po wpisaniu `(`, `[`, `{` lub `` ` `` para domyka się
sama, a kursor zostaje w środku; jeśli tekst jest zaznaczony, zostaje nim otoczony.
Gdy wpiszesz znak zamykający tuż przed jego parą, edytor go „przeskakuje” zamiast
duplikować.

## Nagłówki, listy i bloki

- **Nagłówki** H1–H6 z **Format → Nagłówek** lub przez Ctrl+1 … Ctrl+6.
- **Listy**: punktowane, numerowane i zadań (z polem wyboru). Enter na końcu
  punktu tworzy automatycznie następny; Enter na pustym punkcie opuszcza listę.
  **Kliknięcie pola wyboru** zadania zaznacza je lub odznacza.
- **Cytat** (`>` na początku akapitu) i **blok kodu** stosuje się z paska; oba
  poprawnie wracają do Markdown. Za pomocą **Format → Język bloku…** wybierasz
  język bloku kodu (gdy kursor jest w jego wnętrzu), aby jego składnia została
  podświetlona.
- **Wcięcie**: **Format → Zwiększ/Zmniejsz wcięcie** zagnieżdża listy i cytaty.

## Przekształcanie tekstu i schowek

- **Edycja → Przekształć tekst** działa na zaznaczeniu: **WIELKIE LITERY**, **małe
  litery**, **Kapitalizuj** i **Sortuj wiersze**.
- **Inteligentna typografia** (w tym samym menu) zamienia w zaznaczeniu myślniki
  `--`/`---` na `–`/`—`, `...` na `…` oraz proste cudzysłowy na typograficzne w
  zależności od kontekstu.
- **Wklej jako zwykły tekst** (Ctrl+Shift+V) wkleja bez formatowania. **Wklej jako
  Markdown** (Ctrl+Alt+V) konwertuje sformatowaną zawartość schowka (HTML) na
  Markdown zamiast osadzać formatowanie źródła.
- **Kopiuj jako HTML** kopiuje zaznaczenie (lub dokument) jako HTML, do wklejenia
  w e-mailu, CMS-ie itp.
- Po wklejeniu **adresu URL** na zaznaczonym tekście tekst zostaje automatycznie
  podlinkowany.
- **Edycja → Oczyść Markdown** normalizuje cały dokument za jednym razem: ujednolica
  punkty list do `-`, przycina spacje na końcu każdego wiersza, łączy nadmiarowe
  puste wiersze i poprawia odstęp po `#` w nagłówkach. Działa zachowawczo: nie
  rusza wnętrza bloków kodu.

## Odnośniki i obrazy

- **Wstaw → Odnośnik…** otwiera okno z tekstem i adresem URL. Istniejące
  zaznaczenie jest użyte jako tekst.
- **Ctrl+kliknięcie** odnośnika otwiera go w przeglądarce systemowej; po najechaniu
  myszą URL pojawia się na pasku stanu.
- **Obrazy**: przeciągnij plik, wklej obraz ze schowka lub użyj **Wstaw → Wklej
  obraz**. Obraz jest zapisywany jako PNG obok `.md` i wstawiany jako
  `![alt](ścieżka-względna)`; dzięki temu przetrwa konwersję do Markdown (obrazy
  osadzone nie).

## Przypisy

- **Wstaw → Przypis** (Ctrl+Shift+N) wstawia w miejscu kursora numerowane
  odwołanie `[^n]` i tworzy jego definicję `[^n]:` na końcu dokumentu, gotową na
  tekst przypisu.
- Odwołania są pokazywane w **indeksie górnym**; **kliknięcie** jednego z nich
  przenosi kursor do jego definicji.
- Są zapisywane jako standardowy Markdown (`tekst[^1]` w treści i niżej
  `[^1]: przypis`), więc są zgodne z innymi edytorami.

## Wyróżnienia, symbole i skróty tekstowe

- **Wstaw → Wyróżnienie** tworzy *callout* w stylu GitHuba: cytat, którego pierwszy
  wiersz to `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` lub `[!CAUTION]`.
  Jest pokazywany z zabarwionym tłem i kolorowym tytułem oraz zapisywany jako
  Markdown zgodny z GitHubem.
- **Wstaw → Symbole specjalne…** otwiera mapę znaków według kategorii
  (matematyczne, greka, strzałki, waluty, interpunkcja…); kliknięcie wstawia symbol,
  a okno pozostaje otwarte, aby wstawić kilka.
- **Skróty `:nazwa:`**: po wpisaniu kodu takiego jak `:alpha:` lub `:euro:` jest on
  rozwijany do odpowiedniego symbolu (α, €…).
- **Wstaw → Data** i **Data i godzina** wstawiają bieżącą datę (i godzinę) w
  formacie lokalnym.

## Snippety (fragmenty wielokrotnego użytku)

**Snippet** to fragment Markdown, który zapisujesz pod nazwą, aby później wstawić go
za pomocą paru kliknięć: podpis, szablon tabeli, często powtarzane ostrzeżenie…

- **Wstaw → Snippet** rozwija listę tych, które masz; po wybraniu jednego jego
  zawartość zostaje wstawiona w miejscu kursora (działa też w widoku kodu).
- **Wstaw → Snippet → Zarządzaj snippetami…** otwiera okno do tworzenia, edycji i
  usuwania Twoich snippetów. Każdy ma **nazwę** (tę widoczną w menu) oraz **treść**
  w Markdown.
- Są zapisywane w ustawieniach aplikacji, więc są dostępne we wszystkich Twoich
  dokumentach, nie tylko w bieżącym.

## Tabele

- **Tabela → Wstaw tabelę…** pyta o wiersze i kolumny.
- Akcje menu **Tabela** (dodaj/usuń wiersz lub kolumnę, wyrównaj kolumnę) są
  aktywne tylko, gdy kursor jest w tabeli.
- Wyrównanie kolumny (do lewej/środka/prawej) jest zachowywane przy zapisie jako
  `:--`/`:-:`/`--:`.

## Wzory matematyczne

md-editor obsługuje **wzory TeX** w wierszu (`$...$`) i w bloku (`$$...$$`), ze
zwykłą składnią LaTeX (Pandoc, Obsidian, Quarto…). Nie jest potrzebna żadna
zewnętrzna zależność.

- **Wstaw → Wzór…** (Ctrl+Shift+F) otwiera okno z polem na TeX i **podglądem na
  żywo**: w miarę pisania widzisz wynik. Wybierz *W wierszu* lub *Blok* i
  zatwierdź, aby go wstawić.
- Wzory są składane w **prawdziwym 2D**: ułamki (`\frac`) są piętrzone z kreską,
  duże operatory (`\sum`, `\int`, `\prod`…) pokazują granice nad i pod, pierwiastki
  (`\sqrt`) mają swoją kreskę, a są też macierze (`\begin{pmatrix}`…),
  współczynniki dwumianowe (`\binom`) i akcenty (`\hat`, `\vec`, `\bar`…).
  Prostsze (potęgi, indeksy dolne, greka) są składane w wierszu. Rysunek skaluje
  się z powiększeniem.
- **Dwukrotne kliknięcie** wzoru ponownie otwiera okno z wczytanym oryginalnym
  TeX-em: edytujesz, a po zatwierdzeniu zostaje zastąpiony.
- Wzory są **atomowe**: jeśli wpiszesz coś w środku, aplikacja przypomni o
  dwukrotnym kliknięciu; Backspace/Delete na krawędzi usuwają całą grupę.
- Przy **eksporcie** są zachowywane: do LaTeX-a są zapisywane bez zmian (z
  `amsmath` i `amssymb` w preambule); do HTML/PDF/ODF są sprowadzane do ich
  przybliżenia w wierszu.
- W **widoku kodu** pojawiają się jako `$...$` / `$$...$$`, ze wszystkimi znakami
  TeX (`\sum`, `\frac`, `_`, `*`) nienaruszonymi przy zapisie.

Przykłady:

```
Energia to $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> W kodzie źródłowym `$$...$$` może obejmować kilka wierszy (styl Obsidian/Pandoc);
> `$...$` musi otwierać się i zamykać w tym samym wierszu.

## Diagramy

Blok kodu z językiem `mermaid` lub `plantuml` jest **wyświetlany jako obraz**
podglądu tuż pod blokiem, bez naruszania kodu (który pozostaje edytowalny) ani
zapisanego Markdown.

- Wymaga zainstalowanego odpowiedniego narzędzia: **`plantuml`** (z Javą) dla
  PlantUML lub **`mmdc`** (mermaid-cli, z Node) dla Mermaid.
- Jeśli narzędzia brakuje, pod blokiem pojawia się komunikat z poleceniem
  instalacji dla Twojego systemu operacyjnego; blok pozostaje kodem.
- Obraz jest tylko prezentacją: nie jest zapisywany w Markdown i nie liczy się jako
  niezapisana zmiana.

Na przykład blok kodu oznaczony `mermaid` zawierający `flowchart LR  A --> B
--> C` jest wyświetlany jako odpowiedni schemat blokowy.

## Sprawdzanie pisowni

- Podkreśla na czerwono błędnie napisane słowa według **języka dokumentu** (z
  front matter `lang`, ustawienia języka lub systemu). Nie sprawdza kodu, wzorów
  ani odnośników.
- **Kliknięcie prawym przyciskiem** na podkreślonym słowie oferuje **podpowiedzi**
  (kliknięcie zastępuje słowo), **Dodaj do słownika** (trwała lista osobista) i
  **Ignoruj** (na czas sesji).
- Włącza się/wyłącza w **Widok → Sprawdzanie pisowni**, a język ustawia się w
  **Widok → Język sprawdzania pisowni** (lub zostawia automatyczny).
- Potrzebuje słowników Hunspell: w Linuksie systemowych (`hunspell-es`,
  `hunspell-en-us`…); w Windows/macOS są dołączone do aplikacji.

## Znajdź i zamień

- **Znajdź** (Ctrl+F) otwiera dolny pasek z polami do wyszukiwania i zamiany oraz
  opcjami (wielkość liter, całe słowo).
- **Znajdź następny** F3 / **Znajdź poprzedni** Shift+F3.

## Struktura dokumentu

Lewy panel boczny pokazuje strukturę nagłówków (spis treści): aktualizuje się
podczas pisania, a po kliknięciu pozycji kursor przeskakuje do tego nagłówka.
Pokazuje się/ukrywa klawiszem F9. Za pomocą **Ctrl+Shift+O** przenosisz fokus
klawiatury na konspekt (pokazując go, jeśli jest ukryty); tam klawisze strzałek
przesuwają się po nagłówkach, a **Enter** przeskakuje do wybranego i przywraca
fokus do edytora. Ponowne naciśnięcie **Ctrl+Shift+O** po prostu przywraca fokus
do edytora.

Pozycję struktury możesz **przeciągnąć**, aby **zmienić kolejność** tej sekcji —jej
nagłówka, treści i podsekcji— w dokumencie, bez zmiany poziomu. Ponadto **Wstaw →
Spis treści (TOC)** wstawia do dokumentu zagnieżdżoną listę nagłówków. **Widok →
Przejdź do nagłówka…** (Ctrl+G) przeskakuje do nagłówka po wpisaniu części jego
tekstu.

## Statystyki dokumentu

- **Widok → Statystyki dokumentu…** pokazuje słowa, znaki, akapity, zdania i
  szacowany czas czytania (dokumentu lub zaznaczenia).
- **Widok → Pokaż licznik słów** włącza stały licznik na pasku stanu.

## Tryb bez rozpraszania

**Widok → Bez rozpraszania** (F11) przechodzi na pełny ekran z ukrytym menu i
paskami oraz tekstem wyśrodkowanym w kolumnie do czytania. Struktura, jeśli jest
widoczna, pozostaje przy środkowym bloku. ESC lub F11 wychodzą.

## Tryb skupienia

**Widok → Tryb skupienia** (F12) pomaga skoncentrować się na tym, co piszesz, bez
opuszczania zwykłego okna. Jeden przełącznik włącza dwie rzeczy naraz:

- **Maszyna do pisania**: wiersz kursora pozostaje wyśrodkowany w pionie. W miarę
  pisania tekst przewija się tak, aby aktywny wiersz znajdował się na połowie
  wysokości, zamiast przylegać do dolnej krawędzi.
- **Przyciemnienie**: cały dokument jest wygaszony z wyjątkiem akapitu, w którym
  znajduje się kursor — ten pozostaje wyraźny.

Działa w edytorze wizualnym i w widoku kodu oraz jest **niezależny** od trybu bez
rozpraszania (F11): możesz używać obu naraz albo każdego z osobna.

## Widok kodu

**Widok → Źródło Markdown** (Ctrl+Shift+M) przełącza między edytorem wizualnym a
edytorem zwykłego tekstu, na pełnym ekranie, z surowym Markdown. Zmiany w trybie
źródła są przenoszone do dokumentu po powrocie do trybu wizualnego.

**Widok → Widok podzielony** (Ctrl+Shift+D) pokazuje oba naraz, obok siebie: edytor
wizualny i źródło, zsynchronizowane (to, co wpiszesz w jednym, odbija się w
drugim). Wyklucza się z trybem źródła na pełnym ekranie.

## Eksport i drukowanie

**Plik → Eksportuj** oferuje **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)** i **EPUB (.epub)**. W ODF, DOCX, LaTeX i EPUB osadzany jest język
dokumentu (z front matter `lang`/`language`, ustawienia aplikacji lub, w ostatniej
kolejności, języka systemu).

Możesz też wyeksportować **tylko zaznaczenie do PDF** i użyć **Podglądu wydruku**.

**Plik → Drukuj** (Ctrl+P) otwiera okno systemowe; **Drukuj zaznaczenie** drukuje
tylko to, co zaznaczone.

## Motywy i wygląd

- **Widok → Motyw** oferuje Jasny, Ciemny, GitHub Light, GitHub Dark, Monokai, Wysoki kontrast, Solarized Light i Solarized Dark. **Podążaj za systemem** dopasowuje motyw jasny/ciemny do
  systemu operacyjnego.
- **Widok → Nocne ciepłe światło** przyciemnia błękity tła zależnie od godziny.
- **Powiększenie**: Ctrl+kółko myszy, Ctrl++ / Ctrl+- i **Rozmiar normalny**
  (Ctrl+0) skalują cały interfejs (nie tylko tekst edytora).
- **Widok → Język** zmienia język interfejsu; stosuje się od razu (okno jest
  tworzone na nowo).

## Automatyczne odzyskiwanie

Podczas edycji treść jest automatycznie zapisywana co kilka sekund w kopii
roboczej. Jeśli aplikacja zamknie się nieprawidłowo, przy ponownym otwarciu
proponuje odzyskanie tego, co pisałeś.

## Dostępność

- **Czytniki ekranu**: edytor, panel konspektu, pola wyszukiwania i pozostałe elementy sterujące mają dostępne nazwy; ponadto komunikaty stanu (zapisano, „nie znaleziono”, zmiany na dysku…) są odczytywane na głos.
- **Tylko klawiatura**: każda akcja ma skrót lub pozycję w menu (F10 lub Alt otwiera pasek menu). Zobacz tabelę [Skróty](#skroty).
- **Kontrast i rozmiar**: motyw **Wysoki kontrast** oraz **powiększenie** całego interfejsu pomagają przy słabym wzroku; początkowy rozmiar czcionki pochodzi z systemu.
- **Fokus**: element z fokusem jest wyróżniany kolorem zaznaczenia motywu.

## Skróty

| Akcja                     | Skrót            |
|---------------------------|------------------|
| Nowy                      | Ctrl+N           |
| Zamknij kartę             | Ctrl+W           |
| Następna / poprzednia karta | Ctrl+Page Down / Ctrl+Page Up (lub Ctrl+Tab / Ctrl+Shift+Tab) |
| Otwórz                    | Ctrl+O           |
| Zapisz                    | Ctrl+S           |
| Zapisz jako               | Ctrl+Shift+S     |
| Drukuj                    | Ctrl+P           |
| Cofnij / Ponów            | Ctrl+Z / Ctrl+Y  |
| Pogrubienie / Kursywa     | Ctrl+B / Ctrl+I  |
| Podkreślenie              | Ctrl+U           |
| Wklej jako zwykły tekst   | Ctrl+Shift+V     |
| Wklej jako Markdown       | Ctrl+Alt+V       |
| Znajdź                    | Ctrl+F           |
| Znajdź następny/poprzedni | F3 / Shift+F3    |
| Nagłówek H1 … H6          | Ctrl+1 … Ctrl+6  |
| Wstaw wzór                | Ctrl+Shift+F     |
| Wstaw przypis             | Ctrl+Shift+N     |
| Przejdź do nagłówka       | Ctrl+G           |
| Fokus na konspekt / powrót do edytora | Ctrl+Shift+O     |
| Widok źródła Markdown     | Ctrl+Shift+M     |
| Widok podzielony          | Ctrl+Shift+D     |
| Struktura                 | F9               |
| Bez rozpraszania          | F11              |
| Tryb skupienia            | F12              |
| Powiększenie + / − / Normalne | Ctrl++ / Ctrl+− / Ctrl+0 |
| Pomoc                     | F1               |
