# Markdown numa página

O **Markdown** é uma forma de escrever texto formatado usando símbolos
simples. O que está à esquerda é o que você digita; à direita, como fica. No
md-editor não é preciso digitar estes símbolos: você aplica-os pela barra de
ferramentas e, ao salvar, o editor escreve-os por si.

## Índice

- [Parágrafos e quebras de linha](#paragrafos-e-quebras-de-linha)
- [Cabeçalhos](#cabecalhos)
- [Ênfase](#enfase)
- [Listas](#listas)
- [Citações](#citacoes)
- [Código](#codigo)
- [Links e imagens](#links-e-imagens)
- [Réguas horizontais](#reguas-horizontais)
- [Tabelas](#tabelas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Escapes](#escapes)

## Parágrafos e quebras de linha

Separe parágrafos com uma **linha em branco**. Dentro de um parágrafo, dois
espaços no fim de uma linha forçam uma quebra de linha sem iniciar um novo
parágrafo.

## Cabeçalhos

```
# Cabeçalho de nível 1
## Cabeçalho de nível 2
### Cabeçalho de nível 3
```

Até seis níveis (`######`). No md-editor também pode aplicá-los a partir de
**Formatar → Cabeçalho** ou com Ctrl+1 … Ctrl+6.

## Ênfase

- `*itálico*` ou `_itálico_` → *itálico*
- `**negrito**` ou `__negrito__` → **negrito**
- `***negrito e itálico***` → ***negrito e itálico***
- `~~tachado~~` → ~~tachado~~

## Listas

**Marcadores** (com `-`, `*` ou `+`):

```
- Maçã
- Pera
  - Rocha
  - Passe-Crassane
```

**Numeradas**:

```
1. Primeiro
2. Segundo
3. Terceiro
```

**Tarefas** (caixas de seleção):

```
- [x] Feito
- [ ] Pendente
```

## Citações

Uma ou mais linhas precedidas por `>`:

```
> Quem muito lê e muito anda, muito vê e muito sabe.
> — Miguel de Cervantes
```

## Código

**Em linha**: rodeie com um acento grave: `` `código` ``.

**Bloco**: três acentos graves no início e no fim; opcionalmente, o nome do
linguagem para colori-lo:

````
```python
def saudar(nome):
    print(f"Olá, {nome}")
```
````

## Links e imagens

- **Link**: `[texto](https://exemplo.com)`
- **Link com título**: `[texto](https://exemplo.com "Título da dica")`
- **Imagem**: `![texto alternativo](caminho/imagem.png)` — igual ao link, mas
  com um `!` à frente.

No md-editor, **Ctrl+clique** sobre um link abre-o no navegador do sistema.

## Réguas horizontais

Três ou mais hifens, asteriscos ou sublinhados numa linha só deles:

```
---
```

## Tabelas

```
| Produto | Quantidade | Preço  |
|---------|-----------:|:------:|
| Pão     |          2 | 1,20 € |
| Leite   |          1 | 0,95 € |
```

Os dois pontos na linha de separação definem o alinhamento da coluna: `:--`
à esquerda, `:-:` ao centro, `--:` à direita. O md-editor preserva o
alinhamento ao salvar.

## Fórmulas matemáticas

O Markdown padrão **não** define fórmulas, mas há uma convenção muito difundida
(Pandoc, Obsidian, Quarto, GitHub) que admite a sintaxe de TeX entre `$...$`
(em linha) e `$$...$$` (em bloco). O md-editor implementa esta convenção.

```
A fórmula $E = mc^2$ é famosa.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

Os caracteres especiais de TeX (`\`, `_`, `*`, `{`, `}`) são mantidos intactos
dentro das fórmulas — o editor protege-os para que o analisador de Markdown não
os confunda com itálico ou negrito.

No md-editor as fórmulas aparecem renderizadas com sobrescritos e subscritos
reais (não como `$x^2$` literal). Insira uma com **Inserir → Fórmula…**
(Ctrl+Shift+F) ou faça duplo clique sobre uma existente para editá-la.

## Escapes

Para que um símbolo de Markdown apareça literal (sem atuar como formatação),
ponha uma barra invertida à frente dele: `\*não é itálico\*` → \*não é itálico\*.

Os símbolos que se podem escapar são:
```
\ ` * _ { } [ ] ( ) # + - . ! |
```
