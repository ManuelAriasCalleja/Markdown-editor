# Manuel d'utilisation

**md-editor** est un éditeur Markdown visuel (WYSIWYG) : vous écrivez et
appliquez la mise en forme sur le texte déjà rendu, sans voir le code. À
l'enregistrement, le document est sérialisé de nouveau en Markdown pur.

## Sommaire

- [Ouvrir et enregistrer](#ouvrir-et-enregistrer)
- [Mettre le texte en forme](#mettre-le-texte-en-forme)
- [Titres, listes et blocs](#titres-listes-et-blocs)
- [Liens et images](#liens-et-images)
- [Tableaux](#tableaux)
- [Formules mathematiques](#formules-mathematiques)
- [Rechercher et remplacer](#rechercher-et-remplacer)
- [Plan du document](#plan-du-document)
- [Mode sans distraction](#mode-sans-distraction)
- [Vue source](#vue-source)
- [Exporter et imprimer](#exporter-et-imprimer)
- [Thèmes et apparence](#themes-et-apparence)
- [Récupération automatique](#recuperation-automatique)
- [Raccourcis](#raccourcis)

## Ouvrir et enregistrer

- **Fichier → Nouveau** (Ctrl+N) crée un document vide.
- **Fichier → Ouvrir…** (Ctrl+O) ouvre un `.md` existant. L'application
  mémorise les fichiers les plus récents dans **Fichier → Ouvrir les
  récents**.
- **Enregistrer** (Ctrl+S) et **Enregistrer sous…** (Ctrl+Shift+S) écrivent
  le document en UTF-8.
- Si le fichier change en dehors de l'éditeur, l'application le détecte et,
  si vous n'avez aucune modification non enregistrée, le recharge ; sinon,
  elle demande quoi faire.
- Vous pouvez aussi **glisser-déposer** un fichier sur la fenêtre pour
  l'ouvrir.

### Front matter

Si le document commence par un bloc `---…---` (YAML) ou `+++…+++` (TOML), il
est conservé tel quel à l'enregistrement : il n'apparaît pas dans l'éditeur
et n'est pas modifiable. Il sert aux métadonnées telles que `title`, `lang`,
etc., utilisées lors de l'exportation.

## Mettre le texte en forme

Sélectionnez un fragment et appliquez la mise en forme depuis la barre
d'outils ou le menu **Format** :

- **Gras** (Ctrl+B), **Italique** (Ctrl+I), **Souligné** (Ctrl+U),
  **Barré**.
- **Code en ligne** pour les fragments en `chasse fixe`.
- **Lien** : ajoute `[texte](url)` sur la sélection.

Les boutons de la barre reflètent la mise en forme active sous le curseur.

## Titres, listes et blocs

- **Titres** H1–H6 depuis **Format → Titre** ou avec Ctrl+1 … Ctrl+6.
- **Listes** : à puces, numérotées et de tâches (avec une case à cocher).
  Appuyer sur Entrée à la fin d'un élément crée le suivant automatiquement ;
  appuyer sur Entrée sur un élément vide sort de la liste.
- **Citation** (`>` au début d'un paragraphe) et **bloc de code** s'appliquent
  depuis la barre ; tous deux effectuent correctement l'aller-retour vers
  Markdown.

## Liens et images

- **Insertion → Lien…** ouvre une boîte de dialogue avec les champs texte et
  URL. Si vous aviez une sélection, elle est utilisée comme texte.
- **Ctrl+clic** sur un lien l'ouvre dans le navigateur du système ; le survol
  affiche l'URL dans la barre d'état.
- **Images** : glissez un fichier, collez une image depuis le presse-papiers,
  ou utilisez **Insertion → Coller l'image**. L'image est enregistrée en PNG
  à côté du `.md` et insérée comme `![alt](chemin-relatif)` ; elle survit
  ainsi à l'aller-retour vers Markdown (ce n'est pas le cas des images
  incorporées).

## Tableaux

- **Tableau → Insérer un tableau…** demande le nombre de lignes et de
  colonnes.
- Les actions du menu **Tableau** (ajouter/supprimer une ligne ou une
  colonne, aligner une colonne) ne sont activées que lorsque le curseur se
  trouve à l'intérieur d'un tableau.
- L'alignement des colonnes (gauche/centre/droite) est conservé à
  l'enregistrement sous la forme `:--`/`:-:`/`--:`.

## Formules mathématiques

md-editor prend en charge les **formules TeX** en ligne (`$...$`) et en bloc
(`$$...$$`), avec la syntaxe LaTeX habituelle (Pandoc, Obsidian, Quarto…).
Aucune dépendance externe n'est nécessaire.

- **Insertion → Formule…** (Ctrl+Shift+F) ouvre une boîte de dialogue avec un
  champ TeX et un **aperçu en direct** : à mesure que vous tapez, vous voyez
  le rendu final. Choisissez *En ligne* ou *Bloc* et validez pour l'insérer.
- Dans l'éditeur, les formules apparaissent en italique avec la couleur
  d'accentuation du thème, avec de **vrais exposants/indices** (et non des
  caractères Unicode plats) : `x²`, `Hᵢ`, et ainsi de suite — l'alignement
  vertical de Qt met n'importe quel caractère à l'échelle correctement.
- **Double-cliquez** sur une formule pour rouvrir la boîte de dialogue avec
  son TeX d'origine préchargé : modifiez et validez pour la remplacer.
- Les formules sont **atomiques** : taper à l'intérieur de l'une d'elles
  déclenche un rappel d'utiliser le double-clic pour l'éditer ;
  Retour arrière/Suppr au bord supprime tout le groupe.
- À l'**exportation**, les formules sont conservées : LaTeX les émet telles
  quelles (avec `amsmath` et `amssymb` dans le préambule) ; HTML/PDF/ODF
  conservent les exposants/indices en alignement vertical de Qt dans le
  format cible.
- En **vue source**, vous les voyez comme `$...$` / `$$...$$`, avec tous les
  caractères TeX (`\sum`, `\frac`, `_`, `*`) intacts à l'enregistrement.

Exemples :

```
L'énergie est $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Limitation : dans la source, `$$...$$` peut s'étendre sur plusieurs lignes
> (style Obsidian/Pandoc) ; `$...$` doit s'ouvrir et se fermer sur la même
> ligne.

## Rechercher et remplacer

- **Rechercher** (Ctrl+F) ouvre une barre inférieure avec des champs de
  recherche et de remplacement, plus des options (casse, mot entier).
- **Rechercher suivant** F3 / **Rechercher précédent** Shift+F3.

## Plan du document

Le panneau latéral de gauche affiche l'index des titres (TOC) : il se met à
jour à mesure que vous tapez, et cliquer sur une entrée fait sauter le
curseur vers ce titre. On l'affiche/masque avec F9.

## Mode sans distraction

**Affichage → Mode sans distraction** (F11) passe en plein écran avec le menu
et les barres d'outils masqués et le texte centré dans une colonne de
lecture. Le plan, s'il est visible, reste rattaché au bloc central. ESC ou
F11 en sortent.

## Vue source

**Affichage → Source Markdown** (Ctrl+Shift+M) bascule entre l'éditeur visuel
et un éditeur de texte brut en plein écran affichant le Markdown brut. Les
modifications faites en mode source sont reportées dans le document lorsque
vous revenez au mode visuel.

**Affichage → Vue divisée** (Ctrl+Shift+D) affiche les deux côte à côte :
l'éditeur visuel et la source, maintenus synchronisés (ce que vous tapez dans
l'un se reflète dans l'autre). Ce mode est mutuellement exclusif avec le mode
source plein écran.

## Exporter et imprimer

**Fichier → Exporter** propose **PDF**, **HTML**, **ODF (.odt)** et
**LaTeX (.tex)**. Pour ODF et LaTeX, la langue du document est incorporée
(prise dans le `lang`/`language` du front matter, dans le réglage de
l'application ou, en dernier recours, dans les paramètres régionaux du
système).

**Fichier → Imprimer** (Ctrl+P) ouvre la boîte de dialogue du système.

## Thèmes et apparence

- **Affichage → Thème** propose Clair, Sombre, GitHub Light, GitHub Dark,
  Monokai et Contraste élevé.
- **Affichage → Lumière chaude nocturne** atténue les bleus du fond selon
  l'heure de la journée.
- **Zoom** : Ctrl+molette de la souris, Ctrl++ / Ctrl+- et **Taille normale**
  (Ctrl+0) mettent à l'échelle toute l'interface (pas seulement le texte de
  l'éditeur).
- **Affichage → Langue** change la langue de l'interface ; l'effet est immédiat (la fenêtre est recréée).

## Récupération automatique

Pendant que vous éditez, le contenu est enregistré automatiquement toutes les
quelques secondes dans une copie de brouillon. Si l'application se ferme de
façon inattendue, au prochain lancement elle propose de récupérer ce que vous
étiez en train d'écrire.

## Raccourcis

| Action                    | Raccourci        |
|---------------------------|------------------|
| Nouveau                   | Ctrl+N           |
| Ouvrir                    | Ctrl+O           |
| Enregistrer               | Ctrl+S           |
| Enregistrer sous          | Ctrl+Shift+S     |
| Imprimer                  | Ctrl+P           |
| Annuler / Rétablir        | Ctrl+Z / Ctrl+Y  |
| Gras / Italique           | Ctrl+B / Ctrl+I  |
| Souligné                  | Ctrl+U           |
| Rechercher                | Ctrl+F           |
| Rechercher suivant / précédent | F3 / Shift+F3 |
| Titre H1 … H6             | Ctrl+1 … Ctrl+6  |
| Insérer une formule       | Ctrl+Shift+F     |
| Vue source Markdown       | Ctrl+Shift+M     |
| Vue divisée               | Ctrl+Shift+D     |
| Plan                      | F9               |
| Mode sans distraction     | F11              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Aide                      | F1               |
