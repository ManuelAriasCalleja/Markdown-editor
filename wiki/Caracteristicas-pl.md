# Funkcje

Podsumowanie wszystkiego, co oferuje md-editor. Pełną i techniczną dokumentację
znajdziesz w pliku `especificacion.md` w repozytorium.

## Edycja WYSIWYG i round-trip

Edytujesz wyrenderowany tekst, a przy zapisie jest on serializowany do czystego
Markdownu w UTF-8. To, co otwierasz, jest tym, co zapisujesz: tabele z wyrównaniem,
listy zagnieżdżone, listy zadań, cytaty, bloki kodu i wzory są wiernie zachowywane.

## Tryby widoku

- WYSIWYG, Źródło Markdown (Ctrl+Shift+M) i Widok podzielony (Ctrl+Shift+D).
- W widoku podzielonym wynik renderowania i kod synchronizują się: aktualizowany jest
  tylko panel, którego nie edytujesz, bez przeskoków kursora.

## Tryb bez rozproszeń

F11 przechodzi do trybu pełnoekranowego z tekstem wyśrodkowanym w kolumnie do
czytania i bez pasków. ESC lub F11 kończą tryb.

## Motywy i ciepłe światło nocne

- **Sześć motywów**: Jasny, Ciemny, GitHub Light, GitHub Dark, Monokai i Wysoki
  kontrast.
- **Ciepłe światło nocne** (domyślnie włączone): wytłumia niebieski odcień tła
  automatycznie i stopniowo w zależności od pory, aby zmniejszyć zmęczenie wzroku w
  nocy. Neutralne w dzień (07–19 h), ociepla się po południu (19–23 h), maksymalne w
  nocy (23–06 h) i chłodnieje o świcie (06–07 h). Przelicza się samo co minutę i
  wpływa wyłącznie na tło (nie na odnośniki ani na podświetlanie składni).

## Konspekt dokumentu

Boczny panel (F9) ze spisem nagłówków; kliknięcie przenosi do sekcji.

## Wzory TeX

Wzory w wierszu (`$...$`) i w bloku (`$$...$$`) ze składnią LaTeX, bez zewnętrznych
zależności:

- Wstawianie z podglądem na żywo (Ctrl+Shift+F) i edycja dwukrotnym kliknięciem.
- Prawdziwe indeksy górne i dolne, litery greckie, operatory, `\frac`, `\sqrt`,
  `\mathbb`…
- Są atomowe w edytorze i przetrwają round-trip oraz eksport.
- Ograniczenia: `$...$` musi otwierać się i zamykać w tej samej linii; nie ma
  *układu* 2D (duże ułamki w postaci `(a)/(b)`).

## Podświetlanie składni

Bloki kodu są kolorowane zgodnie z ich językiem (rodziny C/C++/Java…, JS/TS/JSON,
Python, shell/YAML/TOML… oraz tryb ogólny).

## Obrazy

Wklejenie lub upuszczenie obrazu zapisuje go jako PNG obok dokumentu i wstawia jako
`![](ruta)` — nie osadza go — dzięki czemu Markdown pozostaje przenośny.

## Eksport i drukowanie

PDF, HTML, ODF (.odt) i LaTeX (.tex), a także drukowanie (Ctrl+P). ODF i LaTeX
osadzają język dokumentu (z front matter, z ustawienia aplikacji lub systemu).

## Powiększenie całego interfejsu

Ctrl++, Ctrl+- i Ctrl+0 (lub Ctrl + kółko) skalują cały interfejs, a nie tylko tekst
edytora. Poziom jest zapamiętywany.

## Wyszukiwanie i zamiana

Ctrl+F / Ctrl+H, z funkcjami poprzedni/następny, zamień wszystko i rozróżnianiem
wielkości liter.

## Pliki i bezpieczeństwo Twoich danych

- **Ostatnie pliki**, otwieranie przez przeciągnięcie i potwierdzanie niezapisanych
  zmian.
- **Front matter** YAML/TOML zachowywany dosłownie.
- **Nadzór nad plikiem na dysku**: wykrywa zmiany zewnętrzne i proponuje przeładowanie.
- **Automatyczny zapis i odzyskiwanie** po nieoczekiwanym zamknięciu.

## Internacjonalizacja

Interfejs w 9 językach: hiszpański, angielski, niemiecki, francuski, włoski,
portugalski, polski, niderlandzki i rumuński (Widok → Język; stosuje się po ponownym
uruchomieniu).
