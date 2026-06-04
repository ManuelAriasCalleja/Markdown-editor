# Functies

Overzicht van alles wat md-editor biedt. Voor de volledige en technische referentie,
raadpleeg `especificacion.md` in de repository.

## WYSIWYG-bewerking en round-trip

Je bewerkt de gerenderde tekst en bij het opslaan wordt geserialiseerd naar schone
Markdown in UTF-8. Wat je opent is wat je opslaat: tabellen met uitlijning, geneste
lijsten, takenlijsten, citaten, codeblokken en formules blijven getrouw behouden.

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

Zijpaneel (F9) met de inhoudsopgave van koppen; een klik springt naar de sectie.

## TeX-formules

Inline-formules (`$...$`) en blokformules (`$$...$$`) met LaTeX-syntaxis, zonder
externe afhankelijkheden:

- Invoegen met live voorbeeldweergave (Ctrl+Shift+F) en bewerken met dubbelklik.
- Echte superscripten en subscripten, Griekse letters, operatoren, `\frac`, `\sqrt`,
  `\mathbb`…
- Ze zijn atomisch in de editor en overleven de round-trip en de export.
- Beperkingen: `$...$` moet op dezelfde regel openen en sluiten; er is geen 2D-*layout*
  (grote breuken zoals `(a)/(b)`).

## Syntaxismarkering

Codeblokken worden gekleurd naargelang hun taal (families C/C++/Java…, JS/TS/JSON,
Python, shell/YAML/TOML… en een generieke modus).

## Afbeeldingen

Een afbeelding plakken of neerzetten slaat deze op als PNG naast het document en voegt
deze in als `![](ruta)` —het bedt deze niet in—, zodat de Markdown draagbaar blijft.

## Exporteren en afdrukken

PDF, HTML, ODF (.odt) en LaTeX (.tex), plus afdrukken (Ctrl+P). ODF en LaTeX bedden de
taal van het document in (uit de front matter, uit de app-instelling of uit het
systeem).

## Zoom van de hele interface

Ctrl++, Ctrl+- en Ctrl+0 (of Ctrl + wiel) schalen de hele interface, niet alleen de
tekst van de editor. Het niveau wordt onthouden.

## Zoeken en vervangen

Ctrl+F / Ctrl+H, met vorige/volgende, alles vervangen en hoofdlettergevoeligheid.

## Bestanden en de veiligheid van je gegevens

- **Recente bestanden**, openen door slepen en bevestiging van niet-opgeslagen
  wijzigingen.
- **Front matter** YAML/TOML letterlijk behouden.
- **Bewaking van het bestand op schijf**: detecteert externe wijzigingen en biedt aan
  om opnieuw te laden.
- **Automatisch opslaan en herstel** na een abnormale afsluiting.

## Internationalisatie

Interface in 9 talen: Spaans, Engels, Duits, Frans, Italiaans, Portugees, Pools,
Nederlands en Roemeens (Beeld → Taal; wordt bij het herstarten toegepast).
