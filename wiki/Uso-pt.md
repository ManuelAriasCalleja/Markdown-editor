# Utilização

## Abrir e guardar

- Novo (Ctrl+N), Abrir (Ctrl+O), Salvar (Ctrl+S), Salvar como (Ctrl+Shift+S).
  Tudo em UTF-8.
- **Separadores**: cada documento aberto ocupa o seu próprio separador; feche um com
  Ctrl+W. Ao voltar a arrancar reabrem-se os separadores da última sessão.
- **Novo a partir de modelo** (Arquivo → Novo a partir de modelo) parte de um esqueleto
  Markdown já preparado.
- **Abrir recentes** lista os seus últimos documentos.
- Também pode arrastar e largar um ficheiro sobre a janela para o abrir.
- Se o ficheiro mudar fora do md-editor, avisa-o: recarrega-o sozinho se não tinha
  alterações, ou pergunta-lhe se as tinha.

### Front matter

Se o seu documento começar com um bloco `---…---` (YAML) ou `+++…+++` (TOML),
conserva-se tal como está ao guardar (não se vê nem se edita). Serve para metadados como
`title` e `lang`, que são usados ao exportar.

## Dar formato

Use o menu Formatar ou a barra de ferramentas. Não precisa de digitar símbolos
Markdown: o editor aplica-os por si.

- Negrito (Ctrl+B), Itálico (Ctrl+I), Sublinhado (Ctrl+U), Tachado, Código em linha,
  Link (Ctrl+K).
- Títulos H1–H6 (Ctrl+1 … Ctrl+6).
- Listas com marcadores, numeradas e de tarefas, com continuação automática ao premir
  Enter (um ponto vazio sai da lista). As caixas de tarefa marcam-se com um clique.
- Citações e blocos de código.

Consulte todos os atalhos em [Atalhos de teclado](Atajos-pt).

## Editar e transformar texto

- **Colar como texto simples** (Ctrl+Shift+V) ou **Colar como Markdown** (Ctrl+Alt+V),
  que converte o HTML da área de transferência para Markdown. Colar um URL sobre uma
  seleção cria automaticamente o link.
- **Editar → Transformar texto**: MAIÚSCULAS, minúsculas, capitalizar, ordenar linhas
  e tipografia inteligente (converte `--`, `---`, `...` e as aspas retas).

## Inserir

- Link e Imagem (com caminho relativo ao documento para que seja portátil).
- **Colar imagem**: a imagem da área de transferência é guardada como PNG junto ao seu `.md` e
  é inserida como `![](ruta)`. Também funciona arrastando ou colando sobre o editor.
- Tabela, Régua horizontal, Índice (TOC) e Fórmula (Ctrl+Shift+F).
- **Nota de rodapé** (Ctrl+Shift+N): insere uma referência `[^n]` e a sua definição.
- **Admoestação**: bloco destacado (nota, conselho, importante, aviso, precaução).
- **Símbolos especiais** e **Data / Data e hora**.

## Tabelas

Com o cursor dentro de uma tabela, o menu Tabela permite adicionar ou eliminar linhas e
colunas e alinhar cada coluna (esquerda/centro/direita). O alinhamento conserva-se
ao guardar.

## Fórmulas

Insira fórmulas TeX em linha (`$...$`) ou em bloco (`$$...$$`) com Inserir → Fórmula
(Ctrl+Shift+F), com pré-visualização ao vivo. Clique duas vezes sobre uma fórmula para a editar. São
pintadas em 2D real (frações, raízes, matrizes, somatórios com limites…). Mais detalhe
em [Características](Caracteristicas-pt#fórmulas-tex).

## Diagramas

Escreva um bloco de código com a linguagem `mermaid` ou `plantuml` e, se tiver
instalada a ferramenta correspondente (`mmdc` / `plantuml`), é renderizado como
imagem sob o bloco. Se faltar, verá o comando para a instalar.

## Correção ortográfica

Ative-a em Exibir → Correção ortográfica (requer Hunspell). O idioma escolhe-se pelo
do documento ou à mão em Exibir → Idioma de correção. Clique direito sobre uma
palavra sublinhada oferece sugestões e adicioná-la ao dicionário pessoal.

## Modos de vista

- **WYSIWYG** (por predefinição): só o resultado renderizado.
- **Código-fonte** (Ctrl+Shift+M): o Markdown em bruto, em ecrã inteiro.
- **Vista dividida** (Ctrl+Shift+D): renderização e código lado a lado, sincronizados.
- **Estrutura** (F9) e **Ir para título** (Ctrl+G) para navegar pelo documento.

## Localizar e substituir

Ctrl+F para localizar, Ctrl+H para substituir. Inclui anterior/seguinte, substituir
tudo e sensibilidade a maiúsculas.

## Exportar e imprimir

Arquivo → Exportar oferece PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) e EPUB
(.epub); também Pré-visualização de impressão e Imprimir (Ctrl+P). Em ODF, DOCX e LaTeX
incorpora-se o idioma do documento.

## Recuperação automática

O md-editor guarda um rascunho a cada poucos segundos. Se a aplicação se fechar de forma
anómala, ao reabrir oferece-lhe recuperar o que estava a escrever.
