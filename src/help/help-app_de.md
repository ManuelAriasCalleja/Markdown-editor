# Benutzerhandbuch

**md-editor** ist ein visueller (WYSIWYG-)Markdown-Editor: Sie schreiben und
formatieren auf dem bereits gerenderten Text, ohne den Code zu sehen. Beim
Speichern wird das Dokument wieder als reines Markdown serialisiert.

## Inhalt

- [Öffnen und Speichern](#offnen-und-speichern)
- [Text formatieren](#text-formatieren)
- [Überschriften, Listen und Blöcke](#uberschriften-listen-und-blocke)
- [Text umwandeln und Zwischenablage](#text-umwandeln-und-zwischenablage)
- [Links und Bilder](#links-und-bilder)
- [Fußnoten](#fußnoten)
- [Hinweise, Symbole und Textkürzel](#hinweise-symbole-und-textkurzel)
- [Snippets (wiederverwendbare Bausteine)](#snippets-wiederverwendbare-bausteine)
- [Tabellen](#tabellen)
- [Mathematische Formeln](#mathematische-formeln)
- [Diagramme](#diagramme)
- [Rechtschreibprüfung](#rechtschreibprufung)
- [Suchen und Ersetzen](#suchen-und-ersetzen)
- [Dokumentgliederung](#dokumentgliederung)
- [Dokumentstatistik](#dokumentstatistik)
- [Ablenkungsfreier Modus](#ablenkungsfreier-modus)
- [Fokusmodus](#fokusmodus)
- [Quelltextansicht](#quelltextansicht)
- [Exportieren und Drucken](#exportieren-und-drucken)
- [Themes und Aussehen](#themes-und-aussehen)
- [Automatische Wiederherstellung](#automatische-wiederherstellung)
- [Barrierefreiheit](#barrierefreiheit)
- [Tastenkürzel](#tastenkurzel)

## Öffnen und Speichern

- **Datei → Neu** (Strg+N) erstellt ein leeres Dokument in einem neuen Tab.
- **Datei → Neu aus Vorlage** erstellt ein Dokument aus einem Gerüst, das nur noch
  ausgefüllt werden muss. Die Vorlagen sind nach Kategorie gruppiert (Persönlich,
  Programmierung, Lehre, Unternehmen, Schreiben…).
- **Als Vorlage speichern…** speichert das aktuelle Dokument als eigene Vorlage (mit
  Name und Kategorie); sie erscheint im obigen Menü neben den mitgelieferten. **Vorlagen
  verwalten…** (am Ende dieses Menüs) erlaubt das Bearbeiten oder Löschen.
- **Datei → Öffnen…** (Strg+O) öffnet eine vorhandene `.md`. Die Anwendung merkt
  sich die zuletzt geöffneten unter **Datei → Zuletzt geöffnet**.
- **Datei → Importieren** öffnet ein Dokument aus einem anderen Format und wandelt es
  in Markdown um, in einem neuen, unbenannten Tab (das Original bleibt unberührt): **Aus
  HTML…** (eine Webseite), **Aus EPUB…** (ein Buch; die Kapitel werden der Reihe nach
  gelesen) und **Andere Formate (Pandoc)…** (DOCX, ODT, RTF, LaTeX, reStructuredText…,
  falls Pandoc installiert ist). Funktioniert am besten mit einfachem Inhalt; der
  angegebene Zeichensatz wird berücksichtigt.
- **Speichern** (Strg+S) und **Speichern unter…** (Strg+Umschalt+S) schreiben das
  Dokument als UTF-8. **Enthaltenden Ordner öffnen** öffnet den Ordner des
  Dokuments im Dateimanager.
- **Auf gespeicherte Version zurücksetzen** verwirft die ungespeicherten
  Änderungen und lädt die Datei neu von der Festplatte (mit Rückfrage). Nur
  verfügbar, wenn das Dokument eine Datei und ausstehende Änderungen hat.
- Ändert sich die Datei außerhalb des Editors, erkennt die Anwendung das und lädt
  sie neu, wenn keine ungespeicherten Änderungen vorliegen; andernfalls fragt sie
  nach.
- Sie können eine Datei auch per **Ziehen und Ablegen** auf das Fenster öffnen.

### Tabs (mehrere Dokumente)

Sie können mehrere Dokumente gleichzeitig geöffnet haben, jedes in seinem eigenen **Tab**:

- **Neu** (Strg+N), **Neu aus Vorlage** und **Öffnen** (Strg+O) erstellen einen
  Tab (oder verwenden den anfänglich leeren Tab wieder). Eine abgelegte Datei wird
  ebenfalls in einem Tab geöffnet; ist sie bereits offen, wird zu ihrem Tab
  gesprungen.
- Wechseln Sie das Dokument per Klick auf seinen Tab; ziehen Sie Tabs, um sie neu
  anzuordnen. Mit der Tastatur springen **Strg+Bild ab / Strg+Bild auf** (oder
  **Strg+Tab / Strg+Umschalt+Tab**) zum nächsten oder vorherigen Tab.
- **Tab schließen** (Strg+W) schließt den aktuellen und fragt bei ungespeicherten
  Änderungen nach. Der letzte Tab wird nicht geschlossen: er wird zu einem neuen
  Dokument.
- **Geschlossenen Tab wieder öffnen** (Strg+Umschalt+R) öffnet den zuletzt
  geschlossenen Tab wieder (nur solche mit Datei auf der Festplatte).
- Die Beschriftung zeigt den Dateinamen und einen Punkt (•) bei ungespeicherten
  Änderungen.
- Beim Schließen der Anwendung werden die offenen Dokumente gemerkt und beim
  nächsten Start alle wieder geöffnet.

### *Front Matter*

Beginnt das Dokument mit einem Block `---…---` (YAML) oder `+++…+++` (TOML), wird
er beim Speichern unverändert beibehalten: Er wird im Editor nicht angezeigt und
nicht bearbeitet. Er dient für Metadaten wie `title`, `lang` usw., die beim
Exportieren verwendet werden.

## Text formatieren

Markieren Sie einen Abschnitt und wenden Sie die Formatierung über die
Symbolleiste oder das Menü **Format** an:

- **Fett** (Strg+B), **Kursiv** (Strg+I), **Unterstrichen** (Strg+U),
  **Durchgestrichen**.
- **Inline-Code** für `monospace`-Abschnitte.
- **Link**: fügt `[Text](url)` über der Auswahl ein.
- **Hervorheben** (Strg+Umschalt+H): umschließt die Auswahl mit `==Markierung==`; der
  Text erscheint mit Hervorhebungshintergrund. Da `==` keine Standard-Markdown-Syntax
  ist, wird es als literaler Text gespeichert.
- **Hochgestellt** (Strg+Umschalt++) und **Tiefgestellt** (Strg+Umschalt+-): heben
  oder senken den ausgewählten Text; werden als `^Text^` und `~Text~` (Pandoc-Stil)
  gespeichert.

Die Schaltflächen der Symbolleiste zeigen die aktive Formatierung unter dem
Cursor an.

**Automatisches Paaren.** Tippen Sie `(`, `[`, `{` oder einen Backtick, wird das
Paar automatisch geschlossen und der Cursor steht in der Mitte; ist Text
markiert, wird er umschlossen. Tippen Sie das schließende Zeichen direkt vor
seinem Gegenstück, „überspringt“ der Editor es, statt es zu verdoppeln.

**Eingaberegeln.** Am Zeilenanfang verwandelt ein Markdown-Blockmarker gefolgt von
einem Leerzeichen die Zeile an Ort und Stelle (ohne den Marker): `#` … `######` +
Leerzeichen → Überschrift H1…H6; `>` → Zitat; `-`, `*` oder `+` → Aufzählung;
`1.` (oder `1)`) → nummerierte Liste. Ergibt dasselbe Format wie die Leiste.

## Überschriften, Listen und Blöcke

- **Überschriften** H1–H6 über **Format → Überschrift** oder mit Strg+1 … Strg+6.
  **Höher-/Tieferstufen** der Überschrift am Cursor um eine Ebene mit
  Strg+Umschalt+[ / Strg+Umschalt+].
- **Listen**: Aufzählungen, nummerierte und Aufgabenlisten (mit Kontrollkästchen).
  Enter am Ende eines Punktes erstellt automatisch den nächsten; Enter auf einem
  leeren Punkt verlässt die Liste. Ein **Klick auf das Kontrollkästchen** einer
  Aufgabe schaltet sie um.
- **Zitat** (`>` am Anfang eines Absatzes) und **Codeblock** werden über die
  Symbolleiste angewendet; beide werden korrekt nach Markdown zurückgewandelt.
  Mit **Format → Sprache des Blocks…** wählen Sie die Sprache eines Codeblocks
  (mit dem Cursor darin), damit dessen Syntax hervorgehoben wird.
- Wenn du mit dem Mauszeiger über einen Codeblock fährst, erscheinen oben rechts seine **Sprache** (zum Ändern anklicken) und eine Schaltfläche zum **Kopieren**.
- **Einrückung**: **Format → Einzug vergrößern/verkleinern** verschachtelt Listen
  und Zitate.

## Text umwandeln und Zwischenablage

- **Bearbeiten → Text umwandeln** wirkt auf die Auswahl: **GROSSBUCHSTABEN**,
  **kleinbuchstaben**, **Großschreibung** und **Zeilen sortieren**.
- **Intelligente Typografie** (im selben Menü) wandelt in der Auswahl die
  Bindestriche `--`/`---` in `–`/`—`, `...` in `…` und gerade Anführungszeichen
  je nach Kontext in typografische um.
- **Als Klartext einfügen** (Strg+Umschalt+V) fügt ohne Formatierung ein. **Als
  Markdown einfügen** (Strg+Alt+V) wandelt den Rich-Inhalt der Zwischenablage
  (HTML) in Markdown um, statt die Formatierung der Quelle einzubetten.
- **Als HTML kopieren** kopiert die Auswahl (oder das Dokument) als HTML, zum
  Einfügen in E-Mail, ein CMS usw.
- **Als Markdown kopieren** kopiert die Auswahl (oder das gesamte Dokument) als
  Markdown-Text, zum Einfügen in einen anderen Markdown-Editor oder ein Codefeld.
- Wenn Sie eine **URL** über eine Textauswahl einfügen, wird der Text automatisch
  verlinkt.
- **Bearbeiten → Markdown bereinigen** normalisiert das gesamte Dokument in einem
  Durchgang: Es vereinheitlicht die Aufzählungszeichen zu -, entfernt die
  Leerzeichen am Zeilenende, fasst überzählige Leerzeilen zusammen und passt den
  Abstand nach den # der Überschriften an. Es ist behutsam: Das Innere von
  Codeblöcken wird nicht angetastet.

## Links und Bilder

- **Einfügen → Link…** öffnet einen Dialog mit Text und URL. Eine vorhandene
  Auswahl wird als Text übernommen.
- **Strg+Klick** auf einen Link öffnet ihn im Systembrowser; beim Überfahren mit
  der Maus wird die URL in einem Hinweis neben dem Cursor und in der Statusleiste angezeigt.
- **Bilder**: Ziehen Sie eine Datei, fügen Sie ein Bild aus der Zwischenablage
  ein oder verwenden Sie **Einfügen → Bild einfügen**. Das Bild wird als PNG neben
  der `.md` gespeichert und als `![alt](relativer-pfad)` eingefügt; so übersteht
  es den Round-Trip nach Markdown (eingebettete Bilder nicht).

## Fußnoten

- **Einfügen → Fußnote** (Strg+Umschalt+N) fügt am Cursor eine nummerierte
  Referenz `[^n]` ein und erstellt deren Definition `[^n]:` am Ende des Dokuments,
  bereit für den Notentext.
- Referenzen werden **hochgestellt** angezeigt; ein **Klick** darauf springt mit
  dem Cursor zur Definition.
- Sie werden als Standard-Markdown gespeichert (`Text[^1]` im Text und unten
  `[^1]: die Notiz`), sodass sie mit anderen Editoren kompatibel sind.

## Hinweise, Symbole und Textkürzel

- **Einfügen → Hinweis** erstellt ein Callout im GitHub-Stil: ein Zitat, dessen
  erste Zeile `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` oder `[!CAUTION]`
  ist. Es wird mit getöntem Hintergrund und farbigem Titel angezeigt und als
  GitHub-kompatibles Markdown gespeichert.
- **Einfügen → Sonderzeichen…** öffnet eine Zeichentabelle nach Kategorien
  (mathematisch, griechisch, Pfeile, Währung, Interpunktion…); ein Klick fügt das
  Zeichen ein und der Dialog bleibt zum Einfügen mehrerer offen.
- **`:name:`-Kürzel**: Beim Tippen eines Codes wie `:alpha:` oder `:euro:` wird er
  zum entsprechenden Symbol erweitert (α, €…).
- **Einfügen → Datum** und **Datum und Uhrzeit** fügen das aktuelle Datum (und die
  Uhrzeit) im lokalisierten Format ein.

## Snippets (wiederverwendbare Bausteine)

Ein **Snippet** ist ein Stück Markdown, das Sie unter einem Namen speichern, um
es später mit ein paar Klicks einzufügen: eine Signatur, eine Tabellenvorlage,
ein Hinweis, den Sie oft wiederholen…

- **Einfügen → Snippet** klappt die Liste der vorhandenen auf; wählen Sie eines,
  wird sein Inhalt an der Cursorposition eingefügt (funktioniert auch in der
  Quelltextansicht).
- **Einfügen → Snippet → Snippets verwalten…** öffnet einen Dialog zum Erstellen,
  Bearbeiten und Löschen Ihrer Snippets. Jedes hat einen **Namen** (den Sie im
  Menü sehen) und einen **Inhalt** in Markdown.
- Sie werden in den Anwendungseinstellungen gespeichert, sind also in allen Ihren
  Dokumenten verfügbar, nicht nur im aktuellen.

## Tabellen

- Wenn der Cursor **in einer Tabelle** steht, erscheint darüber eine **schwebende Leiste** mit Schaltflächen zum Einfügen/Löschen von Zeilen und Spalten und zum Ausrichten der Spalte.
- **Tab** springt zur nächsten Zelle und **Umschalt+Tab** zur vorherigen; **Tab** in der letzten Zelle fügt eine Zeile hinzu.
- **Tabelle → Tabelle einfügen…** fragt nach Zeilen und Spalten.
- **Einfügen → Tabelle aus Zwischenablage** wandelt aus einer Tabellenkalkulation
  oder einer CSV-Datei kopierte TSV/CSV-Daten (durch Tabulatoren oder Kommas
  getrennte Spalten) in eine Tabelle um.
- Die Aktionen des Menüs **Tabelle** (Zeile/Spalte hinzufügen/entfernen, Spalte
  ausrichten) sind nur aktiv, wenn der Cursor in einer Tabelle steht.
- Die Spaltenausrichtung (links/zentriert/rechts) bleibt beim Speichern als
  `:--`/`:-:`/`--:` erhalten.
- **Tabelle → Zeilen nach Spalte sortieren** (aufsteigend/absteigend) ordnet die
  Zeilen nach der Spalte des Cursors und lässt die Kopfzeile fest; erkennt, ob die
  Spalte numerisch oder Text ist.

## Mathematische Formeln

md-editor unterstützt **TeX-Formeln** inline (`$...$`) und als Block (`$$...$$`)
mit der üblichen LaTeX-Syntax (Pandoc, Obsidian, Quarto…). Es ist keine externe
Abhängigkeit nötig.

- **Einfügen → Formel…** (Strg+Umschalt+F) öffnet einen Dialog mit einem Feld für
  das TeX und einer **Live-Vorschau**: Während Sie tippen, sehen Sie das Ergebnis.
  Wählen Sie *Inline* oder *Block* und bestätigen Sie zum Einfügen.
- Formeln werden in **echtem 2D** gesetzt: Brüche (`\frac`) werden mit Strich
  gestapelt, große Operatoren (`\sum`, `\int`, `\prod`…) zeigen ihre Grenzen
  oben und unten, Wurzeln (`\sqrt`) tragen ihren Strich, und es gibt Matrizen
  (`\begin{pmatrix}`…), Binomialkoeffizienten (`\binom`) und Akzente (`\hat`,
  `\vec`, `\bar`…). Einfachere (Potenzen, Indizes, Griechisch) werden inline
  gesetzt. Die Darstellung skaliert mit dem Zoom.
- **Doppelklick** auf eine Formel öffnet den Dialog mit dem ursprünglichen TeX
  erneut: Sie bearbeiten es und beim Bestätigen wird es ersetzt.
- Formeln sind **atomar**: Tippen Sie hinein, erinnert die App an den Doppelklick;
  Rücktaste/Entf am Rand löschen die ganze Gruppe.
- Beim **Exportieren** bleiben sie erhalten: nach LaTeX werden sie unverändert
  ausgegeben (mit `amsmath` und `amssymb` in der Präambel); nach HTML/PDF/ODF
  werden sie auf ihre Inline-Näherung zurückgeführt.
- In der **Quelltextansicht** erscheinen sie als `$...$` / `$$...$$`, mit allen
  TeX-Zeichen (`\sum`, `\frac`, `_`, `*`) beim Speichern unversehrt.

Beispiele:

```
Die Energie ist $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Im Quelltext darf `$$...$$` mehrere Zeilen umfassen (Obsidian/Pandoc-Stil);
> `$...$` muss in derselben Zeile öffnen und schließen.

## Diagramme

Ein Codeblock mit der Sprache `mermaid` oder `plantuml` wird direkt unter dem
Block **als Bild vorschau**, ohne den Code (der bearbeitbar bleibt) oder das
gespeicherte Markdown zu verändern.

- Es ist das entsprechende Werkzeug erforderlich: **`plantuml`** (mit Java) für
  PlantUML oder **`mmdc`** (mermaid-cli, mit Node) für Mermaid.
- Fehlt das Werkzeug, erscheint unter dem Block ein Hinweis mit dem
  Installationsbefehl für Ihr Betriebssystem; der Block bleibt als Code.
- Das Bild dient nur der Darstellung: Es wird nicht ins Markdown geschrieben und
  zählt nicht als ungespeicherte Änderung.

Zum Beispiel wird ein als `mermaid` markierter Codeblock mit `flowchart LR  A
--> B --> C` als das entsprechende Flussdiagramm vorgeschaut.

## Rechtschreibprüfung

- Unterstreicht falsch geschriebene Wörter rot gemäß der **Dokumentsprache** (aus
  dem `lang`-Front-Matter, der Spracheinstellung oder dem System). Code, Formeln
  und Links werden nicht geprüft.
- **Rechtsklick** auf ein unterstrichenes Wort bietet **Vorschläge** (ein Klick
  ersetzt es), **Zum Wörterbuch hinzufügen** (eine dauerhafte persönliche Liste)
  und **Ignorieren** (für die Sitzung).
- Sie wird unter **Ansicht → Rechtschreibprüfung** ein-/ausgeschaltet, und die
  Sprache wird unter **Ansicht → Sprache der Rechtschreibprüfung** festgelegt
  (oder automatisch gelassen).
- Sie benötigt Hunspell-Wörterbücher: unter Linux die des Systems (`hunspell-es`,
  `hunspell-en-us`…); unter Windows/macOS sind sie in der Anwendung enthalten.

## Suchen und Ersetzen

- **Suchen** (Strg+F) öffnet unten eine Leiste mit Feldern zum Suchen und Ersetzen
  sowie Optionen (Groß-/Kleinschreibung, ganzes Wort).
- **Weitersuchen** F3 / **Rückwärts suchen** Umschalt+F3.
- Die Leiste hebt **alle** Treffer im Dokument hervor und zeigt einen Zähler **„N von M“** (auf welchem Treffer von wie vielen). **Alle ersetzen** ersetzt alle auf einmal.

## Dokumentgliederung

Das linke Seitenpanel zeigt die Überschriftengliederung (Inhaltsverzeichnis): Sie
aktualisiert sich beim Tippen, und beim Klick auf einen Eintrag springt der Cursor
zu dieser Überschrift. Ein-/ausblenden mit F9. Mit **F6** verschieben
Sie den Tastaturfokus auf die Gliederung (sie wird eingeblendet, falls verborgen);
dort bewegen die Pfeiltasten durch die Überschriften und **Enter** springt zur
ausgewählten und gibt den Fokus an den Editor zurück. Ein erneutes **F6**
gibt den Fokus einfach an den Editor zurück.

Das **Filterfeld** oben im Panel zeigt nur die Überschriften, die zu deiner
Eingabe passen (und ihre übergeordneten); die Schaltflächen **⊞/⊟** klappen alles
aus oder ein. Das von dir gewählte Ein-/Ausklappen **bleibt erhalten**, auch
während du weiter schreibst.

Sie können einen Gliederungseintrag **ziehen**, um diesen Abschnitt —seine
Überschrift, seinen Inhalt und seine Unterabschnitte— im Dokument **umzuordnen**,
ohne die Ebene zu ändern. Außerdem schreibt **Einfügen → Inhaltsverzeichnis (TOC)**
eine verschachtelte Liste der Überschriften ins Dokument. **Ansicht → Zu
Überschrift gehen…** (Strg+G) springt zu einer Überschrift durch Eingabe eines
Teils ihres Textes, und **Zu Zeile springen…** (Strg+L) bewegt den Cursor zu
einer Zeilennummer (in der Quelltextansicht zur Zeile des Markdowns). Die
**Befehlspalette** (Strg+Umschalt+P) findet und führt jede Menüaktion aus, indem
Sie einen Teil ihres Namens eingeben.

## Dokumentstatistik

- **Ansicht → Dokumentstatistik…** zeigt Wörter, Zeichen, Absätze, Sätze und die
  geschätzte Lesezeit (des Dokuments oder der Auswahl).
- **Ansicht → Wortzähler anzeigen** aktiviert einen dauerhaften Zähler in der
  Statusleiste.
- **Ansicht → Zeile und Spalte anzeigen** zeigt die Cursorposition (Zeile und
  Spalte) in der Statusleiste an.

## Ablenkungsfreier Modus

**Ansicht → Ablenkungsfrei** (F11) wechselt in den Vollbildmodus mit
ausgeblendetem Menü und Leisten und dem Text zentriert in einer Lesespalte. Die
Gliederung bleibt, falls sichtbar, am zentralen Block. ESC oder F11 beenden.

## Fokusmodus

**Ansicht → Fokusmodus** (F12) hilft Ihnen, sich auf das Geschriebene zu
konzentrieren, ohne das normale Fenster zu verlassen. Ein einziger Schalter
aktiviert zwei Dinge zugleich:

- **Schreibmaschine**: Die Cursorzeile bleibt vertikal zentriert. Während Sie
  schreiben, verschiebt sich der Text, sodass die aktive Zeile auf halber Höhe
  bleibt, statt an den unteren Rand zu rücken.
- **Abdunkeln**: Das gesamte Dokument erscheint gedämpft, außer dem Absatz, in
  dem der Cursor steht, der klar hervorgehoben wird.

Es funktioniert im visuellen Editor und in der Quelltextansicht und ist
**unabhängig** vom ablenkungsfreien Modus (F11): Sie können beide gleichzeitig
oder jeden für sich verwenden.

## Quelltextansicht

**Ansicht → Markdown-Quelltext** (Strg+Umschalt+M) wechselt zwischen dem visuellen
Editor und einem Klartext-Editor im Vollbild mit dem rohen Markdown. Änderungen im
Quelltextmodus werden beim Zurückwechseln in den visuellen Modus übernommen.

**Ansicht → Geteilte Ansicht** (Strg+Umschalt+D) zeigt beide gleichzeitig
nebeneinander: den visuellen Editor und den Quelltext, synchronisiert (was Sie in
einem tippen, erscheint im anderen). Sie schließt sich mit dem Vollbild-Quelltext
gegenseitig aus.

In der Quelltextansicht gibt es **Zeilenbefehle** per Tastatur für die Zeile des
Cursors: **Alt+↑ / Alt+↓** verschieben die Zeile nach oben/unten, **Strg+D**
dupliziert sie, **Strg+Umschalt+K** löscht sie und **Strg+J** verbindet sie mit der
nächsten.

## Exportieren und Drucken

**Datei → Exportieren** bietet **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** und **Klartext (.txt)**. In ODF, DOCX, LaTeX und
EPUB wird die Dokumentsprache eingebettet (aus dem `lang`/`language`-Front-Matter,
der Anwendungseinstellung oder zuletzt der Systemsprache). Im PDF werden der Titel
und der Autor eingebettet, wenn sie im front matter stehen (`title`, `author`).

Sie können auch **nur die Auswahl als PDF** exportieren und die
**Druckvorschau** verwenden.

**Datei → Drucken** (Strg+P) öffnet den Systemdialog; **Auswahl drucken** druckt
nur das Markierte.

**Ansicht → Seitenzahlen beim Drucken** (standardmäßig aktiv) fügt die Seitenzahl in
der Fußzeile (`N / M`) beim Drucken und beim PDF-Export hinzu.

## Themes und Aussehen

- **Ansicht → Theme** bietet Hell, Dunkel, GitHub Light, GitHub Dark, Monokai, Hoher Kontrast, Solarized Light und Solarized Dark. **Dem System folgen** passt das helle/dunkle Theme an das des
  Betriebssystems an.
- **Ansicht → Nächtliches warmes Licht** dimmt die Blautöne des Hintergrunds je
  nach Uhrzeit.
- **Ansicht → Zeilenabstand** stellt den Zeilenabstand des Editors ein: Einfach, 1,5 Zeilen oder Doppelt.
- **Ansicht → Aktuelle Zeile hervorheben** hebt die Zeile des Cursors mit einem dezenten Hintergrund hervor.
- **Zoom**: Strg+Mausrad, Strg++ / Strg+- und **Normale Größe** (Strg+0)
  skalieren die gesamte Oberfläche (nicht nur den Editortext).
- **Ansicht → Sprache** ändert die Oberflächensprache; sie wird sofort angewendet
  (das Fenster wird neu erstellt).

## Automatische Wiederherstellung

Während Sie bearbeiten, wird der Inhalt alle paar Sekunden in einer Entwurfskopie
automatisch gespeichert. Schließt die Anwendung unerwartet, bietet sie beim erneuten
Öffnen an, das Geschriebene wiederherzustellen.

## Barrierefreiheit

- **Screenreader**: Der Editor, die Gliederung, die Suchfelder und die übrigen Bedienelemente haben zugängliche Namen; außerdem werden Statusmeldungen (gespeichert, „nicht gefunden“, Änderungen auf der Festplatte …) vorgelesen.
- **Nur Tastatur**: Jede Aktion hat ein Tastenkürzel oder einen Menüeintrag (F10 oder Alt öffnet die Menüleiste). Siehe die Tabelle [Tastenkürzel](#tastenkurzel).
- **Kontrast und Größe**: Das Thema **Hoher Kontrast** und der **Zoom** der gesamten Oberfläche helfen bei Sehschwäche; die anfängliche Schriftgröße ist die des Systems.
- **Fokus**: Das fokussierte Element wird mit der Auswahlfarbe des Themas hervorgehoben.

## Tastenkürzel

| Aktion                    | Kürzel           |
|---------------------------|------------------|
| Neu                       | Strg+N           |
| Tab schließen             | Strg+W           |
| Geschlossenen Tab wieder öffnen | Strg+Umschalt+R |
| Nächster / vorheriger Tab | Strg+Bild ab / Strg+Bild auf (oder Strg+Tab / Strg+Umschalt+Tab) |
| Öffnen                    | Strg+O           |
| Speichern                 | Strg+S           |
| Speichern unter           | Strg+Umschalt+S  |
| Drucken                   | Strg+P           |
| Rückgängig / Wiederholen  | Strg+Z / Strg+Y  |
| Fett / Kursiv             | Strg+B / Strg+I  |
| Hervorheben (==Markierung==) | Strg+Umschalt+H |
| Hochgestellt / Tiefgestellt | Strg+Umschalt++ / Strg+Umschalt+- |
| Unterstrichen             | Strg+U           |
| Als Klartext einfügen     | Strg+Umschalt+V  |
| Als Markdown einfügen     | Strg+Alt+V       |
| Suchen                    | Strg+F           |
| Weiter-/Rückwärtssuchen   | F3 / Umschalt+F3 |
| Überschrift H1 … H6       | Strg+1 … Strg+6  |
| Überschrift höher-/tieferstufen | Strg+Umschalt+[ / Strg+Umschalt+] |
| Formel einfügen           | Strg+Umschalt+F  |
| Fußnote einfügen          | Strg+Umschalt+N  |
| Zu Überschrift gehen      | Strg+G           |
| Zu Zeile springen         | Strg+L           |
| Befehlspalette            | Strg+Umschalt+P  |
| Fokus auf Gliederung / zurück zum Editor | F6  |
| Markdown-Quelltextansicht | Strg+Umschalt+M  |
| Geteilte Ansicht          | Strg+Umschalt+D  |
| Zeile verschieben ↑ / ↓ (Quelltext) | Alt+↑ / Alt+↓ |
| Zeile duplizieren / löschen / verbinden (Quelltext) | Strg+D / Strg+Umschalt+K / Strg+J |
| Gliederung                | F9               |
| Ablenkungsfrei            | F11              |
| Fokusmodus                | F12              |
| Zoom + / − / Normal       | Strg++ / Strg+− / Strg+0 |
| Hilfe                     | F1               |
