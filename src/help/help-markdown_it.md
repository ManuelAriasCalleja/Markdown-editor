# Markdown in una pagina

**Markdown** è un modo di scrivere testo formattato usando simboli semplici. Ciò
che è a sinistra è quello che digiti; a destra, come appare. In md-editor non
hai bisogno di digitare questi simboli: li applichi dalla barra degli strumenti
e, al salvataggio, l'editor li genera per te.

## Indice

- [Paragrafi e interruzioni di riga](#paragrafi-e-interruzioni-di-riga)
- [Intestazioni](#intestazioni)
- [Enfasi](#enfasi)
- [Elenchi](#elenchi)
- [Citazioni](#citazioni)
- [Codice](#codice)
- [Collegamenti e immagini](#collegamenti-e-immagini)
- [Note a piè di pagina](#note-a-pie-di-pagina)
- [Righe orizzontali](#righe-orizzontali)
- [Tabelle](#tabelle)
- [Formule matematiche](#formule-matematiche)
- [Escape](#escape)

## Paragrafi e interruzioni di riga

Separa i paragrafi con una **riga vuota**. All'interno di un paragrafo, due spazi
in coda alla fine di una riga forzano un'interruzione di riga senza iniziare un
nuovo paragrafo.

## Intestazioni

```
# Intestazione di livello 1
## Intestazione di livello 2
### Intestazione di livello 3
```

Fino a sei livelli (`######`). In md-editor puoi anche applicarle da
**Formato → Intestazione** o con Ctrl+1 … Ctrl+6.

## Enfasi

- `*corsivo*` o `_corsivo_` → *corsivo*
- `**grassetto**` o `__grassetto__` → **grassetto**
- `***grassetto e corsivo***` → ***grassetto e corsivo***
- `~~barrato~~` → ~~barrato~~

## Elenchi

**Puntati** (con `-`, `*` o `+`):

```
- Mela
- Pera
  - Conference
  - Decana
```

**Numerati**:

```
1. Primo
2. Secondo
3. Terzo
```

**Attività** (caselle di spunta):

```
- [x] Fatto
- [ ] Da fare
```

## Citazioni

Una o più righe precedute da `>`:

```
> Chi legge molto e cammina molto, vede molto e sa molto.
> — Miguel de Cervantes
```

## Codice

**Inline**: racchiudi con un accento grave: `` `codice` ``.

**Blocco**: tre accenti gravi all'inizio e alla fine; facoltativamente, il nome
del linguaggio per colorarlo:

````
```python
def saluta(nome):
    print(f"Ciao, {nome}")
```
````

## Collegamenti e immagini

- **Collegamento**: `[testo](https://esempio.com)`
- **Collegamento con titolo**: `[testo](https://esempio.com "Titolo del tooltip")`
- **Immagine**: `![testo alternativo](percorso/immagine.png)` — come un
  collegamento, ma con un `!` davanti.

In md-editor, **Ctrl+clic** su un collegamento lo apre nel browser di sistema.

## Note a piè di pagina

Un **riferimento** nel testo e la sua **definizione** a parte, collegati da un
identificatore `[^id]`:

```
Un'affermazione con la sua sfumatura[^1].

[^1]: Il testo della nota va qui.
```

L'`id` può essere un numero (`[^1]`) o una parola (`[^nota]`). In md-editor,
**Inserisci → Nota a piè di pagina** (Ctrl+Shift+N) crea il riferimento e la sua
definizione al posto tuo; i riferimenti appaiono come apice e un clic salta alla
definizione.

## Righe orizzontali

Tre o più trattini, asterischi o trattini bassi su una riga a sé stante:

```
---
```

## Tabelle

```
| Prodotto | Quantità | Prezzo |
|----------|---------:|:------:|
| Pane     |        2 | 1,20 € |
| Latte    |        1 | 0,95 € |
```

I due punti nella riga di separazione impostano l'allineamento delle colonne:
`:--` a sinistra, `:-:` al centro, `--:` a destra. md-editor conserva
l'allineamento al salvataggio.

## Formule matematiche

Il Markdown standard **non** definisce le formule, ma una convenzione molto
diffusa (Pandoc, Obsidian, Quarto, GitHub) supporta la sintassi TeX tra `$...$`
(inline) e `$$...$$` (blocco). md-editor implementa questa convenzione.

```
La formula $E = mc^2$ è celebre.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

I caratteri speciali di TeX (`\`, `_`, `*`, `{`, `}`) vengono mantenuti intatti
all'interno delle formule — l'editor li protegge affinché il parser Markdown non
li confonda con corsivo o grassetto.

In md-editor le formule appaiono renderizzate con veri apici e pedici (non come
`$x^2$` letterale). Inseriscine una con **Inserisci → Formula…** (Ctrl+Shift+F) o
fai doppio clic su una esistente per modificarla.

## Escape

Per far apparire un simbolo Markdown come letterale (senza che agisca come
formattazione), anteponigli una barra rovesciata: `\*non è corsivo\*` → \*non è
corsivo\*.

I simboli che si possono mettere in escape sono:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
