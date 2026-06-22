# Utilizare

## Deschidere și salvare

- Nou (Ctrl+N), Deschide (Ctrl+O), Salvează (Ctrl+S), Salvează ca (Ctrl+Shift+S).
  Totul în UTF-8.
- **File**: fiecare document deschis ocupă propria filă; închizi una cu Ctrl+W. La
  repornire se redeschid filele din ultima sesiune.
- **Nou din șablon** (Fișier → Nou din șablon) pornește de la un schelet Markdown deja
  pregătit.
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
  tastei Enter (un punct gol iese din listă). Casetele de sarcină se bifează cu un clic.
- Citate și blocuri de cod.

Consultă toate scurtăturile în [Scurtături de tastatură](Atajos-ro).

## Editare și transformare a textului

- **Lipește ca text simplu** (Ctrl+Shift+V) sau **Lipește ca Markdown** (Ctrl+Alt+V),
  care convertește HTML-ul din clipboard în Markdown. Lipirea unui URL peste o
  selecție o transformă automat în legătură.
- **Editare → Transformă textul**: MAJUSCULE, minuscule, capitalizare, sortarea liniilor
  și tipografie inteligentă (convertește `--`, `---`, `...` și ghilimelele drepte).

## Inserare

- Legătură și Imagine (cu cale relativă la document pentru a fi portabilă).
- **Lipește imaginea**: imaginea din clipboard se salvează ca PNG lângă fișierul tău
  `.md` și se inserează ca `![](ruta)`. Funcționează și prin tragere sau lipire peste editor.
- Tabel, Linie orizontală, Cuprins (TOC) și Formulă (Ctrl+Shift+F).
- **Notă de subsol** (Ctrl+Shift+N): inserează o referință `[^n]` și definiția ei.
- **Avertisment**: bloc evidențiat (notă, sfat, important, atenționare, precauție).
- **Simboluri speciale** și **Dată / Dată și oră**.

## Tabele

Cu cursorul în interiorul unui tabel, meniul Tabel permite adăugarea sau eliminarea de
rânduri și coloane și alinierea fiecărei coloane (stânga/centru/dreapta). Alinierea se
păstrează la salvare.

## Formule

Inserează formule TeX în linie (`$...$`) sau în bloc (`$$...$$`) cu Inserare → Formulă
(Ctrl+Shift+F), cu previzualizare în timp real. Dublu clic pe o formulă o editează. Se
desenează în 2D real (fracții, radicali, matrici, sumatorii cu limite…). Mai multe
detalii în [Caracteristici](Caracteristicas-ro#formule-tex).

## Diagrame

Scrie un bloc de cod cu limbajul `mermaid` sau `plantuml` și, dacă ai instalat
instrumentul corespunzător (`mmdc` / `plantuml`), se randează ca imagine sub bloc. Dacă
lipsește, vei vedea comanda pentru a-l instala.

## Corector ortografic

Activează-l în Vizualizare → Corector ortografic (necesită Hunspell). Limba se alege
după cea a documentului sau manual în Vizualizare → Limba corectorului. Clic dreapta pe
un cuvânt subliniat oferă sugestii și adăugarea lui la dicționarul personal.

## Moduri de vizualizare

- **WYSIWYG** (implicit): doar rezultatul randat.
- **Sursă Markdown** (Ctrl+Shift+M): codul Markdown brut, pe ecran complet.
- **Vizualizare divizată** (Ctrl+Shift+D): randare și cod unul lângă altul, sincronizate.
- **Schiță** (F9) și **Mergi la titlu** (Ctrl+G) pentru a naviga documentul.

## Căutare și înlocuire

Ctrl+F pentru a căuta, Ctrl+H pentru a înlocui. Include anterior/următor, înlocuirea
tuturor și diferențierea majuscule/minuscule.

## Export și tipărire

Fișier → Exportă oferă PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) și EPUB
(.epub); de asemenea Previzualizare a tipăririi și Tipărește (Ctrl+P). În ODF, DOCX și
LaTeX se încorporează limba documentului.

## Recuperare automată

md-editor salvează o ciornă la fiecare câteva secunde. Dacă aplicația se închide în mod
anormal, la redeschidere îți oferă posibilitatea de a recupera ceea ce scriai.
