# Manuale d'uso

**md-editor** è un editor visuale (WYSIWYG) di Markdown: scrivi e applichi la
formattazione sul testo già renderizzato, senza vedere il codice. Al salvataggio
il documento viene serializzato di nuovo in Markdown puro.

## Indice

- [Aprire e salvare](#aprire-e-salvare)
- [Formattare il testo](#formattare-il-testo)
- [Titoli, elenchi e blocchi](#titoli-elenchi-e-blocchi)
- [Trasformare il testo e gli appunti](#trasformare-il-testo-e-gli-appunti)
- [Collegamenti e immagini](#collegamenti-e-immagini)
- [Note a piè di pagina](#note-a-pie-di-pagina)
- [Riquadri, simboli e scorciatoie di testo](#riquadri-simboli-e-scorciatoie-di-testo)
- [Snippet (frammenti riutilizzabili)](#snippet-frammenti-riutilizzabili)
- [Tabelle](#tabelle)
- [Formule matematiche](#formule-matematiche)
- [Diagrammi](#diagrammi)
- [Controllo ortografico](#controllo-ortografico)
- [Trova e sostituisci](#trova-e-sostituisci)
- [Struttura del documento](#struttura-del-documento)
- [Statistiche del documento](#statistiche-del-documento)
- [Modalità senza distrazioni](#modalita-senza-distrazioni)
- [Modalità focus](#modalita-focus)
- [Vista del codice](#vista-del-codice)
- [Esportare e stampare](#esportare-e-stampare)
- [Temi e aspetto](#temi-e-aspetto)
- [Recupero automatico](#recupero-automatico)
- [Accessibilità](#accessibilita)
- [Scorciatoie](#scorciatoie)

## Aprire e salvare

- **File → Nuovo** (Ctrl+N) crea un documento vuoto in una nuova scheda.
- **File → Nuovo da modello** crea un documento a partire da uno scheletro
  (lettera, verbale, esame…) pronto da compilare.
- **File → Apri…** (Ctrl+O) apre un `.md` esistente. L'applicazione ricorda gli
  ultimi aperti in **File → Apri recenti**.
- **Salva** (Ctrl+S) e **Salva come…** (Ctrl+Maiusc+S) scrivono il documento in
  UTF-8. **Apri cartella contenitore** apre la cartella del documento nel gestore
  file.
- **Ripristina alla versione salvata** scarta le modifiche non salvate e ricarica
  il file dal disco (chiede conferma). Disponibile solo se il documento ha un file
  e modifiche in sospeso.
- Se il file cambia fuori dall'editor, l'applicazione lo rileva e, se non hai
  modifiche non salvate, lo ricarica; altrimenti chiede cosa fare.
- Puoi anche **trascinare e rilasciare** un file sulla finestra per aprirlo.

### Schede (più documenti)

Puoi avere più documenti aperti contemporaneamente, ciascuno nella propria **scheda**:

- **Nuovo** (Ctrl+N), **Nuovo da modello** e **Apri** (Ctrl+O) creano una scheda (o
  riutilizzano la scheda vuota iniziale). Trascinare un file lo apre anch'esso in
  una scheda; se è già aperto, salta alla sua scheda.
- Cambia documento facendo clic sulla sua scheda; trascina le schede per
  riordinarle. Da tastiera, **Ctrl+PagGiù / Ctrl+PagSu** (o **Ctrl+Tab /
  Ctrl+Maiusc+Tab**) passano alla scheda successiva o precedente.
- **Chiudi scheda** (Ctrl+W) chiude quella corrente, chiedendo se ha modifiche non
  salvate. L'ultima scheda non si chiude: diventa un documento nuovo.
- **Riapri scheda chiusa** (Ctrl+Maiusc+R) riapre l'ultima scheda che hai chiuso
  (solo quelle che avevano un file su disco).
- L'etichetta mostra il nome del file e un punto (•) se ci sono modifiche non
  salvate.
- Alla chiusura dell'applicazione i documenti aperti vengono ricordati e tutti
  riaperti al successivo avvio.

### *Front matter*

Se il documento inizia con un blocco `---…---` (YAML) o `+++…+++` (TOML), viene
conservato così com'è al salvataggio: non si vede nell'editor e non si modifica.
Serve per metadati come `title`, `lang`, ecc., usati in fase di esportazione.

## Formattare il testo

Seleziona un frammento e applica la formattazione con la barra degli strumenti o
il menu **Formato**:

- **Grassetto** (Ctrl+B), **Corsivo** (Ctrl+I), **Sottolineato** (Ctrl+U),
  **Barrato**.
- **Codice in linea** per frammenti a `spaziatura fissa`.
- **Collegamento**: aggiunge `[testo](url)` sulla selezione.
- **Evidenzia** (Ctrl+Maiusc+H): racchiude la selezione tra `==marca==`; il testo
  appare con uno sfondo di evidenziazione. Poiché `==` non è sintassi Markdown
  standard, viene salvato come testo letterale.
- **Apice** (Ctrl+Maiusc++) e **Pedice** (Ctrl+Maiusc+-): alzano o abbassano il testo
  selezionato; salvati come `^testo^` e `~testo~` (stile Pandoc).

I pulsanti della barra riflettono la formattazione attiva sotto il cursore.

**Accoppiamento automatico.** Digitando `(`, `[`, `{` o `` ` `` la coppia si
chiude da sola e il cursore resta in mezzo; se c'è del testo selezionato, lo
avvolge. Se digiti il carattere di chiusura proprio davanti al suo compagno,
l'editor lo «salta» invece di duplicarlo.

**Regole di inserimento.** A inizio riga, digitare un marcatore Markdown di blocco
seguito da uno spazio trasforma la riga sul posto (senza lasciare il marcatore):
`#` … `######` + spazio → titolo H1…H6; `>` → citazione; `-`, `*` o `+` → elenco
puntato; `1.` (o `1)`) → elenco numerato. Produce lo stesso formato della barra.

## Titoli, elenchi e blocchi

- **Titoli** H1–H6 da **Formato → Titolo** o con Ctrl+1 … Ctrl+6.
  **Alza/abbassa** il titolo al cursore di un livello con
  Ctrl+Maiusc+[ / Ctrl+Maiusc+].
- **Elenchi**: puntati, numerati e di attività (con casella). Premendo Invio alla
  fine di un punto si crea automaticamente il successivo; Invio su un punto vuoto
  esce dall'elenco. Un **clic sulla casella** di un'attività la spunta o la
  deseleziona.
- **Citazione** (`>` all'inizio di un paragrafo) e **blocco di codice** si
  applicano dalla barra; entrambi tornano correttamente a Markdown. Con
  **Formato → Linguaggio del blocco…** scegli il linguaggio di un blocco di
  codice (con il cursore al suo interno) per evidenziarne la sintassi.
- **Rientro**: **Formato → Aumenta/Riduci rientro** annida elenchi e citazioni.

## Trasformare il testo e gli appunti

- **Modifica → Trasforma testo** agisce sulla selezione: **MAIUSCOLO**,
  **minuscolo**, **Capitalizza** e **Ordina righe**.
- **Tipografia intelligente** (nello stesso menu) converte nella selezione i
  trattini `--`/`---` in `–`/`—`, `...` in `…` e le virgolette dritte in
  tipografiche secondo il contesto.
- **Incolla come testo semplice** (Ctrl+Maiusc+V) incolla senza formattazione.
  **Incolla come Markdown** (Ctrl+Alt+V) converte il contenuto formattato degli
  appunti (HTML) in Markdown invece di incorporare la formattazione di origine.
- **Copia come HTML** copia la selezione (o il documento) come HTML, per
  incollarla in un'email, un CMS, ecc.
- **Copia come Markdown** copia la selezione (o il documento intero) come testo
  Markdown, per incollarla in un altro editor di Markdown o in un campo di codice.
- Quando incolli un **URL** su una selezione di testo, il testo viene collegato
  automaticamente.
- **Modifica → Pulisci Markdown** normalizza l'intero documento in un'unica
  passata: uniforma i punti elenco a `-`, elimina gli spazi a fine riga, riduce
  le righe vuote in eccesso e regola lo spazio dopo i `#` dei titoli. È
  conservativo: non tocca l'interno dei blocchi di codice.

## Collegamenti e immagini

- **Inserisci → Collegamento…** apre una finestra con testo e URL. Una selezione
  esistente viene usata come testo.
- **Ctrl+clic** su un collegamento lo apre nel browser di sistema; passandoci
  sopra il mouse, l'URL appare nella barra di stato.
- **Immagini**: trascina un file, incolla un'immagine dagli appunti o usa
  **Inserisci → Incolla immagine**. L'immagine viene salvata come PNG accanto al
  `.md` e inserita come `![alt](percorso-relativo)`; così sopravvive al round-trip
  verso Markdown (le immagini incorporate no).

## Note a piè di pagina

- **Inserisci → Nota a piè di pagina** (Ctrl+Maiusc+N) inserisce un riferimento
  numerato `[^n]` al cursore e crea la sua definizione `[^n]:` in fondo al
  documento, pronta per il testo della nota.
- I riferimenti sono mostrati in **apice**; un **clic** su uno di essi sposta il
  cursore alla sua definizione.
- Vengono salvati come Markdown standard (`testo[^1]` nel corpo e, sotto,
  `[^1]: la nota`), quindi compatibili con altri editor.

## Riquadri, simboli e scorciatoie di testo

- **Inserisci → Riquadro** crea un *callout* in stile GitHub: una citazione la cui
  prima riga è `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` o `[!CAUTION]`.
  Viene mostrato con sfondo colorato e titolo a colori, e salvato come Markdown
  compatibile con GitHub.
- **Inserisci → Simboli speciali…** apre una mappa caratteri per categorie
  (matematici, greco, frecce, valute, punteggiatura…); un clic inserisce il
  simbolo e la finestra resta aperta per inserirne altri.
- **Scorciatoie `:nome:`**: digitando un codice come `:alpha:` o `:euro:` viene
  espanso al simbolo corrispondente (α, €…).
- **Inserisci → Data** e **Data e ora** inseriscono la data (e l'ora) attuale in
  formato localizzato.

## Snippet (frammenti riutilizzabili)

Uno **snippet** è un frammento di Markdown che salvi con un nome per inserirlo
poi con un paio di clic: una firma, un modello di tabella, un avviso che ripeti
spesso…

- **Inserisci → Snippet** apre l'elenco di quelli che hai; scegliendone uno, il
  suo contenuto viene inserito dove si trova il cursore (funziona anche nella
  vista del codice).
- **Inserisci → Snippet → Gestisci snippet…** apre una finestra per creare,
  modificare ed eliminare i tuoi snippet. Ognuno ha un **nome** (quello che vedi
  nel menu) e un **corpo** in Markdown.
- Vengono salvati nelle impostazioni dell'applicazione, quindi sono disponibili
  in tutti i tuoi documenti, non solo in quello corrente.

## Tabelle

- **Tabella → Inserisci tabella…** chiede righe e colonne.
- **Inserisci → Tabella dagli appunti** converte in una tabella i dati TSV/CSV
  (colonne separate da tabulazioni o virgole) copiati da un foglio di calcolo o da
  un file CSV.
- Le azioni del menu **Tabella** (aggiungi/rimuovi riga o colonna, allinea
  colonna) sono attive solo quando il cursore è dentro una tabella.
- L'allineamento della colonna (sinistra/centro/destra) viene conservato al
  salvataggio come `:--`/`:-:`/`--:`.
- **Tabella → Ordina le righe per colonna** (crescente/decrescente) riordina le
  righe in base alla colonna del cursore, mantenendo fissa l'intestazione; rileva se
  la colonna è numerica o testuale.

## Formule matematiche

md-editor supporta le **formule TeX** in linea (`$...$`) e a blocco (`$$...$$`),
con la consueta sintassi LaTeX (Pandoc, Obsidian, Quarto…). Non serve alcuna
dipendenza esterna.

- **Inserisci → Formula…** (Ctrl+Maiusc+F) apre una finestra con un campo per il
  TeX e un'**anteprima dal vivo**: mentre scrivi vedi il risultato. Scegli *In
  linea* o *A blocco* e conferma per inserirla.
- Le formule sono impaginate in **2D reale**: le frazioni (`\frac`) sono
  impilate con una barra, i grandi operatori (`\sum`, `\int`, `\prod`…) mostrano
  i limiti sopra e sotto, le radici (`\sqrt`) hanno il loro vinculum, e ci sono
  matrici (`\begin{pmatrix}`…), coefficienti binomiali (`\binom`) e accenti
  (`\hat`, `\vec`, `\bar`…). Le più semplici (potenze, pedici, greco) sono
  composte in linea. Il rendering scala con lo zoom.
- **Doppio clic** su una formula riapre la finestra con il suo TeX originale
  precaricato: la modifichi e alla conferma viene sostituita.
- Le formule sono **atomiche**: se digiti all'interno, l'app ricorda di usare il
  doppio clic; Backspace/Canc al bordo eliminano l'intero gruppo.
- All'**esportazione** vengono conservate: in LaTeX vengono emesse così come sono
  (con `amsmath` e `amssymb` nel preambolo); in HTML/PDF/ODF vengono ridotte alla
  loro approssimazione in linea.
- Nella **vista del codice** appaiono come `$...$` / `$$...$$`, con tutti i
  caratteri TeX (`\sum`, `\frac`, `_`, `*`) intatti al salvataggio.

Esempi:

```
L'energia è $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Nel sorgente, `$$...$$` può estendersi su più righe (stile Obsidian/Pandoc);
> `$...$` deve aprirsi e chiudersi sulla stessa riga.

## Diagrammi

Un blocco di codice con linguaggio `mermaid` o `plantuml` viene **mostrato in
anteprima come immagine** subito sotto il blocco, senza toccare il codice (che
resta modificabile) né il Markdown salvato.

- Richiede lo strumento corrispondente installato: **`plantuml`** (con Java) per
  PlantUML, o **`mmdc`** (mermaid-cli, con Node) per Mermaid.
- Se lo strumento manca, sotto il blocco appare un avviso con il comando di
  installazione per il tuo sistema operativo; il blocco resta come codice.
- L'immagine è solo presentazione: non viene scritta nel Markdown e non conta come
  modifica non salvata.

Ad esempio, un blocco di codice etichettato `mermaid` contenente `flowchart LR  A
--> B --> C` viene mostrato come il diagramma di flusso corrispondente.

## Controllo ortografico

- Sottolinea in rosso le parole errate secondo la **lingua del documento** (presa
  dal front matter `lang`, dall'impostazione di lingua o dal sistema). Non
  controlla il codice, le formule né i collegamenti.
- Il **clic destro** su una parola sottolineata offre **suggerimenti** (un clic la
  sostituisce), **Aggiungi al dizionario** (un elenco personale permanente) e
  **Ignora** (per la sessione).
- Si attiva/disattiva in **Visualizza → Controllo ortografico**, e la lingua si
  imposta in **Visualizza → Lingua del controllo ortografico** (o si lascia
  automatica).
- Servono dizionari Hunspell: su Linux quelli di sistema (`hunspell-es`,
  `hunspell-en-us`…); su Windows/macOS sono forniti con l'applicazione.

## Trova e sostituisci

- **Trova** (Ctrl+F) apre una barra in basso con campi per trovare e sostituire,
  oltre alle opzioni (maiuscole/minuscole, parola intera).
- **Trova successivo** F3 / **Trova precedente** Maiusc+F3.
- La barra evidenzia **tutte** le corrispondenze nel documento e mostra un contatore **«N di M»** (su quale corrispondenza sei, sul totale). **Sostituisci tutto** le sostituisce tutte in una volta.

## Struttura del documento

Il pannello laterale sinistro mostra la struttura dei titoli (TOC): si aggiorna
mentre scrivi e, facendo clic su una voce, il cursore salta a quel titolo. Si
mostra/nasconde con F9. Con **F6** sposti il fuoco della tastiera
sulla struttura (mostrandola se è nascosta); lì i tasti freccia scorrono tra i
titoli e **Invio** salta a quello selezionato riportando il fuoco all'editor.
Premendo di nuovo **F6** il fuoco torna semplicemente all'editor.

Il **campo di filtro** in alto nel pannello mostra solo i titoli che corrispondono
a ciò che digiti (e i loro antenati); i pulsanti **⊞/⊟** espandono o comprimono
tutto. La compressione che imposti si **conserva** anche mentre continui a modificare.

Puoi **trascinare** una voce della struttura per **riordinare** quella sezione —il
suo titolo, il suo contenuto e le sue sottosezioni— nel documento, senza cambiare
il livello. Inoltre, **Inserisci → Indice (TOC)** inserisce nel documento un
elenco annidato dei titoli. **Visualizza → Vai al titolo…** (Ctrl+G) salta a un
titolo digitando parte del suo testo, e **Vai alla riga…** (Ctrl+L) porta il
cursore a un numero di riga (nella vista sorgente, alla riga del Markdown). La
**Palette dei comandi** (Ctrl+Maiusc+P) trova ed esegue qualsiasi azione dei
menu digitando parte del suo nome.

## Statistiche del documento

- **Visualizza → Statistiche del documento…** mostra parole, caratteri,
  paragrafi, frasi e tempo di lettura stimato (del documento o della selezione).
- **Visualizza → Mostra conteggio parole** attiva un contatore permanente nella
  barra di stato.
- **Visualizza → Mostra riga e colonna** mostra la posizione del cursore (riga e
  colonna) nella barra di stato.

## Modalità senza distrazioni

**Visualizza → Senza distrazioni** (F11) passa a schermo intero con menu e barre
nascosti e il testo centrato in una colonna di lettura. La struttura, se visibile,
resta accostata al blocco centrale. ESC o F11 escono.

## Modalità focus

**Visualizza → Modalità focus** (F12) ti aiuta a concentrarti su ciò che scrivi senza
uscire dalla finestra normale. Un unico interruttore attiva due cose insieme:

- **Macchina da scrivere**: la riga del cursore resta centrata in verticale.
  Mentre scrivi, il testo scorre in modo che la riga attiva rimanga a metà
  altezza, invece di accostarsi via via al bordo inferiore.
- **Attenuazione**: tutto il documento appare smorzato tranne il paragrafo in
  cui si trova il cursore, che spicca nitido.

Funziona nell'editor visuale e nella vista del codice, ed è **indipendente**
dalla modalità senza distrazioni (F11): puoi usarle entrambe insieme o ciascuna
per conto suo.

## Vista del codice

**Visualizza → Sorgente Markdown** (Ctrl+Maiusc+M) alterna tra l'editor visuale e
un editor di testo semplice, a schermo intero, con il Markdown grezzo. Le
modifiche in modalità sorgente vengono riportate nel documento al ritorno alla
modalità visuale.

**Visualizza → Vista divisa** (Ctrl+Maiusc+D) mostra entrambi insieme, fianco a
fianco: l'editor visuale e il sorgente, sincronizzati (ciò che digiti in uno si
riflette nell'altro). È esclusiva con la modalità sorgente a schermo intero.

Nella vista sorgente ci sono **comandi di riga** da tastiera sulla riga del cursore:
**Alt+↑ / Alt+↓** spostano la riga su/giù, **Ctrl+D** la duplica, **Ctrl+Maiusc+K**
la elimina e **Ctrl+J** la unisce alla successiva.

## Esportare e stampare

**File → Esporta** offre **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** e **testo semplice (.txt)**. In ODF, DOCX, LaTeX
ed EPUB viene incorporata la lingua del documento (presa dal front matter
`lang`/`language`, dall'impostazione dell'applicazione o, in ultima istanza, dalla
lingua del sistema). In PDF vengono incorporati il titolo e l'autore quando sono
presenti nel front matter (`title`, `author`).

Puoi anche esportare **solo la selezione in PDF** e usare l'**Anteprima di
stampa**.

**File → Stampa** (Ctrl+P) apre la finestra di sistema; **Stampa selezione** stampa
solo ciò che è selezionato.

**Visualizza → Numeri di pagina in stampa** (attivo per impostazione predefinita)
aggiunge il numero di pagina nel piè di pagina (`N / M`) in stampa e nell'esportazione PDF.

## Temi e aspetto

- **Visualizza → Tema** offre Chiaro, Scuro, GitHub Light, GitHub Dark, Monokai, Alto contrasto, Solarized Light e Solarized Dark. **Segui il sistema** allinea il tema chiaro/scuro a quello del
  sistema operativo.
- **Visualizza → Luce calda notturna** attenua i blu dello sfondo in base all'ora.
- **Visualizza → Interlinea** imposta l'altezza di riga dell'editor: Singola, 1,5 righe o Doppia.
- **Visualizza → Evidenzia la riga corrente** segna la riga del cursore con uno sfondo tenue.
- **Zoom**: Ctrl+rotellina, Ctrl++ / Ctrl+- e **Dimensione normale** (Ctrl+0)
  scalano tutta l'interfaccia (non solo il testo dell'editor).
- **Visualizza → Lingua** cambia la lingua dell'interfaccia; si applica all'istante
  (la finestra viene ricreata).

## Recupero automatico

Mentre modifichi, il contenuto viene salvato automaticamente ogni pochi secondi in
una copia di bozza. Se l'applicazione si chiude in modo anomalo, alla riapertura
propone di recuperare ciò che stavi scrivendo.

## Accessibilità

- **Lettori di schermo**: l’editor, il pannello della struttura, i campi di ricerca e gli altri controlli hanno un nome accessibile; inoltre i messaggi di stato (salvato, «non trovato», modifiche su disco…) vengono annunciati a voce.
- **Solo da tastiera**: ogni azione ha una scorciatoia o una voce di menu (F10 o Alt apre la barra dei menu). Vedi la tabella [Scorciatoie](#scorciatoie).
- **Contrasto e dimensione**: il tema **Alto contrasto** e lo **zoom** dell’intera interfaccia aiutano in caso di ipovisione; la dimensione iniziale del carattere è quella di sistema.
- **Fuoco**: l’elemento con il fuoco viene evidenziato con il colore di selezione del tema.

## Scorciatoie

| Azione                    | Scorciatoia      |
|---------------------------|------------------|
| Nuovo                     | Ctrl+N           |
| Chiudi scheda             | Ctrl+W           |
| Riapri scheda chiusa      | Ctrl+Maiusc+R    |
| Scheda successiva / precedente | Ctrl+PagGiù / Ctrl+PagSu (o Ctrl+Tab / Ctrl+Maiusc+Tab) |
| Apri                      | Ctrl+O           |
| Salva                     | Ctrl+S           |
| Salva come                | Ctrl+Maiusc+S    |
| Stampa                    | Ctrl+P           |
| Annulla / Ripeti          | Ctrl+Z / Ctrl+Y  |
| Grassetto / Corsivo       | Ctrl+B / Ctrl+I  |
| Evidenzia (==marca==)     | Ctrl+Maiusc+H    |
| Apice / Pedice            | Ctrl+Maiusc++ / Ctrl+Maiusc+- |
| Sottolineato              | Ctrl+U           |
| Incolla come testo semplice | Ctrl+Maiusc+V  |
| Incolla come Markdown     | Ctrl+Alt+V       |
| Trova                     | Ctrl+F           |
| Trova successivo/precedente | F3 / Maiusc+F3 |
| Titolo H1 … H6            | Ctrl+1 … Ctrl+6  |
| Alza / abbassa titolo     | Ctrl+Maiusc+[ / Ctrl+Maiusc+] |
| Inserisci formula         | Ctrl+Maiusc+F    |
| Inserisci nota            | Ctrl+Maiusc+N    |
| Vai al titolo             | Ctrl+G           |
| Vai alla riga             | Ctrl+L           |
| Palette dei comandi       | Ctrl+Maiusc+P    |
| Struttura (fuoco) / editor | F6    |
| Vista sorgente Markdown   | Ctrl+Maiusc+M    |
| Vista divisa              | Ctrl+Maiusc+D    |
| Sposta riga ↑ / ↓ (sorgente) | Alt+↑ / Alt+↓ |
| Duplica / elimina / unisci riga (sorgente) | Ctrl+D / Ctrl+Maiusc+K / Ctrl+J |
| Struttura                 | F9               |
| Senza distrazioni         | F11              |
| Modalità focus            | F12              |
| Zoom + / − / Normale      | Ctrl++ / Ctrl+− / Ctrl+0 |
| Aiuto                     | F1               |
