# Utilização

## Abrir e guardar

- Novo (Ctrl+N), Abrir (Ctrl+O), Salvar (Ctrl+S), Salvar como (Ctrl+Shift+S).
  Tudo em UTF-8.
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
  Enter (um ponto vazio sai da lista).
- Citações e blocos de código.

Consulte todos os atalhos em [Atalhos de teclado](Atajos-pt).

## Inserir

- Link e Imagem (com caminho relativo ao documento para que seja portátil).
- **Colar imagem**: a imagem da área de transferência é guardada como PNG junto ao seu `.md` e
  é inserida como `![](ruta)`. Também funciona arrastando ou colando sobre o editor.
- Tabela, Régua horizontal e Fórmula (Ctrl+Shift+F).

## Tabelas

Com o cursor dentro de uma tabela, o menu Tabela permite adicionar ou eliminar linhas e
colunas e alinhar cada coluna (esquerda/centro/direita). O alinhamento conserva-se
ao guardar.

## Fórmulas

Insira fórmulas TeX em linha (`$...$`) ou em bloco (`$$...$$`) com Inserir → Fórmula
(Ctrl+Shift+F), com pré-visualização ao vivo. Clique duas vezes sobre uma fórmula para a editar. Mais
detalhe em [Características](Caracteristicas-pt#fórmulas-tex).

## Modos de vista

- **WYSIWYG** (por predefinição): só o resultado renderizado.
- **Código-fonte** (Ctrl+Shift+M): o Markdown em bruto, em ecrã inteiro.
- **Vista dividida** (Ctrl+Shift+D): renderização e código lado a lado, sincronizados.

## Localizar e substituir

Ctrl+F para localizar, Ctrl+H para substituir. Inclui anterior/seguinte, substituir
tudo e sensibilidade a maiúsculas.

## Exportar e imprimir

Arquivo → Exportar oferece PDF, HTML, ODF (.odt) e LaTeX (.tex); Imprimir é Ctrl+P.
Em ODF e LaTeX incorpora-se o idioma do documento.

## Recuperação automática

O md-editor guarda um rascunho a cada poucos segundos. Se a aplicação se fechar de forma
anómala, ao reabrir oferece-lhe recuperar o que estava a escrever.
