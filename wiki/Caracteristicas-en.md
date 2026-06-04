# Features

An overview of everything md-editor offers. For the complete, technical reference see
`especificacion.md` in the repository.

## WYSIWYG editing and round-trip

You edit on the rendered text and, on save, it is serialized to clean Markdown in
UTF-8. What you open is what you save: tables with alignment, nested lists, task lists,
quotes, code blocks and formulas are all preserved faithfully.

## View modes

- WYSIWYG, Markdown source (Ctrl+Shift+M) and Split view (Ctrl+Shift+D).
- In split view, rendering and source stay synchronized: only the pane you are not
  editing is updated, with no cursor jumps.

## Distraction-free mode

F11 enters full screen with the text centered in a reading column and no toolbars. ESC
or F11 exit.

## Themes and warm night light

- **Six themes**: Light, Dark, GitHub Light, GitHub Dark, Monokai and High contrast.
- **Warm night light** (on by default): dims the blue of the background automatically
  and gradually according to the time of day, to reduce eye strain at night. Neutral
  during the day (07–19 h), warming up in the evening (19–23 h), at its maximum at
  night (23–06 h) and cooling down at dawn (06–07 h). It re-evaluates itself every
  minute and only affects the background (not links or highlighting).

## Document outline

A side panel (F9) with the heading index; a click jumps to the section.

## TeX formulas

Inline (`$...$`) and block (`$$...$$`) formulas with LaTeX syntax, with no external
dependencies:

- Insertion with a live preview (Ctrl+Shift+F) and editing with a double-click.
- Real super/subscripts, Greek letters, operators, `\frac`, `\sqrt`, `\mathbb`…
- They are atomic in the editor and survive the round-trip and export.
- Limitations: `$...$` must open and close on the same line; there is no 2D *layout*
  (large fractions appear as `(a)/(b)`).

## Syntax highlighting

Code blocks are colored according to their language (C/C++/Java… families, JS/TS/JSON,
Python, shell/YAML/TOML… and a generic mode).

## Images

Pasting or dropping an image saves it as PNG next to the document and inserts it as
`![](path)` —it is not embedded—, so the Markdown stays portable.

## Export and print

PDF, HTML, ODF (.odt) and LaTeX (.tex), plus printing (Ctrl+P). ODF and LaTeX embed the
document language (from the front matter, the app setting or the system).

## Full-interface zoom

Ctrl++, Ctrl+- and Ctrl+0 (or Ctrl + wheel) scale the whole interface, not just the
editor text. The level is remembered.

## Find and replace

Ctrl+F / Ctrl+H, with previous/next, replace all and case sensitivity.

## Files and the safety of your data

- **Recent files**, drag-to-open and confirmation of unsaved changes.
- **Front matter** YAML/TOML kept verbatim.
- **Disk file watching**: detects external changes and offers to reload.
- **Autosave and recovery** after an abnormal close.

## Internationalization

Interface in 9 languages: Spanish, English, German, French, Italian, Portuguese,
Polish, Dutch and Romanian (View → Language; applied on restart).
