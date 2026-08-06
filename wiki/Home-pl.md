# md-editor

Edytor/przeglądarka Markdown typu WYSIWYG napisany w Qt6 + C++17. Domyślnie edytujesz
już wyrenderowany tekst, bez zajmowania się składnią; opcjonalnie możesz jednak
podejrzeć kod Markdown, a nawet mieć kod i jego wynik renderowania obok siebie (widok
podzielony) i edytować po dowolnej ze stron. Przy zapisie dokument jest zawsze
serializowany do czystego Markdownu.

## Co robi za Ciebie

- **Prawdziwy WYSIWYG**: widzisz wynik, a nie symbole.
- **Wierny round-trip**: to, co otwierasz, jest tym, co zapisujesz — z wyrównanymi
  tabelami, listami zadań, cytatami, blokami kodu, przypisami dolnymi, wyróżnieniami
  (admonitions) i wzorami.
- **Edycja w kartach**: kilka dokumentów otwartych jednocześnie, każdy we własnej
  karcie.
- **Trzy sposoby pracy**: tylko wynik renderowania (domyślnie), tylko kod, albo oba
  obok siebie (zsynchronizowany widok podzielony).
- **Tryb bez rozproszeń**: wyśrodkowana kolumna do czytania, bez pasków (F11), z
  opcjonalnym spisem treści.
- **Dbałość o wzrok**: *Ciepłe światło nocne* stopniowo wytłumia niebieski odcień tła
  w zależności od pory dnia, aby zmniejszyć zmęczenie oczu w nocy.
- **Wzory TeX** z prawdziwym układem 2D (ułamki piętrowe, pierwiastki, macierze,
  sumy z granicami…) i podglądem na żywo, bez zewnętrznych zależności.
- **Sprawdzanie pisowni** opcjonalne (Hunspell) oraz **diagramy** Mermaid/PlantUML.
- **Eksport** do PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) i EPUB (.epub),
  z zachowaniem języka dokumentu i formatowania wzorów.
- **Wyświetlanie**: 1) 8 motywów jasnych i ciemnych, 2) powiększenie całego interfejsu,
  3) interfejs przetłumaczony na 10 języków.

## Na początek

- [Instalacja](Instalacion-pl)
- [Użytkowanie](Uso-pl)
- [Funkcje](Caracteristicas-pl)
- [Skróty klawiszowe](Atajos-pl)

---

*md-editor jest tworzony przez Manuel Arias Calleja. Licencja GPL-3.0.*
