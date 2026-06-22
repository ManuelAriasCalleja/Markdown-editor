# Caracteristici

Rezumat al tot ceea ce oferă md-editor. Pentru referința completă și tehnică,
consultă `especificacion.md` în depozit.

## Editare WYSIWYG și round-trip

Editezi direct pe textul randat și, la salvare, se serializează în Markdown curat în
UTF-8. Ceea ce deschizi este ceea ce salvezi: tabelele cu aliniere, listele imbricate,
listele de sarcini, citatele, blocurile de cod, notele de subsol, avertismentele și
formulele se păstrează fidel.

## Editare pe file

Deschide mai multe documente simultan, fiecare în fila lui, și comută între ele. Închizi
o filă cu Ctrl+W. Sesiunea redeschide filele la repornire.

## Moduri de vizualizare

- WYSIWYG, Sursă Markdown (Ctrl+Shift+M) și Vizualizare divizată (Ctrl+Shift+D).
- În vizualizarea divizată, randarea și codul se sincronizează: se actualizează doar
  panoul pe care nu îl editezi, fără salturi de cursor.

## Mod fără distrageri

F11 intră pe ecran complet cu textul centrat într-o coloană de lectură și fără bare.
ESC sau F11 ies.

## Teme și lumină caldă nocturnă

- **Șase teme**: Luminoasă, Întunecată, GitHub Light, GitHub Dark, Monokai și Contrast ridicat.
- **Lumina caldă nocturnă** (activată implicit): atenuează albastrul fundalului în mod
  automat și gradual în funcție de oră, pentru a reduce oboseala vizuală pe timpul nopții.
  Neutră ziua (07–19 h), se încălzește seara (19–23 h), maximă noaptea (23–06 h) și se
  răcește în zori (06–07 h). Se reevaluează singură la fiecare minut și afectează doar
  fundalul (nu și legăturile sau evidențierea).

## Schița documentului

Panou lateral (F9) cu indexul titlurilor; un clic sare la secțiune. „Mergi la titlu”
(Ctrl+G) deschide un căutător rapid de titluri.

## Formule TeX

Formule în linie (`$...$`) și în bloc (`$$...$$`) cu sintaxă LaTeX, fără dependențe
externe:

- Inserare cu previzualizare în timp real (Ctrl+Shift+F) și editare cu dublu clic.
- **Layout 2D real**: fracții stivuite (`\frac`), radicali cu vinculum (`\sqrt`),
  coeficienți binomiali (`\binom`), matrici și medii (`matrix`, `pmatrix`, `cases`…),
  operatori mari cu limite deasupra și dedesubt (`\sum`, `\int`, `\prod`…), accente
  (`\hat`, `\vec`…), superscripte și subscripte reale, litere grecești și `\mathbb`.
- Sunt atomice în editor, scalează cu zoom-ul și supraviețuiesc round-trip-ului și
  exportului. Blocurile `$$...$$` pot ocupa mai multe linii.
- Limitări: `$...$` trebuie să se deschidă și să se închidă pe aceeași linie; formulele
  2D în linie rămân puțin înalte (cele în bloc se văd bine).

## Corector ortografic (opțional)

Subliniază cuvintele scrise greșit în funcție de limba documentului (Vizualizare →
Corector ortografic). Limba se alege singură (front matter, setare sau sistem) sau
manual (Vizualizare → Limba corectorului). Clic dreapta oferă sugestii și adăugarea la
dicționarul personal. Necesită Hunspell; fără el, restul funcționează la fel.

## Diagrame (opțional)

Blocurile ```` ```mermaid ```` și ```` ```plantuml ```` se randează ca imagine sub
bloc, executând instrumentul extern (`mmdc` / `plantuml`) dacă este instalat. Dacă
lipsește, se afișează comanda de instalare pentru sistemul tău. Imaginea nu se salvează
în Markdown.

## Evidențierea sintaxei

Blocurile de cod se colorează în funcție de limbajul lor (familiile C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… și un mod generic).

## Imagini

Lipirea sau plasarea unei imagini o salvează ca PNG lângă document și o inserează ca
`![](ruta)` —nu o încorporează—, astfel încât Markdown-ul rămâne portabil.

## Inserare și transformare

- Inserare: legătură, imagine, tabel, linie, cuprins (TOC), formulă, notă de subsol,
  avertisment (notă/atenționare…), simboluri speciale și dată/oră.
- Lipește ca Markdown (Ctrl+Alt+V) convertește HTML-ul din clipboard în Markdown.
- Transformă textul: MAJUSCULE/minuscule, capitalizare, sortarea liniilor și tipografie
  inteligentă (—, –, …, ghilimele tipografice).
- Statistici ale documentului: cuvinte, caractere, paragrafe, propoziții și timp de
  citire.

## Export și tipărire

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) și EPUB (.epub), plus previzualizare
a tipăririi și tipărire (Ctrl+P). ODF, DOCX și LaTeX încorporează limba documentului
(din front matter, din setarea aplicației sau din sistem).

## Zoom pentru întreaga interfață

Ctrl++, Ctrl+- și Ctrl+0 (sau Ctrl + rotiță) scalează întreaga interfață, nu doar textul
editorului. Nivelul este reținut.

## Căutare și înlocuire

Ctrl+F / Ctrl+H, cu anterior/următor, înlocuirea tuturor și diferențierea
majuscule/minuscule.

## Fișiere și securitatea datelor tale

- **Fișiere recente**, deschidere prin tragere și confirmarea modificărilor nesalvate.
- **Șabloane de document** (Fișier → Nou din șablon).
- **Front matter** YAML/TOML păstrat verbatim.
- **Supravegherea fișierului pe disc**: detectează modificările externe și oferă reîncărcarea.
- **Salvare automată și recuperare** după o închidere anormală.

## Internaționalizare

Interfață în 9 limbi: spaniolă, engleză, germană, franceză, italiană, portugheză, poloneză,
neerlandeză și română (Vizualizare → Limbă; se aplică la repornire).
