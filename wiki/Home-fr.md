# md-editor

Éditeur/visionneuse WYSIWYG de Markdown en Qt6 + C++17. Par défaut, vous éditez sur
le texte déjà rendu, sans vous soucier de la syntaxe ; mais vous pouvez aussi, en
option, voir le code Markdown, et même avoir le code et son rendu en parallèle (vue
divisée) et éditer de chaque côté. À l'enregistrement, le document est toujours
sérialisé en Markdown propre.

## Ce qu'il fait pour vous

- **Vrai WYSIWYG** : vous voyez le résultat, pas les symboles.
- **Round-trip fidèle** : ce que vous ouvrez est ce que vous enregistrez, avec des
  tableaux alignés, des listes de tâches, des citations, des blocs de code et des
  formules.
- **Trois façons de travailler** : rendu seul (par défaut), code seul, ou les deux
  en parallèle (vue divisée synchronisée).
- **Mode sans distraction** : colonne de lecture centrée, sans barres (F11), avec la
  table des matières en option (vous l'affichez ou la masquez).
- **Confort visuel** : la *Lumière chaude nocturne* atténue le bleu du fond de façon
  progressive selon l'heure de la journée, pour réduire la fatigue oculaire la nuit.
- **Formules TeX** : [en ligne](Caracteristicas-fr#formules-tex) et [en bloc](Caracteristicas-fr#formules-tex),
  avec de vrais indices/exposants et un aperçu en direct, sans dépendances externes.
- **Export** vers PDF, HTML, ODF (.odt) et LaTeX (.tex), en conservant la langue du
  document et le format des formules.
- **Affichage** : 1) 6 thèmes clairs et sombres, 2) zoom de toute l'interface, 3)
  interface traduite en 9 langues.

## Pour commencer

- [Installation](Instalacion-fr)
- [Utilisation](Uso-fr)
- [Fonctionnalités](Caracteristicas-fr)
- [Raccourcis clavier](Atajos-fr)

---

*md-editor est développé par Manuel Arias Calleja. Licence CC BY-ND 4.0.*
