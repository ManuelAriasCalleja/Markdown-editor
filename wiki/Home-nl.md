# md-editor

WYSIWYG-Markdown-editor/-viewer in Qt6 + C++17. Standaard bewerk je de reeds
gerenderde tekst, zonder met de syntaxis te hoeven omgaan; maar optioneel kun je de
Markdown-code zien, en zelfs de code en de weergave ervan naast elkaar hebben
(gesplitste weergave) en aan beide kanten bewerken. Bij het opslaan wordt altijd naar
schone Markdown geserialiseerd.

## Wat het voor je doet

- **Echte WYSIWYG**: je ziet het resultaat, niet de symbolen.
- **Getrouwe round-trip**: wat je opent is wat je opslaat, met uitgelijnde tabellen,
  takenlijsten, citaten, codeblokken en formules.
- **Drie manieren van werken**: alleen gerenderd (standaard), alleen code, of beide
  naast elkaar (gesynchroniseerde gesplitste weergave).
- **Afleidingsvrije modus**: gecentreerde leeskolom, zonder werkbalken (F11), met de
  optionele inhoudsopgave (je toont of verbergt deze).
- **Oogvriendelijk**: het *Warm nachtlicht* dempt het blauw van de achtergrond
  geleidelijk naargelang het tijdstip van de dag, om vermoeide ogen 's nachts te
  verminderen.
- **TeX-formules**: [inline](Caracteristicas-nl#tex-formules) en [in blok](Caracteristicas-nl#tex-formules),
  met echte superscripten/subscripten en live voorbeeldweergave, zonder externe
  afhankelijkheden.
- **Export** naar PDF, HTML, ODF (.odt) en LaTeX (.tex), met behoud van de taal van
  het document en de opmaak van de formules.
- **Weergave**: 1) 6 lichte en donkere thema's, 2) zoom van de hele interface, 3)
  interface vertaald in 9 talen.

## Beginnen

- [Installatie](Instalacion-nl)
- [Gebruik](Uso-nl)
- [Functies](Caracteristicas-nl)
- [Sneltoetsen](Atajos-nl)

---

*md-editor wordt ontwikkeld door Manuel Arias Calleja. Licentie CC BY-ND 4.0.*
