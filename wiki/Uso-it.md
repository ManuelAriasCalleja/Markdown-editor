# Utilizzo

## Aprire e salvare

- Nuovo (Ctrl+N), Apri (Ctrl+O), Salva (Ctrl+S), Salva con nome (Ctrl+Shift+S).
  Tutto in UTF-8.
- **Schede**: ogni documento aperto occupa la propria scheda; chiudine una con
  Ctrl+W. Al successivo avvio vengono riaperte le schede dell'ultima sessione.
- **Nuovo da modello** (File → Nuovo da modello) parte da uno scheletro
  Markdown già pronto.
- **Apri recenti** elenca i tuoi ultimi documenti.
- Puoi anche trascinare e rilasciare un file sulla finestra per aprirlo.
- Se il file cambia al di fuori di md-editor, ti avvisa: lo ricarica da solo se non avevi
  modifiche, oppure ti chiede se le avevi.

### Front matter

Se il tuo documento inizia con un blocco `---…---` (YAML) o `+++…+++` (TOML), viene
conservato tale e quale al salvataggio (non si vede né si modifica). Serve per metadati come
`title` e `lang`, che vengono usati durante l'esportazione.

## Formattare

Usa il menu Formato o la barra degli strumenti. Non hai bisogno di digitare simboli
Markdown: li applica l'editor al posto tuo.

- Grassetto (Ctrl+B), Corsivo (Ctrl+I), Sottolineato (Ctrl+U), Barrato, Codice inline,
  Collegamento (Ctrl+K).
- Intestazioni H1–H6 (Ctrl+1 … Ctrl+6).
- Elenchi puntati, numerati e di attività, con continuazione automatica premendo
  Enter (un punto vuoto esce dall'elenco). Le caselle di attività si spuntano con un clic.
- Citazioni e blocchi di codice.

Consulta tutte le scorciatoie in [Scorciatoie da tastiera](Atajos-it).

## Modificare e trasformare testo

- **Incolla come testo semplice** (Ctrl+Shift+V) o **Incolla come Markdown** (Ctrl+Alt+V),
  che converte l'HTML degli appunti in Markdown. Incollare un URL su una
  selezione la auto-collega.
- **Modifica → Trasforma testo**: MAIUSCOLE, minuscole, capitalizza, ordina righe
  e tipografia intelligente (converte `--`, `---`, `...` e le virgolette dritte).

## Inserire

- Collegamento e Immagine (con percorso relativo al documento affinché sia portatile).
- **Incolla immagine**: l'immagine degli appunti viene salvata come PNG accanto al tuo `.md` e
  inserita come `![](ruta)`. Funziona anche trascinando o incollando sull'editor.
- Tabella, Linea orizzontale, Indice (TOC) e Formula (Ctrl+Shift+F).
- **Nota a piè di pagina** (Ctrl+Shift+N): inserisce un riferimento `[^n]` e la sua definizione.
- **Ammonizione**: blocco in evidenza (nota, consiglio, importante, avvertenza, attenzione).
- **Simboli speciali** e **Data / Data e ora**.

## Tabelle

Con il cursore all'interno di una tabella, il menu Tabella permette di aggiungere o eliminare righe e
colonne e di allineare ogni colonna (sinistra/centro/destra). L'allineamento si conserva
al salvataggio.

## Formule

Inserisci formule TeX inline (`$...$`) o a blocco (`$$...$$`) con Inserisci → Formula
(Ctrl+Shift+F), con anteprima in tempo reale. Un doppio clic su una formula la modifica. Vengono
disegnate in 2D reale (frazioni, radici, matrici, sommatorie con limiti…). Maggiori
dettagli in [Caratteristiche](Caracteristicas-it#formule-tex).

## Diagrammi

Scrivi un blocco di codice con linguaggio `mermaid` o `plantuml` e, se hai
installato lo strumento corrispondente (`mmdc` / `plantuml`), viene renderizzato come
immagine sotto il blocco. Se manca, vedrai il comando per installarlo.

## Correzione ortografica

Attivala in Visualizza → Correzione ortografica (richiede Hunspell). La lingua viene scelta in base a
quella del documento o a mano in Visualizza → Lingua di correzione. Il clic destro su una
parola sottolineata offre suggerimenti e l'aggiunta al dizionario personale.

## Modalità di visualizzazione

- **WYSIWYG** (predefinita): solo il risultato renderizzato.
- **Sorgente Markdown** (Ctrl+Shift+M): il Markdown grezzo, a schermo intero.
- **Vista divisa** (Ctrl+Shift+D): rendering e codice affiancati, sincronizzati.
- **Struttura** (F9) e **Vai a intestazione** (Ctrl+G) per navigare il documento.

## Cerca e sostituisci

Ctrl+F per cercare, Ctrl+H per sostituire. Include precedente/successivo, sostituisci
tutto e sensibilità alle maiuscole.

## Esportare e stampare

File → Esporta offre PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) ed EPUB
(.epub); anche Anteprima di stampa e Stampa (Ctrl+P). In ODF, DOCX e LaTeX
viene incorporata la lingua del documento.

## Recupero automatico

md-editor salva una bozza ogni pochi secondi. Se l'applicazione si chiude in modo
anomalo, alla riapertura ti offre di recuperare ciò che stavi scrivendo.
