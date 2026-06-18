# md-editor

Editor/visualizador WYSIWYG de Markdown em Qt6 + C++17. Por predefinição edita sobre
o texto já renderizado, sem lidar com a sintaxe; mas, opcionalmente, pode ver o código
Markdown, e até ter o código e a sua renderização em paralelo (vista dividida) e
editar de qualquer um dos dois lados. Ao guardar, serializa sempre para Markdown
limpo.

## O que faz por si

- **WYSIWYG real**: vê o resultado, não os símbolos.
- **Round-trip fiel**: o que abre é o que guarda, com tabelas alinhadas, listas
  de tarefas, citações, blocos de código e fórmulas.
- **Três formas de trabalhar**: só renderizado (por predefinição), só código, ou ambos
  em paralelo (vista dividida sincronizada).
- **Modo sem distrações**: coluna de leitura centrada, sem barras (F11), com o
  índice de conteúdos opcional (mostra-o ou oculta-o).
- **Cuidado ocular**: a *Luz quente noturna* atenua o azul do fundo de forma
  gradual consoante a hora do dia, para reduzir a fadiga ocular à noite.
- **Fórmulas TeX**: [em linha](Caracteristicas-pt#fórmulas-tex) e [em bloco](Caracteristicas-pt#fórmulas-tex),
  com superíndices/subíndices reais e pré-visualização ao vivo, sem dependências externas.
- **Exportação** para PDF, HTML, ODF (.odt) e LaTeX (.tex), conservando o idioma do
  documento e o formato das fórmulas.
- **Visualização**: 1) 6 temas claros e escuros, 2) zoom de toda a interface, 3)
  interface traduzida para 9 idiomas.

## Começar

- [Instalação](Instalacion-pt)
- [Utilização](Uso-pt)
- [Características](Caracteristicas-pt)
- [Atalhos de teclado](Atajos-pt)

---

*O md-editor é desenvolvido por Manuel Arias Calleja. Licença GPL-3.0.*
