# Características

Resumo de tudo o que o md-editor oferece. Para a referência completa e técnica,
consulte `especificacion.md` no repositório.

## Edição WYSIWYG e round-trip

Edita sobre o texto renderizado e, ao guardar, serializa-se para Markdown limpo em
UTF-8. O que abre é o que guarda: tabelas com alinhamento, listas aninhadas, listas
de tarefas, citações, blocos de código e fórmulas conservam-se fielmente.

## Modos de vista

- WYSIWYG, Código-fonte (Ctrl+Shift+M) e Vista dividida (Ctrl+Shift+D).
- Na vista dividida, renderização e código sincronizam-se: só se atualiza o painel que não
  está a editar, sem saltos de cursor.

## Modo sem distrações

F11 entra em ecrã inteiro com o texto centrado numa coluna de leitura e sem
barras. ESC ou F11 saem.

## Temas e luz quente noturna

- **Seis temas**: Claro, Escuro, GitHub Light, GitHub Dark, Monokai e Alto contraste.
- **Luz quente noturna** (ativada por predefinição): atenua o azul do fundo de forma
  automática e gradual consoante a hora, para reduzir a fadiga visual à noite.
  Neutra de dia (07–19 h), vai aquecendo à tarde (19–23 h), máxima à noite
  (23–06 h) e arrefece ao amanhecer (06–07 h). Reavalia-se sozinha a cada minuto e só
  afeta o fundo (não os links nem o realce).

## Estrutura do documento

Painel lateral (F9) com o índice de títulos; um clique salta para a secção.

## Fórmulas TeX

Fórmulas em linha (`$...$`) e em bloco (`$$...$$`) com sintaxe LaTeX, sem
dependências externas:

- Inserção com pré-visualização ao vivo (Ctrl+Shift+F) e edição com clique duplo.
- Superíndices e subíndices reais, letras gregas, operadores, `\frac`, `\sqrt`, `\mathbb`…
- São atómicas no editor e sobrevivem ao round-trip e à exportação.
- Limitações: `$...$` deve abrir e fechar na mesma linha; não há *layout* 2D
  (frações grandes como `(a)/(b)`).

## Realce de sintaxe

Os blocos de código são coloridos consoante a sua linguagem (famílias C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… e um modo genérico).

## Imagens

Colar ou largar uma imagem guarda-a como PNG junto ao documento e insere-a como
`![](ruta)` —não a incorpora—, de modo que o Markdown continua a ser portátil.

## Exportação e impressão

PDF, HTML, ODF (.odt) e LaTeX (.tex), além de impressão (Ctrl+P). ODF e LaTeX incorporam
o idioma do documento (do front matter, do ajuste da aplicação ou do sistema).

## Zoom de toda a interface

Ctrl++, Ctrl+- e Ctrl+0 (ou Ctrl + roda) escalam toda a interface, não só o texto
do editor. O nível é memorizado.

## Localizar e substituir

Ctrl+F / Ctrl+H, com anterior/seguinte, substituir tudo e sensibilidade a maiúsculas.

## Ficheiros e segurança dos seus dados

- **Ficheiros recentes**, abertura por arrasto e confirmação de alterações por guardar.
- **Front matter** YAML/TOML conservado verbatim.
- **Vigilância do ficheiro no disco**: deteta alterações externas e oferece recarregar.
- **Autoguardado e recuperação** após um fecho anómalo.

## Internacionalização

Interface em 9 idiomas: espanhol, inglês, alemão, francês, italiano, português, polaco,
neerlandês e romeno (Exibir → Idioma; aplica-se ao reiniciar).
