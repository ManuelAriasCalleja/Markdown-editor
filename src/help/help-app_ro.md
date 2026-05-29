# Manual de utilizare

**md-editor** este un editor vizual (WYSIWYG) de Markdown: scrii și aplici
formatare peste textul deja randat, fără să vezi codul. La salvare, documentul
este serializat înapoi în Markdown pur.

## Cuprins

- [Deschiderea și salvarea](#deschiderea-si-salvarea)
- [Formatarea textului](#formatarea-textului)
- [Titluri, liste și blocuri](#titluri-liste-si-blocuri)
- [Legături și imagini](#legaturi-si-imagini)
- [Tabele](#tabele)
- [Formule matematice](#formule-matematice)
- [Caută și înlocuiește](#cauta-si-inlocuieste)
- [Schița documentului](#schita-documentului)
- [Mod fără distrageri](#mod-fara-distrageri)
- [Vizualizarea sursei](#vizualizarea-sursei)
- [Export și tipărire](#export-si-tiparire)
- [Teme și aspect](#teme-si-aspect)
- [Recuperare automată](#recuperare-automata)
- [Scurtături](#scurtaturi)

## Deschiderea și salvarea

- **Fișier → Nou** (Ctrl+N) creează un document gol.
- **Fișier → Deschide…** (Ctrl+O) deschide un `.md` existent. Aplicația
  reține cele mai recente fișiere în **Fișier → Deschide recente**.
- **Salvează** (Ctrl+S) și **Salvează ca…** (Ctrl+Shift+S) scriu documentul
  în UTF-8.
- Dacă fișierul se modifică în afara editorului, aplicația detectează acest
  lucru și, dacă nu ai modificări nesalvate, îl reîncarcă; dacă ai, te
  întreabă ce să facă.
- De asemenea, poți **trage și plasa** un fișier peste fereastră pentru a-l
  deschide.

### *Front matter*

Dacă documentul începe cu un bloc `---…---` (YAML) sau `+++…+++` (TOML),
acesta este păstrat ca atare la salvare: nu se afișează în editor și nu se
editează. Servește pentru metadate precum `title`, `lang` etc., care sunt
folosite la export.

## Formatarea textului

Selectează un fragment și aplică formatarea din bara de instrumente sau din
meniul **Format**:

- **Îngroșat** (Ctrl+B), **Cursiv** (Ctrl+I), **Subliniat** (Ctrl+U),
  **Tăiat**.
- **Cod în linie** pentru fragmente `monospațiate`.
- **Legătură**: adaugă `[text](url)` peste selecție.

Butoanele din bară reflectă formatarea activă de sub cursor.

## Titluri, liste și blocuri

- **Titluri** H1–H6 din **Format → Titlu** sau cu Ctrl+1 … Ctrl+6.
- **Liste**: cu marcatori, numerotate și de sarcini (cu casetă de bifare).
  Apăsând Enter la sfârșitul unui element se creează automat următorul;
  apăsând Enter pe un element gol se iese din listă.
- **Citatul** (`>` la începutul unui paragraf) și **blocul de cod** se aplică
  din bară; ambele fac round-trip corect către Markdown.

## Legături și imagini

- **Inserare → Legătură…** deschide un dialog cu câmpuri pentru text și URL.
  Dacă aveai o selecție, aceasta este folosită ca text.
- **Ctrl+clic** pe o legătură o deschide în navigatorul sistemului; trecând
  cursorul peste ea se afișează URL-ul în bara de stare.
- **Imagini**: trage un fișier, lipește o imagine din clipboard sau folosește
  **Inserare → Lipește imaginea**. Imaginea se salvează ca PNG lângă `.md` și
  se inserează ca `![alt](cale-relativă)`; astfel supraviețuiește round-trip-ului
  către Markdown (imaginile încorporate, nu).

## Tabele

- **Tabel → Inserează tabel…** cere numărul de rânduri și coloane.
- Acțiunile din meniul **Tabel** (adăugare/ștergere rând sau coloană,
  alinierea coloanei) sunt activate doar când cursorul se află într-un tabel.
- Alinierea coloanei (stânga/centru/dreapta) se păstrează la salvare ca
  `:--`/`:-:`/`--:`.

## Formule matematice

md-editor acceptă **formule TeX** în linie (`$...$`) și în bloc (`$$...$$`),
cu sintaxa obișnuită LaTeX (Pandoc, Obsidian, Quarto…). Nu este nevoie de
nicio dependență externă.

- **Inserare → Formulă…** (Ctrl+Shift+F) deschide un dialog cu un câmp pentru
  TeX și o **previzualizare în timp real**: pe măsură ce scrii vezi cum va
  arăta. Alege *În linie* sau *Bloc* și acceptă pentru a o insera.
- În editor, formulele apar cu cursive și culoarea de accent a temei, cu
  **superscripte și subscripte reale** (nu caractere Unicode plate): `x²`,
  `Hᵢ` ș.a.m.d. — vertical-align-ul Qt scalează corect orice caracter.
- **Dublu clic** pe o formulă redeschide dialogul cu TeX-ul ei original
  preîncărcat: o editezi și, la acceptare, se înlocuiește.
- Formulele sunt **atomice**: dacă tastezi în interiorul uneia, aplicația îți
  amintește să folosești dublul clic; Backspace/Delete la marginea ei șterge
  întregul grup.
- La **export** se păstrează: în LaTeX se emit ca atare (cu `amsmath` și
  `amssymb` în preambul); în HTML/PDF/ODF se păstrează superscriptele/subscriptele
  cu vertical-align ale Qt în formatul de destinație.
- În **vizualizarea sursei** apar ca `$...$` / `$$...$$`, cu toate caracterele
  TeX (`\sum`, `\frac`, `_`, `*`) intacte la salvare.

Exemple:

```
Energia este $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitare: în sursă, `$$...$$` poate să se întindă pe mai multe linii (stil
> Obsidian/Pandoc); `$...$` trebuie să se deschidă și să se închidă pe aceeași
> linie.

## Caută și înlocuiește

- **Caută** (Ctrl+F) deschide o bară inferioară cu câmpuri pentru căutare și
  înlocuire, plus opțiuni (majuscule, cuvânt întreg).
- **Caută următorul** F3 / **Caută anteriorul** Shift+F3.

## Schița documentului

Panoul lateral din stânga afișează indexul titlurilor (TOC): se actualizează
pe măsură ce scrii și, dând clic pe o intrare, cursorul sare la acel titlu. Se
afișează/ascunde cu F9.

## Mod fără distrageri

**Vizualizare → Mod fără distrageri** (F11) intră pe tot ecranul, cu meniul și
barele ascunse și textul centrat într-o coloană de lectură. Schița, dacă este
vizibilă, rămâne lipită de blocul central. ESC sau F11 ies.

## Vizualizarea sursei

**Vizualizare → Sursă Markdown** (Ctrl+Shift+M) comută între editorul vizual și
un editor de text simplu, pe tot ecranul, cu Markdown-ul brut. Modificările din
modul sursă se transferă în document la revenirea în modul vizual.

**Vizualizare → Vizualizare divizată** (Ctrl+Shift+D) afișează ambele simultan,
unul lângă altul: editorul vizual și sursa, sincronizate (ce scrii într-unul se
reflectă în celălalt). Este reciproc exclusivă cu modul sursă pe tot ecranul.

## Export și tipărire

**Fișier → Exportă** oferă **PDF**, **HTML**, **ODF (.odt)** și
**LaTeX (.tex)**. Pentru ODF și LaTeX se încorporează limba documentului
(preluată din front matter `lang`/`language`, din setarea aplicației sau, în
ultimă instanță, din limba sistemului).

**Fișier → Tipărește** (Ctrl+P) deschide dialogul sistemului.

## Teme și aspect

- **Vizualizare → Temă** oferă Luminoasă, Întunecată, GitHub Light, GitHub
  Dark, Monokai și Contrast ridicat.
- **Vizualizare → Lumină caldă nocturnă** atenuează albastrurile din fundal în
  funcție de ora zilei.
- **Zoom**: Ctrl+rotița mouse-ului, Ctrl++ / Ctrl+- și **Dimensiune normală**
  (Ctrl+0) scalează întreaga interfață (nu doar textul editorului).
- **Vizualizare → Limbă** schimbă limba interfeței; se aplică la repornire.

## Recuperare automată

În timp ce editezi, conținutul se salvează automat la fiecare câteva secunde
într-o copie ciornă. Dacă aplicația se închide anormal, la următoarea pornire
oferă recuperarea a ceea ce scriai.

## Scurtături

| Acțiune                   | Scurtătură       |
|---------------------------|------------------|
| Nou                       | Ctrl+N           |
| Deschide                  | Ctrl+O           |
| Salvează                  | Ctrl+S           |
| Salvează ca               | Ctrl+Shift+S     |
| Tipărește                 | Ctrl+P           |
| Anulează / Refă           | Ctrl+Z / Ctrl+Y  |
| Îngroșat / Cursiv         | Ctrl+B / Ctrl+I  |
| Subliniat                 | Ctrl+U           |
| Caută                     | Ctrl+F           |
| Caută următorul/anteriorul| F3 / Shift+F3    |
| Titlu H1 … H6             | Ctrl+1 … Ctrl+6  |
| Inserează formulă         | Ctrl+Shift+F     |
| Vizualizare sursă Markdown| Ctrl+Shift+M     |
| Vizualizare divizată      | Ctrl+Shift+D     |
| Schiță                    | F9               |
| Mod fără distrageri       | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ajutor                    | F1               |
