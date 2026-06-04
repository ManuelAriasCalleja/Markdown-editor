# Utilizare

## Deschidere și salvare

- Nou (Ctrl+N), Deschide (Ctrl+O), Salvează (Ctrl+S), Salvează ca (Ctrl+Shift+S).
  Totul în UTF-8.
- **Deschide recente** afișează ultimele tale documente.
- De asemenea, poți trage și plasa un fișier peste fereastră pentru a-l deschide.
- Dacă fișierul se schimbă în afara md-editor, ești avertizat: îl reîncarcă automat
  dacă nu aveai modificări, sau te întreabă dacă le aveai.

### Front matter

Dacă documentul tău începe cu un bloc `---…---` (YAML) sau `+++…+++` (TOML), acesta se
păstrează ca atare la salvare (nu se vede și nu se editează). Servește pentru metadate
precum `title` și `lang`, care se folosesc la export.

## Formatare

Folosește meniul Format sau bara de instrumente. Nu trebuie să tastezi simboluri
Markdown: editorul le aplică pentru tine.

- Îngroșat (Ctrl+B), Cursiv (Ctrl+I), Subliniat (Ctrl+U), Tăiat, Cod în linie,
  Legătură (Ctrl+K).
- Titluri H1–H6 (Ctrl+1 … Ctrl+6).
- Liste cu marcatori, numerotate și de sarcini, cu continuare automată la apăsarea
  tastei Enter (un punct gol iese din listă).
- Citate și blocuri de cod.

Consultă toate scurtăturile în [Scurtături de tastatură](Atajos-ro).

## Inserare

- Legătură și Imagine (cu cale relativă la document pentru a fi portabilă).
- **Lipește imaginea**: imaginea din clipboard se salvează ca PNG lângă fișierul tău
  `.md` și se inserează ca `![](ruta)`. Funcționează și prin tragere sau lipire peste editor.
- Tabel, Linie orizontală și Formulă (Ctrl+Shift+F).

## Tabele

Cu cursorul în interiorul unui tabel, meniul Tabel permite adăugarea sau eliminarea de
rânduri și coloane și alinierea fiecărei coloane (stânga/centru/dreapta). Alinierea se
păstrează la salvare.

## Formule

Inserează formule TeX în linie (`$...$`) sau în bloc (`$$...$$`) cu Inserare → Formulă
(Ctrl+Shift+F), cu previzualizare în timp real. Dublu clic pe o formulă o editează. Mai
multe detalii în [Caracteristici](Caracteristicas-ro#formule-tex).

## Moduri de vizualizare

- **WYSIWYG** (implicit): doar rezultatul randat.
- **Sursă Markdown** (Ctrl+Shift+M): codul Markdown brut, pe ecran complet.
- **Vizualizare divizată** (Ctrl+Shift+D): randare și cod unul lângă altul, sincronizate.

## Căutare și înlocuire

Ctrl+F pentru a căuta, Ctrl+H pentru a înlocui. Include anterior/următor, înlocuirea
tuturor și diferențierea majuscule/minuscule.

## Export și tipărire

Fișier → Exportă oferă PDF, HTML, ODF (.odt) și LaTeX (.tex); Tipărirea este Ctrl+P.
În ODF și LaTeX se încorporează limba documentului.

## Recuperare automată

md-editor salvează o ciornă la fiecare câteva secunde. Dacă aplicația se închide în mod
anormal, la redeschidere îți oferă posibilitatea de a recupera ceea ce scriai.
