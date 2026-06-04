# Caracteristici

Rezumat al tot ceea ce oferă md-editor. Pentru referința completă și tehnică,
consultă `especificacion.md` în depozit.

## Editare WYSIWYG și round-trip

Editezi direct pe textul randat și, la salvare, se serializează în Markdown curat în
UTF-8. Ceea ce deschizi este ceea ce salvezi: tabelele cu aliniere, listele imbricate,
listele de sarcini, citatele, blocurile de cod și formulele se păstrează fidel.

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

Panou lateral (F9) cu indexul titlurilor; un clic sare la secțiune.

## Formule TeX

Formule în linie (`$...$`) și în bloc (`$$...$$`) cu sintaxă LaTeX, fără dependențe
externe:

- Inserare cu previzualizare în timp real (Ctrl+Shift+F) și editare cu dublu clic.
- Superscripte și subscripte reale, litere grecești, operatori, `\frac`, `\sqrt`, `\mathbb`…
- Sunt atomice în editor și supraviețuiesc round-trip-ului și exportului.
- Limitări: `$...$` trebuie să se deschidă și să se închidă pe aceeași linie; nu există
  *layout* 2D (fracții mari precum `(a)/(b)`).

## Evidențierea sintaxei

Blocurile de cod se colorează în funcție de limbajul lor (familiile C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… și un mod generic).

## Imagini

Lipirea sau plasarea unei imagini o salvează ca PNG lângă document și o inserează ca
`![](ruta)` —nu o încorporează—, astfel încât Markdown-ul rămâne portabil.

## Export și tipărire

PDF, HTML, ODF (.odt) și LaTeX (.tex), plus tipărire (Ctrl+P). ODF și LaTeX încorporează
limba documentului (din front matter, din setarea aplicației sau din sistem).

## Zoom pentru întreaga interfață

Ctrl++, Ctrl+- și Ctrl+0 (sau Ctrl + rotiță) scalează întreaga interfață, nu doar textul
editorului. Nivelul este reținut.

## Căutare și înlocuire

Ctrl+F / Ctrl+H, cu anterior/următor, înlocuirea tuturor și diferențierea
majuscule/minuscule.

## Fișiere și securitatea datelor tale

- **Fișiere recente**, deschidere prin tragere și confirmarea modificărilor nesalvate.
- **Front matter** YAML/TOML păstrat verbatim.
- **Supravegherea fișierului pe disc**: detectează modificările externe și oferă reîncărcarea.
- **Salvare automată și recuperare** după o închidere anormală.

## Internaționalizare

Interfață în 9 limbi: spaniolă, engleză, germană, franceză, italiană, portugheză, poloneză,
neerlandeză și română (Vizualizare → Limbă; se aplică la repornire).
