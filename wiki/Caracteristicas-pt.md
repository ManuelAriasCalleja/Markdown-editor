# Características

Resumo de tudo o que o md-editor oferece. Para a referência completa e técnica,
consulte `docs/REQUISITOS.md` no repositório.

## Edição WYSIWYG e round-trip

Edita sobre o texto renderizado e, ao guardar, serializa-se para Markdown limpo em
UTF-8. O que abre é o que guarda: tabelas com alinhamento, listas aninhadas, listas
de tarefas, citações, blocos de código, notas de rodapé, admoestações e fórmulas
conservam-se fielmente.

## Edição por separadores

Abra vários documentos ao mesmo tempo, cada um no seu separador, e alterne entre eles.
Fechar separador com Ctrl+W. A sessão reabre os separadores ao voltar a arrancar.

## Modos de vista

- WYSIWYG, Código-fonte (Ctrl+Shift+M) e Vista dividida (Ctrl+Shift+D).
- Na vista dividida, renderização e código sincronizam-se: só se atualiza o painel que não
  está a editar, sem saltos de cursor.

## Modo sem distrações

F11 entra em ecrã inteiro com o texto centrado numa coluna de leitura e sem
barras. ESC ou F11 saem.

## Temas e luz quente noturna

- **Oito temas**: Claro, Escuro, GitHub Light, GitHub Dark, Monokai, Alto contraste,
  Solarized Light e Solarized Dark.
- **Luz quente noturna** (ativada por predefinição): atenua o azul do fundo de forma
  automática e gradual consoante a hora, para reduzir a fadiga visual à noite.
  Neutra de dia (07–19 h), vai aquecendo à tarde (19–23 h), máxima à noite
  (23–06 h) e arrefece ao amanhecer (06–07 h). Reavalia-se sozinha a cada minuto e só
  afeta o fundo (não os links nem o realce).

## Estrutura do documento

Painel lateral (F9) com o índice de títulos; um clique salta para a secção. «Ir para
título» (Ctrl+G) abre um localizador rápido de títulos.

## Fórmulas TeX

Fórmulas em linha (`$...$`) e em bloco (`$$...$$`) com sintaxe LaTeX, sem
dependências externas:

- Inserção com pré-visualização ao vivo (Ctrl+Shift+F) e edição com clique duplo.
- **Layout 2D real**: frações empilhadas (`\frac`), raízes com vínculo
  (`\sqrt`), binómios (`\binom`), matrizes e ambientes (`matrix`, `pmatrix`, `cases`…),
  grandes operadores com limites em cima e em baixo (`\sum`, `\int`, `\prod`…), acentos
  (`\hat`, `\vec`…), superíndices e subíndices reais, letras gregas e `\mathbb`.
- São atómicas no editor, escalam com o zoom e sobrevivem ao round-trip e à
  exportação. Os blocos `$$...$$` podem ocupar várias linhas.
- Limitações: `$...$` deve abrir e fechar na mesma linha; as fórmulas 2D em
  linha ficam um pouco altas (as de bloco veem-se bem).

## Corretor ortográfico (opcional)

Sublinha as palavras mal escritas consoante o idioma do documento (Exibir → Correção
ortográfica). O idioma escolhe-se sozinho (front matter, ajuste ou sistema) ou à mão
(Exibir → Idioma de correção). Clique direito oferece sugestões e adicionar ao
dicionário pessoal. Requer Hunspell; sem ele, o resto funciona na mesma.

## Diagramas (opcional)

Os blocos ```` ```mermaid ```` e ```` ```plantuml ```` são renderizados como imagem
sob o bloco, executando a ferramenta externa (`mmdc` / `plantuml`) se estiver
instalada. Se faltar, mostra-se o comando de instalação para o seu sistema. A imagem
não se guarda no Markdown.

## Realce de sintaxe

Os blocos de código são coloridos consoante a sua linguagem (famílias C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… e um modo genérico).

## Imagens

Colar ou largar uma imagem guarda-a como PNG junto ao documento e insere-a como
`![](ruta)` —não a incorpora—, de modo que o Markdown continua a ser portátil.

## Inserir e transformar

- Inserir: link, imagem, tabela, régua, índice (TOC), fórmula, nota de rodapé,
  admoestação (nota/aviso…), símbolos especiais e data/hora.
- Colar como Markdown (Ctrl+Alt+V) converte o HTML da área de transferência para Markdown.
- Transformar texto: MAIÚSCULAS/minúsculas, capitalizar, ordenar linhas e tipografia
  inteligente (—, –, …, aspas tipográficas).
- Estatísticas do documento: palavras, caracteres, parágrafos, frases e tempo de
  leitura.

## Exportação e impressão

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) e EPUB (.epub), além de pré-visualização de
impressão e impressão (Ctrl+P). ODF, DOCX e LaTeX incorporam o idioma do documento
(do front matter, do ajuste da aplicação ou do sistema).

## Zoom de toda a interface

Ctrl++, Ctrl+- e Ctrl+0 (ou Ctrl + roda) escalam toda a interface, não só o texto
do editor. O nível é memorizado.

## Localizar e substituir

Ctrl+F / Ctrl+H, com anterior/seguinte, substituir tudo e sensibilidade a maiúsculas.

## Ficheiros e segurança dos seus dados

- **Ficheiros recentes**, abertura por arrasto e confirmação de alterações por guardar.
- **Modelos de documento** (Arquivo → Novo a partir de modelo).
- **Front matter** YAML/TOML conservado verbatim.
- **Vigilância do ficheiro no disco**: deteta alterações externas e oferece recarregar.
- **Autoguardado e recuperação** após um fecho anómalo.

## Internacionalização

Interface em 10 idiomas: espanhol, inglês, alemão, francês, italiano, português, polaco,
neerlandês, romeno e chinês simplificado (Exibir → Idioma; aplica-se de imediato: a
janela é reconstruída).
