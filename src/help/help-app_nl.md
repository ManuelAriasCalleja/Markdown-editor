# Gebruikershandleiding

**md-editor** is een visuele (WYSIWYG) Markdown-editor: je schrijft en maakt op
in de reeds gerenderde tekst, zonder de code te zien. Bij het opslaan wordt het
document weer naar pure Markdown geserialiseerd.

## Inhoud

- [Openen en opslaan](#openen-en-opslaan)
- [Tekst opmaken](#tekst-opmaken)
- [Koppen, lijsten en blokken](#koppen-lijsten-en-blokken)
- [Tekst omzetten en het klembord](#tekst-omzetten-en-het-klembord)
- [Koppelingen en afbeeldingen](#koppelingen-en-afbeeldingen)
- [Voetnoten](#voetnoten)
- [Aandachtsblokken, symbolen en tekstsnelkoppelingen](#aandachtsblokken-symbolen-en-tekstsnelkoppelingen)
- [Snippets (herbruikbare fragmenten)](#snippets-herbruikbare-fragmenten)
- [Tabellen](#tabellen)
- [Wiskundige formules](#wiskundige-formules)
- [Diagrammen](#diagrammen)
- [Spellingcontrole](#spellingcontrole)
- [Zoeken en vervangen](#zoeken-en-vervangen)
- [Documentoverzicht](#documentoverzicht)
- [Documentstatistieken](#documentstatistieken)
- [Afleidingsvrije modus](#afleidingsvrije-modus)
- [Focusmodus](#focusmodus)
- [Codeweergave](#codeweergave)
- [Exporteren en afdrukken](#exporteren-en-afdrukken)
- [Thema's en uiterlijk](#themas-en-uiterlijk)
- [Automatisch herstel](#automatisch-herstel)
- [Toegankelijkheid](#toegankelijkheid)
- [Sneltoetsen](#sneltoetsen)

## Openen en opslaan

- **Bestand → Nieuw** (Ctrl+N) maakt een leeg document in een nieuw tabblad.
- **Bestand → Nieuw op basis van sjabloon** maakt een document op basis van een skelet
  klaar om in te vullen. De sjablonen zijn gegroepeerd per categorie (Persoonlijk,
  Programmeren, Onderwijs, Bedrijf, Schrijven…).
- **Opslaan als sjabloon…** slaat het huidige document op als je eigen sjabloon (met
  naam en categorie); het verschijnt in het bovenstaande menu naast de ingebouwde.
  **Sjablonen beheren…** (onderaan dat menu) laat je ze bewerken of verwijderen.
- **Bestand → Openen…** (Ctrl+O) opent een bestaand `.md`. De toepassing onthoudt
  de laatst geopende in **Bestand → Recent geopend**.
- **Bestand → Importeren** opent een document uit een ander formaat en zet het om naar
  Markdown in een nieuw naamloos tabblad (het origineel blijft ongemoeid): **Uit HTML…**
  (een webpagina), **Uit EPUB…** (een boek; de hoofdstukken worden op volgorde gelezen)
  en **Andere formaten (Pandoc)…** (DOCX, ODT, RTF, LaTeX, reStructuredText…, als Pandoc
  is geïnstalleerd). Werkt het best met eenvoudige inhoud; houdt rekening met de
  opgegeven tekenset.
- **Opslaan** (Ctrl+S) en **Opslaan als…** (Ctrl+Shift+S) schrijven het document
  in UTF-8. **Bevattende map openen** opent de map van het document in de
  bestandsbeheerder.
- **Terugzetten naar opgeslagen versie** verwerpt de niet-opgeslagen wijzigingen
  en herlaadt het bestand van schijf (vraagt om bevestiging). Alleen beschikbaar
  als het document een bestand en openstaande wijzigingen heeft.
- Als het bestand buiten de editor verandert, detecteert de toepassing dat en
  herlaadt het als je geen niet-opgeslagen wijzigingen hebt; anders vraagt ze wat
  te doen.
- Je kunt ook een bestand op het venster **slepen en neerzetten** om het te
  openen.

### Tabbladen (meerdere documenten)

Je kunt meerdere documenten tegelijk open hebben, elk in zijn eigen **tabblad**:

- **Nieuw** (Ctrl+N), **Nieuw op basis van sjabloon** en **Openen** (Ctrl+O) maken
  een tabblad (of hergebruiken het lege begintabblad). Een bestand neerzetten opent
  het ook in een tabblad; als het al open is, springt het naar dat tabblad.
- Wissel van document door op het tabblad te klikken; sleep tabbladen om ze te
  herordenen. Met het toetsenbord springen **Ctrl+PageDown / Ctrl+PageUp** (of
  **Ctrl+Tab / Ctrl+Shift+Tab**) naar het volgende of vorige tabblad.
- **Tabblad sluiten** (Ctrl+W) sluit het huidige en vraagt of er niet-opgeslagen
  wijzigingen zijn. Het laatste tabblad sluit niet: het wordt een nieuw document.
- **Gesloten tabblad opnieuw openen** (Ctrl+Shift+R) opent het laatst gesloten
  tabblad opnieuw (alleen die met een bestand op schijf).
- Het label toont de bestandsnaam en een punt (•) bij niet-opgeslagen wijzigingen.
- **Rechtsklikken** op een tabblad biedt **Bovenliggende map openen**,
  **Bestandsnaam kopiëren** en **Volledig pad kopiëren**.
- Bij het sluiten van de toepassing worden de open documenten onthouden en de
  volgende keer allemaal heropend.

### *Front matter*

Als het document begint met een blok `---…---` (YAML) of `+++…+++` (TOML), wordt
het bij het opslaan ongewijzigd behouden: het wordt niet in de editor getoond en
niet bewerkt. Het dient voor metagegevens zoals `title`, `lang` enz., die bij het
exporteren worden gebruikt.

## Tekst opmaken

Selecteer een fragment en pas opmaak toe met de werkbalk of het menu **Opmaak**:

- **Vet** (Ctrl+B), **Cursief** (Ctrl+I), **Onderstreept** (Ctrl+U),
  **Doorgehaald** (Ctrl+Shift+X).
- **Code in regel** (Ctrl+E) voor `monospace`-fragmenten.
- **Koppeling** (Ctrl+K): voegt `[tekst](url)` over de selectie toe.
- **Markeren** (Ctrl+Shift+H): omsluit de selectie met `==markering==`; de tekst
  krijgt een markeerachtergrond. Omdat `==` geen standaard-Markdown is, wordt het als
  letterlijke tekst opgeslagen.
- **Superscript** (Ctrl+Shift++) en **Subscript** (Ctrl+Shift+-): verhogen of verlagen
  de geselecteerde tekst; opgeslagen als `^tekst^` en `~tekst~` (Pandoc-stijl).

De werkbalkknoppen weerspiegelen de actieve opmaak onder de cursor.

**Automatisch koppelen.** Bij het typen van `(`, `[`, `{` of `` ` `` wordt het
paar vanzelf gesloten en blijft de cursor ertussen staan; is er tekst
geselecteerd, dan wordt die omsloten. Typ je het sluitteken net vóór zijn
tegenhanger, dan "springt" de editor eroverheen in plaats van het te verdubbelen.

**Invoerregels.** Aan het begin van een regel verandert een Markdown-blokmarkering
gevolgd door een spatie de regel ter plekke (zonder de markering te laten staan):
`#` … `######` + spatie → kop H1…H6; `>` → citaat; `-`, `*` of `+` → opsomming;
`1.` (of `1)`) → genummerde lijst. Levert dezelfde opmaak als de werkbalk.

## Koppen, lijsten en blokken

- **Koppen** H1–H6 via **Opmaak → Kop** of met Ctrl+1 … Ctrl+6.
  **Promoveren/degraderen** van de kop bij de cursor met één niveau via
  Ctrl+Shift+[ / Ctrl+Shift+].
- **Lijsten**: opsommingen (Ctrl+Shift+U), genummerd (Ctrl+Shift+O) en taken
  (Ctrl+Shift+T, met selectievakje). Enter aan het
  einde van een punt maakt automatisch het volgende; Enter op een leeg punt
  verlaat de lijst. Een **klik op het selectievakje** van een taak schakelt deze
  om.
- **Citaat** (Ctrl+Shift+Q, `>` aan het begin van een alinea) en **codeblok**
  (Ctrl+Shift+K) pas je toe via de
  werkbalk; beide gaan correct heen en terug naar Markdown. Met **Opmaak → Taal
  van het blok…** kies je de taal van een codeblok (met de cursor erin) zodat de
  syntaxis ervan wordt gemarkeerd.
- Als je met de muis **over** een codeblok gaat, verschijnen rechtsboven de **taal** (klik om te wijzigen) en een knop om de code te **kopiëren**.
- **Inspringen**: **Opmaak → Inspringen vergroten/verkleinen** (Ctrl+] / Ctrl+[)
  nestelt lijsten en citaten.

## Tekst omzetten en het klembord

- **Bewerken → Tekst omzetten** verandert het hoofdlettergebruik:
  **HOOFDLETTERS**, **kleine letters** en **Met hoofdletter**. Het werkt op de
  selectie of, als er geen selectie is, op het woord onder de cursor.
- **Bewerken → Regels sorteren** sorteert de geselecteerde regels alfabetisch.
  Hier is een selectie wél nodig: zonder selectie gebeurt er niets.
- **Slimme typografie** (in hetzelfde menu) zet in de selectie de streepjes
  `--`/`---` om in `–`/`—`, `...` in `…` en rechte aanhalingstekens in
  typografische, afhankelijk van de context.
- **Plakken als platte tekst** (Ctrl+Shift+V) plakt zonder opmaak. **Plakken als
  Markdown** (Ctrl+Alt+V) zet de opgemaakte klembordinhoud (HTML) om naar Markdown
  in plaats van de opmaak van de bron in te bedden.
- **Kopiëren als HTML** kopieert de selectie (of het document) als HTML, om in een
  e-mail, een CMS enz. te plakken.
- **Kopiëren als Markdown** kopieert de selectie (of het hele document) als
  Markdown-tekst, om in een andere Markdown-editor of een codeveld te plakken.
- Als je een **URL** over een tekstselectie plakt, wordt de tekst automatisch
  gekoppeld.
- **Bewerken → Markdown opschonen** normaliseert het hele document in één keer:
  het maakt de opsommingstekens uniform met `-`, snijdt de spaties aan het einde
  van elke regel weg, vouwt overtollige lege regels samen en past de spatie na de
  `#` van de koppen aan. Het is voorzichtig: het raakt de inhoud van de codeblokken
  niet aan.

## Koppelingen en afbeeldingen

- **Invoegen → Koppeling…** opent een dialoog met tekst en URL. Een bestaande
  selectie wordt als tekst gebruikt.
- **Ctrl+klik** op een koppeling opent deze in de systeembrowser; bij het zweven
  met de muis verschijnt de URL in een tooltip naast de cursor en in de statusbalk.
- **Invoegen → Afbeelding…** vraagt om de alternatieve tekst en het pad van het
  bestand (of een URL). Is het document al opgeslagen, dan wordt het pad
  **relatief** aan de `.md` geschreven, zodat het document overdraagbaar blijft.
- **Een afbeelding plakken** van het klembord (Ctrl+V, of **Invoegen →
  Afbeelding plakken**) slaat ze als PNG naast de `.md` op en voegt ze in als
  `![alt](relatief-pad)` in plaats van ze in te bedden: zo overleeft ze de
  heen-en-terugconversie naar Markdown (ingebedde afbeeldingen niet).
- Een afbeeldingsbestand op het venster slepen voegt het **niet** in: net als elk
  ander bestand wordt het in een tabblad geopend.

## Voetnoten

- **Invoegen → Voetnoot** (Ctrl+Shift+N) voegt bij de cursor een genummerde
  verwijzing `[^n]` in en maakt de definitie `[^n]:` aan het einde van het
  document, klaar voor de noottekst.
- Verwijzingen worden in **superscript** getoond; een **klik** erop verplaatst de
  cursor naar de definitie.
- Ze worden als standaard-Markdown opgeslagen (`tekst[^1]` in de tekst en eronder
  `[^1]: de noot`), dus compatibel met andere editors.

## Aandachtsblokken, symbolen en tekstsnelkoppelingen

- **Invoegen → Aandachtsblok** maakt een callout in GitHub-stijl: een citaat
  waarvan de eerste regel `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` of
  `[!CAUTION]` is. Het wordt getoond met een getinte achtergrond en een gekleurde
  titel, en opgeslagen als GitHub-compatibele Markdown.
- **Invoegen → Horizontale lijn** voegt tussen blokken een scheidingslijn in
  (`---` in Markdown).
- **Invoegen → Speciale symbolen…** opent een tekentabel per categorie (wiskundig,
  Grieks, pijlen, valuta, leestekens…); een klik voegt het symbool in en het
  venster blijft open om er meerdere in te voegen.
- **`:naam:`-snelkoppelingen**: bij het typen van een code zoals `:alpha:` of
  `:euro:` wordt deze uitgebreid tot het bijbehorende symbool (α, €…).
- **Invoegen → Datum** en **Datum en tijd** voegen de huidige datum (en tijd) in
  gelokaliseerde notatie in.

## Snippets (herbruikbare fragmenten)

Een **snippet** is een stuk Markdown dat je onder een naam opslaat om het later
met een paar klikken in te voegen: een handtekening, een tabelsjabloon, een
melding die je vaak herhaalt…

- **Invoegen → Snippet** klapt de lijst uit van de snippets die je hebt; kies je
  er een, dan wordt de inhoud ervan ingevoegd waar de cursor staat (werkt ook in
  de codeweergave).
- **Invoegen → Snippet → Snippets beheren…** opent een dialoog om je snippets te
  maken, te bewerken en te verwijderen. Elk heeft een **naam** (die je in het menu
  ziet) en een **inhoud** in Markdown.
- Ze worden in de instellingen van de toepassing opgeslagen, dus ze zijn in al je
  documenten beschikbaar, niet alleen in het huidige.

## Tabellen

- Als de cursor **in een tabel** staat, verschijnt erboven een **zwevende balk** met knoppen om rijen en kolommen in te voegen/verwijderen en de kolom uit te lijnen.
- **Tab** gaat naar de volgende cel en **Shift+Tab** naar de vorige; **Tab** in de laatste cel voegt een rij toe.
- **Invoegen → Tabel…** vraagt om rijen en kolommen en maakt de tabel op de
  cursorpositie.
- **Invoegen → Tabel uit klembord** zet TSV/CSV-gegevens (kolommen gescheiden door
  tabs of komma's) die uit een spreadsheet of een CSV-bestand zijn gekopieerd, om in
  een tabel.
- De acties van het menu **Tabel** (rij of kolom toevoegen/verwijderen, kolom
  uitlijnen) zijn alleen actief wanneer de cursor in een tabel staat.
- De kolomuitlijning (links/midden/rechts) blijft bij het opslaan behouden als
  `:--`/`:-:`/`--:`.
- **Tabel → Rijen sorteren op kolom** (oplopend/aflopend) herschikt de rijen op de
  kolom van de cursor, met de koprij vast; detecteert of de kolom numeriek of tekst
  is.

## Wiskundige formules

md-editor ondersteunt **TeX-formules** in regel (`$...$`) en in blok (`$$...$$`),
met de gebruikelijke LaTeX-syntaxis (Pandoc, Obsidian, Quarto…). Er is geen
externe afhankelijkheid nodig.

- **Invoegen → Formule…** (Ctrl+Shift+F) opent een dialoog met een veld voor de
  TeX en een **live voorvertoning**: terwijl je typt zie je het resultaat. Kies
  *In regel* of *Blok* en bevestig om in te voegen.
- Formules worden in **echt 2D** opgemaakt: breuken (`\frac`) worden met een
  streep gestapeld, grote operatoren (`\sum`, `\int`, `\prod`…) tonen hun grenzen
  boven en onder, wortels (`\sqrt`) dragen hun streep, en er zijn matrices
  (`\begin{pmatrix}`…), binomiaalcoëfficiënten (`\binom`) en accenten (`\hat`,
  `\vec`, `\bar`…). Eenvoudigere (machten, subscripts, Grieks) worden in regel
  opgemaakt. De weergave schaalt mee met de zoom.
- **Dubbelklik** op een formule heropent de dialoog met de oorspronkelijke TeX
  voorgeladen: je bewerkt en bij bevestiging wordt ze vervangen.
- Formules zijn **atomair**: typ je erin, dan herinnert de app aan de dubbelklik;
  Backspace/Delete aan de rand verwijderen de hele groep.
- Bij het **exporteren** blijven ze behouden: naar LaTeX worden ze ongewijzigd
  uitgevoerd (met `amsmath` en `amssymb` in de preambule); naar HTML/PDF/ODF worden
  ze teruggebracht tot hun benadering in regel.
- In de **codeweergave** verschijnen ze als `$...$` / `$$...$$`, met alle
  TeX-tekens (`\sum`, `\frac`, `_`, `*`) intact bij het opslaan.

Voorbeelden:

```
De energie is $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> In de bron mag `$$...$$` meerdere regels beslaan (Obsidian/Pandoc-stijl);
> `$...$` moet op dezelfde regel openen en sluiten.

## Diagrammen

Een codeblok met taal `mermaid` of `plantuml` wordt **als afbeelding
voorvertoond** vlak onder het blok, zonder de code (die bewerkbaar blijft) of de
opgeslagen Markdown aan te raken.

- Het vereist dat het bijbehorende hulpprogramma is geïnstalleerd: **`plantuml`**
  (met Java) voor PlantUML, of **`mmdc`** (mermaid-cli, met Node) voor Mermaid.
- Ontbreekt het hulpprogramma, dan verschijnt onder het blok een melding met het
  installatiecommando voor jouw besturingssysteem; het blok blijft code.
- De afbeelding is alleen presentatie: ze wordt niet naar de Markdown geschreven
  en telt niet als een niet-opgeslagen wijziging.
- **Beeld → Diagrammen voorvertonen** schakelt deze automatische voorvertoning in
  of uit (standaard ingeschakeld). Zet het uit als je de blokken liever als code
  laat staan — bijvoorbeeld wanneer je zelf al een vooraf gerenderde afbeelding
  onder het blok zet en die niet dubbel wilt zien.

Een codeblok met label `mermaid` dat `flowchart LR  A --> B --> C` bevat, wordt
bijvoorbeeld voorvertoond als het bijbehorende stroomschema.

## Spellingcontrole

- Onderstreept verkeerd gespelde woorden in het rood volgens de **documenttaal**
  (uit de `lang`-front-matter, de taalinstelling of het systeem). Code, formules
  en koppelingen worden niet gecontroleerd.
- **Rechtsklik** op een onderstreept woord biedt **suggesties** (een klik vervangt
  het), **Aan woordenboek toevoegen** (een permanente persoonlijke lijst) en
  **Negeren** (voor de sessie).
- Ze wordt in-/uitgeschakeld via **Beeld → Spellingcontrole**, en de taal stel je
  in via **Beeld → Taal van spellingcontrole** (of laat je automatisch).
- Ze heeft Hunspell-woordenboeken nodig: op Linux die van het systeem
  (`hunspell-es`, `hunspell-en-us`…); op Windows/macOS worden ze met de toepassing
  meegeleverd.

## Zoeken en vervangen

- **Zoeken** (Ctrl+F) opent onderaan een balk met velden om te zoeken en te
  vervangen, plus opties (hoofdlettergevoelig, heel woord, regex).
  **Vervangen…** (Ctrl+H) opent dezelfde balk met de focus in het vervangveld.
  ESC sluit hem.
- **Volgende zoeken** (F3) en **Vorige zoeken** (Shift+F3) herhalen de laatste
  zoekopdracht zonder terug te gaan naar de balk, ook als die gesloten is; heb je
  nog niets gezocht, dan opent F3 de balk. Aan het einde begint de zoekopdracht
  weer bij het begin.
- De balk markeert **alle** overeenkomsten in het document en toont een teller **'N van M'** (op welke overeenkomst je bent, van hoeveel). **Alles vervangen** vervangt ze allemaal in één keer.

## Documentoverzicht

Het linker zijpaneel toont het koppenoverzicht (inhoudsopgave): het werkt bij
tijdens het typen en, bij een klik op een item, springt de cursor naar die kop.
Het wordt getoond/verborgen met F9. Met **F6** verplaats je de
toetsenbordfocus naar het overzicht (dat zo nodig wordt getoond); daar verplaatsen
de pijltjestoetsen je door de koppen en springt **Enter** naar de geselecteerde kop
en keert de focus terug naar de editor. Druk je nogmaals op **F6**, dan
keert de focus simpelweg terug naar de editor.

Het **filterveld** boven in het paneel toont alleen de koppen die overeenkomen met
wat je typt (en hun voorouders); de knoppen **⊞/⊟** vouwen alles uit of samen. Het
samenvouwen dat je instelt **blijft behouden**, ook terwijl je verder typt.

Je kunt een overzichtsitem **slepen** om die sectie —de kop, de inhoud en de
subsecties— in het document te **herordenen**, zonder het niveau te wijzigen.
Bovendien plaatst **Invoegen → Inhoudsopgave (TOC)** een geneste lijst van de
koppen in het document. **Beeld → Ga naar kop…** (Ctrl+G) springt naar een kop
door een deel van de tekst te typen, en **Ga naar regel…** (Ctrl+L) brengt de
cursor naar een regelnummer (in de bronweergave, naar de regel van de Markdown).
Het **Opdrachtenpalet** (Ctrl+Shift+P) zoekt en voert elke menu-actie uit door
een deel van de naam te typen.

## Documentstatistieken

- **Beeld → Documentstatistieken…** toont woorden, tekens (met en zonder
  spaties), alinea's, zinnen en de geschatte leestijd (van het document of de
  selectie).
- **Beeld → Woordenteller tonen** schakelt een permanente teller in de statusbalk
  in.
- **Beeld → Regel en kolom tonen** toont de cursorpositie (regel en kolom) in de
  statusbalk.

## Afleidingsvrije modus

**Beeld → Afleidingsvrij** (F11) gaat naar volledig scherm met het menu en de
balken verborgen en de tekst gecentreerd in een leeskolom. Het overzicht blijft,
indien zichtbaar, tegen het centrale blok. ESC of F11 verlaten.

## Focusmodus

**Beeld → Focusmodus** (F12) helpt je je te concentreren op wat je schrijft zonder het
normale venster te verlaten. Eén enkele schakelaar activeert twee dingen tegelijk:

- **Typemachine**: de regel met de cursor blijft verticaal gecentreerd. Terwijl je
  typt, verschuift de tekst zodat de actieve regel op halve hoogte blijft, in
  plaats van tegen de onderrand aan te kruipen.
- **Dimmen**: het hele document wordt gedempt weergegeven, behalve de alinea waar
  de cursor staat, die helder oplicht.

Het werkt in de visuele editor en in de codeweergave, en is **onafhankelijk** van
de afleidingsvrije modus (F11): je kunt beide tegelijk gebruiken of elk
afzonderlijk.

## Codeweergave

**Beeld → Markdown-bron** (Ctrl+Shift+M) wisselt tussen de visuele editor en een
platte-tekst-editor, op volledig scherm, met de ruwe Markdown. Wijzigingen in de
bronmodus worden naar het document doorgevoerd bij terugkeer naar de visuele modus.

**Beeld → Gesplitste weergave** (Ctrl+Shift+D) toont beide tegelijk, naast elkaar:
de visuele editor en de bron, gesynchroniseerd (wat je in de ene typt, verschijnt
in de andere). Ze sluit elkaar uit met de bronmodus op volledig scherm.

In de bronweergave zijn er **regelcommando's** via het toetsenbord voor de regel van
de cursor: **Alt+↑ / Alt+↓** verplaatsen de regel omhoog/omlaag, **Ctrl+D**
dupliceert hem, **Ctrl+Shift+K** verwijdert hem en **Ctrl+J** voegt hem samen met de
volgende.

## Exporteren en afdrukken

**Bestand → Exporteren** biedt **PDF**, **HTML**, **ODF (.odt)**, **DOCX
(.docx)**, **LaTeX (.tex)**, **EPUB (.epub)** en **platte tekst (.txt)**. In ODF,
DOCX, LaTeX en EPUB wordt de documenttaal ingebed (uit de
`lang`/`language`-front-matter, de toepassingsinstelling of, als laatste, de
systeemtaal). In PDF worden de titel en de auteur ingebed wanneer ze in de
front matter staan (`title`, `author`).

Je kunt ook **alleen de selectie naar PDF** exporteren en het **Afdrukvoorbeeld**
gebruiken.

**Bestand → Afdrukken** (Ctrl+P) opent het systeemdialoogvenster; **Selectie
afdrukken** drukt alleen het geselecteerde af.

**Beeld → Paginanummers bij afdrukken** (standaard aan) voegt het paginanummer toe in
de voettekst (`N / M`) bij afdrukken en bij exporteren naar PDF.

## Thema's en uiterlijk

- **Bewerken → Voorkeuren…** (Ctrl+,) verzamelt de meest gebruikte instellingen (thema, warm licht, regelafstand, huidige regel markeren, focusmodus, paginanummers) in een dialoog met tabbladen.
- **Beeld → Thema** biedt Licht, Donker, GitHub Light, GitHub Dark, Monokai, Hoog contrast, Solarized Light en Solarized Dark. **Systeem volgen** stemt het lichte/donkere thema af op dat van
  het besturingssysteem.
- **Beeld → Nachtelijk warm licht** dempt de blauwtinten van de achtergrond
  afhankelijk van het tijdstip.
- **Beeld → Regelafstand** stelt de regelhoogte van de editor in: Enkel, 1,5 regel of Dubbel.
- **Beeld → Huidige regel markeren** markeert de regel van de cursor met een subtiele achtergrond.
- **Zoom**: Ctrl+muiswiel, Ctrl++ / Ctrl+- en **Normale grootte** (Ctrl+0) schalen
  de hele interface (niet alleen de editortekst).
- **Beeld → Taal** wijzigt de interfacetaal; ze wordt meteen toegepast (het venster
  wordt opnieuw gemaakt).

## Automatisch herstel

Terwijl je bewerkt, wordt de inhoud om de paar seconden automatisch opgeslagen in
een conceptkopie. Sluit de toepassing abnormaal af, dan biedt ze bij het opnieuw
openen aan om te herstellen wat je aan het schrijven was.

## Toegankelijkheid

- **Schermlezers**: de editor, het overzichtspaneel, de zoekvelden en de overige bedieningselementen hebben een toegankelijke naam; bovendien worden statusberichten (opgeslagen, “niet gevonden”, wijzigingen op schijf…) hardop aangekondigd.
- **Alleen toetsenbord**: elke actie heeft een sneltoets of een menu-item (F10 of Alt opent de menubalk). Zie de tabel [Sneltoetsen](#sneltoetsen).
- **Contrast en grootte**: het thema **Hoog contrast** en de **zoom** van de hele interface helpen bij slechtziendheid; de aanvankelijke lettergrootte is die van het systeem.
- **Focus**: het gefocuste element wordt gemarkeerd met de selectiekleur van het thema.

## Sneltoetsen

| Actie                     | Sneltoets        |
|---------------------------|------------------|
| Nieuw                     | Ctrl+N           |
| Tabblad sluiten           | Ctrl+W           |
| Gesloten tabblad opnieuw openen | Ctrl+Shift+R |
| Volgend / vorig tabblad   | Ctrl+PageDown / Ctrl+PageUp (of Ctrl+Tab / Ctrl+Shift+Tab) |
| Openen                    | Ctrl+O           |
| Opslaan                   | Ctrl+S           |
| Opslaan als               | Ctrl+Shift+S     |
| Afdrukken                 | Ctrl+P           |
| Voorkeuren                | Ctrl+,           |
| Ongedaan maken / Opnieuw  | Ctrl+Z / Ctrl+Y  |
| Vet / Cursief             | Ctrl+B / Ctrl+I  |
| Onderstreept / Doorgehaald | Ctrl+U / Ctrl+Shift+X |
| Code in regel             | Ctrl+E           |
| Koppeling                 | Ctrl+K           |
| Markeren (==markering==)   | Ctrl+Shift+H     |
| Superscript / Subscript   | Ctrl+Shift++ / Ctrl+Shift+- |
| Plakken als platte tekst  | Ctrl+Shift+V     |
| Plakken als Markdown      | Ctrl+Alt+V       |
| Zoeken / Vervangen        | Ctrl+F / Ctrl+H  |
| Volgende/vorige zoeken    | F3 / Shift+F3    |
| Kop H1 … H6               | Ctrl+1 … Ctrl+6  |
| Kop promoveren / degraderen | Ctrl+Shift+[ / Ctrl+Shift+] |
| Opsommings- / genummerde / takenlijst | Ctrl+Shift+U / Ctrl+Shift+O / Ctrl+Shift+T |
| Inspringen vergroten / verkleinen | Ctrl+] / Ctrl+[ |
| Citaat                    | Ctrl+Shift+Q     |
| Codeblok                  | Ctrl+Shift+K     |
| Formule invoegen          | Ctrl+Shift+F     |
| Voetnoot invoegen         | Ctrl+Shift+N     |
| Ga naar kop               | Ctrl+G           |
| Ga naar regel             | Ctrl+L           |
| Opdrachtenpalet           | Ctrl+Shift+P     |
| Overzicht focussen / terug| F6     |
| Markdown-bronweergave     | Ctrl+Shift+M     |
| Gesplitste weergave       | Ctrl+Shift+D     |
| Regel verplaatsen ↑ / ↓ (bron) | Alt+↑ / Alt+↓ |
| Regel dupliceren / verwijderen / samenvoegen (bron) | Ctrl+D / Ctrl+Shift+K / Ctrl+J |
| Overzicht                 | F9               |
| Afleidingsvrij            | F11              |
| Focusmodus                | F12              |
| Zoom + / − / Normaal      | Ctrl++ / Ctrl+− / Ctrl+0 |
| Help                      | F1               |

> **Ctrl+Shift+K** doet twee dingen, afhankelijk van waar je bent: in de visuele
> editor past het een **codeblok** toe; in de codeweergave **verwijdert het de
> regel**. Ze botsen niet, omdat de opmaakacties uitgeschakeld zijn zolang het
> codepaneel de focus heeft.
