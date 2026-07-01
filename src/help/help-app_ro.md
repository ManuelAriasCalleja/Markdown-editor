# Manual de utilizare

**md-editor** este un editor vizual (WYSIWYG) de Markdown: scrii și aplici
formatare pe textul deja randat, fără a vedea codul. La salvare, documentul este
serializat înapoi în Markdown pur.

## Cuprins

- [Deschidere și salvare](#deschidere-si-salvare)
- [Formatarea textului](#formatarea-textului)
- [Titluri, liste și blocuri](#titluri-liste-si-blocuri)
- [Transformarea textului și clipboardul](#transformarea-textului-si-clipboardul)
- [Linkuri și imagini](#linkuri-si-imagini)
- [Note de subsol](#note-de-subsol)
- [Casete, simboluri și scurtături de text](#casete-simboluri-si-scurtaturi-de-text)
- [Snippeturi (fragmente reutilizabile)](#snippeturi-fragmente-reutilizabile)
- [Tabele](#tabele)
- [Formule matematice](#formule-matematice)
- [Diagrame](#diagrame)
- [Corectare ortografică](#corectare-ortografica)
- [Căutare și înlocuire](#cautare-si-inlocuire)
- [Structura documentului](#structura-documentului)
- [Statistici despre document](#statistici-despre-document)
- [Mod fără distrageri](#mod-fara-distrageri)
- [Mod de concentrare](#mod-de-concentrare)
- [Vizualizarea codului](#vizualizarea-codului)
- [Export și tipărire](#export-si-tiparire)
- [Teme și aspect](#teme-si-aspect)
- [Recuperare automată](#recuperare-automata)
- [Accesibilitate](#accesibilitate)
- [Scurtături](#scurtaturi)

## Deschidere și salvare

- **Fișier → Nou** (Ctrl+N) creează un document gol într-o filă nouă.
- **Fișier → Nou din șablon** creează un document pornind de la un schelet
  (scrisoare, proces-verbal, examen…) gata de completat.
- **Fișier → Deschide…** (Ctrl+O) deschide un `.md` existent. Aplicația reține
  ultimele deschise în **Fișier → Deschise recent**.
- **Fișier → Importă → Din HTML…** convertește o pagină HTML în Markdown și o
  deschide ca document nou fără titlu (fișierul original nu este modificat).
  Funcționează cel mai bine cu HTML simplu; respectă setul de caractere declarat în
  pagină.
- **Salvează** (Ctrl+S) și **Salvează ca…** (Ctrl+Shift+S) scriu documentul în
  UTF-8. **Deschide folderul documentului** deschide folderul documentului în
  managerul de fișiere.
- **Revino la versiunea salvată** renunță la modificările nesalvate și reîncarcă
  fișierul de pe disc (cere confirmare). Disponibil doar dacă documentul are fișier
  și modificări în așteptare.
- Dacă fișierul se schimbă în afara editorului, aplicația detectează asta și, dacă
  nu ai modificări nesalvate, îl reîncarcă; dacă ai, întreabă ce să facă.
- Poți de asemenea **trage și plasa** un fișier pe fereastră pentru a-l deschide.

### File (mai multe documente)

Poți avea mai multe documente deschise în același timp, fiecare în **fila** sa:

- **Nou** (Ctrl+N), **Nou din șablon** și **Deschide** (Ctrl+O) creează o filă (sau
  refolosesc fila goală inițială). Plasarea unui fișier îl deschide tot într-o
  filă; dacă este deja deschis, sare la fila lui.
- Schimbă documentul făcând clic pe fila sa; trage filele pentru a le reordona. De
  la tastatură, **Ctrl+PageDown / Ctrl+PageUp** (sau **Ctrl+Tab / Ctrl+Shift+Tab**)
  sar la fila următoare sau anterioară.
- **Închide fila** (Ctrl+W) o închide pe cea curentă, întrebând dacă are modificări
  nesalvate. Ultima filă nu se închide: devine un document nou.
- **Redeschide fila închisă** (Ctrl+Shift+R) redeschide ultima filă închisă (doar
  cele care aveau un fișier pe disc).
- Eticheta arată numele fișierului și un punct (•) când există modificări
  nesalvate.
- La închiderea aplicației, documentele deschise sunt reținute și toate redeschise
  la următoarea pornire.

### *Front matter*

Dacă documentul începe cu un bloc `---…---` (YAML) sau `+++…+++` (TOML), acesta
este păstrat ca atare la salvare: nu se vede în editor și nu se editează. Servește
pentru metadate precum `title`, `lang` etc., folosite la export.

## Formatarea textului

Selectează un fragment și aplică formatarea din bara de instrumente sau din meniul
**Format**:

- **Aldin** (Ctrl+B), **Cursiv** (Ctrl+I), **Subliniat** (Ctrl+U), **Tăiat**.
- **Cod în linie** pentru fragmente `monospațiate`.
- **Link**: adaugă `[text](url)` peste selecție.
- **Evidențiază** (Ctrl+Shift+H): încadrează selecția în `==marcaj==`; textul apare
  cu fundal de evidențiere. Deoarece `==` nu este sintaxă Markdown standard, se
  salvează ca text literal.
- **Exponent** (Ctrl+Shift++) și **indice** (Ctrl+Shift+-): ridică sau coboară textul
  selectat; salvate ca `^text^` și `~text~` (stil Pandoc).

Butoanele din bară reflectă formatarea activă sub cursor.

**Împerechere automată.** Când tastezi `(`, `[`, `{` sau `` ` ``, perechea se
închide singură și cursorul rămâne la mijloc; dacă ai text selectat, acesta este
încadrat. Dacă tastezi caracterul de închidere chiar în fața perechii sale,
editorul îl „sare” în loc să-l dubleze.

**Reguli de introducere.** La începutul unui rând, tastarea unui marcaj Markdown de
bloc urmat de un spațiu transformă rândul pe loc (fără a lăsa marcajul): `#` …
`######` + spațiu → titlu H1…H6; `>` → citat; `-`, `*` sau `+` → listă cu
marcatori; `1.` (sau `1)`) → listă numerotată. Produce același format ca bara.

## Titluri, liste și blocuri

- **Titluri** H1–H6 din **Format → Titlu** sau cu Ctrl+1 … Ctrl+6.
  **Promovează/retrogradează** titlul de la cursor cu un nivel prin
  Ctrl+Shift+[ / Ctrl+Shift+].
- **Liste**: cu marcatori, numerotate și de sarcini (cu casetă). Enter la sfârșitul
  unui punct creează automat următorul; Enter pe un punct gol iese din listă. Un
  **clic pe caseta** unei sarcini o bifează sau o debifează.
- **Citat** (`>` la începutul unui paragraf) și **bloc de cod** se aplică din bară;
  ambele revin corect la Markdown. Cu **Format → Limbajul blocului…** alegi
  limbajul unui bloc de cod (având cursorul în interior) pentru a-i evidenția
  sintaxa.
- **Indentare**: **Format → Mărește/Micșorează indentarea** imbrică listele și
  citatele.

## Transformarea textului și clipboardul

- **Editare → Transformă textul** acționează asupra selecției: **MAJUSCULE**,
  **minuscule**, **Capitalizează** și **Sortează liniile**.
- **Tipografie inteligentă** (în același meniu) convertește în selecție liniuțele
  `--`/`---` în `–`/`—`, `...` în `…` și ghilimelele drepte în ghilimele
  tipografice în funcție de context.
- **Lipește ca text simplu** (Ctrl+Shift+V) lipește fără formatare. **Lipește ca
  Markdown** (Ctrl+Alt+V) convertește conținutul formatat din clipboard (HTML) în
  Markdown în loc să încorporeze formatarea sursei.
- **Copiază ca HTML** copiază selecția (sau documentul) ca HTML, pentru a o lipi
  într-un e-mail, un CMS etc.
- **Copiază ca Markdown** copiază selecția (sau întregul document) ca text
  Markdown, pentru a o lipi în alt editor de Markdown sau într-un câmp de cod.
- Când lipești o **adresă URL** peste o selecție de text, textul devine link
  automat.
- **Editare → Curăță Markdown** normalizează tot documentul dintr-o singură
  trecere: uniformizează marcatorii la `-`, taie spațiile de la sfârșitul fiecărei
  linii, comprimă liniile goale în plus și ajustează spațiul de după `#`-urile
  titlurilor. Este conservatoare: nu atinge interiorul blocurilor de cod.

## Linkuri și imagini

- **Inserare → Link…** deschide o fereastră cu text și URL. O selecție existentă
  este folosită ca text.
- **Ctrl+clic** pe un link îl deschide în browserul sistemului; la trecerea cu
  mouse-ul, URL-ul apare în bara de stare.
- **Imagini**: trage un fișier, lipește o imagine din clipboard sau folosește
  **Inserare → Lipește imagine**. Imaginea este salvată ca PNG lângă `.md` și
  inserată ca `![alt](cale-relativă)`; astfel supraviețuiește conversiei dus-întors
  în Markdown (imaginile încorporate nu).

## Note de subsol

- **Inserare → Notă de subsol** (Ctrl+Shift+N) inserează la cursor o referință
  numerotată `[^n]` și creează definiția ei `[^n]:` la sfârșitul documentului,
  gata pentru textul notei.
- Referințele sunt afișate ca **exponent**; un **clic** pe una mută cursorul la
  definiția ei.
- Sunt salvate ca Markdown standard (`text[^1]` în corp și, dedesubt,
  `[^1]: nota`), deci sunt compatibile cu alte editoare.

## Casete, simboluri și scurtături de text

- **Inserare → Casetă** creează un *callout* în stil GitHub: un citat a cărui primă
  linie este `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` sau `[!CAUTION]`.
  Este afișat cu fundal colorat și titlu colorat și salvat ca Markdown compatibil
  cu GitHub.
- **Inserare → Simboluri speciale…** deschide o hartă de caractere pe categorii
  (matematice, grecești, săgeți, monedă, punctuație…); un clic inserează simbolul,
  iar fereastra rămâne deschisă pentru a insera mai multe.
- **Scurtături `:nume:`**: tastând un cod precum `:alpha:` sau `:euro:`, acesta este
  extins la simbolul corespunzător (α, €…).
- **Inserare → Dată** și **Dată și oră** inserează data (și ora) curentă în format
  localizat.

## Snippeturi (fragmente reutilizabile)

Un **snippet** este o bucată de Markdown pe care o salvezi cu un nume pentru a o
insera apoi cu câteva clicuri: o semnătură, un șablon de tabel, un avertisment pe
care îl repeți des…

- **Inserare → Snippet** desfășoară lista celor pe care îi ai; la alegerea unuia,
  conținutul lui se inserează acolo unde este cursorul (funcționează și în
  vizualizarea codului).
- **Inserare → Snippet → Gestionează snippeturile…** deschide o fereastră pentru a
  crea, edita și șterge snippeturile tale. Fiecare are un **nume** (cel pe care îl
  vezi în meniu) și un **corp** în Markdown.
- Sunt salvate în setările aplicației, așa că sunt disponibile în toate
  documentele tale, nu doar în cel curent.

## Tabele

- **Tabel → Inserează tabel…** cere rânduri și coloane.
- **Inserare → Tabel din clipboard** transformă într-un tabel datele TSV/CSV
  (coloane separate prin tabulatori sau virgule) copiate dintr-o foaie de calcul sau
  dintr-un fișier CSV.
- Acțiunile meniului **Tabel** (adaugă/elimină rând sau coloană, aliniază coloana)
  sunt active doar când cursorul este într-un tabel.
- Alinierea coloanei (stânga/centru/dreapta) este păstrată la salvare ca
  `:--`/`:-:`/`--:`.
- **Tabel → Sortează rândurile după coloană** (crescător/descrescător) reordonează
  rândurile după coloana cursorului, păstrând antetul fix; detectează dacă coloana
  este numerică sau text.

## Formule matematice

md-editor acceptă **formule TeX** în linie (`$...$`) și în bloc (`$$...$$`), cu
sintaxa LaTeX obișnuită (Pandoc, Obsidian, Quarto…). Nu este necesară nicio
dependență externă.

- **Inserare → Formulă…** (Ctrl+Shift+F) deschide o fereastră cu un câmp pentru TeX
  și o **previzualizare în timp real**: pe măsură ce scrii vezi rezultatul. Alege
  *În linie* sau *Bloc* și confirmă pentru a o insera.
- Formulele sunt aranjate în **2D real**: fracțiile (`\frac`) sunt stivuite cu o
  bară, operatorii mari (`\sum`, `\int`, `\prod`…) își arată limitele deasupra și
  dedesubt, radicalii (`\sqrt`) își poartă bara, iar există matrice
  (`\begin{pmatrix}`…), coeficienți binomiali (`\binom`) și accente (`\hat`,
  `\vec`, `\bar`…). Cele mai simple (puteri, indici, greacă) sunt compuse în linie.
  Desenul se scalează cu zoomul.
- **Dublu clic** pe o formulă redeschide fereastra cu TeX-ul original preîncărcat:
  o editezi și, la confirmare, este înlocuită.
- Formulele sunt **atomice**: dacă tastezi în interior, aplicația îți amintește să
  folosești dublul clic; Backspace/Delete pe margine șterg întregul grup.
- La **export** sunt păstrate: în LaTeX sunt emise ca atare (cu `amsmath` și
  `amssymb` în preambul); în HTML/PDF/ODF sunt reduse la aproximarea lor în linie.
- În **vizualizarea codului** apar ca `$...$` / `$$...$$`, cu toate caracterele TeX
  (`\sum`, `\frac`, `_`, `*`) intacte la salvare.

Exemple:

```
Energia este $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> În sursă, `$$...$$` poate cuprinde mai multe linii (stil Obsidian/Pandoc);
> `$...$` trebuie să se deschidă și să se închidă pe aceeași linie.

## Diagrame

Un bloc de cod cu limbajul `mermaid` sau `plantuml` este **previzualizat ca
imagine** chiar sub bloc, fără a atinge codul (care rămâne editabil) sau Markdown-ul
salvat.

- Necesită instalarea instrumentului corespunzător: **`plantuml`** (cu Java) pentru
  PlantUML sau **`mmdc`** (mermaid-cli, cu Node) pentru Mermaid.
- Dacă instrumentul lipsește, sub bloc apare un avertisment cu comanda de instalare
  pentru sistemul tău de operare; blocul rămâne cod.
- Imaginea este doar prezentare: nu este scrisă în Markdown și nu contează ca
  modificare nesalvată.

De exemplu, un bloc de cod etichetat `mermaid` care conține `flowchart LR  A --> B
--> C` este previzualizat ca diagrama de flux corespunzătoare.

## Corectare ortografică

- Subliniază cu roșu cuvintele scrise greșit în funcție de **limba documentului**
  (din front matter `lang`, din setarea de limbă sau din sistem). Nu verifică
  codul, formulele sau linkurile.
- **Clic dreapta** pe un cuvânt subliniat oferă **sugestii** (un clic îl
  înlocuiește), **Adaugă în dicționar** (o listă personală permanentă) și
  **Ignoră** (pe durata sesiunii).
- Se activează/dezactivează din **Vizualizare → Corectare ortografică**, iar limba
  se stabilește din **Vizualizare → Limba de corectare** (sau se lasă automată).
- Are nevoie de dicționare Hunspell: pe Linux, cele ale sistemului (`hunspell-es`,
  `hunspell-en-us`…); pe Windows/macOS sunt livrate cu aplicația.

## Căutare și înlocuire

- **Caută** (Ctrl+F) deschide o bară jos cu câmpuri pentru căutare și înlocuire,
  plus opțiuni (majuscule/minuscule, cuvânt întreg).
- **Caută următorul** F3 / **Caută anteriorul** Shift+F3.
- Bara evidențiază **toate** potrivirile din document și afișează un contor **„N din M”** (pe ce potrivire ești, din câte). **Înlocuiește tot** le înlocuiește pe toate deodată.

## Structura documentului

Panoul lateral din stânga arată structura titlurilor (cuprins): se actualizează pe
măsură ce scrii și, la clic pe o intrare, cursorul sare la acel titlu. Se
afișează/ascunde cu F9. Cu **F6** muți focalizarea tastaturii pe
structură (afișând-o dacă este ascunsă); acolo, tastele săgeți se deplasează prin
titluri, iar **Enter** sare la cel selectat și readuce focalizarea în editor.
Apăsând din nou **F6** revii pur și simplu cu focalizarea în editor.

**Câmpul de filtrare** din partea de sus a panoului afișează doar titlurile care se
potrivesc cu ce tastezi (și strămoșii lor); butoanele **⊞/⊟** extind sau restrâng
tot. Plierea pe care o stabilești se **păstrează** chiar și în timp ce continui să editezi.

Poți **trage** o intrare din structură pentru a **reordona** acea secțiune —titlul,
conținutul și subsecțiunile ei— în document, fără a schimba nivelul. În plus,
**Inserare → Cuprins (TOC)** plasează în document o listă imbricată a titlurilor.
**Vizualizare → Mergi la titlu…** (Ctrl+G) sare la un titlu tastând o parte din
textul lui, iar **Salt la linie…** (Ctrl+L) duce cursorul la un anumit număr de
linie (în vizualizarea sursă, la linia din Markdown). **Paleta de comenzi**
(Ctrl+Shift+P) găsește și execută orice acțiune din meniuri tastând o parte din
numele ei.

## Statistici despre document

- **Vizualizare → Statistici despre document…** arată cuvinte, caractere,
  paragrafe, propoziții și timpul estimat de citire (al documentului sau al
  selecției).
- **Vizualizare → Arată contorul de cuvinte** activează un contor permanent în bara
  de stare.
- **Vizualizare → Afișează linia și coloana** arată poziția cursorului (linia și
  coloana) în bara de stare.

## Mod fără distrageri

**Vizualizare → Fără distrageri** (F11) intră pe ecran complet cu meniul și barele
ascunse și textul centrat într-o coloană de citire. Structura, dacă este vizibilă,
rămâne lipită de blocul central. ESC sau F11 ies.

## Mod de concentrare

**Vizualizare → Mod de concentrare** (F12) te ajută să te concentrezi pe ceea ce scrii
fără a ieși din fereastra normală. Un singur comutator activează două lucruri
deodată:

- **Mașină de scris**: linia cursorului se menține centrată pe verticală. Pe
  măsură ce scrii, textul se deplasează pentru ca linia activă să rămână la
  jumătatea înălțimii, în loc să se lipească de marginea de jos.
- **Estompare**: tot documentul se vede stins, cu excepția paragrafului în care
  se află cursorul, care iese în evidență clar.

Funcționează în editorul vizual și în vizualizarea codului și este
**independent** de modul fără distrageri (F11): le poți folosi pe ambele
simultan sau pe fiecare separat.

## Vizualizarea codului

**Vizualizare → Sursă Markdown** (Ctrl+Shift+M) comută între editorul vizual și un
editor de text simplu, pe ecran complet, cu Markdown-ul brut. Modificările din
modul sursă sunt aplicate documentului la revenirea în modul vizual.

**Vizualizare → Vizualizare divizată** (Ctrl+Shift+D) le arată pe ambele
simultan, una lângă alta: editorul vizual și sursa, sincronizate (ce tastezi
într-unul se reflectă în celălalt). Se exclude reciproc cu modul sursă pe ecran
complet.

În vizualizarea sursă există **comenzi de linie** de la tastatură pentru linia
cursorului: **Alt+↑ / Alt+↓** mută linia în sus/jos, **Ctrl+D** o duplică,
**Ctrl+Shift+K** o șterge, iar **Ctrl+J** o unește cu următoarea.

## Export și tipărire

**Fișier → Exportă** oferă **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** și **text simplu (.txt)**. În ODF, DOCX, LaTeX și
EPUB este încorporată limba documentului (din front matter `lang`/`language`, din
setarea aplicației sau, în ultimă instanță, din limba sistemului). În PDF sunt
încorporate titlul și autorul când se află în front matter (`title`, `author`).

Poți, de asemenea, să exporți **doar selecția în PDF** și să folosești
**Previzualizarea tipăririi**.

**Fișier → Tipărește** (Ctrl+P) deschide dialogul sistemului; **Tipărește selecția**
tipărește doar ce este selectat.

**Vizualizare → Numere de pagină la tipărire** (activat implicit) adaugă numărul
paginii în subsol (`N / M`) la tipărire și la exportul PDF.

## Teme și aspect

- **Vizualizare → Temă** oferă Luminoasă, Întunecată, GitHub Light, GitHub Dark,
  Monokai, Contrast ridicat, Solarized Light și Solarized Dark. **Urmează sistemul** potrivește tema
  luminoasă/întunecată cu cea a sistemului de operare.
- **Vizualizare → Lumină caldă nocturnă** atenuează albastrurile fundalului în
  funcție de oră.
- **Vizualizare → Spațiere între rânduri** stabilește înălțimea rândului în editor: Simplă, 1,5 rânduri sau Dublă.
- **Vizualizare → Evidențiază linia curentă** marchează linia cursorului cu un fundal subtil.
- **Zoom**: Ctrl+rotița mouse-ului, Ctrl++ / Ctrl+- și **Dimensiune normală**
  (Ctrl+0) scalează întreaga interfață (nu doar textul editorului).
- **Vizualizare → Limbă** schimbă limba interfeței; se aplică imediat (fereastra
  este recreată).

## Recuperare automată

În timp ce editezi, conținutul este salvat automat la câteva secunde într-o copie
ciornă. Dacă aplicația se închide anormal, la redeschidere oferă recuperarea a ceea
ce scriai.

## Accesibilitate

- **Cititoare de ecran**: editorul, panoul de schiță, câmpurile de căutare și celelalte controale au nume accesibil; în plus, mesajele de stare (salvat, „negăsit”, modificări pe disc…) sunt anunțate vocal.
- **Doar de la tastatură**: fiecare acțiune are o scurtătură sau o intrare de meniu (F10 sau Alt deschide bara de meniuri). Vezi tabelul [Scurtături](#scurtaturi).
- **Contrast și dimensiune**: tema **Contrast ridicat** și **zoomul** întregii interfețe ajută la vedere slabă; dimensiunea inițială a fontului este cea a sistemului.
- **Focus**: elementul focalizat este evidențiat cu culoarea de selecție a temei.

## Scurtături

| Acțiune                   | Scurtătură       |
|---------------------------|------------------|
| Nou                       | Ctrl+N           |
| Închide fila              | Ctrl+W           |
| Redeschide fila închisă   | Ctrl+Shift+R     |
| Fila următoare / anterioară | Ctrl+PageDown / Ctrl+PageUp (sau Ctrl+Tab / Ctrl+Shift+Tab) |
| Deschide                  | Ctrl+O           |
| Salvează                  | Ctrl+S           |
| Salvează ca               | Ctrl+Shift+S     |
| Tipărește                 | Ctrl+P           |
| Anulează / Refă           | Ctrl+Z / Ctrl+Y  |
| Aldin / Cursiv            | Ctrl+B / Ctrl+I  |
| Evidențiază (==marcaj==)  | Ctrl+Shift+H     |
| Exponent / Indice         | Ctrl+Shift++ / Ctrl+Shift+- |
| Subliniat                 | Ctrl+U           |
| Lipește ca text simplu    | Ctrl+Shift+V     |
| Lipește ca Markdown       | Ctrl+Alt+V       |
| Caută                     | Ctrl+F           |
| Caută următorul/anteriorul | F3 / Shift+F3   |
| Titlu H1 … H6             | Ctrl+1 … Ctrl+6  |
| Promovează / retrogradează titlul | Ctrl+Shift+[ / Ctrl+Shift+] |
| Inserează formulă         | Ctrl+Shift+F     |
| Inserează notă de subsol  | Ctrl+Shift+N     |
| Mergi la titlu            | Ctrl+G           |
| Salt la linie             | Ctrl+L           |
| Paletă de comenzi         | Ctrl+Shift+P     |
| Focus structură / editor  | F6     |
| Vizualizare sursă Markdown | Ctrl+Shift+M    |
| Vizualizare divizată      | Ctrl+Shift+D     |
| Mută linia ↑ / ↓ (sursă)  | Alt+↑ / Alt+↓    |
| Duplică / șterge / unește linia (sursă) | Ctrl+D / Ctrl+Shift+K / Ctrl+J |
| Structură                 | F9               |
| Fără distrageri           | F11              |
| Mod focalizare            | F12              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ajutor                    | F1               |
