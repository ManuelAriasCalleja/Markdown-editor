# md-editor

A WYSIWYG Markdown editor/viewer built with Qt6 + C++17. By default you edit on the
rendered text, without dealing with the syntax; but you can optionally view the
Markdown source, or even have the source and its rendering side by side (split view)
and edit on either side. On save it always serializes back to clean Markdown.

## What it does for you

- **Real WYSIWYG**: you see the result, not the symbols.
- **Faithful round-trip**: what you open is what you save — aligned tables, task
  lists, quotes, code blocks and formulas.
- **Three ways to work**: rendered only (the default), source only, or both side by
  side (synchronized split view).
- **Distraction-free mode**: centered reading column, no toolbars (F11), with an
  optional table of contents (show it or hide it).
- **Eye care**: the *warm night light* gradually dims the blue of the background
  according to the time of day, to reduce eye strain at night.
- **TeX formulas**: [inline](Caracteristicas-en#tex-formulas) and [block](Caracteristicas-en#tex-formulas),
  with real super/subscripts and a live preview, with no external dependencies.
- **Export** to PDF, HTML, ODF (.odt) and LaTeX (.tex), preserving the document
  language and the formula formatting.
- **Display**: 1) 6 light and dark themes, 2) full-interface zoom, 3) interface
  translated into 9 languages.

## Getting started

- [Installation](Instalacion-en)
- [Usage](Uso-en)
- [Features](Caracteristicas-en)
- [Keyboard shortcuts](Atajos-en)

---

*md-editor is developed by Manuel Arias Calleja. Licensed under CC BY-ND 4.0.*
