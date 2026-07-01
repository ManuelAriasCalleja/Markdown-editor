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
- [Snippets (fragmentos reutilizáveis)](#snippets-fragmentos-reutilizaveis)
- [Tabelas](#tabelas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Diagramas](#diagramas)
- [Verificação ortográfica](#verificacao-ortografica)
- [Localizar e substituir](#localizar-e-substituir)
- [Estrutura do documento](#estrutura-do-documento)
- [Estatísticas do documento](#estatisticas-do-documento)
- [Modo sem distrações](#modo-sem-distracoes)
- [Modo foco](#modo-foco)
- [Vista de código](#vista-de-codigo)
- [Exportar e imprimir](#exportar-e-imprimir)
- [Temas e aparência](#temas-e-aparencia)
- [Recuperação automática](#recuperacao-automatica)
- [Acessibilidade](#acessibilidade)
- [Atalhos](#atalhos)

## Abrir e guardar

- **Ficheiro → Novo** (Ctrl+N) cria um documento vazio num separador novo.
- **Ficheiro → Novo a partir de modelo** cria um documento a partir de um esqueleto
  pronto a preencher. Os modelos estão agrupados por categoria (Pessoal, Programação,
  Ensino, Empresa, Escrita…).
- **Guardar como modelo…** guarda o documento atual como um modelo teu (com nome e
  categoria); reaparece no menu acima junto aos predefinidos. **Gerir modelos…** (no fim
  desse menu) permite editá-los ou eliminá-los.
- **Ficheiro → Abrir…** (Ctrl+O) abre um `.md` existente. A aplicação recorda os
  últimos abertos em **Ficheiro → Abrir recentes**.
- **Ficheiro → Importar** abre um documento de outro formato convertendo-o em Markdown
  num novo separador sem título (o original não é tocado): **De HTML…** (uma página web),
  **De EPUB…** (um livro; os capítulos são lidos por ordem) e **Outros formatos
  (Pandoc)…** (DOCX, ODT, RTF, LaTeX, reStructuredText…, se o Pandoc estiver instalado).
  Funciona melhor com conteúdo simples; respeita o conjunto de caracteres declarado.
- **Guardar** (Ctrl+S) e **Guardar como…** (Ctrl+Shift+S) escrevem o documento em
  UTF-8. **Abrir pasta do documento** abre a pasta do documento no gestor de
  ficheiros.
- **Reverter para o guardado** descarta as alterações por guardar e recarrega o
  ficheiro a partir do disco (pede confirmação). Só está disponível se o documento
  tiver ficheiro e alterações pendentes.
- Se o ficheiro mudar fora do editor, a aplicação deteta-o e, se não tiver
  alterações por guardar, recarrega-o; se tiver, pergunta o que fazer.
- Também pode **arrastar e largar** um ficheiro sobre a janela para o abrir.

### Separadores (vários documentos)

Pode ter vários documentos abertos ao mesmo tempo, cada um no seu **separador**:

- **Novo** (Ctrl+N), **Novo a partir de modelo** e **Abrir** (Ctrl+O) criam um
  separador (ou reutilizam o separador vazio inicial). Largar um ficheiro também o
  abre num separador; se já estiver aberto, salta para o seu.
- Mude de documento clicando no seu separador; arraste-os para os reordenar. Com
  o teclado, **Ctrl+Page Down / Ctrl+Page Up** (ou **Ctrl+Tab / Ctrl+Shift+Tab**)
  saltam para o separador seguinte ou anterior.
- **Fechar separador** (Ctrl+W) fecha o atual, perguntando se tem alterações por
  guardar. O último não se fecha: fica como documento novo.
- **Reabrir separador fechado** (Ctrl+Shift+R) reabre o último separador que fechou
  (apenas os que tinham ficheiro no disco).
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
- **Realçar** (Ctrl+Shift+H): envolve a seleção em `==marca==`; o texto aparece com
  fundo de realce. Como `==` não é sintaxe Markdown padrão, é guardado como texto
  literal.
- **Sobrescrito** (Ctrl+Shift++) e **Subscrito** (Ctrl+Shift+-): elevam ou baixam o
  texto selecionado; guardados como `^texto^` e `~texto~` (estilo Pandoc).

Os botões da barra refletem a formatação ativa sob o cursor.

**Emparelhamento automático.** Ao escrever `(`, `[`, `{` ou `` ` `` fecha-se
sozinho o par e o cursor fica no meio; se houver texto selecionado, envolve-o. Se
escrever o fecho mesmo à frente do seu par, o editor «salta-o» em vez de o
duplicar.

**Regras de entrada.** No início de uma linha, escrever um marcador Markdown de bloco
seguido de um espaço transforma a linha no sítio (sem deixar o marcador): `#` …
`######` + espaço → título H1…H6; `>` → citação; `-`, `*` ou `+` → lista com
marcadores; `1.` (ou `1)`) → lista numerada. Produz o mesmo formato que a barra.

## Títulos, listas e blocos

- **Títulos** H1–H6 a partir de **Formato → Título** ou com Ctrl+1 … Ctrl+6.
  **Promover/rebaixar** o título no cursor um nível com
  Ctrl+Shift+[ / Ctrl+Shift+].
- **Listas**: com marcadores, numeradas e de tarefas (com caixa). Premindo Enter
  no fim de um ponto cria-se automaticamente o seguinte; Enter num ponto vazio sai
  da lista. Um **clique na caixa** de uma tarefa marca-a ou desmarca-a.
- **Citação** (`>` no início de um parágrafo) e **bloco de código** aplicam-se a
  partir da barra; ambos voltam corretamente a Markdown. Com **Formato →
  Linguagem do bloco…** escolhe a linguagem de um bloco de código (com o cursor lá
  dentro) para que a sua sintaxe seja realçada.
- Ao **passar o rato** sobre um bloco de código, aparecem no canto superior direito a sua **linguagem** (clique para a alterar) e um botão para **copiar** o código.
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
- **Copiar como Markdown** copia a seleção (ou o documento inteiro) como texto
  Markdown, para colar noutro editor de Markdown ou num campo de código.
- Ao colar um **URL** sobre uma seleção de texto, o texto fica ligado
  automaticamente.
- **Editar → Limpar Markdown** normaliza todo o documento de uma só vez: uniformiza
  os marcadores para `-`, recorta os espaços no fim de cada linha, colapsa as linhas
  em branco a mais e ajusta o espaço a seguir aos `#` dos títulos. É conservador:
  não toca no interior dos blocos de código.

## Ligações e imagens

- **Inserir → Ligação…** abre uma caixa com texto e URL. Uma seleção existente é
  usada como texto.
- **Ctrl+clique** numa ligação abre-a no navegador do sistema; ao passar o rato
  por cima, o URL aparece numa dica junto ao cursor e na barra de estado.
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

## Snippets (fragmentos reutilizáveis)

Um **snippet** é um pedaço de Markdown que guarda com um nome para o inserir depois
com um par de cliques: uma assinatura, um modelo de tabela, um aviso que repete com
frequência…

- **Inserir → Snippet** abre a lista dos que tem; ao escolher um, o seu conteúdo é
  inserido onde está o cursor (funciona também na vista de código).
- **Inserir → Snippet → Gerir snippets…** abre uma caixa para criar, editar e apagar
  os seus snippets. Cada um tem um **nome** (o que vê no menu) e um **corpo** em
  Markdown.
- São guardados nas definições da aplicação, pelo que estão disponíveis em todos os
  seus documentos, não só no atual.

## Tabelas

- **Tabela → Inserir tabela…** pede linhas e colunas.
- **Inserir → Tabela da área de transferência** converte numa tabela os dados
  TSV/CSV (colunas separadas por tabulações ou vírgulas) copiados de uma folha de
  cálculo ou de um ficheiro CSV.
- As ações do menu **Tabela** (adicionar/remover linha ou coluna, alinhar coluna)
  só ficam ativas quando o cursor está dentro de uma tabela.
- O alinhamento da coluna (esquerda/centro/direita) é mantido ao guardar como
  `:--`/`:-:`/`--:`.
- **Tabela → Ordenar linhas por coluna** (ascendente/descendente) reordena as linhas
  pela coluna do cursor, mantendo o cabeçalho fixo; deteta se a coluna é numérica ou
  de texto.

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
- A barra realça **todas** as correspondências no documento e mostra um contador **«N de M»** (em que correspondência está, de quantas). **Substituir tudo** substitui-as todas de uma vez.

## Estrutura do documento

O painel lateral esquerdo mostra a estrutura de títulos (TOC): atualiza-se ao
escrever e, ao clicar numa entrada, o cursor salta para esse título.
Mostra-se/oculta-se com F9. Com **F6** move o foco do teclado para a
estrutura (mostrando-a se estiver oculta); aí, as teclas de seta percorrem os
títulos e **Enter** salta para o selecionado e devolve o foco ao editor. Premir
**F6** novamente devolve simplesmente o foco ao editor.

O **campo de filtro** no topo do painel mostra apenas os títulos que correspondem
ao que escreve (e os seus ascendentes); os botões **⊞/⊟** expandem ou recolhem
tudo. O recolhimento que define é **mantido** mesmo enquanto continua a editar.

Pode **arrastar** uma entrada da estrutura para **reordenar** essa secção —o seu
título, o seu conteúdo e as suas subsecções— dentro do documento, sem mudar o
nível. Além disso, **Inserir → Índice (TOC)** insere no documento uma lista
aninhada dos títulos. **Ver → Ir para título…** (Ctrl+G) salta para um título
escrevendo parte do seu texto, e **Ir para a linha…** (Ctrl+L) leva o cursor para um número de linha (na vista de código, para a linha do Markdown). A **Paleta de comandos** (Ctrl+Shift+P) procura e executa qualquer ação dos menus escrevendo parte do seu nome.

## Estatísticas do documento

- **Ver → Estatísticas do documento…** mostra palavras, carateres, parágrafos,
  frases e tempo de leitura estimado (do documento ou da seleção).
- **Ver → Mostrar contador de palavras** ativa um contador permanente na barra de
  estado.
- **Ver → Mostrar linha e coluna** mostra a posição do cursor (linha e coluna) na
  barra de estado.

## Modo sem distrações

**Ver → Sem distrações** (F11) entra em ecrã inteiro com o menu e as barras
ocultos e o texto centrado numa coluna de leitura. A estrutura, se visível, fica
encostada ao bloco central. ESC ou F11 saem.

## Modo foco

**Ver → Modo foco** (F12) ajuda-o a concentrar-se no que escreve sem sair da janela
normal. Um único interruptor ativa duas coisas ao mesmo tempo:

- **Máquina de escrever**: a linha do cursor mantém-se centrada na vertical. À
  medida que escreve, o texto desloca-se para que a linha ativa fique a meia altura,
  em vez de se ir colando à borda inferior.
- **Atenuação**: todo o documento se vê apagado exceto o parágrafo onde está o
  cursor, que se destaca nítido.

Funciona no editor visual e na vista de código, e é **independente** do modo sem
distrações (F11): pode usar os dois ao mesmo tempo ou cada um por sua conta.

## Vista de código

**Ver → Código-fonte Markdown** (Ctrl+Shift+M) alterna entre o editor visual e um
editor de texto simples, em ecrã inteiro, com o Markdown bruto. As alterações no
modo de código são aplicadas ao documento ao voltar ao modo visual.

**Ver → Vista dividida** (Ctrl+Shift+D) mostra ambos ao mesmo tempo, lado a lado:
o editor visual e o código-fonte, sincronizados (o que escreve num reflete-se no
outro). É exclusiva com o modo de código em ecrã inteiro.

Na vista de código há **comandos de linha** por teclado para a linha do cursor:
**Alt+↑ / Alt+↓** movem a linha para cima/baixo, **Ctrl+D** duplica-a,
**Ctrl+Shift+K** elimina-a e **Ctrl+J** une-a à seguinte.

## Exportar e imprimir

**Ficheiro → Exportar** oferece **PDF**, **HTML**, **ODF (.odt)**, **DOCX
(.docx)**, **LaTeX (.tex)**, **EPUB (.epub)** e **texto simples (.txt)**. Em ODF,
DOCX, LaTeX e EPUB é incorporado o idioma do documento (obtido do front matter
`lang`/`language`, da definição da aplicação ou, em último caso, do idioma do
sistema). No PDF são incorporados o título e o autor quando constam do front
matter (`title`, `author`).

Também pode exportar **apenas a seleção para PDF** e usar a **Pré-visualização de
impressão**.

**Ficheiro → Imprimir** (Ctrl+P) abre a caixa de diálogo do sistema; **Imprimir
seleção** imprime apenas o que está selecionado.

**Ver → Números de página ao imprimir** (ativado por predefinição) adiciona o número
de página no rodapé (`N / M`) ao imprimir e ao exportar para PDF.

## Temas e aparência

- **Ver → Tema** oferece Claro, Escuro, GitHub Light, GitHub Dark, Monokai, Alto contraste, Solarized Light e Solarized Dark. **Seguir o sistema** ajusta o tema claro/escuro ao do sistema
  operativo.
- **Ver → Luz quente noturna** atenua os azuis do fundo conforme a hora.
- **Ver → Entrelinha** define a altura de linha do editor: Simples, 1,5 linhas ou Duplo.
- **Ver → Realçar a linha atual** marca a linha do cursor com um fundo subtil.
- **Zoom**: Ctrl+roda do rato, Ctrl++ / Ctrl+- e **Tamanho normal** (Ctrl+0)
  escalam toda a interface (não só o texto do editor).
- **Ver → Idioma** muda o idioma da interface; aplica-se de imediato (a janela é
  recriada).

## Recuperação automática

Enquanto edita, o conteúdo é guardado automaticamente a cada poucos segundos numa
cópia de rascunho. Se a aplicação fechar de forma anómala, ao reabrir oferece
recuperar o que estava a escrever.

## Acessibilidade

- **Leitores de ecrã**: o editor, o painel de esquema, os campos de pesquisa e os restantes controlos têm nome acessível; além disso, as mensagens de estado (guardado, «não encontrado», alterações no disco…) são anunciadas por voz.
- **Apenas com teclado**: todas as ações têm atalho ou entrada de menu (F10 ou Alt abre a barra de menus). Consulta a tabela [Atalhos](#atalhos).
- **Contraste e tamanho**: o tema **Contraste elevado** e o **zoom** de toda a interface ajudam na baixa visão; o tamanho de letra inicial é o do sistema.
- **Foco**: o elemento focado é realçado com a cor de seleção do tema.

## Atalhos

| Ação                      | Atalho           |
|---------------------------|------------------|
| Novo                      | Ctrl+N           |
| Fechar separador          | Ctrl+W           |
| Reabrir separador fechado | Ctrl+Shift+R     |
| Separador seguinte / anterior | Ctrl+Page Down / Ctrl+Page Up (ou Ctrl+Tab / Ctrl+Shift+Tab) |
| Abrir                     | Ctrl+O           |
| Guardar                   | Ctrl+S           |
| Guardar como              | Ctrl+Shift+S     |
| Imprimir                  | Ctrl+P           |
| Anular / Refazer          | Ctrl+Z / Ctrl+Y  |
| Negrito / Itálico         | Ctrl+B / Ctrl+I  |
| Realçar (==marca==)       | Ctrl+Shift+H     |
| Sobrescrito / Subscrito   | Ctrl+Shift++ / Ctrl+Shift+- |
| Sublinhado                | Ctrl+U           |
| Colar como texto simples  | Ctrl+Shift+V     |
| Colar como Markdown       | Ctrl+Alt+V       |
| Localizar                 | Ctrl+F           |
| Localizar seguinte/anterior | F3 / Shift+F3  |
| Título H1 … H6            | Ctrl+1 … Ctrl+6  |
| Promover / rebaixar título | Ctrl+Shift+[ / Ctrl+Shift+] |
| Inserir fórmula           | Ctrl+Shift+F     |
| Inserir nota de rodapé    | Ctrl+Shift+N     |
| Ir para título            | Ctrl+G           |
| Ir para a linha           | Ctrl+L           |
| Paleta de comandos        | Ctrl+Shift+P     |
| Focar estrutura / voltar ao editor | F6     |
| Vista de código Markdown  | Ctrl+Shift+M     |
| Vista dividida            | Ctrl+Shift+D     |
| Mover linha ↑ / ↓ (código) | Alt+↑ / Alt+↓   |
| Duplicar / eliminar / unir linha (código) | Ctrl+D / Ctrl+Shift+K / Ctrl+J |
| Estrutura                 | F9               |
| Sem distrações            | F11              |
| Modo de foco              | F12              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ajuda                     | F1               |
