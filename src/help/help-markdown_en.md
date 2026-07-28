# Markdown on one page

**Markdown** is a way of writing formatted text using simple symbols. What
is on the left is what you type; on the right, what it looks like. In
md-editor you do not need to type these symbols: you apply them from the
toolbar and, on save, the editor writes them for you.

## Contents

- [Paragraphs and line breaks](#paragraphs-and-line-breaks)
- [Headings](#headings)
- [Emphasis](#emphasis)
- [Lists](#lists)
- [Quotes](#quotes)
- [Code](#code)
- [Links and images](#links-and-images)
- [Footnotes](#footnotes)
- [Horizontal rules](#horizontal-rules)
- [Tables](#tables)
- [Math formulas](#math-formulas)
- [Extensions supported by md-editor](#extensions-supported-by-md-editor)
- [Front matter](#front-matter)
- [Escapes](#escapes)

## Paragraphs and line breaks

Separate paragraphs with a **blank line**. Within a paragraph, two trailing
spaces at the end of a line force a line break without starting a new
paragraph.

## Headings

```
# Level 1 heading
## Level 2 heading
### Level 3 heading
```

Up to six levels (`######`). In md-editor you can also apply them from
**Format → Heading** or with Ctrl+1 … Ctrl+6.

## Emphasis

- `*italic*` or `_italic_` → *italic*
- `**bold**` or `__bold__` → **bold**
- `***bold and italic***` → ***bold and italic***
- `~~strikethrough~~` → ~~strikethrough~~

## Lists

**Bullets** (with `-`, `*` or `+`):

```
- Apple
- Pear
  - Conference
  - Ercolini
```

**Numbered**:

```
1. First
2. Second
3. Third
```

**Tasks** (checkboxes):

```
- [x] Done
- [ ] Pending
```

## Quotes

One or more lines starting with `>`:

```
> The man who reads much and walks much sees much and knows much.
> — Miguel de Cervantes
```

## Code

**Inline**: wrap with a backtick: `` `code` ``.

**Block**: three backticks at the start and end; optionally, the language
name to color it:

````
```python
def greet(name):
    print(f"Hello, {name}")
```
````

## Links and images

- **Link**: `[text](https://example.com)`
- **Link with title**: `[text](https://example.com "Tooltip title")`
- **Image**: `![alternative text](path/image.png)` — like a link, but with
  a `!` in front.

In md-editor, **Ctrl+click** on a link opens it in the system browser.

## Footnotes

A **reference** in the text and its **definition** elsewhere, linked by an
identifier `[^id]`:

```
A claim with its nuance[^1].

[^1]: The note's text goes here.
```

The `id` can be a number (`[^1]`) or a word (`[^nota]`). In md-editor,
**Insert → Footnote** (Ctrl+Shift+N) creates the reference and its
definition for you; references appear as a superscript and a click jumps to
the definition.

## Horizontal rules

Three or more hyphens, asterisks or underscores on a line of their own:

```
---
```

## Tables

```
| Product | Quantity | Price  |
|---------|---------:|:------:|
| Bread   |        2 |  €1.20 |
| Milk    |        1 |  €0.95 |
```

The colons in the separator line set the column alignment: `:--` left,
`:-:` center, `--:` right. md-editor preserves the alignment on save.

## Math formulas

Standard Markdown does **not** define formulas, but a widespread convention
(Pandoc, Obsidian, Quarto, GitHub) supports TeX syntax between `$...$`
(inline) and `$$...$$` (block). md-editor implements this convention.

```
The formula $E = mc^2$ is famous.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

Special TeX characters (`\`, `_`, `*`, `{`, `}`) are kept intact inside
formulas — the editor protects them so the Markdown parser does not confuse
them with italics or bold.

In md-editor formulas appear rendered with real super- and subscripts (not
as literal `$x^2$`). Insert one with **Insert → Formula…** (Ctrl+Shift+F) or
double-click an existing one to edit it.

## Extensions supported by md-editor

Besides everything above — which is standard Markdown — md-editor understands
four widely used conventions. They are not part of original Markdown, so another
editor may show them as literal text; the file itself is saved as is, so nothing
is lost.

**Highlight** (GitHub/Obsidian style): two equals signs on each side.

```
This is ==highlighted== as if with a marker pen.
```

**Superscript and subscript** (Pandoc style): caret and tilde.

```
The area is 12 m^2^ and the formula for water is H~2~O.
```

**Admonitions** or *callouts* (GitHub style): a quote whose first line is a tag
in brackets. Valid tags are `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]`
and `[!CAUTION]`.

```
> [!WARNING]
> This step deletes the previous data.
```

**Diagrams**: a code block with the language `mermaid` or `plantuml`. The editor
previews it as an image below the block if you have the corresponding tool
installed.

````
```mermaid
flowchart LR
  A --> B --> C
```
````

## Front matter

Many site generators (Jekyll, Hugo, Quarto…) start the file with a metadata
block between `---` (YAML) or `+++` (TOML):

```
---
title: Annual report
lang: en
---
```

md-editor keeps it **as is** when saving: it is neither edited nor shown in the
editor. It takes `title` and `lang` from there when exporting and to pick the
spell checker dictionary.

## Escapes

To make a Markdown symbol appear literal (without acting as formatting),
put a backslash in front of it: `\*not italic\*` → \*not italic\*.

The escapable symbols are:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
