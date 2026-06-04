# md-editor

Edytor/przeglądarka Markdown typu WYSIWYG napisany w Qt6 + C++17. Domyślnie edytujesz
już wyrenderowany tekst, bez zajmowania się składnią; opcjonalnie możesz jednak
podejrzeć kod Markdown, a nawet mieć kod i jego wynik renderowania obok siebie (widok
podzielony) i edytować po dowolnej ze stron. Przy zapisie dokument jest zawsze
serializowany do czystego Markdownu.

## Co robi za Ciebie

- **Prawdziwy WYSIWYG**: widzisz wynik, a nie symbole.
- **Wierny round-trip**: to, co otwierasz, jest tym, co zapisujesz — z wyrównanymi
  tabelami, listami zadań, cytatami, blokami kodu i wzorami.
- **Trzy sposoby pracy**: tylko wynik renderowania (domyślnie), tylko kod, albo oba
  obok siebie (zsynchronizowany widok podzielony).
- **Tryb bez rozproszeń**: wyśrodkowana kolumna do czytania, bez pasków (F11), z
  opcjonalnym spisem treści (możesz go pokazać lub ukryć).
- **Dbałość o wzrok**: *Ciepłe światło nocne* stopniowo wytłumia niebieski odcień tła
  w zależności od pory dnia, aby zmniejszyć zmęczenie oczu w nocy.
- **Wzory TeX**: [w wierszu](Caracteristicas-pl#wzory-tex) i [w bloku](Caracteristicas-pl#wzory-tex),
  z prawdziwymi indeksami górnymi/dolnymi i podglądem na żywo, bez zewnętrznych
  zależności.
- **Eksport** do PDF, HTML, ODF (.odt) i LaTeX (.tex), z zachowaniem języka dokumentu
  i formatowania wzorów.
- **Wyświetlanie**: 1) 6 motywów jasnych i ciemnych, 2) powiększenie całego interfejsu,
  3) interfejs przetłumaczony na 9 języków.

## Na początek

- [Instalacja](Instalacion-pl)
- [Użytkowanie](Uso-pl)
- [Funkcje](Caracteristicas-pl)
- [Skróty klawiszowe](Atajos-pl)

---

*md-editor jest tworzony przez Manuel Arias Calleja. Licencja CC BY-ND 4.0.*
