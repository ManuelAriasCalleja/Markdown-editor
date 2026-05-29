# Markdown pe o singură pagină

**Markdown** este o modalitate de a scrie text formatat folosind simboluri
simple. Ce este în stânga este ceea ce tastezi; în dreapta, cum arată. În
md-editor nu trebuie să tastezi aceste simboluri: le aplici din bara de
instrumente și, la salvare, editorul le generează pentru tine.

## Cuprins

- [Paragrafe și întreruperi de linie](#paragrafe-si-intreruperi-de-linie)
- [Titluri](#titluri)
- [Accentuare](#accentuare)
- [Liste](#liste)
- [Citate](#citate)
- [Cod](#cod)
- [Legături și imagini](#legaturi-si-imagini)
- [Linii orizontale](#linii-orizontale)
- [Tabele](#tabele)
- [Formule matematice](#formule-matematice)
- [Escape-uri](#escape-uri)

## Paragrafe și întreruperi de linie

Separă paragrafele cu o **linie goală**. În interiorul unui paragraf, două
spații la sfârșitul unei linii forțează o întrerupere fără a deschide un
paragraf nou.

## Titluri

```
# Titlu de nivel 1
## Titlu de nivel 2
### Titlu de nivel 3
```

Până la șase niveluri (`######`). În md-editor le poți aplica și din
**Format → Titlu** sau cu Ctrl+1 … Ctrl+6.

## Accentuare

- `*cursiv*` sau `_cursiv_` → *cursiv*
- `**îngroșat**` sau `__îngroșat__` → **îngroșat**
- `***îngroșat și cursiv***` → ***îngroșat și cursiv***
- `~~tăiat~~` → ~~tăiat~~

## Liste

**Marcatori** (cu `-`, `*` sau `+`):

```
- Măr
- Pară
  - Conference
  - Ercolini
```

**Numerotate**:

```
1. Primul
2. Al doilea
3. Al treilea
```

**Sarcini** (casete de bifare):

```
- [x] Făcut
- [ ] De făcut
```

## Citate

Una sau mai multe linii precedate de `>`:

```
> Cine citește mult și umblă mult vede mult și știe mult.
> — Miguel de Cervantes
```

## Cod

**În linie**: înconjoară cu un accent grav: `` `cod` ``.

**Bloc**: trei accente grave la început și la sfârșit; opțional, numele
limbajului pentru a-l colora:

````
```python
def salut(nume):
    print(f"Salut, {nume}")
```
````

## Legături și imagini

- **Legătură**: `[text](https://exemplu.ro)`
- **Legătură cu titlu**: `[text](https://exemplu.ro "Titlu sugestie")`
- **Imagine**: `![text alternativ](cale/imagine.png)` — la fel ca legătura,
  dar cu `!` în față.

În md-editor, **Ctrl+clic** pe o legătură o deschide în navigator.

## Linii orizontale

Trei sau mai multe cratime, asteriscuri sau liniuțe de subliniere pe o linie
proprie:

```
---
```

## Tabele

```
| Produs | Cantitate |  Preț  |
|--------|----------:|:------:|
| Pâine  |         2 | 6,20 lei|
| Lapte  |         1 | 4,95 lei|
```

Cele două puncte din linia de separare marchează alinierea coloanei: `:--`
stânga, `:-:` centru, `--:` dreapta. md-editor păstrează alinierea la salvare.

## Formule matematice

Markdown standard **nu** definește formule, dar există o convenție foarte
răspândită (Pandoc, Obsidian, Quarto, GitHub) pe care md-editor o acceptă:
sintaxa TeX între `$...$` (în linie) și `$$...$$` (în bloc).

```
Formula $E = mc^2$ este foarte celebră.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

Caracterele speciale ale TeX (`\`, `_`, `*`, `{`, `}`) se păstrează intacte în
interiorul formulelor — editorul le protejează pentru ca analizatorul Markdown
să nu le confunde cu cursive sau îngroșat.

În md-editor formulele apar randate cu superscripte și subscripte reale (nu ca
`$x^2$` literal). Inserează una cu **Inserare → Formulă…** (Ctrl+Shift+F) sau
dă dublu clic pe una existentă pentru a o edita.

## Escape-uri

Pentru ca un simbol Markdown să apară literal (fără a acționa ca formatare),
pune-i în față o bară oblică inversă: `\*nu e cursiv\*` → \*nu e cursiv\*.

Simbolurile care pot fi escapate sunt:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
