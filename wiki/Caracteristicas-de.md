# Funktionen

Überblick über alles, was md-editor bietet. Die vollständige technische Referenz findest
du in `especificacion.md` im Repository.

## WYSIWYG-Bearbeitung und Round-Trip

Du bearbeitest den gerenderten Text und beim Speichern wird sauberes Markdown in UTF-8
serialisiert. Was du öffnest, speicherst du auch: Tabellen mit Ausrichtung, verschachtelte
Listen, Aufgabenlisten, Zitate, Codeblöcke und Formeln bleiben originalgetreu erhalten.

## Ansichtsmodi

- WYSIWYG, Markdown-Quelltext (Ctrl+Shift+M) und Geteilte Ansicht (Ctrl+Shift+D).
- In der geteilten Ansicht werden gerenderte Darstellung und Code synchronisiert: Es wird
  nur der Bereich aktualisiert, den du gerade nicht bearbeitest, ohne Cursorsprünge.

## Ablenkungsfreier Modus

F11 wechselt in den Vollbildmodus mit dem Text zentriert in einer Lesespalte und ohne
Leisten. ESC oder F11 verlassen ihn.

## Designs und warmes Nachtlicht

- **Sechs Designs**: Hell, Dunkel, GitHub Light, GitHub Dark, Monokai und Hoher Kontrast.
- **Warmes Nachtlicht** (standardmäßig aktiviert): dämpft das Blau des Hintergrunds
  automatisch und schrittweise je nach Uhrzeit, um die Ermüdung der Augen in der Nacht zu
  verringern. Neutral tagsüber (07–19 Uhr), wird abends wärmer (19–23 Uhr), maximal nachts
  (23–06 Uhr) und kühlt sich bei Tagesanbruch ab (06–07 Uhr). Es wird jede Minute von
  selbst neu bewertet und betrifft nur den Hintergrund (nicht Links oder Hervorhebung).

## Dokumentgliederung

Seitenleiste (F9) mit dem Verzeichnis der Überschriften; ein Klick springt zum Abschnitt.

## TeX-Formeln

Inline-Formeln (`$...$`) und Block-Formeln (`$$...$$`) mit LaTeX-Syntax, ohne externe
Abhängigkeiten:

- Einfügen mit Live-Vorschau (Ctrl+Shift+F) und Bearbeiten per Doppelklick.
- Echte Hoch- und Tiefstellungen, griechische Buchstaben, Operatoren, `\frac`, `\sqrt`, `\mathbb`…
- Sie sind im Editor atomar und überstehen den Round-Trip und den Export.
- Einschränkungen: `$...$` muss in derselben Zeile öffnen und schließen; es gibt kein 2D-*Layout*
  (große Brüche wie `(a)/(b)`).

## Syntaxhervorhebung

Codeblöcke werden je nach Sprache eingefärbt (Familien C/C++/Java…, JS/TS/JSON, Python,
Shell/YAML/TOML… und ein generischer Modus).

## Bilder

Ein Bild einzufügen oder abzulegen speichert es als PNG neben dem Dokument und fügt es als
`![](ruta)` ein – es wird nicht eingebettet –, sodass das Markdown weiterhin portabel
bleibt.

## Exportieren und Drucken

PDF, HTML, ODF (.odt) und LaTeX (.tex), sowie Drucken (Ctrl+P). ODF und LaTeX betten die
Dokumentsprache ein (aus dem Front matter, der App-Einstellung oder vom System).

## Zoom der gesamten Oberfläche

Ctrl++, Ctrl+- und Ctrl+0 (oder Ctrl + Mausrad) skalieren die gesamte Oberfläche, nicht nur
den Text des Editors. Die Stufe wird gemerkt.

## Suchen und Ersetzen

Ctrl+F / Ctrl+H, mit vorherige/nächste, alles ersetzen und Groß-/Kleinschreibung beachten.

## Dateien und Sicherheit deiner Daten

- **Zuletzt geöffnete Dateien**, Öffnen per Ziehen und Bestätigung nicht gespeicherter Änderungen.
- **Front matter** in YAML/TOML wird wortgetreu beibehalten.
- **Überwachung der Datei auf der Festplatte**: erkennt externe Änderungen und bietet das Neuladen an.
- **Automatisches Speichern und Wiederherstellung** nach einem anormalen Schließen.

## Internationalisierung

Oberfläche in 9 Sprachen: Spanisch, Englisch, Deutsch, Französisch, Italienisch,
Portugiesisch, Polnisch, Niederländisch und Rumänisch (Ansicht → Sprache; wird beim Neustart
angewendet).
