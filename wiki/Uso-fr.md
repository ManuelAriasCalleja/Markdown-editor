# Utilisation

## Ouvrir et enregistrer

- Nouveau (Ctrl+N), Ouvrir (Ctrl+O), Enregistrer (Ctrl+S), Enregistrer sous
  (Ctrl+Shift+S). Le tout en UTF-8.
- **Ouvrir les récents** liste vos derniers documents.
- Vous pouvez aussi glisser-déposer un fichier sur la fenêtre pour l'ouvrir.
- Si le fichier change en dehors de md-editor, vous êtes averti : il est rechargé
  automatiquement si vous n'aviez pas de modifications, ou il vous demande si vous en
  aviez.

### Front matter

Si votre document commence par un bloc `---…---` (YAML) ou `+++…+++` (TOML), il est
conservé tel quel à l'enregistrement (il n'est ni affiché ni édité). Il sert aux
métadonnées comme `title` et `lang`, utilisées lors de l'export.

## Mettre en forme

Utilisez le menu Format ou la barre d'outils. Vous n'avez pas besoin de taper des
symboles Markdown : l'éditeur les applique pour vous.

- Gras (Ctrl+B), Italique (Ctrl+I), Souligné (Ctrl+U), Barré, Code en ligne, Lien
  (Ctrl+K).
- Titres H1–H6 (Ctrl+1 … Ctrl+6).
- Listes à puces, numérotées et de tâches, avec continuation automatique en appuyant
  sur Enter (un point vide sort de la liste).
- Citations et blocs de code.

Consultez tous les raccourcis dans [Raccourcis clavier](Atajos-fr).

## Insérer

- Lien et Image (avec chemin relatif au document pour rester portable).
- **Coller l'image** : l'image du presse-papiers est enregistrée en PNG à côté de
  votre `.md` et insérée sous la forme `![](ruta)`. Cela fonctionne aussi en glissant
  ou en collant sur l'éditeur.
- Tableau, Ligne horizontale et Formule (Ctrl+Shift+F).

## Tableaux

Avec le curseur dans un tableau, le menu Tableau permet d'ajouter ou de supprimer des
lignes et des colonnes et d'aligner chaque colonne (gauche/centre/droite).
L'alignement est conservé à l'enregistrement.

## Formules

Insérez des formules TeX en ligne (`$...$`) ou en bloc (`$$...$$`) avec Insertion →
Formule (Ctrl+Shift+F), avec un aperçu en direct. Un double-clic sur une formule
l'édite. Plus de détails dans [Fonctionnalités](Caracteristicas-fr#formules-tex).

## Modes d'affichage

- **WYSIWYG** (par défaut) : seulement le résultat rendu.
- **Source Markdown** (Ctrl+Shift+M) : le Markdown brut, en plein écran.
- **Vue divisée** (Ctrl+Shift+D) : rendu et code côte à côte, synchronisés.

## Rechercher et remplacer

Ctrl+F pour rechercher, Ctrl+H pour remplacer. Inclut précédent/suivant, remplacer
tout et sensibilité à la casse.

## Exporter et imprimer

Fichier → Exporter propose PDF, HTML, ODF (.odt) et LaTeX (.tex) ; Imprimer est
Ctrl+P. En ODF et LaTeX, la langue du document est incorporée.

## Récupération automatique

md-editor enregistre un brouillon toutes les quelques secondes. Si l'application se
ferme de façon anormale, à la réouverture elle vous propose de récupérer ce que vous
étiez en train d'écrire.
