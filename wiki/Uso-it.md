# Utilizzo

## Aprire e salvare

- Nuovo (Ctrl+N), Apri (Ctrl+O), Salva (Ctrl+S), Salva con nome (Ctrl+Shift+S).
  Tutto in UTF-8.
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
  Enter (un punto vuoto esce dall'elenco).
- Citazioni e blocchi di codice.

Consulta tutte le scorciatoie in [Scorciatoie da tastiera](Atajos-it).

## Inserire

- Collegamento e Immagine (con percorso relativo al documento affinché sia portatile).
- **Incolla immagine**: l'immagine degli appunti viene salvata come PNG accanto al tuo `.md` e
  inserita come `![](ruta)`. Funziona anche trascinando o incollando sull'editor.
- Tabella, Linea orizzontale e Formula (Ctrl+Shift+F).

## Tabelle

Con il cursore all'interno di una tabella, il menu Tabella permette di aggiungere o eliminare righe e
colonne e di allineare ogni colonna (sinistra/centro/destra). L'allineamento si conserva
al salvataggio.

## Formule

Inserisci formule TeX inline (`$...$`) o a blocco (`$$...$$`) con Inserisci → Formula
(Ctrl+Shift+F), con anteprima in tempo reale. Un doppio clic su una formula la modifica. Maggiori
dettagli in [Caratteristiche](Caracteristicas-it#formule-tex).

## Modalità di visualizzazione

- **WYSIWYG** (predefinita): solo il risultato renderizzato.
- **Sorgente Markdown** (Ctrl+Shift+M): il Markdown grezzo, a schermo intero.
- **Vista divisa** (Ctrl+Shift+D): rendering e codice affiancati, sincronizzati.

## Cerca e sostituisci

Ctrl+F per cercare, Ctrl+H per sostituire. Include precedente/successivo, sostituisci
tutto e sensibilità alle maiuscole.

## Esportare e stampare

File → Esporta offre PDF, HTML, ODF (.odt) e LaTeX (.tex); Stampa è Ctrl+P.
In ODF e LaTeX viene incorporata la lingua del documento.

## Recupero automatico

md-editor salva una bozza ogni pochi secondi. Se l'applicazione si chiude in modo
anomalo, alla riapertura ti offre di recuperare ciò che stavi scrivendo.
