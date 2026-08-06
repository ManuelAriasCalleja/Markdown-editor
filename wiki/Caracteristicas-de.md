# Funktionen

Überblick über alles, was md-editor bietet. Die vollständige technische Referenz findest
du in `docs/REQUISITOS.md` im Repository.

## WYSIWYG-Bearbeitung und Round-Trip

Du bearbeitest den gerenderten Text und beim Speichern wird sauberes Markdown in UTF-8
serialisiert. Was du öffnest, speicherst du auch: Tabellen mit Ausrichtung, verschachtelte
Listen, Aufgabenlisten, Zitate, Codeblöcke, Fußnoten, Admonitions und Formeln bleiben
originalgetreu erhalten.

## Bearbeitung mit Tabs

Öffne mehrere Dokumente gleichzeitig, jedes in seinem eigenen Tab, und wechsle zwischen
ihnen. Tab schließen mit Ctrl+W. Die Sitzung öffnet die Tabs beim erneuten Start wieder.

## Ansichtsmodi

- WYSIWYG, Markdown-Quelltext (Ctrl+Shift+M) und Geteilte Ansicht (Ctrl+Shift+D).
- In der geteilten Ansicht werden gerenderte Darstellung und Code synchronisiert: Es wird
  nur der Bereich aktualisiert, den du gerade nicht bearbeitest, ohne Cursorsprünge.

## Ablenkungsfreier Modus

F11 wechselt in den Vollbildmodus mit dem Text zentriert in einer Lesespalte und ohne
Leisten. ESC oder F11 verlassen ihn.

## Designs und warmes Nachtlicht

- **Acht Designs**: Hell, Dunkel, GitHub Light, GitHub Dark, Monokai, Hoher Kontrast,
  Solarized Light und Solarized Dark.
- **Warmes Nachtlicht** (standardmäßig aktiviert): dämpft das Blau des Hintergrunds
  automatisch und schrittweise je nach Uhrzeit, um die Ermüdung der Augen in der Nacht zu
  verringern. Neutral tagsüber (07–19 Uhr), wird abends wärmer (19–23 Uhr), maximal nachts
  (23–06 Uhr) und kühlt sich bei Tagesanbruch ab (06–07 Uhr). Es wird jede Minute von
  selbst neu bewertet und betrifft nur den Hintergrund (nicht Links oder Hervorhebung).

## Dokumentgliederung

Seitenleiste (F9) mit dem Verzeichnis der Überschriften; ein Klick springt zum Abschnitt.
„Zu Überschrift springen“ (Ctrl+G) öffnet eine schnelle Überschriftensuche.

## TeX-Formeln

Inline-Formeln (`$...$`) und Block-Formeln (`$$...$$`) mit LaTeX-Syntax, ohne externe
Abhängigkeiten:

- Einfügen mit Live-Vorschau (Ctrl+Shift+F) und Bearbeiten per Doppelklick.
- **Echtes 2D-Layout**: gestapelte Brüche (`\frac`), Wurzeln mit Wurzelstrich
  (`\sqrt`), Binomialkoeffizienten (`\binom`), Matrizen und Umgebungen (`matrix`,
  `pmatrix`, `cases`…), große Operatoren mit Grenzen darüber und darunter (`\sum`,
  `\int`, `\prod`…), Akzente (`\hat`, `\vec`…), echte Hoch- und Tiefstellungen,
  griechische Buchstaben und `\mathbb`.
- Sie sind im Editor atomar, skalieren mit dem Zoom und überstehen den Round-Trip und den
  Export. Die Blöcke `$$...$$` können sich über mehrere Zeilen erstrecken.
- Einschränkungen: `$...$` muss in derselben Zeile öffnen und schließen; Inline-2D-Formeln
  stehen etwas hoch (Block-Formeln werden gut dargestellt).

## Rechtschreibprüfung (optional)

Unterstreicht falsch geschriebene Wörter je nach Dokumentsprache (Ansicht →
Rechtschreibprüfung). Die Sprache wird automatisch gewählt (Front matter, Einstellung oder
System) oder von Hand (Ansicht → Sprache der Rechtschreibprüfung). Ein Rechtsklick bietet
Vorschläge und das Hinzufügen zum persönlichen Wörterbuch. Erfordert Hunspell; ohne es
funktioniert der Rest genauso.

## Diagramme (optional)

Die Blöcke ```` ```mermaid ```` und ```` ```plantuml ```` werden als Bild unter dem Block
gerendert, indem das externe Werkzeug (`mmdc` / `plantuml`) ausgeführt wird, falls es
installiert ist. Fehlt es, wird der Installationsbefehl für dein System angezeigt. Das Bild
wird nicht im Markdown gespeichert.

## Syntaxhervorhebung

Codeblöcke werden je nach Sprache eingefärbt (Familien C/C++/Java…, JS/TS/JSON, Python,
Shell/YAML/TOML… und ein generischer Modus).

## Bilder

Ein Bild einzufügen oder abzulegen speichert es als PNG neben dem Dokument und fügt es als
`![](ruta)` ein – es wird nicht eingebettet –, sodass das Markdown weiterhin portabel
bleibt.

## Einfügen und Umwandeln

- Einfügen: Link, Bild, Tabelle, Linie, Inhaltsverzeichnis (TOC), Formel, Fußnote,
  Admonition (Notiz/Warnung…), Sonderzeichen und Datum/Uhrzeit.
- Als Markdown einfügen (Ctrl+Alt+V) wandelt das HTML aus der Zwischenablage in Markdown um.
- Text umwandeln: GROSSBUCHSTABEN/Kleinbuchstaben, Großschreibung, Zeilen sortieren und
  intelligente Typografie (—, –, …, typografische Anführungszeichen).
- Dokumentstatistik: Wörter, Zeichen, Absätze, Sätze und Lesezeit.

## Exportieren und Drucken

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) und EPUB (.epub), dazu Druckvorschau und
Drucken (Ctrl+P). ODF, DOCX und LaTeX betten die Dokumentsprache ein (aus dem Front matter,
der App-Einstellung oder vom System).

## Zoom der gesamten Oberfläche

Ctrl++, Ctrl+- und Ctrl+0 (oder Ctrl + Mausrad) skalieren die gesamte Oberfläche, nicht nur
den Text des Editors. Die Stufe wird gemerkt.

## Suchen und Ersetzen

Ctrl+F / Ctrl+H, mit vorherige/nächste, alles ersetzen und Groß-/Kleinschreibung beachten.

## Dateien und Sicherheit deiner Daten

- **Zuletzt geöffnete Dateien**, Öffnen per Ziehen und Bestätigung nicht gespeicherter Änderungen.
- **Dokumentvorlagen** (Datei → Neu aus Vorlage).
- **Front matter** in YAML/TOML wird wortgetreu beibehalten.
- **Überwachung der Datei auf der Festplatte**: erkennt externe Änderungen und bietet das Neuladen an.
- **Automatisches Speichern und Wiederherstellung** nach einem anormalen Schließen.

## Internationalisierung

Oberfläche in 10 Sprachen: Spanisch, Englisch, Deutsch, Französisch, Italienisch,
Portugiesisch, Polnisch, Niederländisch, Rumänisch und vereinfachtes Chinesisch
(Ansicht → Sprache; wird sofort angewendet – das Fenster wird neu aufgebaut).
