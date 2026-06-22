# Functies

Overzicht van alles wat md-editor biedt. Voor de volledige en technische referentie,
raadpleeg `especificacion.md` in de repository.

## WYSIWYG-bewerking en round-trip

Je bewerkt de gerenderde tekst en bij het opslaan wordt geserialiseerd naar schone
Markdown in UTF-8. Wat je opent is wat je opslaat: tabellen met uitlijning, geneste
lijsten, takenlijsten, citaten, codeblokken, voetnoten, admonities en formules blijven
getrouw behouden.

## Bewerking met tabbladen

Open meerdere documenten tegelijk, elk in zijn eigen tabblad, en wissel ertussen.
Tabblad sluiten met Ctrl+W. De sessie heropent de tabbladen bij het opnieuw opstarten.

## Weergavemodi

- WYSIWYG, Markdown-broncode (Ctrl+Shift+M) en Gesplitste weergave (Ctrl+Shift+D).
- In gesplitste weergave synchroniseren weergave en code zich: alleen het paneel dat
  je niet bewerkt wordt bijgewerkt, zonder cursorsprongen.

## Afleidingsvrije modus

F11 gaat naar het volledige scherm met de tekst gecentreerd in een leeskolom en zonder
werkbalken. ESC of F11 sluiten af.

## Thema's en warm nachtlicht

- **Zes thema's**: Licht, Donker, GitHub Light, GitHub Dark, Monokai en Hoog contrast.
- **Warm nachtlicht** (standaard ingeschakeld): dempt het blauw van de achtergrond
  automatisch en geleidelijk naargelang het tijdstip, om vermoeide ogen 's nachts te
  verminderen. Neutraal overdag (07–19 u), wordt 's avonds warmer (19–23 u), maximaal
  's nachts (23–06 u) en koelt af bij het ochtendgloren (06–07 u). Het wordt elke
  minuut vanzelf opnieuw geëvalueerd en beïnvloedt alleen de achtergrond (niet de
  koppelingen noch de markering).

## Documentoverzicht

Zijpaneel (F9) met de inhoudsopgave van koppen; een klik springt naar de sectie. «Ga
naar kop» (Ctrl+G) opent een snelzoeker van koppen.

## TeX-formules

Inline-formules (`$...$`) en blokformules (`$$...$$`) met LaTeX-syntaxis, zonder
externe afhankelijkheden:

- Invoegen met live voorbeeldweergave (Ctrl+Shift+F) en bewerken met dubbelklik.
- **Echte 2D-layout**: gestapelde breuken (`\frac`), wortels met overkapping
  (`\sqrt`), binomiaalcoëfficiënten (`\binom`), matrices en omgevingen (`matrix`,
  `pmatrix`, `cases`…), grote operatoren met grenzen boven en onder (`\sum`, `\int`,
  `\prod`…), accenten (`\hat`, `\vec`…), echte superscripten en subscripten, Griekse
  letters en `\mathbb`.
- Ze zijn atomisch in de editor, schalen mee met de zoom en overleven de round-trip en
  de export. Blokken `$$...$$` mogen meerdere regels beslaan.
- Beperkingen: `$...$` moet op dezelfde regel openen en sluiten; inline 2D-formules
  staan wat hoog (blokformules zien er goed uit).

## Spellingcontrole (optioneel)

Onderstreept verkeerd gespelde woorden naargelang de taal van het document (Beeld →
Spellingcontrole). De taal wordt vanzelf gekozen (front matter, instelling of systeem)
of handmatig (Beeld → Controletaal). Rechtsklik biedt suggesties en toevoegen aan het
persoonlijke woordenboek. Vereist Hunspell; zonder dit werkt de rest gewoon door.

## Diagrammen (optioneel)

De blokken ```` ```mermaid ```` en ```` ```plantuml ```` worden gerenderd als
afbeelding onder het blok, door het externe hulpprogramma (`mmdc` / `plantuml`) uit te
voeren als het is geïnstalleerd. Ontbreekt het, dan wordt de installatieopdracht voor
je systeem getoond. De afbeelding wordt niet in de Markdown opgeslagen.

## Syntaxismarkering

Codeblokken worden gekleurd naargelang hun taal (families C/C++/Java…, JS/TS/JSON,
Python, shell/YAML/TOML… en een generieke modus).

## Afbeeldingen

Een afbeelding plakken of neerzetten slaat deze op als PNG naast het document en voegt
deze in als `![](ruta)` —het bedt deze niet in—, zodat de Markdown draagbaar blijft.

## Invoegen en transformeren

- Invoegen: koppeling, afbeelding, tabel, lijn, inhoudsopgave (TOC), formule, voetnoot,
  admonitie (notitie/waarschuwing…), speciale symbolen en datum/tijd.
- Plakken als Markdown (Ctrl+Alt+V) zet de HTML van het klembord om naar Markdown.
- Tekst transformeren: HOOFDLETTERS/kleine letters, beginhoofdletters, regels sorteren
  en slimme typografie (—, –, …, typografische aanhalingstekens).
- Documentstatistieken: woorden, tekens, alinea's, zinnen en leestijd.

## Exporteren en afdrukken

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) en EPUB (.epub), plus
afdrukvoorbeeld en afdrukken (Ctrl+P). ODF, DOCX en LaTeX bedden de taal van het
document in (uit de front matter, uit de app-instelling of uit het systeem).

## Zoom van de hele interface

Ctrl++, Ctrl+- en Ctrl+0 (of Ctrl + wiel) schalen de hele interface, niet alleen de
tekst van de editor. Het niveau wordt onthouden.

## Zoeken en vervangen

Ctrl+F / Ctrl+H, met vorige/volgende, alles vervangen en hoofdlettergevoeligheid.

## Bestanden en de veiligheid van je gegevens

- **Recente bestanden**, openen door slepen en bevestiging van niet-opgeslagen
  wijzigingen.
- **Documentsjablonen** (Bestand → Nieuw vanuit sjabloon).
- **Front matter** YAML/TOML letterlijk behouden.
- **Bewaking van het bestand op schijf**: detecteert externe wijzigingen en biedt aan
  om opnieuw te laden.
- **Automatisch opslaan en herstel** na een abnormale afsluiting.

## Internationalisatie

Interface in 9 talen: Spaans, Engels, Duits, Frans, Italiaans, Portugees, Pools,
Nederlands en Roemeens (Beeld → Taal; wordt bij het herstarten toegepast).
