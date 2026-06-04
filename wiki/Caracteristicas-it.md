# Caratteristiche

Riepilogo di tutto ciò che offre md-editor. Per il riferimento completo e tecnico,
consulta `especificacion.md` nel repository.

## Editing WYSIWYG e round-trip

Modifichi il testo renderizzato e, al salvataggio, viene serializzato in Markdown pulito in
UTF-8. Ciò che apri è ciò che salvi: tabelle con allineamento, elenchi annidati, elenchi
di attività, citazioni, blocchi di codice e formule si conservano fedelmente.

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

Pannello laterale (F9) con l'indice delle intestazioni; un clic salta alla sezione.

## Formule TeX

Formule inline (`$...$`) e a blocco (`$$...$$`) con sintassi LaTeX, senza
dipendenze esterne:

- Inserimento con anteprima in tempo reale (Ctrl+Shift+F) e modifica con doppio clic.
- Apici e pedici reali, lettere greche, operatori, `\frac`, `\sqrt`, `\mathbb`…
- Sono atomiche nell'editor e sopravvivono al round-trip e all'esportazione.
- Limitazioni: `$...$` deve aprirsi e chiudersi sulla stessa riga; non c'è *layout* 2D
  (frazioni grandi come `(a)/(b)`).

## Evidenziazione della sintassi

I blocchi di codice vengono colorati a seconda del loro linguaggio (famiglie C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… e una modalità generica).

## Immagini

Incollare o rilasciare un'immagine la salva come PNG accanto al documento e la inserisce come
`![](ruta)` —non la incorpora—, in modo che il Markdown rimanga portatile.

## Esportazione e stampa

PDF, HTML, ODF (.odt) e LaTeX (.tex), oltre alla stampa (Ctrl+P). ODF e LaTeX incorporano
la lingua del documento (dal front matter, dall'impostazione dell'app o dal sistema).

## Zoom dell'intera interfaccia

Ctrl++, Ctrl+- e Ctrl+0 (o Ctrl + rotellina) scalano l'intera interfaccia, non solo il testo
dell'editor. Il livello viene ricordato.

## Cerca e sostituisci

Ctrl+F / Ctrl+H, con precedente/successivo, sostituisci tutto e sensibilità alle maiuscole.

## File e sicurezza dei tuoi dati

- **File recenti**, apertura tramite trascinamento e conferma delle modifiche non salvate.
- **Front matter** YAML/TOML conservato verbatim.
- **Monitoraggio del file su disco**: rileva modifiche esterne e offre di ricaricare.
- **Salvataggio automatico e recupero** dopo una chiusura anomala.

## Internazionalizzazione

Interfaccia in 9 lingue: spagnolo, inglese, tedesco, francese, italiano, portoghese, polacco,
olandese e rumeno (Visualizza → Lingua; si applica al riavvio).
