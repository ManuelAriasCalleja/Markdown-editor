# Caratteristiche

Riepilogo di tutto ciò che offre md-editor. Per il riferimento completo e tecnico,
consulta `especificacion.md` nel repository.

## Editing WYSIWYG e round-trip

Modifichi il testo renderizzato e, al salvataggio, viene serializzato in Markdown pulito in
UTF-8. Ciò che apri è ciò che salvi: tabelle con allineamento, elenchi annidati, elenchi
di attività, citazioni, blocchi di codice, note a piè di pagina, ammonizioni e formule si
conservano fedelmente.

## Editing a schede

Apri più documenti contemporaneamente, ciascuno nella sua scheda, e passa dall'uno
all'altro. Chiudi una scheda con Ctrl+W. La sessione riapre le schede al successivo avvio.

## Modalità di visualizzazione

- WYSIWYG, Sorgente Markdown (Ctrl+Shift+M) e Vista divisa (Ctrl+Shift+D).
- In vista divisa, rendering e codice si sincronizzano: si aggiorna solo il pannello che non
  stai modificando, senza salti del cursore.

## Modalità senza distrazioni

F11 entra in schermo intero con il testo centrato in una colonna di lettura e senza
barre. ESC o F11 escono.

## Temi e luce calda notturna

- **Sei temi**: Chiaro, Scuro, GitHub Light, GitHub Dark, Monokai e Contrasto elevato.
- **Luce calda notturna** (attivata per impostazione predefinita): attenua il blu dello sfondo in modo
  automatico e graduale a seconda dell'ora, per ridurre l'affaticamento visivo di notte.
  Neutra di giorno (07–19 h), si riscalda nel pomeriggio (19–23 h), massima di notte
  (23–06 h) e si raffredda all'alba (06–07 h). Si rivaluta da sola ogni minuto e influisce solo
  sullo sfondo (non sui collegamenti né sull'evidenziazione).

## Struttura del documento

Pannello laterale (F9) con l'indice delle intestazioni; un clic salta alla sezione. «Vai a
intestazione» (Ctrl+G) apre una ricerca rapida delle intestazioni.

## Formule TeX

Formule inline (`$...$`) e a blocco (`$$...$$`) con sintassi LaTeX, senza
dipendenze esterne:

- Inserimento con anteprima in tempo reale (Ctrl+Shift+F) e modifica con doppio clic.
- **Layout 2D reale**: frazioni impilate (`\frac`), radici con vincolo
  (`\sqrt`), coefficienti binomiali (`\binom`), matrici e ambienti (`matrix`, `pmatrix`, `cases`…),
  grandi operatori con limiti sopra e sotto (`\sum`, `\int`, `\prod`…), accenti
  (`\hat`, `\vec`…), apici e pedici reali, lettere greche e `\mathbb`.
- Sono atomiche nell'editor, scalano con lo zoom e sopravvivono al round-trip e
  all'esportazione. I blocchi `$$...$$` possono occupare più righe.
- Limitazioni: `$...$` deve aprirsi e chiudersi sulla stessa riga; le formule 2D inline
  risultano un po' alte (quelle a blocco si vedono bene).

## Correzione ortografica (opzionale)

Sottolinea le parole scritte male a seconda della lingua del documento (Visualizza → Correzione
ortografica). La lingua viene scelta automaticamente (front matter, impostazione o sistema) o a mano (Visualizza
→ Lingua di correzione). Il clic destro offre suggerimenti e l'aggiunta al dizionario
personale. Richiede Hunspell; senza di esso, il resto funziona ugualmente.

## Diagrammi (opzionale)

I blocchi ```` ```mermaid ```` e ```` ```plantuml ```` vengono renderizzati come immagine
sotto il blocco, eseguendo lo strumento esterno (`mmdc` / `plantuml`) se è
installato. Se manca, viene mostrato il comando di installazione per il tuo sistema. L'immagine non
viene salvata nel Markdown.

## Evidenziazione della sintassi

I blocchi di codice vengono colorati a seconda del loro linguaggio (famiglie C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… e una modalità generica).

## Immagini

Incollare o rilasciare un'immagine la salva come PNG accanto al documento e la inserisce come
`![](ruta)` —non la incorpora—, in modo che il Markdown rimanga portatile.

## Inserire e trasformare

- Inserire: collegamento, immagine, tabella, linea, indice (TOC), formula, nota a piè di pagina,
  ammonizione (nota/avviso…), simboli speciali e data/ora.
- Incolla come Markdown (Ctrl+Alt+V) converte l'HTML degli appunti in Markdown.
- Trasformare testo: MAIUSCOLE/minuscole, capitalizza, ordina righe e tipografia
  intelligente (—, –, …, virgolette tipografiche).
- Statistiche del documento: parole, caratteri, paragrafi, frasi e tempo di
  lettura.

## Esportazione e stampa

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) ed EPUB (.epub), oltre all'anteprima di
stampa e alla stampa (Ctrl+P). ODF, DOCX e LaTeX incorporano la lingua del documento
(dal front matter, dall'impostazione dell'app o dal sistema).

## Zoom dell'intera interfaccia

Ctrl++, Ctrl+- e Ctrl+0 (o Ctrl + rotellina) scalano l'intera interfaccia, non solo il testo
dell'editor. Il livello viene ricordato.

## Cerca e sostituisci

Ctrl+F / Ctrl+H, con precedente/successivo, sostituisci tutto e sensibilità alle maiuscole.

## File e sicurezza dei tuoi dati

- **File recenti**, apertura tramite trascinamento e conferma delle modifiche non salvate.
- **Modelli di documento** (File → Nuovo da modello).
- **Front matter** YAML/TOML conservato verbatim.
- **Monitoraggio del file su disco**: rileva modifiche esterne e offre di ricaricare.
- **Salvataggio automatico e recupero** dopo una chiusura anomala.

## Internazionalizzazione

Interfaccia in 9 lingue: spagnolo, inglese, tedesco, francese, italiano, portoghese, polacco,
olandese e rumeno (Visualizza → Lingua; si applica al riavvio).
