# Manual de utilização

O **md-editor** é um editor visual (WYSIWYG) de Markdown: você escreve e aplica
formatação sobre o texto já renderizado, sem ver o código. Ao salvar, o
documento é serializado de volta para Markdown puro.

## Índice

- [Abrir e salvar](#abrir-e-salvar)
- [Formatar texto](#formatar-texto)
- [Cabeçalhos, listas e blocos](#cabecalhos-listas-e-blocos)
- [Links e imagens](#links-e-imagens)
- [Notas de rodapé](#notas-de-rodape)
- [Tabelas](#tabelas)
- [Fórmulas matemáticas](#formulas-matematicas)
- [Localizar e substituir](#localizar-e-substituir)
- [Estrutura do documento](#estrutura-do-documento)
- [Modo sem distrações](#modo-sem-distracoes)
- [Código-fonte](#codigo-fonte)
- [Exportar e imprimir](#exportar-e-imprimir)
- [Temas e aparência](#temas-e-aparencia)
- [Recuperação automática](#recuperacao-automatica)
- [Atalhos](#atalhos)

## Abrir e salvar

- **Arquivo → Novo** (Ctrl+N) cria um documento vazio.
- **Arquivo → Abrir…** (Ctrl+O) abre um `.md` existente. A aplicação
  lembra-se dos arquivos mais recentes em **Arquivo → Abrir recentes**.
- **Salvar** (Ctrl+S) e **Salvar como…** (Ctrl+Shift+S) gravam o documento
  em UTF-8.
- Se o arquivo for alterado fora do editor, a aplicação detecta-o e, se você
  não tiver alterações por salvar, recarrega-o; caso contrário, pergunta o
  que fazer.
- Também pode **arrastar e soltar** um arquivo sobre a janela para abri-lo.

### Front matter

Se o documento começar com um bloco `---…---` (YAML) ou `+++…+++` (TOML), ele
é preservado tal e qual ao salvar: não aparece no editor e não é editável.
Destina-se a metadados como `title`, `lang`, etc., que são usados ao exportar.

## Formatar texto

Selecione um trecho e aplique a formatação pela barra de ferramentas ou pelo
menu **Formatar**:

- **Negrito** (Ctrl+B), **Itálico** (Ctrl+I), **Sublinhado** (Ctrl+U),
  **Tachado**.
- **Código em linha** para trechos `monoespaçados`.
- **Link**: adiciona `[texto](url)` sobre a seleção.

Os botões da barra refletem a formatação ativa sob o cursor.

## Cabeçalhos, listas e blocos

- **Cabeçalhos** H1–H6 a partir de **Formatar → Cabeçalho** ou com
  Ctrl+1 … Ctrl+6.
- **Listas**: com marcadores, numeradas e de tarefas (com caixa de seleção).
  Ao pressionar Enter no fim de um item, o seguinte é criado automaticamente;
  ao pressionar Enter num item vazio, sai-se da lista. Um **clique na caixa de
  seleção de uma tarefa** marca-a ou desmarca-a.
- **Citação** (`>` no início de um parágrafo) e **bloco de código** aplicam-se
  pela barra; ambos fazem round-trip para Markdown corretamente.

## Links e imagens

- **Inserir → Link…** abre uma caixa de diálogo com os campos de texto e URL.
  Se você tinha uma seleção, ela é usada como texto.
- **Ctrl+clique** sobre um link abre-o no navegador do sistema; ao passar o
  rato por cima, a URL é exibida na barra de estado.
- **Imagens**: arraste um arquivo, cole uma imagem da área de transferência ou
  use **Inserir → Colar imagem**. A imagem é salva como PNG junto ao `.md` e
  inserida como `![alt](caminho-relativo)`; assim sobrevive ao round-trip para
  Markdown (as imagens incorporadas não sobrevivem).

## Notas de rodapé

- **Inserir → Nota de rodapé** (Ctrl+Shift+N) insere uma referência numerada
  `[^n]` onde está o cursor e cria a sua definição `[^n]:` no fim do documento,
  pronta para que você escreva o texto da nota.
- As referências são exibidas como **sobrescrito**; ao fazer **clique** sobre
  uma, o cursor salta para a sua definição.
- São salvas como Markdown padrão (`texto[^1]` no corpo e, abaixo, `[^1]: a
  nota`), portanto são compatíveis com outros editores.

## Tabelas

- **Tabela → Inserir tabela…** pede o número de linhas e colunas.
- As ações do menu **Tabela** (inserir/eliminar linha ou coluna, alinhar
  coluna) só ficam ativas quando o cursor está dentro de uma tabela.
- O alinhamento de coluna (esquerda/centro/direita) é preservado ao salvar
  como `:--`/`:-:`/`--:`.

## Fórmulas matemáticas

O md-editor admite **fórmulas TeX** em linha (`$...$`) e em bloco (`$$...$$`),
com a sintaxe habitual de LaTeX (Pandoc, Obsidian, Quarto…). Não é necessária
nenhuma dependência externa.

- **Inserir → Fórmula…** (Ctrl+Shift+F) abre uma caixa de diálogo com um campo
  para o TeX e uma **pré-visualização em tempo real**: à medida que escreve, vê
  como ficará. Escolha *Em linha* ou *Bloco* e confirme para inseri-la.
- No editor, as fórmulas aparecem em itálico com a cor de destaque do tema, com
  **sobrescritos e subscritos reais** (não caracteres Unicode planos): `x²`,
  `Hᵢ`, e assim por diante — o vertical-align do Qt dimensiona qualquer
  caractere corretamente.
- **Duplo clique** sobre uma fórmula reabre a caixa de diálogo com o seu TeX
  original pré-carregado: edita e, ao confirmar, ela é substituída.
- As fórmulas são **atómicas**: se digitar dentro de uma, a aplicação lembra-o
  de usar o duplo clique para editar; Backspace/Delete na borda apagam o grupo
  inteiro.
- Ao **exportar**, as fórmulas são preservadas: para LaTeX são emitidas tal e
  qual (com `amsmath` e `amssymb` no preâmbulo); para HTML/PDF/ODF mantém-se o
  sobrescrito/subscrito vertical-align do Qt no formato de destino.
- Na **vista de código** veem-se como `$...$` / `$$...$$`, com todos os
  caracteres TeX (`\sum`, `\frac`, `_`, `*`) intactos ao salvar.

Exemplos:

```
A energia é $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitação: na fonte, `$$...$$` pode abranger várias linhas (estilo
> Obsidian/Pandoc); `$...$` deve abrir e fechar na mesma linha.

## Localizar e substituir

- **Localizar** (Ctrl+F) abre uma barra inferior com campos para localizar e
  substituir, além de opções (maiúsculas/minúsculas, palavra inteira).
- **Localizar seguinte** F3 / **Localizar anterior** Shift+F3.

## Estrutura do documento

O painel lateral esquerdo mostra o índice de cabeçalhos (TOC): atualiza-se à
medida que escreve e, ao clicar numa entrada, o cursor salta para esse
cabeçalho. Mostra-se/oculta-se com F9.

Você pode **arrastar** uma entrada da estrutura para **reordenar** essa secção
—o seu cabeçalho, o seu conteúdo e as suas subsecções— dentro do documento, sem
mudar o nível. Além disso, **Inserir → Índice (TOC)** verte para o documento uma
lista aninhada com os cabeçalhos.

## Modo sem distrações

**Exibir → Modo sem distrações** (F11) entra em tela cheia com o menu e as
barras ocultos e o texto centrado numa coluna de leitura. A estrutura, se
estiver visível, fica fixa ao bloco central. ESC ou F11 saem.

## Código-fonte

**Exibir → Código-fonte Markdown** (Ctrl+Shift+M) alterna entre o editor visual
e um editor de texto simples, em tela cheia, com o Markdown bruto. As
alterações feitas no modo fonte são vertidas para o documento ao voltar ao
modo visual.

**Exibir → Vista dividida** (Ctrl+Shift+D) mostra ambos lado a lado: o editor
visual e o código-fonte, mantidos sincronizados (o que você escreve num
reflete-se no outro). É mutuamente exclusiva com o modo fonte em tela cheia.

## Exportar e imprimir

**Arquivo → Exportar** oferece **PDF**, **HTML**, **ODF (.odt)** e
**LaTeX (.tex)**. Para ODF e LaTeX, o idioma do documento é incorporado
(obtido do front matter `lang`/`language`, da configuração da aplicação ou,
em último caso, do idioma do sistema).

**Arquivo → Imprimir** (Ctrl+P) abre a caixa de diálogo do sistema.

## Temas e aparência

- **Exibir → Tema** oferece Claro, Escuro, GitHub Light, GitHub Dark, Monokai
  e Alto contraste.
- **Exibir → Luz quente noturna** atenua os azuis do fundo conforme a hora do
  dia.
- **Zoom**: Ctrl+roda do rato, Ctrl++ / Ctrl+- e **Tamanho normal** (Ctrl+0)
  dimensionam toda a interface (não apenas o texto do editor).
- **Exibir → Idioma** muda o idioma da interface; aplica-se imediatamente (a janela é recriada).

## Recuperação automática

Enquanto edita, o conteúdo é salvo automaticamente de poucos em poucos segundos
numa cópia de rascunho. Se a aplicação fechar de forma anómala, no arranque
seguinte oferece-se a recuperar o que estava a escrever.

## Atalhos

| Ação                      | Atalho           |
|---------------------------|------------------|
| Novo                      | Ctrl+N           |
| Abrir                     | Ctrl+O           |
| Salvar                    | Ctrl+S           |
| Salvar como               | Ctrl+Shift+S     |
| Imprimir                  | Ctrl+P           |
| Desfazer / Refazer        | Ctrl+Z / Ctrl+Y  |
| Negrito / Itálico         | Ctrl+B / Ctrl+I  |
| Sublinhado                | Ctrl+U           |
| Localizar                 | Ctrl+F           |
| Localizar seguinte/anterior | F3 / Shift+F3  |
| Cabeçalho H1 … H6         | Ctrl+1 … Ctrl+6  |
| Inserir fórmula           | Ctrl+Shift+F     |
| Inserir nota de rodapé    | Ctrl+Shift+N     |
| Vista de código Markdown  | Ctrl+Shift+M     |
| Vista dividida            | Ctrl+Shift+D     |
| Estrutura                 | F9               |
| Modo sem distrações       | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Ajuda                     | F1               |
