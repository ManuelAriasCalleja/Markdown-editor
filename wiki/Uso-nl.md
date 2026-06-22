# Gebruik

## Openen en opslaan

- Nieuw (Ctrl+N), Openen (Ctrl+O), Opslaan (Ctrl+S), Opslaan als (Ctrl+Shift+S).
  Alles in UTF-8.
- **Tabbladen**: elk geopend document neemt zijn eigen tabblad in; sluit er een met
  Ctrl+W. Bij het opnieuw opstarten worden de tabbladen van de laatste sessie
  heropend.
- **Nieuw vanuit sjabloon** (Bestand → Nieuw vanuit sjabloon) vertrekt van een reeds
  voorbereid Markdown-geraamte.
- **Recente openen** toont je laatste documenten.
- Je kunt ook een bestand naar het venster slepen en neerzetten om het te openen.
- Als het bestand buiten md-editor verandert, krijg je een melding: het wordt vanzelf
  opnieuw geladen als je geen wijzigingen had, of er wordt gevraagd of je ze had.

### Front matter

Als je document begint met een blok `---…---` (YAML) of `+++…+++` (TOML), wordt dit
bij het opslaan ongewijzigd behouden (het wordt niet getoond noch bewerkt). Het dient
voor metadata zoals `title` en `lang`, die bij het exporteren worden gebruikt.

## Opmaken

Gebruik het menu Opmaak of de werkbalk. Je hoeft geen Markdown-symbolen te typen: de
editor past ze voor je toe.

- Vet (Ctrl+B), Cursief (Ctrl+I), Onderstrepen (Ctrl+U), Doorhalen, Inline-code,
  Koppeling (Ctrl+K).
- Koppen H1–H6 (Ctrl+1 … Ctrl+6).
- Opsommingslijsten, genummerde lijsten en takenlijsten, met automatische
  voortzetting bij het indrukken van Enter (een leeg punt verlaat de lijst). De
  taakvakjes worden met een klik aangevinkt.
- Citaten en codeblokken.

Raadpleeg alle sneltoetsen in [Sneltoetsen](Atajos-nl).

## Tekst bewerken en transformeren

- **Plakken als platte tekst** (Ctrl+Shift+V) of **Plakken als Markdown** (Ctrl+Alt+V),
  dat de HTML van het klembord omzet naar Markdown. Een URL op een selectie plakken
  maakt er automatisch een koppeling van.
- **Bewerken → Tekst transformeren**: HOOFDLETTERS, kleine letters, beginhoofdletters,
  regels sorteren en slimme typografie (zet `--`, `---`, `...` en rechte
  aanhalingstekens om).

## Invoegen

- Koppeling en Afbeelding (met een pad relatief aan het document zodat het draagbaar is).
- **Afbeelding plakken**: de afbeelding van het klembord wordt opgeslagen als PNG naast
  je `.md` en ingevoegd als `![](ruta)`. Werkt ook door slepen of plakken op de editor.
- Tabel, Horizontale lijn, Inhoudsopgave (TOC) en Formule (Ctrl+Shift+F).
- **Voetnoot** (Ctrl+Shift+N): voegt een verwijzing `[^n]` en de definitie ervan in.
- **Admonitie**: uitgelicht blok (notitie, tip, belangrijk, waarschuwing, let op).
- **Speciale symbolen** en **Datum / Datum en tijd**.

## Tabellen

Met de cursor in een tabel maakt het menu Tabel het mogelijk om rijen en kolommen toe
te voegen of te verwijderen en elke kolom uit te lijnen (links/midden/rechts). De
uitlijning blijft behouden bij het opslaan.

## Formules

Voeg TeX-formules in inline (`$...$`) of in blok (`$$...$$`) in via Invoegen → Formule
(Ctrl+Shift+F), met live voorbeeldweergave. Dubbelklik op een formule om deze te
bewerken. Ze worden in echte 2D getekend (breuken, wortels, matrices, sommaties met
grenzen…). Meer details in [Functies](Caracteristicas-nl#tex-formules).

## Diagrammen

Schrijf een codeblok met taal `mermaid` of `plantuml` en, als je het bijbehorende
hulpprogramma (`mmdc` / `plantuml`) hebt geïnstalleerd, wordt het gerenderd als
afbeelding onder het blok. Ontbreekt het, dan zie je de opdracht om het te installeren.

## Spellingcontrole

Schakel deze in via Beeld → Spellingcontrole (vereist Hunspell). De taal wordt gekozen
op basis van die van het document of handmatig via Beeld → Controletaal. Rechtsklik op
een onderstreept woord biedt suggesties en toevoegen aan het persoonlijke woordenboek.

## Weergavemodi

- **WYSIWYG** (standaard): alleen het gerenderde resultaat.
- **Markdown-broncode** (Ctrl+Shift+M): de ruwe Markdown, schermvullend.
- **Gesplitste weergave** (Ctrl+Shift+D): weergave en code naast elkaar,
  gesynchroniseerd.
- **Overzicht** (F9) en **Ga naar kop** (Ctrl+G) om door het document te navigeren.

## Zoeken en vervangen

Ctrl+F om te zoeken, Ctrl+H om te vervangen. Inclusief vorige/volgende, alles
vervangen en hoofdlettergevoeligheid.

## Exporteren en afdrukken

Bestand → Exporteren biedt PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) en EPUB
(.epub); ook Afdrukvoorbeeld en Afdrukken (Ctrl+P). Bij ODF, DOCX en LaTeX wordt de
taal van het document ingebed.

## Automatisch herstel

md-editor slaat om de paar seconden een concept op. Als de toepassing abnormaal wordt
afgesloten, wordt je bij het opnieuw openen aangeboden om te herstellen wat je aan het
schrijven was.
