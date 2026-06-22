# Usage

## Opening and saving

- New (Ctrl+N), Open (Ctrl+O), Save (Ctrl+S), Save As (Ctrl+Shift+S). All in UTF-8.
- **Tabs**: each open document takes its own tab; close one with Ctrl+W. On the next
  launch the tabs from the last session are reopened.
- **New from template** (File → New from template) starts from a ready-made Markdown
  skeleton.
- **Open recent** lists your latest documents.
- You can also drag and drop a file onto the window to open it.
- If the file changes outside md-editor, it warns you: it reloads on its own if you
  had no changes, or asks you if you did.

### Front matter

If your document starts with a `---…---` (YAML) or `+++…+++` (TOML) block, it is kept
verbatim on save (not shown or edited). It holds metadata such as `title` and `lang`,
which are used on export.

## Formatting

Use the Format menu or the toolbar. You don't need to type Markdown symbols: the
editor applies them for you.

- Bold (Ctrl+B), Italic (Ctrl+I), Underline (Ctrl+U), Strikethrough, Inline code,
  Link (Ctrl+K).
- Headings H1–H6 (Ctrl+1 … Ctrl+6).
- Bulleted, numbered and task lists, with automatic continuation when you press Enter
  (an empty item leaves the list). Task checkboxes are toggled with a click.
- Quotes and code blocks.

See all shortcuts in [Keyboard shortcuts](Atajos-en).

## Editing and transforming text

- **Paste as plain text** (Ctrl+Shift+V) or **Paste as Markdown** (Ctrl+Alt+V), which
  converts the clipboard HTML to Markdown. Pasting a URL over a selection auto-links it.
- **Edit → Transform text**: UPPERCASE, lowercase, capitalize, sort lines and smart
  typography (converts `--`, `---`, `...` and straight quotes).

## Inserting

- Link and Image (with a path relative to the document so it stays portable).
- **Paste image**: the clipboard image is saved as PNG next to your `.md` and inserted
  as `![](path)`. It also works by dragging or pasting onto the editor.
- Table, Horizontal rule, Table of contents (TOC) and Formula (Ctrl+Shift+F).
- **Footnote** (Ctrl+Shift+N): inserts a `[^n]` reference and its definition.
- **Admonition**: a highlighted block (note, tip, important, warning, caution).
- **Special symbols** and **Date / Date and time**.

## Tables

With the cursor inside a table, the Table menu lets you add or remove rows and columns
and align each column (left/center/right). The alignment is kept on save.

## Formulas

Insert TeX formulas inline (`$...$`) or block (`$$...$$`) with Insert → Formula
(Ctrl+Shift+F), with a live preview. Double-click a formula to edit it. They are
painted in real 2D (fractions, roots, matrices, summations with limits…). More detail
in [Features](Caracteristicas-en#tex-formulas).

## Diagrams

Write a code block with the `mermaid` or `plantuml` language and, if you have the
corresponding tool installed (`mmdc` / `plantuml`), it is rendered as an image below
the block. If it is missing, you will see the command to install it.

## Spell checking

Enable it in View → Spell checking (requires Hunspell). The language is chosen by the
document's own or by hand in View → Spell-check language. Right-clicking an underlined
word offers suggestions and adding it to your personal dictionary.

## View modes

- **WYSIWYG** (the default): just the rendered result.
- **Markdown source** (Ctrl+Shift+M): the raw Markdown, full screen.
- **Split view** (Ctrl+Shift+D): rendering and source side by side, synchronized.
- **Outline** (F9) and **Go to heading** (Ctrl+G) to navigate the document.

## Find and replace

Ctrl+F to find, Ctrl+H to replace. Includes previous/next, replace all and case
sensitivity.

## Export and print

File → Export offers PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) and EPUB
(.epub); also Print preview and Print (Ctrl+P). ODF, DOCX and LaTeX embed the document
language.

## Automatic recovery

md-editor saves a draft every few seconds. If the app closes abnormally, on reopening
it offers to recover what you were writing.
