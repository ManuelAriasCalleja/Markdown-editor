# Benutzerhandbuch

**md-editor** ist ein visueller (WYSIWYG-)Markdown-Editor: Sie schreiben und
formatieren über dem bereits gerenderten Text, ohne den Code zu sehen. Beim
Speichern wird das Dokument wieder in reines Markdown serialisiert.

## Inhalt

- [Öffnen und Speichern](#offnen-und-speichern)
- [Text formatieren](#text-formatieren)
- [Überschriften, Listen und Blöcke](#uberschriften-listen-und-blocke)
- [Links und Bilder](#links-und-bilder)
- [Fußnoten](#fußnoten)
- [Tabellen](#tabellen)
- [Mathematische Formeln](#mathematische-formeln)
- [Suchen und Ersetzen](#suchen-und-ersetzen)
- [Dokumentgliederung](#dokumentgliederung)
- [Ablenkungsfreier Modus](#ablenkungsfreier-modus)
- [Quelltextansicht](#quelltextansicht)
- [Exportieren und Drucken](#exportieren-und-drucken)
- [Designs und Erscheinungsbild](#designs-und-erscheinungsbild)
- [Automatische Wiederherstellung](#automatische-wiederherstellung)
- [Tastenkürzel](#tastenkurzel)

## Öffnen und Speichern

- **Datei → Neu** (Ctrl+N) erstellt ein leeres Dokument.
- **Datei → Öffnen…** (Ctrl+O) öffnet eine vorhandene `.md`. Die Anwendung
  merkt sich die zuletzt geöffneten Dateien unter **Datei → Zuletzt geöffnet**.
- **Speichern** (Ctrl+S) und **Speichern unter…** (Ctrl+Shift+S) schreiben das
  Dokument in UTF-8.
- Ändert sich die Datei außerhalb des Editors, erkennt die Anwendung dies und
  lädt sie neu, sofern Sie keine ungespeicherten Änderungen haben; andernfalls
  fragt sie, was zu tun ist.
- Sie können eine Datei auch per **Ziehen und Ablegen** auf das Fenster
  öffnen.

### Front matter

Beginnt das Dokument mit einem Block `---…---` (YAML) oder `+++…+++` (TOML),
wird er beim Speichern unverändert beibehalten: er wird im Editor nicht
angezeigt und ist nicht bearbeitbar. Er dient für Metadaten wie `title`,
`lang` usw., die beim Exportieren verwendet werden.

## Text formatieren

Markieren Sie einen Abschnitt und wenden Sie die Formatierung über die
Werkzeugleiste oder das Menü **Format** an:

- **Fett** (Ctrl+B), **Kursiv** (Ctrl+I), **Unterstrichen** (Ctrl+U),
  **Durchgestrichen**.
- **Inline-Code** für `nichtproportionale` Abschnitte.
- **Link**: fügt `[Text](url)` über der Auswahl ein.

Die Schaltflächen der Werkzeugleiste spiegeln die aktive Formatierung unter
dem Cursor wider.

## Überschriften, Listen und Blöcke

- **Überschriften** H1–H6 über **Format → Überschrift** oder mit
  Ctrl+1 … Ctrl+6.
- **Listen**: Aufzählungen, nummerierte Listen und Aufgabenlisten (mit
  Kontrollkästchen). Drücken Sie Enter am Ende eines Punktes, wird der nächste
  automatisch erstellt; Enter auf einem leeren Punkt verlässt die Liste. Ein
  **Klick auf das Kontrollkästchen einer Aufgabe** hakt sie ab oder hebt die
  Markierung wieder auf.
- **Zitat** (`>` am Anfang eines Absatzes) und **Codeblock** werden über die
  Werkzeugleiste angewendet; beide werden korrekt nach Markdown
  zurückübersetzt (round-trip).

## Links und Bilder

- **Einfügen → Link…** öffnet einen Dialog mit Text- und URL-Feldern. Wenn Sie
  etwas markiert hatten, wird es als Text verwendet.
- **Ctrl+Klick** auf einen Link öffnet ihn im Systembrowser; beim Überfahren
  mit der Maus wird die URL in der Statusleiste angezeigt.
- **Bilder**: ziehen Sie eine Datei, fügen Sie ein Bild aus der Zwischenablage
  ein oder verwenden Sie **Einfügen → Bild einfügen**. Das Bild wird als PNG
  neben der `.md` gespeichert und als `![alt](relativer-Pfad)` eingefügt; so
  übersteht es den Round-Trip nach Markdown (eingebettete Bilder nicht).

## Fußnoten

- **Einfügen → Fußnote** (Ctrl+Shift+N) fügt eine nummerierte Referenz `[^n]`
  an der Cursorposition ein und legt ihre Definition `[^n]:` am Ende des
  Dokuments an, bereit, damit Sie den Text der Fußnote schreiben.
- Die Referenzen werden als **Hochstellung** angezeigt; beim **Klick** auf eine
  springt der Cursor zu ihrer Definition.
- Sie werden als Standard-Markdown gespeichert (`text[^1]` im Text und unten
  `[^1]: die Fußnote`), sodass sie mit anderen Editoren kompatibel sind.

## Tabellen

- **Tabelle → Tabelle einfügen…** fragt nach Zeilen und Spalten.
- Die Aktionen im Menü **Tabelle** (Zeile oder Spalte hinzufügen/entfernen,
  Spalte ausrichten) sind nur aktiviert, wenn sich der Cursor innerhalb einer
  Tabelle befindet.
- Die Spaltenausrichtung (links/zentriert/rechts) wird beim Speichern als
  `:--`/`:-:`/`--:` beibehalten.

## Mathematische Formeln

md-editor unterstützt **TeX-Formeln** inline (`$...$`) und als Blöcke
(`$$...$$`) mit der üblichen LaTeX-Syntax (Pandoc, Obsidian, Quarto…). Es sind
keine externen Abhängigkeiten nötig.

- **Einfügen → Formel…** (Ctrl+Shift+F) öffnet einen Dialog mit einem
  TeX-Feld und einer **Live-Vorschau**: während Sie tippen, sehen Sie, wie es
  aussehen wird. Wählen Sie *Inline* oder *Block* und bestätigen Sie, um es
  einzufügen.
- Im Editor erscheinen Formeln kursiv in der Akzentfarbe des Designs, mit
  **echten Hoch-/Tiefstellungen** (nicht mit flachen Unicode-Zeichen): `x²`,
  `Hᵢ` usw. — Qt skaliert mit Vertical-Align jedes Zeichen korrekt.
- Ein **Doppelklick** auf eine Formel öffnet den Dialog erneut, mit dem
  ursprünglichen TeX vorausgefüllt: bearbeiten und bestätigen ersetzt sie.
- Formeln sind **atomar**: Tippen innerhalb einer Formel löst einen Hinweis
  aus, zum Bearbeiten doppelzuklicken; Backspace/Delete am Rand entfernt die
  gesamte Gruppe.
- Beim **Exportieren** bleiben Formeln erhalten: LaTeX gibt sie unverändert aus
  (mit `amsmath` und `amssymb` in der Präambel); HTML/PDF/ODF behalten die
  Vertical-Align-Hoch-/Tiefstellungen von Qt im Zielformat.
- In der **Quelltextansicht** sehen Sie sie als `$...$` / `$$...$$`, wobei alle
  TeX-Zeichen (`\sum`, `\frac`, `_`, `*`) beim Speichern intakt bleiben.

Beispiele:

```
Die Energie ist $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Einschränkung: in der Quelle kann sich `$$...$$` über mehrere Zeilen
> erstrecken (Stil Obsidian/Pandoc); `$...$` muss in derselben Zeile geöffnet
> und geschlossen werden.

## Suchen und Ersetzen

- **Suchen** (Ctrl+F) öffnet eine Leiste am unteren Rand mit Feldern zum
  Suchen und Ersetzen sowie Optionen (Groß-/Kleinschreibung, ganzes Wort).
- **Weitersuchen** F3 / **Rückwärts suchen** Shift+F3.

## Dokumentgliederung

Das linke Seitenfenster zeigt das Verzeichnis der Überschriften (TOC): es wird
beim Tippen aktualisiert, und beim Klick auf einen Eintrag springt der Cursor
zu dieser Überschrift. Es wird mit F9 ein- und ausgeblendet.

Sie können einen Eintrag der Gliederung **ziehen**, um diesen Abschnitt — seine
Überschrift, seinen Inhalt und seine Unterabschnitte — innerhalb des Dokuments
**neu anzuordnen**, ohne die Ebene zu ändern. Außerdem gibt **Einfügen →
Inhaltsverzeichnis (TOC)** eine verschachtelte Liste mit den Überschriften in
das Dokument aus.

## Ablenkungsfreier Modus

**Ansicht → Ablenkungsfreier Modus** (F11) wechselt in den Vollbildmodus mit
ausgeblendeten Menüs und Leisten und dem Text zentriert in einer Lesespalte.
Die Gliederung bleibt, sofern sichtbar, am zentralen Block angeheftet. ESC oder
F11 beenden ihn.

## Quelltextansicht

**Ansicht → Markdown-Quelltext** (Ctrl+Shift+M) wechselt zwischen dem visuellen
Editor und einem Vollbild-Klartexteditor, der das rohe Markdown anzeigt.
Änderungen im Quelltextmodus werden in das Dokument übernommen, wenn Sie in den
visuellen Modus zurückkehren.

**Ansicht → Geteilte Ansicht** (Ctrl+Shift+D) zeigt beide nebeneinander: den
visuellen Editor und den Quelltext, synchron gehalten (was Sie in dem einen
tippen, spiegelt sich im anderen wider). Sie schließt sich mit dem
Vollbild-Quelltextmodus gegenseitig aus.

## Exportieren und Drucken

**Datei → Exportieren** bietet **PDF**, **HTML**, **ODF (.odt)** und
**LaTeX (.tex)**. Bei ODF und LaTeX wird die Dokumentsprache eingebettet
(entnommen dem Front matter `lang`/`language`, der Anwendungseinstellung oder,
als letzter Ausweg, dem Systemgebietsschema).

**Datei → Drucken** (Ctrl+P) öffnet den Systemdialog.

## Designs und Erscheinungsbild

- **Ansicht → Design** bietet Hell, Dunkel, GitHub Light, GitHub Dark, Monokai
  und Hoher Kontrast.
- **Ansicht → Warmes Nachtlicht** dämpft die Blautöne im Hintergrund je nach
  Tageszeit.
- **Zoom**: Ctrl+Mausrad, Ctrl++ / Ctrl+- und **Normale Größe** (Ctrl+0)
  skalieren die gesamte Oberfläche (nicht nur den Editortext).
- **Ansicht → Sprache** ändert die Sprache der Oberfläche; sofort wirksam (das Fenster wird neu erstellt).

## Automatische Wiederherstellung

Während Sie bearbeiten, wird der Inhalt alle paar Sekunden in eine
Entwurfskopie automatisch gespeichert. Schließt sich die Anwendung
unerwartet, bietet sie beim nächsten Start an, das Geschriebene
wiederherzustellen.

## Tastenkürzel

| Aktion                    | Tastenkürzel     |
|---------------------------|------------------|
| Neu                       | Ctrl+N           |
| Öffnen                    | Ctrl+O           |
| Speichern                 | Ctrl+S           |
| Speichern unter           | Ctrl+Shift+S     |
| Drucken                   | Ctrl+P           |
| Rückgängig / Wiederholen  | Ctrl+Z / Ctrl+Y  |
| Fett / Kursiv             | Ctrl+B / Ctrl+I  |
| Unterstrichen             | Ctrl+U           |
| Suchen                    | Ctrl+F           |
| Weiter-/Rückwärtssuchen   | F3 / Shift+F3    |
| Überschrift H1 … H6       | Ctrl+1 … Ctrl+6  |
| Formel einfügen           | Ctrl+Shift+F     |
| Fußnote einfügen          | Ctrl+Shift+N     |
| Markdown-Quelltextansicht | Ctrl+Shift+M     |
| Geteilte Ansicht          | Ctrl+Shift+D     |
| Gliederung                | F9               |
| Ablenkungsfreier Modus    | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Hilfe                     | F1               |
