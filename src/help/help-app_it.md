# Manuale d'uso

**md-editor** è un editor visuale (WYSIWYG) di Markdown: scrivi e applichi la
formattazione sul testo già renderizzato, senza vedere il codice. Quando salvi,
il documento viene serializzato di nuovo in Markdown puro.

## Indice

- [Aprire e salvare](#aprire-e-salvare)
- [Formattare il testo](#formattare-il-testo)
- [Intestazioni, elenchi e blocchi](#intestazioni-elenchi-e-blocchi)
- [Collegamenti e immagini](#collegamenti-e-immagini)
- [Tabelle](#tabelle)
- [Formule matematiche](#formule-matematiche)
- [Trova e sostituisci](#trova-e-sostituisci)
- [Struttura del documento](#struttura-del-documento)
- [Modalità senza distrazioni](#modalita-senza-distrazioni)
- [Vista sorgente](#vista-sorgente)
- [Esportare e stampare](#esportare-e-stampare)
- [Temi e aspetto](#temi-e-aspetto)
- [Ripristino automatico](#ripristino-automatico)
- [Scorciatoie](#scorciatoie)

## Aprire e salvare

- **File → Nuovo** (Ctrl+N) crea un documento vuoto.
- **File → Apri…** (Ctrl+O) apre un `.md` esistente. L'applicazione
  ricorda i file più recenti in **File → Apri recenti**.
- **Salva** (Ctrl+S) e **Salva con nome…** (Ctrl+Shift+S) scrivono il
  documento in UTF-8.
- Se il file cambia al di fuori dell'editor, l'applicazione lo rileva e, se
  non hai modifiche non salvate, lo ricarica; in caso contrario chiede cosa fare.
- Puoi anche **trascinare e rilasciare** un file sulla finestra per aprirlo.

### Front matter

Se il documento inizia con un blocco `---…---` (YAML) o `+++…+++` (TOML), viene
conservato testualmente al salvataggio: non è mostrato nell'editor e non è
modificabile. Serve per metadati come `title`, `lang`, ecc., che vengono usati
durante l'esportazione.

## Formattare il testo

Seleziona un frammento e applica la formattazione dalla barra degli strumenti o
dal menu **Formato**:

- **Grassetto** (Ctrl+B), **Corsivo** (Ctrl+I), **Sottolineato** (Ctrl+U),
  **Barrato**.
- **Codice inline** per frammenti `a spaziatura fissa`.
- **Collegamento**: aggiunge `[testo](url)` sulla selezione.

I pulsanti della barra riflettono la formattazione attiva sotto il cursore.

## Intestazioni, elenchi e blocchi

- **Intestazioni** H1–H6 da **Formato → Intestazione** o con Ctrl+1 … Ctrl+6.
- **Elenchi**: puntati, numerati e di attività (con casella di spunta). Premendo
  Invio alla fine di un elemento si crea automaticamente il successivo; premendo
  Invio su un elemento vuoto si esce dall'elenco.
- **Citazione** (`>` all'inizio di un paragrafo) e **blocco di codice** si
  applicano dalla barra degli strumenti; entrambi mantengono correttamente il
  round-trip verso Markdown.

## Collegamenti e immagini

- **Inserisci → Collegamento…** apre una finestra con i campi testo e URL. Se
  avevi una selezione, viene usata come testo.
- **Ctrl+clic** su un collegamento lo apre nel browser di sistema; passandoci
  sopra il puntatore viene mostrato l'URL nella barra di stato.
- **Immagini**: trascina un file, incolla un'immagine dagli appunti o usa
  **Inserisci → Incolla immagine**. L'immagine viene salvata come PNG accanto al
  `.md` e inserita come `![alt](percorso-relativo)`; in questo modo sopravvive al
  round-trip verso Markdown (le immagini incorporate no).

## Tabelle

- **Tabella → Tabella…** chiede righe e colonne.
- Le azioni del menu **Tabella** (aggiungi/rimuovi riga o colonna, allinea
  colonna) sono abilitate solo quando il cursore si trova dentro una tabella.
- L'allineamento delle colonne (sinistra/centro/destra) viene conservato al
  salvataggio come `:--`/`:-:`/`--:`.

## Formule matematiche

md-editor supporta le **formule TeX** inline (`$...$`) e in blocco
(`$$...$$`), con la consueta sintassi LaTeX (Pandoc, Obsidian, Quarto…). Non
serve alcuna dipendenza esterna.

- **Inserisci → Formula…** (Ctrl+Shift+F) apre una finestra con un campo TeX e
  un'**anteprima dal vivo**: mentre digiti vedi come apparirà. Scegli
  *Inline* o *Blocco* e conferma per inserirla.
- Nell'editor le formule appaiono in corsivo con il colore d'accento del tema,
  con **veri apici e pedici** (non caratteri Unicode piatti): `x²`, `Hᵢ`,
  e così via — il vertical-align di Qt ridimensiona correttamente qualsiasi
  carattere.
- **Doppio clic** su una formula riapre la finestra con il suo TeX originale
  precaricato: la modifichi e confermando viene sostituita.
- Le formule sono **atomiche**: digitare al loro interno attiva un promemoria a
  fare doppio clic per modificarle; Backspace/Canc al bordo elimina l'intero
  gruppo.
- All'**esportazione** le formule vengono conservate: LaTeX le emette
  testualmente (con `amsmath` e `amssymb` nel preambolo); HTML/PDF/ODF mantengono
  gli apici/pedici con vertical-align di Qt nel formato di destinazione.
- Nella **vista sorgente** le vedi come `$...$` / `$$...$$`, con tutti i
  caratteri TeX (`\sum`, `\frac`, `_`, `*`) intatti al salvataggio.

Esempi:

```
L'energia è $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitazione: nel sorgente, `$$...$$` può estendersi su più righe (stile
> Obsidian/Pandoc); `$...$` deve aprirsi e chiudersi sulla stessa riga.

## Trova e sostituisci

- **Trova** (Ctrl+F) apre una barra inferiore con i campi per trovare e
  sostituire, oltre alle opzioni (maiuscole/minuscole, parola intera).
- **Trova successivo** F3 / **Trova precedente** Shift+F3.

## Struttura del documento

Il pannello laterale sinistro mostra l'indice delle intestazioni (TOC): si
aggiorna mentre digiti e, facendo clic su una voce, il cursore salta a quella
intestazione. Si mostra/nasconde con F9.

## Modalità senza distrazioni

**Visualizza → Modalità senza distrazioni** (F11) entra a schermo intero con il
menu e le barre degli strumenti nascosti e il testo centrato in una colonna di
lettura. La struttura, se visibile, resta agganciata al blocco centrale. ESC o
F11 escono.

## Vista sorgente

**Visualizza → Sorgente Markdown** (Ctrl+Shift+M) alterna tra l'editor visuale e
un editor di testo semplice a schermo intero che mostra il Markdown grezzo. Le
modifiche fatte in modalità sorgente vengono riversate nel documento quando torni
alla modalità visuale.

**Visualizza → Vista divisa** (Ctrl+Shift+D) mostra entrambi affiancati: l'editor
visuale e il sorgente, mantenuti sincronizzati (ciò che digiti in uno si riflette
nell'altro). È mutuamente esclusiva con la modalità sorgente a schermo intero.

## Esportare e stampare

**File → Esporta** offre **PDF**, **HTML**, **ODF (.odt)** e
**LaTeX (.tex)**. Per ODF e LaTeX la lingua del documento viene incorporata
(presa dal front matter `lang`/`language`, dall'impostazione dell'applicazione o,
come ultima risorsa, dalle impostazioni locali del sistema).

**File → Stampa** (Ctrl+P) apre la finestra di sistema.

## Temi e aspetto

- **Visualizza → Tema** offre Chiaro, Scuro, GitHub Light, GitHub Dark, Monokai
  e Contrasto elevato.
- **Visualizza → Luce calda notturna** attenua i toni di blu dello sfondo in base
  all'ora del giorno.
- **Zoom**: Ctrl+rotella del mouse, Ctrl++ / Ctrl+- e **Dimensione normale**
  (Ctrl+0) ridimensionano tutta l'interfaccia (non solo il testo dell'editor).
- **Visualizza → Lingua** cambia la lingua dell'interfaccia; ha effetto al riavvio.

## Ripristino automatico

Mentre modifichi, il contenuto viene salvato automaticamente ogni pochi secondi
in una copia di bozza. Se l'applicazione si chiude in modo anomalo, all'avvio
successivo propone di recuperare ciò che stavi scrivendo.

## Scorciatoie

| Azione                    | Scorciatoia      |
|---------------------------|------------------|
| Nuovo                     | Ctrl+N           |
| Apri                      | Ctrl+O           |
| Salva                     | Ctrl+S           |
| Salva con nome            | Ctrl+Shift+S     |
| Stampa                    | Ctrl+P           |
| Annulla / Ripeti          | Ctrl+Z / Ctrl+Y  |
| Grassetto / Corsivo       | Ctrl+B / Ctrl+I  |
| Sottolineato              | Ctrl+U           |
| Trova                     | Ctrl+F           |
| Trova successivo / precedente | F3 / Shift+F3 |
| Intestazione H1 … H6      | Ctrl+1 … Ctrl+6  |
| Inserisci formula         | Ctrl+Shift+F     |
| Vista sorgente Markdown   | Ctrl+Shift+M     |
| Vista divisa              | Ctrl+Shift+D     |
| Struttura                 | F9               |
| Senza distrazioni         | F11              |
| Zoom + / − / Normale      | Ctrl++ / Ctrl+− / Ctrl+0 |
| Aiuto                     | F1               |
