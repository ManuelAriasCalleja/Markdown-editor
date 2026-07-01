# User manual

**md-editor** is a visual (WYSIWYG) Markdown editor: you write and format on the
already-rendered text, without seeing the code. On save, the document is
serialized back to plain Markdown.

## Index

- [Opening and saving](#opening-and-saving)
- [Formatting text](#formatting-text)
- [Headings, lists and blocks](#headings-lists-and-blocks)
- [Transforming text and the clipboard](#transforming-text-and-the-clipboard)
- [Links and images](#links-and-images)
- [Footnotes](#footnotes)
- [Admonitions, symbols and text shortcuts](#admonitions-symbols-and-text-shortcuts)
- [Snippets (reusable fragments)](#snippets-reusable-fragments)
- [Tables](#tables)
- [Math formulas](#math-formulas)
- [Diagrams](#diagrams)
- [Spell checking](#spell-checking)
- [Find and replace](#find-and-replace)
- [Document outline](#document-outline)
- [Document statistics](#document-statistics)
- [Distraction-free mode](#distraction-free-mode)
- [Focus mode](#focus-mode)
- [Source view](#source-view)
- [Export and print](#export-and-print)
- [Themes and appearance](#themes-and-appearance)
- [Automatic recovery](#automatic-recovery)
- [Accessibility](#accessibility)
- [Shortcuts](#shortcuts)

## Opening and saving

- **File → New** (Ctrl+N) creates an empty document in a new tab.
- **File → New from template** creates a document from a skeleton ready to fill in.
  Templates are grouped by category (Personal, Programming, Teaching, Business,
  Writing…).
- **File → Open…** (Ctrl+O) opens an existing `.md`. The application remembers
  the most recent ones in **File → Open recent**.
- **File → Import** opens a document from another format, converting it to Markdown
  in a new untitled tab (the original is left untouched): **From HTML…** (a web page)
  and **From EPUB…** (a book; its chapters are read in order). Works best with simple
  content; it respects the declared character set.
- **Save** (Ctrl+S) and **Save as…** (Ctrl+Shift+S) write the document in UTF-8.
  **Open containing folder** opens the document's folder in the file manager.
- **Revert to saved** discards unsaved changes and reloads the file from disk
  (asks for confirmation). Available only when the document has a file and
  pending changes.
- If the file changes outside the editor, the application detects it and, if you
  have no unsaved changes, reloads it; if you do, it asks what to do.
- You can also **drag and drop** a file onto the window to open it.

### Tabs (multiple documents)

You can have several documents open at once, each in its own **tab**:

- **New** (Ctrl+N), **New from template** and **Open** (Ctrl+O) create a tab (or
  reuse the empty initial tab). Dropping a file also opens it in a tab; if it is
  already open, it jumps to its tab.
- Switch documents by clicking their tab; drag tabs to reorder them. With the
  keyboard, **Ctrl+PageDown / Ctrl+PageUp** (or **Ctrl+Tab / Ctrl+Shift+Tab**)
  jump to the next or previous tab.
- **Close tab** (Ctrl+W) closes the current one, asking whether it has unsaved
  changes. The last tab is not closed: it becomes a new document.
- **Reopen closed tab** (Ctrl+Shift+R) reopens the last tab you closed (only
  those that had a file on disk).
- The label shows the file name and a dot (•) when there are unsaved changes.
- On closing the application the open documents are remembered and all reopened
  the next time you start.

### *Front matter*

If the document starts with a `---…---` (YAML) or `+++…+++` (TOML) block, it is
preserved verbatim on save: it is not shown in the editor and not edited. It is
used for metadata such as `title`, `lang`, etc., which are used when exporting.

## Formatting text

Select a fragment and apply formatting with the toolbar or the **Format** menu:

- **Bold** (Ctrl+B), **Italic** (Ctrl+I), **Underline** (Ctrl+U),
  **Strikethrough**.
- **Inline code** for `monospaced` fragments.
- **Link**: adds `[text](url)` over the selection.
- **Highlight** (Ctrl+Shift+H): wraps the selection in `==mark==`; the text is shown
  with a highlight background. Since `==` isn't standard Markdown syntax, it's saved
  as literal text.
- **Superscript** (Ctrl+Shift++) and **Subscript** (Ctrl+Shift+-): raise or lower the
  selected text; saved as `^text^` and `~text~` (Pandoc style).

The toolbar buttons reflect the active formatting under the cursor.

**Auto-pairing.** When you type `(`, `[`, `{` or `` ` `` the pair is closed
automatically and the cursor stays in the middle; if there is a selection, it is
wrapped. If you type the closing character right in front of its match, the
editor "skips over" it instead of duplicating it.

**Input rules.** At the start of a line, typing a Markdown block marker followed by a
space transforms the line in place (without leaving the marker): `#` … `######` +
space → heading H1…H6; `>` → quote; `-`, `*` or `+` → bullet list; `1.` (or `1)`) →
numbered list. Produces the same format as the toolbar.

## Headings, lists and blocks

- **Headings** H1–H6 from **Format → Heading** or with Ctrl+1 … Ctrl+6.
  **Promote/demote** the heading at the cursor by one level with
  Ctrl+Shift+[ / Ctrl+Shift+].
- **Lists**: bullets, numbered and task lists (with a checkbox). Pressing Enter
  at the end of an item automatically creates the next one; Enter on an empty
  item leaves the list. A **click on the checkbox** of a task toggles it.
- **Quote** (`>` at the start of a paragraph) and **code block** are applied
  from the toolbar; both round-trip to Markdown correctly. With **Format → Block
  language…** you choose the language of a code block (with the cursor inside it)
  so its syntax gets highlighted.
- **Indentation**: **Format → Increase/Decrease indent** nests lists and quotes.

## Transforming text and the clipboard

- **Edit → Transform text** acts on the selection: **UPPERCASE**, **lowercase**,
  **Capitalize** and **Sort lines**.
- **Smart typography** (in the same menu) converts the dashes `--`/`---` in the
  selection to `–`/`—`, `...` to `…` and straight quotes to typographic ones
  according to context.
- **Paste as plain text** (Ctrl+Shift+V) pastes without formatting. **Paste as
  Markdown** (Ctrl+Alt+V) converts the clipboard's rich content (HTML) to
  Markdown instead of embedding the source's formatting.
- **Copy as HTML** copies the selection (or the document) as HTML, to paste into
  email, a CMS, etc.
- **Copy as Markdown** copies the selection (or the whole document) as Markdown
  text, to paste into another Markdown editor or a code field.
- When you paste a **URL** over a text selection, the text is linked
  automatically.
- **Edit → Clean up Markdown** normalizes the whole document in one pass: it
  unifies bullets to `-`, trims trailing spaces at the end of each line,
  collapses extra blank lines and fixes the spacing after the `#` of headings.
  It is conservative: it does not touch the inside of code blocks.

## Links and images

- **Insert → Link…** opens a dialog with text and URL. If you had a selection,
  it is used as the text.
- **Ctrl+click** on a link opens it in the system browser; hovering over it
  shows the URL in the status bar.
- **Images**: drag a file, paste an image from the clipboard or use **Insert →
  Paste image**. The image is saved as PNG next to the `.md` and inserted as
  `![alt](relative-path)`; this way it survives the round-trip to Markdown
  (embedded images do not).

## Footnotes

- **Insert → Footnote** (Ctrl+Shift+N) inserts a numbered reference `[^n]` at the
  cursor and creates its definition `[^n]:` at the end of the document, ready for
  you to write the note text.
- References are shown as **superscript**; **clicking** one jumps the cursor to
  its definition.
- They are saved as standard Markdown (`text[^1]` in the body and, below,
  `[^1]: the note`), so they are compatible with other editors.

## Admonitions, symbols and text shortcuts

- **Insert → Admonition** creates a GitHub-style callout: a quote whose first
  line is `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` or `[!CAUTION]`. It
  is shown with a tinted background and a colored title, and is saved as
  GitHub-compatible Markdown.
- **Insert → Special symbols…** opens a character map by categories
  (mathematical, Greek, arrows, currency, punctuation…); a click inserts the
  symbol and the dialog stays open to insert several.
- **`:name:` shortcuts**: typing a code such as `:alpha:` or `:euro:` expands to
  the corresponding symbol (α, €…).
- **Insert → Date** and **Date and time** insert the current date (and time) in
  localized format.

## Snippets (reusable fragments)

A **snippet** is a piece of Markdown that you save under a name to insert it
later with a couple of clicks: a signature, a table template, a notice you often
repeat…

- **Insert → Snippet** unfolds the list of the ones you have; when you choose
  one, its content is inserted where the cursor is (it also works in the source
  view).
- **Insert → Snippet → Manage snippets…** opens a dialog to create, edit and
  delete your snippets. Each one has a **name** (the one you see in the menu) and
  a **body** in Markdown.
- They are saved in the application settings, so they are available in all your
  documents, not just the current one.

## Tables

- **Table → Insert table…** asks for rows and columns.
- **Insert → Table from clipboard** turns TSV/CSV data (columns separated by tabs
  or commas) copied from a spreadsheet or a CSV file into a table.
- The **Table** menu actions (add/remove row or column, align column) are only
  enabled when the cursor is inside a table.
- Column alignment (left/center/right) is preserved on save as `:--`/`:-:`/`--:`.
- **Table → Sort rows by column** (ascending/descending) reorders the rows by the
  cursor's column, keeping the header fixed; it detects whether the column is
  numeric or text.

## Math formulas

md-editor supports **TeX formulas** inline (`$...$`) and in block (`$$...$$`),
with the usual LaTeX syntax (Pandoc, Obsidian, Quarto…). No external dependency
is needed.

- **Insert → Formula…** (Ctrl+Shift+F) opens a dialog with a field for the TeX
  and a **live preview**: as you type you see how it will look. Choose *Inline*
  or *Block* and accept to insert it.
- Formulas are laid out in **real 2D**: fractions (`\frac`) stack with a bar,
  large operators (`\sum`, `\int`, `\prod`…) show their limits above and below,
  roots (`\sqrt`) carry their vinculum, and there are matrices
  (`\begin{pmatrix}`…), binomials (`\binom`) and accents (`\hat`, `\vec`,
  `\bar`…). Simpler ones (powers, subscripts, Greek) are composed inline. The
  rendering scales with the zoom.
- **Double-click** on a formula reopens the dialog with its original TeX
  preloaded: you edit and on accept it is replaced.
- Formulas are **atomic**: if you type inside, the app reminds you to use the
  double-click; Backspace/Delete at their edge delete the whole group.
- When **exporting** they are preserved: to LaTeX they are emitted verbatim
  (with `amsmath` and `amssymb` in the preamble); to HTML/PDF/ODF they fall back
  to their inline approximation.
- In the **source view** they appear as `$...$` / `$$...$$`, with all the TeX
  characters (`\sum`, `\frac`, `_`, `*`) intact on save.

Examples:

```
The energy is $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> In the source, `$$...$$` may span several lines (Obsidian/Pandoc style);
> `$...$` must open and close on the same line.

## Diagrams

A code block with language `mermaid` or `plantuml` is **previewed as an image**
right below the block, without touching the code (which stays editable) or the
saved Markdown.

- It requires the corresponding tool to be installed: **`plantuml`** (with Java)
  for PlantUML, or **`mmdc`** (mermaid-cli, with Node) for Mermaid.
- If the tool is missing, a notice with the installation command for your
  operating system appears below the block; the block stays as code.
- The image is presentation only: it is not written to the Markdown and does not
  count as an unsaved change.

For example, a code block tagged `mermaid` containing `flowchart LR  A --> B
--> C` is previewed as the corresponding flowchart.

## Spell checking

- Underlines misspelled words in red according to the **document language**
  (taken from the `lang` front matter, the language setting or the system). It
  does not check code, formulas or links.
- **Right-click** on an underlined word offers **suggestions** (a click replaces
  it), **Add to dictionary** (a permanent personal list) and **Ignore** (for the
  session).
- It is turned on/off in **View → Spell checking**, and the language is set in
  **View → Spell-check language** (or left automatic).
- It needs Hunspell dictionaries: on Linux, the system ones (`hunspell-es`,
  `hunspell-en-us`…); on Windows/macOS they ship with the application.

## Find and replace

- **Find** (Ctrl+F) opens a bottom bar with fields to find and replace, plus
  options (case, whole word).
- **Find next** F3 / **Find previous** Shift+F3.
- The bar highlights **all** matches in the document and shows an **"N of M"** counter (which match you're on, out of how many). **Replace all** replaces them all at once.

## Document outline

The left side panel shows the heading outline (TOC): it updates as you type and,
when you click an entry, the cursor jumps to that heading. It is shown/hidden
with F9. With **F6** you move keyboard focus to the outline (showing
it if hidden); there, the arrow keys move through the headings and **Enter**
jumps to the selected one and returns focus to the editor. Pressing
**F6** again simply returns focus to the editor.

The **filter field** at the top of the panel shows only the headings matching what
you type (and their ancestors); the **⊞/⊟** buttons expand or collapse everything.
The folding you set is **kept** even as you keep editing.

You can **drag** an outline entry to **reorder** that section —its heading, its
content and its subsections— within the document, without changing its level.
Also, **Insert → Table of contents (TOC)** dumps a nested list of the headings
into the document. **View → Go to heading…** (Ctrl+G) jumps to a heading by
typing part of its text, and **Go to line…** (Ctrl+L) takes the cursor to a line
number (in the source view, to the Markdown line). The **Command palette**
(Ctrl+Shift+P) finds and runs any menu action by typing part of its name.

## Document statistics

- **View → Document statistics…** shows words, characters, paragraphs, sentences
  and estimated reading time (of the document or the selection).
- **View → Show word count** enables a permanent counter in the status bar.
- **View → Show line and column** shows the cursor position (line and column) in
  the status bar.

## Distraction-free mode

**View → Distraction-free** (F11) enters full screen with the menu and toolbars
hidden and the text centered in a reading column. The outline, if visible, stays
attached to the central block. ESC or F11 leave.

## Focus mode

**View → Focus mode** (F12) helps you concentrate on what you are writing without
leaving the normal window. A single switch turns on two things at once:

- **Typewriter**: the cursor's line stays vertically centered. As you write, the
  text scrolls so the active line stays at mid-height, instead of creeping toward
  the bottom edge.
- **Dimming**: the whole document looks faded except for the paragraph where the
  cursor is, which stands out crisply.

It works in the visual editor and in the source view, and it is **independent**
of distraction-free mode (F11): you can use both at once or each on its own.

## Source view

**View → Markdown source** (Ctrl+Shift+M) toggles between the visual editor and
a plain-text editor, full screen, with the raw Markdown. Changes in the source
mode are committed to the document when you return to the visual mode.

**View → Split view** (Ctrl+Shift+D) shows both at once, side by side: the visual
editor and the source code, synchronized (what you type in one is reflected in
the other). It is mutually exclusive with the full-screen source mode.

In the source view there are keyboard **line commands** for the cursor's line:
**Alt+↑ / Alt+↓** move the line up/down, **Ctrl+D** duplicates it,
**Ctrl+Shift+K** deletes it and **Ctrl+J** joins it with the next.

## Export and print

**File → Export** offers **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** and **plain text (.txt)**. In ODF, DOCX, LaTeX
and EPUB the document language is embedded (taken from the `lang`/`language`
front matter, the application setting or, as a last resort, the system language).
In **PDF**, the title and author are embedded when present in the front matter
(`title`, `author`).

You can also export **only the selection to PDF** and use the **Print preview**.

**File → Print** (Ctrl+P) opens the system dialog; **Print selection** prints
only what is selected.

**View → Page numbers when printing** (on by default) adds the page number in the
footer (`N / M`) when printing and when exporting to PDF.

## Themes and appearance

- **View → Theme** offers Light, Dark, GitHub Light, GitHub Dark, Monokai, High contrast, Solarized Light and Solarized Dark. **Follow the system** matches the light/dark theme to the OS.
- **View → Night warm light** dims the background blues according to the time.
- **View → Line spacing** sets the editor's line height: Single, 1.5 lines or Double.
- **View → Highlight the current line** marks the cursor's line with a subtle background.
- **Zoom**: Ctrl+mouse wheel, Ctrl++ / Ctrl+- and **Normal size** (Ctrl+0) scale
  the whole interface (not just the editor text).
- **View → Language** changes the interface language; it applies instantly
  (recreates the window).

## Automatic recovery

While you edit, the content is auto-saved every few seconds into a draft copy.
If the application closes abnormally, on reopening it offers to recover what you
were writing.

## Accessibility

- **Screen readers**: the editor, the outline panel, the search fields and the other controls have accessible names; status messages (saved, “not found”, changes on disk…) are also announced aloud.
- **Keyboard only**: every action has a shortcut or a menu entry (F10 or Alt opens the menu bar). See the [Shortcuts](#shortcuts) table.
- **Contrast and size**: the **High contrast** theme and the whole-interface **zoom** help with low vision; the starting font size is the system’s.
- **Focus**: the focused element is highlighted with the theme’s selection colour.

## Shortcuts

| Action                    | Shortcut         |
|---------------------------|------------------|
| New                       | Ctrl+N           |
| Close tab                 | Ctrl+W           |
| Reopen closed tab         | Ctrl+Shift+R     |
| Next / previous tab       | Ctrl+PageDown / Ctrl+PageUp (or Ctrl+Tab / Ctrl+Shift+Tab) |
| Open                      | Ctrl+O           |
| Save                      | Ctrl+S           |
| Save as                   | Ctrl+Shift+S     |
| Print                     | Ctrl+P           |
| Undo / Redo               | Ctrl+Z / Ctrl+Y  |
| Bold / Italic             | Ctrl+B / Ctrl+I  |
| Highlight (==mark==)      | Ctrl+Shift+H     |
| Superscript / Subscript   | Ctrl+Shift++ / Ctrl+Shift+- |
| Underline                 | Ctrl+U           |
| Paste as plain text       | Ctrl+Shift+V     |
| Paste as Markdown         | Ctrl+Alt+V       |
| Find                      | Ctrl+F           |
| Find next/previous        | F3 / Shift+F3    |
| Heading H1 … H6           | Ctrl+1 … Ctrl+6  |
| Promote / demote heading  | Ctrl+Shift+[ / Ctrl+Shift+] |
| Insert formula            | Ctrl+Shift+F     |
| Insert footnote           | Ctrl+Shift+N     |
| Go to heading             | Ctrl+G           |
| Go to line                | Ctrl+L           |
| Command palette           | Ctrl+Shift+P     |
| Focus outline / back to editor | F6     |
| Markdown source view      | Ctrl+Shift+M     |
| Split view                | Ctrl+Shift+D     |
| Move line ↑ / ↓ (source)  | Alt+↑ / Alt+↓    |
| Duplicate / delete / join line (source) | Ctrl+D / Ctrl+Shift+K / Ctrl+J |
| Outline                   | F9               |
| Distraction-free          | F11              |
| Focus Mode                | F12              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Help                      | F1               |
