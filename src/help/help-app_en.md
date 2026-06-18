# User manual

**md-editor** is a visual (WYSIWYG) Markdown editor: you write and apply
formatting over the already-rendered text, without seeing the code. When you
save, the document is serialized back to plain Markdown.

## Contents

- [Opening and saving](#opening-and-saving)
- [Formatting text](#formatting-text)
- [Headings, lists and blocks](#headings-lists-and-blocks)
- [Links and images](#links-and-images)
- [Tables](#tables)
- [Math formulas](#math-formulas)
- [Find and replace](#find-and-replace)
- [Document outline](#document-outline)
- [Distraction-free mode](#distraction-free-mode)
- [Source view](#source-view)
- [Export and print](#export-and-print)
- [Themes and appearance](#themes-and-appearance)
- [Automatic recovery](#automatic-recovery)
- [Shortcuts](#shortcuts)

## Opening and saving

- **File → New** (Ctrl+N) creates an empty document.
- **File → Open…** (Ctrl+O) opens an existing `.md`. The application
  remembers the most recent files in **File → Open recent**.
- **Save** (Ctrl+S) and **Save as…** (Ctrl+Shift+S) write the document in
  UTF-8.
- If the file changes outside the editor, the application detects it and, if
  you have no unsaved changes, reloads it; otherwise it asks what to do.
- You can also **drag and drop** a file onto the window to open it.

### Front matter

If the document begins with a `---…---` (YAML) or `+++…+++` (TOML) block, it
is preserved verbatim on save: it is not shown in the editor and is not
editable. It is meant for metadata such as `title`, `lang`, etc., which are
used when exporting.

## Formatting text

Select a fragment and apply formatting from the toolbar or the **Format**
menu:

- **Bold** (Ctrl+B), **Italic** (Ctrl+I), **Underline** (Ctrl+U),
  **Strikethrough**.
- **Inline code** for `monospaced` fragments.
- **Link**: adds `[text](url)` over the selection.

The toolbar buttons reflect the active formatting under the cursor.

## Headings, lists and blocks

- **Headings** H1–H6 from **Format → Heading** or with Ctrl+1 … Ctrl+6.
- **Lists**: bullets, numbered and task lists (with a checkbox). Pressing
  Enter at the end of an item creates the next one automatically; pressing
  Enter on an empty item exits the list.
- **Quote** (`>` at the start of a paragraph) and **code block** are applied
  from the toolbar; both round-trip to Markdown correctly.

## Links and images

- **Insert → Link…** opens a dialog with text and URL fields. If you had a
  selection, it is used as the text.
- **Ctrl+click** on a link opens it in the system browser; hovering shows
  the URL in the status bar.
- **Images**: drag a file, paste an image from the clipboard, or use
  **Insert → Paste image**. The image is saved as PNG next to the `.md` and
  inserted as `![alt](relative-path)`; that way it survives the round-trip
  to Markdown (embedded images do not).

## Tables

- **Table → Insert table…** asks for rows and columns.
- The actions in the **Table** menu (add/remove row or column, align column)
  are only enabled when the cursor is inside a table.
- Column alignment (left/center/right) is preserved on save as
  `:--`/`:-:`/`--:`.

## Math formulas

md-editor supports **TeX formulas** inline (`$...$`) and as blocks
(`$$...$$`), with the usual LaTeX syntax (Pandoc, Obsidian, Quarto…). No
external dependencies needed.

- **Insert → Formula…** (Ctrl+Shift+F) opens a dialog with a TeX field and a
  **live preview**: as you type you see what it will look like. Choose
  *Inline* or *Block* and accept to insert it.
- In the editor, formulas appear in italics with the theme's accent color,
  with **real super/subscripts** (not flat Unicode characters): `x²`, `Hᵢ`,
  and so on — Qt vertical-align scales any character correctly.
- **Double-click** a formula to reopen the dialog with its original TeX
  preloaded: edit and accept to replace it.
- Formulas are **atomic**: typing inside one triggers a reminder to
  double-click for editing; Backspace/Delete at the edge removes the whole
  group.
- On **export**, formulas are preserved: LaTeX emits them verbatim (with
  `amsmath` and `amssymb` in the preamble); HTML/PDF/ODF keep Qt's
  vertical-align super/subscripts in the target format.
- In **source view** you see them as `$...$` / `$$...$$`, with all TeX
  characters (`\sum`, `\frac`, `_`, `*`) intact on save.

Examples:

```
The energy is $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitation: in source, `$$...$$` may span multiple lines (Obsidian/Pandoc
> style); `$...$` must open and close on the same line.

## Find and replace

- **Find** (Ctrl+F) opens a bottom bar with find and replace fields, plus
  options (case, whole word).
- **Find next** F3 / **Find previous** Shift+F3.

## Document outline

The left side panel shows the index of headings (TOC): it updates as you
type, and clicking an entry jumps the cursor to that heading. It is
toggled with F9.

## Distraction-free mode

**View → Distraction-free** (F11) enters full screen with the menu and
toolbars hidden and the text centered in a reading column. The outline, if
visible, stays attached to the central block. ESC or F11 exits.

## Source view

**View → Markdown source** (Ctrl+Shift+M) toggles between the visual editor and a
full-screen plain-text editor showing the raw Markdown. Changes made in source
mode are flushed into the document when you return to visual mode.

**View → Split view** (Ctrl+Shift+D) shows both side by side: the visual editor
and the source, kept in sync (what you type in one is reflected in the other). It
is mutually exclusive with full-screen source mode.

## Export and print

**File → Export** offers **PDF**, **HTML**, **ODF (.odt)** and
**LaTeX (.tex)**. For ODF and LaTeX the document language is embedded
(taken from the front matter `lang`/`language`, the application setting or,
as a last resort, the system locale).

**File → Print** (Ctrl+P) opens the system dialog.

## Themes and appearance

- **View → Theme** offers Light, Dark, GitHub Light, GitHub Dark, Monokai
  and High contrast.
- **View → Warm night light** dims the blues in the background based on the
  time of day.
- **Zoom**: Ctrl+mouse wheel, Ctrl++ / Ctrl+- and **Normal size** (Ctrl+0)
  scale the whole interface (not just the editor text).
- **View → Language** changes the interface language; takes effect immediately (the window is recreated).

## Automatic recovery

While you edit, the content is autosaved every few seconds into a draft
copy. If the application closes unexpectedly, on the next launch it offers
to recover what you were writing.

## Shortcuts

| Action                    | Shortcut         |
|---------------------------|------------------|
| New                       | Ctrl+N           |
| Open                      | Ctrl+O           |
| Save                      | Ctrl+S           |
| Save as                   | Ctrl+Shift+S     |
| Print                     | Ctrl+P           |
| Undo / Redo               | Ctrl+Z / Ctrl+Y  |
| Bold / Italic             | Ctrl+B / Ctrl+I  |
| Underline                 | Ctrl+U           |
| Find                      | Ctrl+F           |
| Find next / previous      | F3 / Shift+F3    |
| Heading H1 … H6           | Ctrl+1 … Ctrl+6  |
| Insert formula            | Ctrl+Shift+F     |
| Markdown source view      | Ctrl+Shift+M     |
| Split view                | Ctrl+Shift+D     |
| Outline                   | F9               |
| Distraction-free          | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Help                      | F1               |
