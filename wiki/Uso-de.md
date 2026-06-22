# Verwendung

## Öffnen und Speichern

- Neu (Ctrl+N), Öffnen (Ctrl+O), Speichern (Ctrl+S), Speichern unter (Ctrl+Shift+S).
  Alles in UTF-8.
- **Tabs**: Jedes geöffnete Dokument belegt seinen eigenen Tab; schließe einen mit
  Ctrl+W. Beim erneuten Start werden die Tabs der letzten Sitzung wieder geöffnet.
- **Neu aus Vorlage** (Datei → Neu aus Vorlage) geht von einem bereits vorbereiteten
  Markdown-Gerüst aus.
- **Zuletzt geöffnet** listet deine letzten Dokumente auf.
- Du kannst auch eine Datei auf das Fenster ziehen und ablegen, um sie zu öffnen.
- Wenn sich die Datei außerhalb von md-editor ändert, wirst du benachrichtigt: Sie wird
  automatisch neu geladen, wenn du keine Änderungen hattest, oder du wirst gefragt, falls
  doch.

### Front matter

Wenn dein Dokument mit einem Block `---…---` (YAML) oder `+++…+++` (TOML) beginnt, wird
dieser beim Speichern unverändert beibehalten (er wird weder angezeigt noch bearbeitet).
Er dient für Metadaten wie `title` und `lang`, die beim Exportieren verwendet werden.

## Formatieren

Verwende das Menü Format oder die Werkzeugleiste. Du musst keine Markdown-Symbole
tippen: Der Editor wendet sie für dich an.

- Fett (Ctrl+B), Kursiv (Ctrl+I), Unterstrichen (Ctrl+U), Durchgestrichen, Inline-Code,
  Link (Ctrl+K).
- Überschriften H1–H6 (Ctrl+1 … Ctrl+6).
- Aufzählungs-, nummerierte und Aufgabenlisten, mit automatischer Fortsetzung beim Drücken
  von Enter (ein leerer Punkt verlässt die Liste). Die Aufgaben-Kontrollkästchen werden
  mit einem Klick markiert.
- Zitate und Codeblöcke.

Alle Tastenkürzel findest du unter [Tastenkürzel](Atajos-de).

## Text bearbeiten und umwandeln

- **Als reinen Text einfügen** (Ctrl+Shift+V) oder **Als Markdown einfügen** (Ctrl+Alt+V),
  was das HTML aus der Zwischenablage in Markdown umwandelt. Das Einfügen einer URL über
  einer Auswahl verlinkt diese automatisch.
- **Bearbeiten → Text umwandeln**: GROSSBUCHSTABEN, Kleinbuchstaben, Großschreibung, Zeilen
  sortieren und intelligente Typografie (wandelt `--`, `---`, `...` und gerade
  Anführungszeichen um).

## Einfügen

- Link und Bild (mit relativem Pfad zum Dokument, damit es portabel bleibt).
- **Bild einfügen**: Das Bild aus der Zwischenablage wird als PNG neben deiner `.md`
  gespeichert und als `![](ruta)` eingefügt. Funktioniert auch durch Ziehen oder Einfügen
  über dem Editor.
- Tabelle, Horizontale Linie, Inhaltsverzeichnis (TOC) und Formel (Ctrl+Shift+F).
- **Fußnote** (Ctrl+Shift+N): fügt eine Referenz `[^n]` und ihre Definition ein.
- **Admonition**: hervorgehobener Block (Notiz, Tipp, Wichtig, Warnung, Vorsicht).
- **Sonderzeichen** und **Datum / Datum und Uhrzeit**.

## Tabellen

Wenn sich der Cursor innerhalb einer Tabelle befindet, kannst du über das Menü Tabelle
Zeilen und Spalten hinzufügen oder entfernen und jede Spalte ausrichten
(links/zentriert/rechts). Die Ausrichtung bleibt beim Speichern erhalten.

## Formeln

Füge TeX-Formeln inline (`$...$`) oder als Block (`$$...$$`) über Einfügen → Formel
(Ctrl+Shift+F) ein, mit Live-Vorschau. Ein Doppelklick auf eine Formel bearbeitet sie. Sie
werden in echtem 2D gezeichnet (Brüche, Wurzeln, Matrizen, Summenzeichen mit Grenzen…).
Mehr Details unter [Funktionen](Caracteristicas-de#tex-formeln).

## Diagramme

Schreibe einen Codeblock mit der Sprache `mermaid` oder `plantuml`, und falls du das
entsprechende Werkzeug (`mmdc` / `plantuml`) installiert hast, wird er als Bild unter dem
Block gerendert. Fehlt es, siehst du den Befehl zu seiner Installation.

## Rechtschreibprüfung

Aktiviere sie unter Ansicht → Rechtschreibprüfung (erfordert Hunspell). Die Sprache wird
über die des Dokuments oder von Hand unter Ansicht → Sprache der Rechtschreibprüfung
gewählt. Ein Rechtsklick auf ein unterstrichenes Wort bietet Vorschläge und das Hinzufügen
zum persönlichen Wörterbuch.

## Ansichtsmodi

- **WYSIWYG** (Standard): nur das gerenderte Ergebnis.
- **Markdown-Quelltext** (Ctrl+Shift+M): das rohe Markdown im Vollbild.
- **Geteilte Ansicht** (Ctrl+Shift+D): gerenderte Darstellung und Code nebeneinander,
  synchronisiert.
- **Gliederung** (F9) und **Zu Überschrift springen** (Ctrl+G) zum Navigieren im Dokument.

## Suchen und Ersetzen

Ctrl+F zum Suchen, Ctrl+H zum Ersetzen. Enthält vorherige/nächste, alles ersetzen und
Groß-/Kleinschreibung beachten.

## Exportieren und Drucken

Datei → Exportieren bietet PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) und EPUB
(.epub); auch Druckvorschau und Drucken (Ctrl+P). Bei ODF, DOCX und LaTeX wird die
Dokumentsprache eingebettet.

## Automatische Wiederherstellung

md-editor speichert alle paar Sekunden einen Entwurf. Wird die Anwendung anormal
geschlossen, bietet sie dir beim erneuten Öffnen an, das wiederherzustellen, woran du
gerade geschrieben hast.
