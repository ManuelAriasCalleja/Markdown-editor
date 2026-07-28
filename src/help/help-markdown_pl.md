# Markdown na jednej stronie

**Markdown** to sposób zapisywania sformatowanego tekstu za pomocą prostych
symboli. To, co jest po lewej, to to, co wpisujesz; po prawej — jak to
wygląda. W md-editor nie musisz wpisywać tych symboli: stosujesz je z paska
narzędzi, a przy zapisie edytor zapisuje je za Ciebie.

## Spis treści

- [Akapity i podziały wiersza](#akapity-i-podziały-wiersza)
- [Nagłówki](#nagłowki)
- [Wyróżnienia](#wyroznienia)
- [Listy](#listy)
- [Cytaty](#cytaty)
- [Kod](#kod)
- [Odnośniki i obrazy](#odnosniki-i-obrazy)
- [Przypisy](#przypisy)
- [Linie poziome](#linie-poziome)
- [Tabele](#tabele)
- [Wzory matematyczne](#wzory-matematyczne)
- [Rozszerzenia obsługiwane przez md-editor](#rozszerzenia-obsługiwane-przez-md-editor)
- [Front matter](#front-matter)
- [Znaki ucieczki](#znaki-ucieczki)

## Akapity i podziały wiersza

Rozdzielaj akapity **pustym wierszem**. W obrębie akapitu dwie spacje na końcu
wiersza wymuszają podział wiersza bez rozpoczynania nowego akapitu.

## Nagłówki

```
# Nagłówek poziomu 1
## Nagłówek poziomu 2
### Nagłówek poziomu 3
```

Do sześciu poziomów (`######`). W md-editor możesz je też zastosować z menu
**Format → Nagłówek** lub skrótami Ctrl+1 … Ctrl+6.

## Wyróżnienia

- `*kursywa*` lub `_kursywa_` → *kursywa*
- `**pogrubienie**` lub `__pogrubienie__` → **pogrubienie**
- `***pogrubienie i kursywa***` → ***pogrubienie i kursywa***
- `~~przekreślenie~~` → ~~przekreślenie~~

## Listy

**Punkty** (z `-`, `*` lub `+`):

```
- Jabłko
- Gruszka
  - Konferencja
  - Ercolini
```

**Numerowane**:

```
1. Pierwszy
2. Drugi
3. Trzeci
```

**Zadania** (pola wyboru):

```
- [x] Zrobione
- [ ] Do zrobienia
```

## Cytaty

Jeden lub więcej wierszy poprzedzonych znakiem `>`:

```
> Kto dużo czyta i dużo chodzi, ten dużo widzi i dużo wie.
> — Miguel de Cervantes
```

## Kod

**W wierszu**: otocz znakiem grawisu: `` `kod` ``.

**Blok**: trzy grawisy na początku i na końcu; opcjonalnie nazwa języka, aby
go pokolorować:

````
```python
def przywitaj(imie):
    print(f"Cześć, {imie}")
```
````

## Odnośniki i obrazy

- **Odnośnik**: `[tekst](https://przyklad.pl)`
- **Odnośnik z tytułem**: `[tekst](https://przyklad.pl "Tytuł dymka")`
- **Obraz**: `![tekst alternatywny](ścieżka/obraz.png)` — tak jak odnośnik,
  ale z `!` z przodu.

W md-editor **Ctrl+kliknięcie** na odnośniku otwiera go w przeglądarce
systemowej.

## Przypisy

**Odwołanie** w tekście i jego osobna **definicja**, połączone
identyfikatorem `[^id]`:

```
Stwierdzenie z jego niuansem[^1].

[^1]: Tekst przypisu znajduje się tutaj.
```

`id` może być liczbą (`[^1]`) lub słowem (`[^nota]`). W md-editor
**Wstaw → Przypis** (Ctrl+Shift+N) tworzy odwołanie i jego definicję za Ciebie;
odwołania są wyświetlane jako indeks górny, a kliknięcie przenosi do definicji.

## Linie poziome

Trzy lub więcej myślników, gwiazdek lub podkreśleń w osobnym wierszu:

```
---
```

## Tabele

```
| Produkt | Ilość | Cena    |
|---------|------:|:-------:|
| Chleb   |     2 |  4,20 zł|
| Mleko   |     1 |  3,95 zł|
```

Dwukropki w wierszu rozdzielającym ustalają wyrównanie kolumny: `:--` do
lewej, `:-:` do środka, `--:` do prawej. md-editor zachowuje wyrównanie przy
zapisie.

## Wzory matematyczne

Standardowy Markdown **nie** definiuje wzorów, ale szeroko rozpowszechniona
konwencja (Pandoc, Obsidian, Quarto, GitHub) obsługuje składnię TeX między
`$...$` (w wierszu) i `$$...$$` (w bloku). md-editor implementuje tę
konwencję.

```
Wzór $E = mc^2$ jest słynny.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

Znaki specjalne TeX (`\`, `_`, `*`, `{`, `}`) są zachowywane nienaruszone
wewnątrz wzorów — edytor je chroni, aby parser Markdown nie pomylił ich z
kursywą czy pogrubieniem.

W md-editor wzory są wyświetlane z prawdziwymi indeksami górnymi i dolnymi
(nie jako dosłowne `$x^2$`). Wstaw wzór z menu **Wstaw → Wzór…**
(Ctrl+Shift+F) lub kliknij dwukrotnie istniejący, aby go edytować.

## Rozszerzenia obsługiwane przez md-editor

Poza powyższym — czyli standardowym Markdown — md-editor rozumie cztery bardzo
rozpowszechnione konwencje. Nie należą do pierwotnego Markdown, więc inny edytor
może pokazać je jako zwykły tekst; plik i tak zapisuje się bez zmian, więc nic
nie ginie.

**Wyróżnienie** (styl GitHub/Obsidian): po dwa znaki równości z każdej strony.

```
To jest ==wyróżnione== jak zakreślaczem.
```

**Indeks górny i dolny** (styl Pandoc): daszek i tylda.

```
Pole wynosi 12 m^2^, a wzór wody to H~2~O.
```

**Wyróżnione bloki** (*callouts*, styl GitHub): cytat, którego pierwszy wiersz
jest etykietą w nawiasach kwadratowych. Dozwolone są `[!NOTE]`, `[!TIP]`,
`[!IMPORTANT]`, `[!WARNING]` i `[!CAUTION]`.

```
> [!WARNING]
> Ten krok usuwa wcześniejsze dane.
```

**Diagramy**: blok kodu z językiem `mermaid` lub `plantuml`. Edytor pokazuje pod
blokiem podgląd w postaci obrazu, jeśli masz zainstalowane odpowiednie narzędzie.

````
```mermaid
flowchart LR
  A --> B --> C
```
````

## Front matter

Wiele generatorów stron (Jekyll, Hugo, Quarto…) zaczyna plik blokiem metadanych
między `---` (YAML) albo `+++` (TOML):

```
---
title: Raport roczny
lang: pl
---
```

md-editor zachowuje go przy zapisie **bez zmian**: nie jest edytowany ani
pokazywany w edytorze. Stamtąd bierze `title` i `lang` przy eksporcie oraz do
wyboru słownika korektora pisowni.

## Znaki ucieczki

Aby symbol Markdown pojawił się dosłownie (bez działania jako formatowanie),
postaw przed nim ukośnik wsteczny: `\*to nie kursywa\*` → \*to nie kursywa\*.

Symbole, które można poprzedzić znakiem ucieczki, to:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
