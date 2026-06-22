# Manual de utilização

O **md-editor** é um editor visual (WYSIWYG) de Markdown: você escreve e aplica
formatação sobre o texto já renderizado, sem ver o código. Ao guardar, o
documento é serializado de volta para Markdown puro.

## Índice

- [Abrir e guardar](#abrir-e-guardar)
- [Formatar o texto](#formatar-o-texto)
- [Títulos, listas e blocos](#titulos-listas-e-blocos)
- [Transformar o texto e a área de transferência](#transformar-o-texto-e-a-area-de-transferencia)
- [Ligações e imagens](#ligacoes-e-imagens)
- [Notas de rodapé](#notas-de-rodape)
- [Destaques, símbolos e atalhos de texto](#destaques-simbolos-e-atalhos-de-texto)
- [Tabelas](#tabelas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Diagramas](#diagramas)
- [Verificação ortográfica](#verificacao-ortografica)
- [Localizar e substituir](#localizar-e-substituir)
- [Estrutura do documento](#estrutura-do-documento)
- [Estatísticas do documento](#estatisticas-do-documento)
- [Modo sem distrações](#modo-sem-distracoes)
- [Vista de código](#vista-de-codigo)
- [Exportar e imprimir](#exportar-e-imprimir)
- [Temas e aparência](#temas-e-aparencia)
- [Recuperação automática](#recuperacao-automatica)
- [Atalhos](#atalhos)

## Abrir e guardar

- **Ficheiro → Novo** (Ctrl+N) cria um documento vazio num separador novo.
- **Ficheiro → Novo a partir de modelo** cria um documento a partir de um
  esqueleto (carta, ata, exame…) pronto a preencher.
- **Ficheiro → Abrir…** (Ctrl+O) abre um `.md` existente. A aplicação recorda os
  últimos abertos em **Ficheiro → Abrir recentes**.
- **Guardar** (Ctrl+S) e **Guardar como…** (Ctrl+Shift+S) escrevem o documento em
  UTF-8. **Abrir pasta do documento** abre a pasta do documento no gestor de
  ficheiros.
- Se o ficheiro mudar fora do editor, a aplicação deteta-o e, se não tiver
  alterações por guardar, recarrega-o; se tiver, pergunta o que fazer.
- Também pode **arrastar e largar** um ficheiro sobre a janela para o abrir.

### Separadores (vários documentos)

Pode ter vários documentos abertos ao mesmo tempo, cada um no seu **separador**:

- **Novo** (Ctrl+N), **Novo a partir de modelo** e **Abrir** (Ctrl+O) criam um
  separador (ou reutilizam o separador vazio inicial). Largar um ficheiro também o
  abre num separador; se já estiver aberto, salta para o seu.
- Mude de documento clicando no seu separador; arraste-os para os reordenar.
- **Fechar separador** (Ctrl+W) fecha o atual, perguntando se tem alterações por
  guardar. O último não se fecha: fica como documento novo.
- A etiqueta mostra o nome do ficheiro e um ponto (•) se houver alterações por
  guardar.
- Ao fechar a aplicação, os documentos abertos são recordados e todos reabertos no
  arranque seguinte.

### *Front matter*

Se o documento começar com um bloco `---…---` (YAML) ou `+++…+++` (TOML), é
mantido tal como está ao guardar: não se vê no editor nem se edita. Serve para
metadados como `title`, `lang`, etc., usados na exportação.

## Formatar o texto

Selecione um fragmento e aplique a formatação com a barra de ferramentas ou o
menu **Formato**:

- **Negrito** (Ctrl+B), **Itálico** (Ctrl+I), **Sublinhado** (Ctrl+U),
  **Rasurado**.
- **Código em linha** para fragmentos `monoespaçados`.
- **Ligação**: adiciona `[texto](url)` sobre a seleção.

Os botões da barra refletem a formatação ativa sob o cursor.

## Títulos, listas e blocos

- **Títulos** H1–H6 a partir de **Formato → Título** ou com Ctrl+1 … Ctrl+6.
- **Listas**: com marcadores, numeradas e de tarefas (com caixa). Premindo Enter
  no fim de um ponto cria-se automaticamente o seguinte; Enter num ponto vazio sai
  da lista. Um **clique na caixa** de uma tarefa marca-a ou desmarca-a.
- **Citação** (`>` no início de um parágrafo) e **bloco de código** aplicam-se a
  partir da barra; ambos voltam corretamente a Markdown.
- **Indentação**: **Formato → Aumentar/Diminuir indentação** aninha listas e
  citações.

## Transformar o texto e a área de transferência

- **Editar → Transformar texto** atua sobre a seleção: **MAIÚSCULAS**,
  **minúsculas**, **Capitalizar** e **Ordenar linhas**.
- **Tipografia inteligente** (no mesmo menu) converte na seleção os traços
  `--`/`---` em `–`/`—`, `...` em `…` e as aspas retas em tipográficas conforme o
  contexto.
- **Colar como texto simples** (Ctrl+Shift+V) cola sem formatação. **Colar como
  Markdown** (Ctrl+Alt+V) converte o conteúdo formatado da área de transferência
  (HTML) em Markdown em vez de incorporar a formatação da origem.
- **Copiar como HTML** copia a seleção (ou o documento) como HTML, para colar num
  email, num CMS, etc.
- Ao colar um **URL** sobre uma seleção de texto, o texto fica ligado
  automaticamente.

## Ligações e imagens

- **Inserir → Ligação…** abre uma caixa com texto e URL. Uma seleção existente é
  usada como texto.
- **Ctrl+clique** numa ligação abre-a no navegador do sistema; ao passar o rato
  por cima, o URL aparece na barra de estado.
- **Imagens**: arraste um ficheiro, cole uma imagem da área de transferência ou
  use **Inserir → Colar imagem**. A imagem é guardada como PNG ao lado do `.md` e
  inserida como `![alt](caminho-relativo)`; assim sobrevive ao round-trip para
  Markdown (as imagens incorporadas não).

## Notas de rodapé

- **Inserir → Nota de rodapé** (Ctrl+Shift+N) insere uma referência numerada
  `[^n]` no cursor e cria a sua definição `[^n]:` no fim do documento, pronta para
  o texto da nota.
- As referências aparecem em **sobrescrito**; um **clique** numa delas leva o
  cursor à sua definição.
- São guardadas como Markdown padrão (`texto[^1]` no corpo e, abaixo,
  `[^1]: a nota`), pelo que são compatíveis com outros editores.

## Destaques, símbolos e atalhos de texto

- **Inserir → Destaque** cria um *callout* ao estilo do GitHub: uma citação cuja
  primeira linha é `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` ou
  `[!CAUTION]`. Aparece com fundo colorido e título a cores, e é guardado como
  Markdown compatível com o GitHub.
- **Inserir → Símbolos especiais…** abre um mapa de carateres por categorias
  (matemáticos, grego, setas, moeda, pontuação…); um clique insere o símbolo e a
  caixa fica aberta para inserir vários.
- **Atalhos `:nome:`**: ao escrever um código como `:alpha:` ou `:euro:`, é
  expandido para o símbolo correspondente (α, €…).
- **Inserir → Data** e **Data e hora** inserem a data (e hora) atual em formato
  localizado.

## Tabelas

- **Tabela → Inserir tabela…** pede linhas e colunas.
- As ações do menu **Tabela** (adicionar/remover linha ou coluna, alinhar coluna)
  só ficam ativas quando o cursor está dentro de uma tabela.
- O alinhamento da coluna (esquerda/centro/direita) é mantido ao guardar como
  `:--`/`:-:`/`--:`.

## Fórmulas matemáticas

O md-editor suporta **fórmulas TeX** em linha (`$...$`) e em bloco (`$$...$$`),
com a sintaxe LaTeX habitual (Pandoc, Obsidian, Quarto…). Não é necessária
nenhuma dependência externa.

- **Inserir → Fórmula…** (Ctrl+Shift+F) abre uma caixa com um campo para o TeX e
  uma **pré-visualização ao vivo**: à medida que escreve vê o resultado. Escolha
  *Em linha* ou *Bloco* e confirme para inseri-la.
- As fórmulas são compostas em **2D real**: as frações (`\frac`) ficam empilhadas
  com uma barra, os grandes operadores (`\sum`, `\int`, `\prod`…) mostram os seus
  limites em cima e em baixo, as raízes (`\sqrt`) levam o seu vínculo, e há
  matrizes (`\begin{pmatrix}`…), coeficientes binomiais (`\binom`) e acentos
  (`\hat`, `\vec`, `\bar`…). As mais simples (potências, índices, grego) são
  compostas em linha. O desenho escala com o zoom.
- **Duplo clique** numa fórmula reabre a caixa com o seu TeX original
  pré-carregado: edita-se e, ao confirmar, é substituída.
- As fórmulas são **atómicas**: se escrever dentro, a aplicação lembra-lhe de usar
  o duplo clique; Backspace/Delete na borda apagam o grupo inteiro.
- Na **exportação** são mantidas: para LaTeX são emitidas tal como estão (com
  `amsmath` e `amssymb` no preâmbulo); para HTML/PDF/ODF são reduzidas à sua
  aproximação em linha.
- Na **vista de código** aparecem como `$...$` / `$$...$$`, com todos os carateres
  TeX (`\sum`, `\frac`, `_`, `*`) intactos ao guardar.

Exemplos:

```
A energia é $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> No código-fonte, `$$...$$` pode ocupar várias linhas (estilo Obsidian/Pandoc);
> `$...$` tem de abrir e fechar na mesma linha.

## Diagramas

Um bloco de código com a linguagem `mermaid` ou `plantuml` é **pré-visualizado
como imagem** logo abaixo do bloco, sem tocar no código (que continua editável)
nem no Markdown guardado.

- Requer a ferramenta correspondente instalada: **`plantuml`** (com Java) para
  PlantUML, ou **`mmdc`** (mermaid-cli, com Node) para Mermaid.
- Se a ferramenta faltar, abaixo do bloco aparece um aviso com o comando de
  instalação do seu sistema operativo; o bloco mantém-se como código.
- A imagem é apenas apresentação: não é escrita no Markdown nem conta como
  alteração por guardar.

Por exemplo, um bloco de código etiquetado `mermaid` com `flowchart LR  A --> B
--> C` é pré-visualizado como o fluxograma correspondente.

## Verificação ortográfica

- Sublinha a vermelho as palavras mal escritas conforme o **idioma do documento**
  (obtido do front matter `lang`, da definição de idioma ou do sistema). Não
  verifica o código, as fórmulas nem as ligações.
- O **clique direito** sobre uma palavra sublinhada oferece **sugestões** (um
  clique substitui-a), **Adicionar ao dicionário** (uma lista pessoal permanente) e
  **Ignorar** (durante a sessão).
- Ativa-se/desativa-se em **Ver → Verificação ortográfica**, e o idioma define-se
  em **Ver → Idioma da verificação** (ou deixa-se automático).
- Precisa de dicionários Hunspell: no Linux, os do sistema (`hunspell-es`,
  `hunspell-en-us`…); no Windows/macOS vêm com a aplicação.

## Localizar e substituir

- **Localizar** (Ctrl+F) abre uma barra inferior com campos para localizar e
  substituir, além de opções (maiúsculas/minúsculas, palavra inteira).
- **Localizar seguinte** F3 / **Localizar anterior** Shift+F3.

## Estrutura do documento

O painel lateral esquerdo mostra a estrutura de títulos (TOC): atualiza-se ao
escrever e, ao clicar numa entrada, o cursor salta para esse título.
Mostra-se/oculta-se com F9.

Pode **arrastar** uma entrada da estrutura para **reordenar** essa secção —o seu
título, o seu conteúdo e as suas subsecções— dentro do documento, sem mudar o
nível. Além disso, **Inserir → Índice (TOC)** insere no documento uma lista
aninhada dos títulos. **Ver → Ir para título…** (Ctrl+G) salta para um título
escrevendo parte do seu texto.

## Estatísticas do documento

- **Ver → Estatísticas do documento…** mostra palavras, carateres, parágrafos,
  frases e tempo de leitura estimado (do documento ou da seleção).
- **Ver → Mostrar contador de palavras** ativa um contador permanente na barra de
  estado.

## Modo sem distrações

**Ver → Sem distrações** (F11) entra em ecrã inteiro com o menu e as barras
ocultos e o texto centrado numa coluna de leitura. A estrutura, se visível, fica
encostada ao bloco central. ESC ou F11 saem.

## Vista de código

**Ver → Código-fonte Markdown** (Ctrl+Shift+M) alterna entre o editor visual e um
editor de texto simples, em ecrã inteiro, com o Markdown bruto. As alterações no
modo de código são aplicadas ao documento ao voltar ao modo visual.

**Ver → Vista dividida** (Ctrl+Shift+D) mostra ambos ao mesmo tempo, lado a lado:
o editor visual e o código-fonte, sincronizados (o que escreve num reflete-se no
outro). É exclusiva com o modo de código em ecrã inteiro.

## Exportar e imprimir

**Ficheiro → Exportar** oferece **PDF**, **HTML**, **ODF (.odt)**, **DOCX
(.docx)**, **LaTeX (.tex)** e **EPUB (.epub)**. Em ODF, DOCX, LaTeX e EPUB é
incorporado o idioma do documento (obtido do front matter `lang`/`language`, da
definição da aplicação ou, em último caso, do idioma do sistema).

Também pode exportar **apenas a seleção para PDF** e usar a **Pré-visualização de
impressão**.

**Ficheiro → Imprimir** (Ctrl+P) abre a caixa de diálogo do sistema; **Imprimir
seleção** imprime apenas o que está selecionado.

## Temas e aparência

- **Ver → Tema** oferece Claro, Escuro, GitHub Light, GitHub Dark, Monokai e Alto
  contraste. **Seguir o sistema** ajusta o tema claro/escuro ao do sistema
  operativo.
- **Ver → Luz quente noturna** atenua os azuis do fundo conforme a hora.
- **Zoom**: Ctrl+roda do rato, Ctrl++ / Ctrl+- e **Tamanho normal** (Ctrl+0)
  escalam toda a interface (não só o texto do editor).
- **Ver → Idioma** muda o idioma da interface; aplica-se de imediato (a janela é
  recriada).

## Recuperação automática

Enquanto edita, o conteúdo é guardado automaticamente a cada poucos segundos numa
cópia de rascunho. Se a aplicação fechar de forma anómala, ao reabrir oferece
recuperar o que estava a escrever.

## Atalhos

| Ação                      | Atalho           |
|---------------------------|------------------|
| Novo                      | Ctrl+N           |
| Fechar separador          | Ctrl+W           |
| Abrir                     | Ctrl+O           |
| Guardar                   | Ctrl+S           |
| Guardar como              | Ctrl+Shift+S     |
| Imprimir                  | Ctrl+P           |
| Anular / Refazer          | Ctrl+Z / Ctrl+Y  |
| Negrito / Itálico         | Ctrl+B / Ctrl+I  |
| Sublinhado                | Ctrl+U           |
| Colar como texto simples  | Ctrl+Shift+V     |
| Colar como Markdown       | Ctrl+Alt+V       |
| Localizar                 | Ctrl+F           |
| Localizar seguinte/anterior | F3 / Shift+F3  |
| Título H1 … H6            | Ctrl+1 … Ctrl+6  |
| Inserir fórmula           | Ctrl+Shift+F     |
| Inserir nota de rodapé    | Ctrl+Shift+N     |
| Ir para título            | Ctrl+G           |
| Vista de código Markdown  | Ctrl+Shift+M     |
| Vista dividida            | Ctrl+Shift+D     |
| Estrutura                 | F9               |
| Sem distrações            | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ajuda                     | F1               |
