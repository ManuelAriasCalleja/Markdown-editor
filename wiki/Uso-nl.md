# Gebruik

## Openen en opslaan

- Nieuw (Ctrl+N), Openen (Ctrl+O), Opslaan (Ctrl+S), Opslaan als (Ctrl+Shift+S).
  Alles in UTF-8.
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
  voortzetting bij het indrukken van Enter (een leeg punt verlaat de lijst).
- Citaten en codeblokken.

Raadpleeg alle sneltoetsen in [Sneltoetsen](Atajos-nl).

## Invoegen

- Koppeling en Afbeelding (met een pad relatief aan het document zodat het draagbaar is).
- **Afbeelding plakken**: de afbeelding van het klembord wordt opgeslagen als PNG naast
  je `.md` en ingevoegd als `![](ruta)`. Werkt ook door slepen of plakken op de editor.
- Tabel, Horizontale lijn en Formule (Ctrl+Shift+F).

## Tabellen

Met de cursor in een tabel maakt het menu Tabel het mogelijk om rijen en kolommen toe
te voegen of te verwijderen en elke kolom uit te lijnen (links/midden/rechts). De
uitlijning blijft behouden bij het opslaan.

## Formules

Voeg TeX-formules in inline (`$...$`) of in blok (`$$...$$`) in via Invoegen → Formule
(Ctrl+Shift+F), met live voorbeeldweergave. Dubbelklik op een formule om deze te
bewerken. Meer details in [Functies](Caracteristicas-nl#tex-formules).

## Weergavemodi

- **WYSIWYG** (standaard): alleen het gerenderde resultaat.
- **Markdown-broncode** (Ctrl+Shift+M): de ruwe Markdown, schermvullend.
- **Gesplitste weergave** (Ctrl+Shift+D): weergave en code naast elkaar,
  gesynchroniseerd.

## Zoeken en vervangen

Ctrl+F om te zoeken, Ctrl+H om te vervangen. Inclusief vorige/volgende, alles
vervangen en hoofdlettergevoeligheid.

## Exporteren en afdrukken

Bestand → Exporteren biedt PDF, HTML, ODF (.odt) en LaTeX (.tex); Afdrukken is Ctrl+P.
Bij ODF en LaTeX wordt de taal van het document ingebed.

## Automatisch herstel

md-editor slaat om de paar seconden een concept op. Als de toepassing abnormaal wordt
afgesloten, wordt je bij het opnieuw openen aangeboden om te herstellen wat je aan het
schrijven was.
