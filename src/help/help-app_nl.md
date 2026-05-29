# Gebruikershandleiding

**md-editor** is een visuele (WYSIWYG) Markdown-editor: je schrijft en past
opmaak toe op de reeds weergegeven tekst, zonder de code te zien. Bij het
opslaan wordt het document terug geserialiseerd naar pure Markdown.

## Inhoud

- [Openen en opslaan](#openen-en-opslaan)
- [Tekst opmaken](#tekst-opmaken)
- [Koppen, lijsten en blokken](#koppen-lijsten-en-blokken)
- [Koppelingen en afbeeldingen](#koppelingen-en-afbeeldingen)
- [Tabellen](#tabellen)
- [Wiskundige formules](#wiskundige-formules)
- [Zoeken en vervangen](#zoeken-en-vervangen)
- [Documentoverzicht](#documentoverzicht)
- [Afleidingsvrije modus](#afleidingsvrije-modus)
- [Broncodeweergave](#broncodeweergave)
- [Exporteren en afdrukken](#exporteren-en-afdrukken)
- [Thema's en weergave](#themas-en-weergave)
- [Automatisch herstel](#automatisch-herstel)
- [Sneltoetsen](#sneltoetsen)

## Openen en opslaan

- **Bestand → Nieuw** (Ctrl+N) maakt een leeg document.
- **Bestand → Openen…** (Ctrl+O) opent een bestaande `.md`. De toepassing
  onthoudt de meest recente bestanden in **Bestand → Recente openen**.
- **Opslaan** (Ctrl+S) en **Opslaan als…** (Ctrl+Shift+S) schrijven het
  document weg in UTF-8.
- Als het bestand buiten de editor wijzigt, detecteert de toepassing dat en,
  als je geen niet-opgeslagen wijzigingen hebt, laadt het het opnieuw; heb je
  die wel, dan vraagt het wat te doen.
- Je kunt een bestand ook **slepen en neerzetten** op het venster om het te
  openen.

### Front matter

Als het document begint met een `---…---` (YAML) of `+++…+++` (TOML) blok,
wordt dat ongewijzigd bewaard bij het opslaan: het wordt niet in de editor
getoond en is niet bewerkbaar. Het is bedoeld voor metadata zoals `title`,
`lang`, enz., die bij het exporteren worden gebruikt.

## Tekst opmaken

Selecteer een fragment en pas opmaak toe via de werkbalk of het menu
**Opmaak**:

- **Vet** (Ctrl+B), **Cursief** (Ctrl+I), **Onderstrepen** (Ctrl+U),
  **Doorhalen**.
- **Inline-code** voor `monospace`-fragmenten.
- **Koppeling**: voegt `[tekst](url)` toe over de selectie.

De werkbalkknoppen weerspiegelen de actieve opmaak onder de cursor.

## Koppen, lijsten en blokken

- **Koppen** H1–H6 via **Opmaak → Kop** of met Ctrl+1 … Ctrl+6.
- **Lijsten**: opsommingstekens, genummerd en takenlijsten (met een
  selectievakje). Door op Enter te drukken aan het einde van een item wordt
  het volgende automatisch aangemaakt; Enter op een leeg item verlaat de
  lijst.
- **Citaat** (`>` aan het begin van een alinea) en **codeblok** worden
  toegepast vanuit de werkbalk; beide round-trippen correct naar Markdown.

## Koppelingen en afbeeldingen

- **Invoegen → Koppeling…** opent een dialoogvenster met velden voor tekst en
  URL. Had je een selectie, dan wordt die als tekst gebruikt.
- **Ctrl+klik** op een koppeling opent die in de systeembrowser; bij het
  zweven met de muis wordt de URL in de statusbalk getoond.
- **Afbeeldingen**: sleep een bestand, plak een afbeelding van het klembord of
  gebruik **Invoegen → Afbeelding plakken**. De afbeelding wordt als PNG naast
  de `.md` opgeslagen en ingevoegd als `![alt](relatief-pad)`; zo overleeft ze
  de round-trip naar Markdown (ingebedde afbeeldingen niet).

## Tabellen

- **Tabel → Tabel invoegen…** vraagt om rijen en kolommen.
- De acties in het menu **Tabel** (rij of kolom toevoegen/verwijderen, kolom
  uitlijnen) zijn alleen actief wanneer de cursor zich in een tabel bevindt.
- De kolomuitlijning (links/midden/rechts) blijft bij het opslaan behouden als
  `:--`/`:-:`/`--:`.

## Wiskundige formules

md-editor ondersteunt **TeX-formules** inline (`$...$`) en als blok
(`$$...$$`), met de gebruikelijke LaTeX-syntaxis (Pandoc, Obsidian, Quarto…).
Er zijn geen externe afhankelijkheden nodig.

- **Invoegen → Formule…** (Ctrl+Shift+F) opent een dialoogvenster met een
  TeX-veld en een **live voorbeeld**: terwijl je typt zie je hoe het eruit zal
  zien. Kies *Inline* of *Blok* en bevestig om de formule in te voegen.
- In de editor verschijnen formules cursief met de accentkleur van het thema,
  met **echte super-/subscripts** (geen platte Unicode-tekens): `x²`, `Hᵢ`,
  enzovoort — de vertical-align van Qt schaalt elk teken correct.
- **Dubbelklik** op een formule om het dialoogvenster opnieuw te openen met de
  oorspronkelijke TeX vooraf ingevuld: bewerk en bevestig om die te vervangen.
- Formules zijn **atomisch**: binnenin typen geeft een herinnering om te
  dubbelklikken voor bewerken; Backspace/Delete aan de rand verwijdert de hele
  groep.
- Bij het **exporteren** blijven formules behouden: LaTeX geeft ze
  ongewijzigd door (met `amsmath` en `amssymb` in het preambule); HTML/PDF/ODF
  behouden de super-/subscripts met vertical-align van Qt in het doelformaat.
- In de **broncodeweergave** zie je ze als `$...$` / `$$...$$`, met alle
  TeX-tekens (`\sum`, `\frac`, `_`, `*`) intact bij het opslaan.

Voorbeelden:

```
De energie is $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Beperking: in de bron mag `$$...$$` meerdere regels beslaan
> (Obsidian-/Pandoc-stijl); `$...$` moet op dezelfde regel openen en sluiten.

## Zoeken en vervangen

- **Zoeken** (Ctrl+F) opent een onderbalk met velden voor zoeken en vervangen,
  plus opties (hoofdletters, heel woord).
- **Volgende zoeken** F3 / **Vorige zoeken** Shift+F3.

## Documentoverzicht

Het zijpaneel links toont de index van koppen (TOC): het werkt bij tijdens het
typen, en door op een item te klikken springt de cursor naar die kop. Het
wordt in-/uitgeschakeld met F9.

## Afleidingsvrije modus

**Beeld → Afleidingsvrije modus** (F11) opent het volledige scherm met
verborgen menu en werkbalken en de tekst gecentreerd in een leeskolom. Het
overzicht blijft, indien zichtbaar, vastgehecht aan het centrale blok. ESC of
F11 sluit af.

## Broncodeweergave

**Beeld → Markdown-broncode** (Ctrl+Shift+M) schakelt tussen de visuele editor
en een schermvullende platte-teksteditor met de ruwe Markdown. Wijzigingen in
de bronmodus worden in het document doorgevoerd wanneer je terugkeert naar de
visuele modus.

**Beeld → Gesplitste weergave** (Ctrl+Shift+D) toont beide naast elkaar: de
visuele editor en de bron, gesynchroniseerd (wat je in de ene typt wordt in de
andere weerspiegeld). Het sluit de schermvullende bronmodus uit.

## Exporteren en afdrukken

**Bestand → Exporteren** biedt **PDF**, **HTML**, **ODF (.odt)** en
**LaTeX (.tex)**. Voor ODF en LaTeX wordt de documenttaal ingebed (genomen uit
de front matter `lang`/`language`, de toepassingsinstelling of, als laatste
redmiddel, de systeemlocale).

**Bestand → Afdrukken** (Ctrl+P) opent het systeemdialoogvenster.

## Thema's en weergave

- **Beeld → Thema** biedt Licht, Donker, GitHub Light, GitHub Dark, Monokai en
  Hoog contrast.
- **Beeld → Warm nachtlicht** dempt de blauwtinten in de achtergrond op basis
  van het tijdstip van de dag.
- **Zoom**: Ctrl+muiswiel, Ctrl++ / Ctrl+- en **Normale grootte** (Ctrl+0)
  schalen de hele interface (niet alleen de editortekst).
- **Beeld → Taal** wijzigt de taal van de interface; werkt na opnieuw
  opstarten.

## Automatisch herstel

Terwijl je bewerkt, wordt de inhoud om de paar seconden automatisch opgeslagen
in een conceptkopie. Sluit de toepassing onverwacht af, dan biedt ze bij de
volgende start aan om te herstellen wat je aan het schrijven was.

## Sneltoetsen

| Actie                     | Sneltoets        |
|---------------------------|------------------|
| Nieuw                     | Ctrl+N           |
| Openen                    | Ctrl+O           |
| Opslaan                   | Ctrl+S           |
| Opslaan als               | Ctrl+Shift+S     |
| Afdrukken                 | Ctrl+P           |
| Ongedaan maken / Opnieuw  | Ctrl+Z / Ctrl+Y  |
| Vet / Cursief             | Ctrl+B / Ctrl+I  |
| Onderstrepen              | Ctrl+U           |
| Zoeken                    | Ctrl+F           |
| Volgende / vorige zoeken  | F3 / Shift+F3    |
| Kop H1 … H6               | Ctrl+1 … Ctrl+6  |
| Formule invoegen          | Ctrl+Shift+F     |
| Markdown-broncodeweergave | Ctrl+Shift+M     |
| Gesplitste weergave       | Ctrl+Shift+D     |
| Overzicht                 | F9               |
| Afleidingsvrije modus     | F11              |
| Zoom + / − / Normaal      | Ctrl++ / Ctrl+− / Ctrl+0 |
| Help                      | F1               |
