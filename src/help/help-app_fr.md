# Manuel d'utilisation

**md-editor** est un éditeur Markdown visuel (WYSIWYG) : vous écrivez et mettez
en forme sur le texte déjà rendu, sans voir le code. À l'enregistrement, le
document est sérialisé de nouveau en Markdown pur.

## Sommaire

- [Ouvrir et enregistrer](#ouvrir-et-enregistrer)
- [Mettre en forme le texte](#mettre-en-forme-le-texte)
- [Titres, listes et blocs](#titres-listes-et-blocs)
- [Transformer le texte et le presse-papiers](#transformer-le-texte-et-le-presse-papiers)
- [Liens et images](#liens-et-images)
- [Notes de bas de page](#notes-de-bas-de-page)
- [Encadrés, symboles et raccourcis de texte](#encadres-symboles-et-raccourcis-de-texte)
- [Snippets (fragments réutilisables)](#snippets-fragments-reutilisables)
- [Tableaux](#tableaux)
- [Formules mathématiques](#formules-mathematiques)
- [Diagrammes](#diagrammes)
- [Correction orthographique](#correction-orthographique)
- [Rechercher et remplacer](#rechercher-et-remplacer)
- [Plan du document](#plan-du-document)
- [Statistiques du document](#statistiques-du-document)
- [Mode sans distraction](#mode-sans-distraction)
- [Mode focus](#mode-focus)
- [Vue du code](#vue-du-code)
- [Exporter et imprimer](#exporter-et-imprimer)
- [Thèmes et apparence](#themes-et-apparence)
- [Récupération automatique](#recuperation-automatique)
- [Accessibilité](#accessibilite)
- [Raccourcis](#raccourcis)

## Ouvrir et enregistrer

- **Fichier → Nouveau** (Ctrl+N) crée un document vide dans un nouvel onglet.
- **Fichier → Nouveau à partir d'un modèle** crée un document à partir d'un squelette
  prêt à remplir. Les modèles sont regroupés par catégorie (Personnel, Programmation,
  Enseignement, Entreprise, Écriture…).
- **Enregistrer comme modèle…** enregistre le document actuel comme votre propre modèle
  (avec un nom et une catégorie) ; il réapparaît dans le menu ci-dessus à côté des modèles
  intégrés. **Gérer les modèles…** (en bas de ce menu) permet de les modifier ou supprimer.
- **Fichier → Ouvrir…** (Ctrl+O) ouvre un `.md` existant. L'application retient
  les derniers ouverts dans **Fichier → Ouvrir récents**.
- **Fichier → Importer** ouvre un document d'un autre format en le convertissant en
  Markdown dans un nouvel onglet sans titre (l'original n'est pas modifié) : **Depuis
  HTML…** (une page web), **Depuis EPUB…** (un livre ; ses chapitres sont lus dans
  l'ordre) et **Autres formats (Pandoc)…** (DOCX, ODT, RTF, LaTeX, reStructuredText…, si
  Pandoc est installé). Fonctionne mieux avec du contenu simple ; respecte le jeu de
  caractères déclaré.
- **Enregistrer** (Ctrl+S) et **Enregistrer sous…** (Ctrl+Maj+S) écrivent le
  document en UTF-8. **Ouvrir le dossier contenant** ouvre le dossier du document
  dans le gestionnaire de fichiers.
- **Revenir à la version enregistrée** abandonne les modifications non enregistrées
  et recharge le fichier depuis le disque (avec confirmation). Disponible seulement
  si le document a un fichier et des modifications en attente.
- Si le fichier change hors de l'éditeur, l'application le détecte et, si vous
  n'avez pas de modifications non enregistrées, le recharge ; sinon, elle demande
  quoi faire.
- Vous pouvez aussi **glisser-déposer** un fichier sur la fenêtre pour l'ouvrir.

### Onglets (plusieurs documents)

Vous pouvez avoir plusieurs documents ouverts à la fois, chacun dans son **onglet** :

- **Nouveau** (Ctrl+N), **Nouveau à partir d'un modèle** et **Ouvrir** (Ctrl+O)
  créent un onglet (ou réutilisent l'onglet vide initial). Déposer un fichier
  l'ouvre aussi dans un onglet ; s'il est déjà ouvert, on saute à son onglet.
- Changez de document en cliquant sur son onglet ; faites glisser les onglets pour
  les réordonner. Au clavier, **Ctrl+Page suiv. / Ctrl+Page préc.** (ou
  **Ctrl+Tab / Ctrl+Maj+Tab**) passent à l'onglet suivant ou précédent.
- **Fermer l'onglet** (Ctrl+W) ferme l'onglet courant en demandant s'il a des
  modifications non enregistrées. Le dernier onglet ne se ferme pas : il devient un
  nouveau document.
- **Rouvrir l'onglet fermé** (Ctrl+Maj+R) rouvre le dernier onglet que vous avez
  fermé (seulement ceux qui avaient un fichier sur le disque).
- L'étiquette affiche le nom du fichier et un point (•) en cas de modifications non
  enregistrées.
- À la fermeture de l'application, les documents ouverts sont mémorisés et tous
  rouverts au prochain démarrage.

### *Front matter*

Si le document commence par un bloc `---…---` (YAML) ou `+++…+++` (TOML), il est
conservé tel quel à l'enregistrement : il n'est ni affiché dans l'éditeur ni
modifié. Il sert aux métadonnées comme `title`, `lang`, etc., utilisées à
l'export.

## Mettre en forme le texte

Sélectionnez un fragment et appliquez la mise en forme via la barre d'outils ou
le menu **Format** :

- **Gras** (Ctrl+B), **Italique** (Ctrl+I), **Souligné** (Ctrl+U), **Barré**.
- **Code en ligne** pour les fragments en `chasse fixe`.
- **Lien** : ajoute `[texte](url)` sur la sélection.
- **Surligner** (Ctrl+Maj+H) : entoure la sélection de `==marque==` ; le texte
  s'affiche avec un fond de surlignage. Comme `==` n'est pas une syntaxe Markdown
  standard, c'est enregistré comme texte littéral.
- **Exposant** (Ctrl+Maj++) et **Indice** (Ctrl+Maj+-) : montent ou descendent le
  texte sélectionné ; enregistrés comme `^texte^` et `~texte~` (style Pandoc).

Les boutons de la barre reflètent la mise en forme active sous le curseur.

**Appariement automatique.** En tapant `(`, `[`, `{` ou `` ` ``, la paire se ferme
toute seule et le curseur reste au milieu ; si du texte est sélectionné, il est
entouré. Si vous tapez le caractère de fermeture juste devant son homologue,
l'éditeur le « saute » au lieu de le dupliquer.

**Règles de saisie.** En début de ligne, taper un marqueur Markdown de bloc suivi
d'un espace transforme la ligne sur place (sans laisser le marqueur) : `#` …
`######` + espace → titre H1…H6 ; `>` → citation ; `-`, `*` ou `+` → liste à puces ;
`1.` (ou `1)`) → liste numérotée. Produit le même format que la barre.

## Titres, listes et blocs

- **Titres** H1–H6 depuis **Format → Titre** ou avec Ctrl+1 … Ctrl+6.
  **Promouvoir/abaisser** le titre au curseur d'un niveau avec
  Ctrl+Maj+[ / Ctrl+Maj+].
- **Listes** : à puces, numérotées et de tâches (avec case). Entrée à la fin d'un
  point crée automatiquement le suivant ; Entrée sur un point vide quitte la
  liste. Un **clic sur la case** d'une tâche la coche ou la décoche.
- **Citation** (`>` au début d'un paragraphe) et **bloc de code** s'appliquent
  depuis la barre ; les deux font correctement l'aller-retour vers Markdown. Avec
  **Format → Langage du bloc…** vous choisissez le langage d'un bloc de code (le
  curseur étant à l'intérieur) pour que sa syntaxe soit colorée.
- **Indentation** : **Format → Augmenter/Diminuer l'indentation** imbrique listes
  et citations.

## Transformer le texte et le presse-papiers

- **Édition → Transformer le texte** agit sur la sélection : **MAJUSCULES**,
  **minuscules**, **Capitaliser** et **Trier les lignes**.
- **Typographie intelligente** (dans le même menu) convertit dans la sélection
  les tirets `--`/`---` en `–`/`—`, `...` en `…` et les guillemets droits en
  guillemets typographiques selon le contexte.
- **Coller comme texte brut** (Ctrl+Maj+V) colle sans mise en forme. **Coller
  comme Markdown** (Ctrl+Alt+V) convertit le contenu enrichi du presse-papiers
  (HTML) en Markdown au lieu d'incruster la mise en forme de la source.
- **Copier comme HTML** copie la sélection (ou le document) en HTML, pour la
  coller dans un courriel, un CMS, etc.
- **Copier comme Markdown** copie la sélection (ou le document entier) comme texte
  Markdown, pour le coller dans un autre éditeur Markdown ou dans un champ de code.
- Quand vous collez une **URL** sur une sélection de texte, le texte est lié
  automatiquement.
- **Édition → Nettoyer le Markdown** normalise tout le document en une passe :
  il uniformise les puces en `-`, supprime les espaces en fin de chaque ligne,
  réduit les lignes vides en trop et ajuste l'espace après les `#` des titres.
  C'est conservateur : il ne touche pas à l'intérieur des blocs de code.

## Liens et images

- **Insérer → Lien…** ouvre une boîte de dialogue avec le texte et l'URL. Une
  sélection existante est reprise comme texte.
- **Ctrl+clic** sur un lien l'ouvre dans le navigateur du système ; au survol,
  l'URL s'affiche dans une info-bulle près du curseur et dans la barre d'état.
- **Images** : glissez un fichier, collez une image du presse-papiers ou utilisez
  **Insérer → Coller l'image**. L'image est enregistrée en PNG à côté du `.md` et
  insérée comme `![alt](chemin-relatif)` ; ainsi elle survit à l'aller-retour vers
  Markdown (pas les images incrustées).

## Notes de bas de page

- **Insérer → Note de bas de page** (Ctrl+Maj+N) insère une référence numérotée
  `[^n]` au curseur et crée sa définition `[^n]:` à la fin du document, prête pour
  le texte de la note.
- Les références s'affichent en **exposant** ; un **clic** dessus déplace le
  curseur vers sa définition.
- Elles sont enregistrées en Markdown standard (`texte[^1]` dans le corps et, en
  bas, `[^1]: la note`), donc compatibles avec d'autres éditeurs.

## Encadrés, symboles et raccourcis de texte

- **Insérer → Encadré** crée un *callout* de style GitHub : une citation dont la
  première ligne est `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` ou
  `[!CAUTION]`. Il s'affiche avec un fond teinté et un titre en couleur, et est
  enregistré en Markdown compatible GitHub.
- **Insérer → Symboles spéciaux…** ouvre une table de caractères par catégories
  (mathématiques, grec, flèches, monnaie, ponctuation…) ; un clic insère le
  symbole et la boîte reste ouverte pour en insérer plusieurs.
- **Raccourcis `:nom:`** : en tapant un code comme `:alpha:` ou `:euro:`, il est
  remplacé par le symbole correspondant (α, €…).
- **Insérer → Date** et **Date et heure** insèrent la date (et l'heure) actuelle
  au format localisé.

## Snippets (fragments réutilisables)

Un **snippet** est un morceau de Markdown que vous enregistrez sous un nom pour
l'insérer ensuite en deux clics : une signature, un modèle de tableau, un avis que
vous répétez souvent…

- **Insérer → Snippet** déroule la liste de ceux que vous avez ; en en choisissant
  un, son contenu est inséré là où se trouve le curseur (cela fonctionne aussi dans
  la vue du code).
- **Insérer → Snippet → Gérer les snippets…** ouvre une boîte de dialogue pour
  créer, modifier et supprimer vos snippets. Chacun a un **nom** (celui que vous
  voyez dans le menu) et un **corps** en Markdown.
- Ils sont enregistrés dans les réglages de l'application : ils sont donc
  disponibles dans tous vos documents, pas seulement le document courant.

## Tableaux

- **Tableau → Insérer un tableau…** demande lignes et colonnes.
- **Insertion → Tableau depuis le presse-papiers** convertit en tableau les données
  TSV/CSV (colonnes séparées par des tabulations ou des virgules) copiées depuis un
  tableur ou un fichier CSV.
- Les actions du menu **Tableau** (ajouter/supprimer ligne ou colonne, aligner
  une colonne) ne sont actives que lorsque le curseur est dans un tableau.
- L'alignement de colonne (gauche/centre/droite) est conservé à l'enregistrement
  sous la forme `:--`/`:-:`/`--:`.
- **Tableau → Trier les lignes par colonne** (croissant/décroissant) réordonne les
  lignes selon la colonne du curseur, en gardant l'en-tête fixe ; détecte si la
  colonne est numérique ou textuelle.

## Formules mathématiques

md-editor prend en charge les **formules TeX** en ligne (`$...$`) et en bloc
(`$$...$$`), avec la syntaxe LaTeX habituelle (Pandoc, Obsidian, Quarto…). Aucune
dépendance externe n'est nécessaire.

- **Insérer → Formule…** (Ctrl+Maj+F) ouvre une boîte avec un champ pour le TeX et
  un **aperçu en direct** : à mesure que vous tapez, vous voyez le rendu.
  Choisissez *En ligne* ou *Bloc* et validez pour l'insérer.
- Les formules sont composées en **2D réel** : les fractions (`\frac`) sont
  empilées avec une barre, les grands opérateurs (`\sum`, `\int`, `\prod`…)
  affichent leurs bornes au-dessus et au-dessous, les racines (`\sqrt`) portent
  leur vinculum, et il y a des matrices (`\begin{pmatrix}`…), des coefficients
  binomiaux (`\binom`) et des accents (`\hat`, `\vec`, `\bar`…). Les plus simples
  (puissances, indices, grec) sont composées en ligne. Le rendu s'adapte au zoom.
- **Double-clic** sur une formule rouvre la boîte avec son TeX d'origine
  préchargé : vous modifiez et à la validation elle est remplacée.
- Les formules sont **atomiques** : si vous tapez à l'intérieur, l'application
  vous rappelle d'utiliser le double-clic ; Retour arrière/Suppr au bord
  suppriment tout le groupe.
- À l'**export**, elles sont conservées : vers LaTeX elles sont émises telles
  quelles (avec `amsmath` et `amssymb` dans le préambule) ; vers HTML/PDF/ODF
  elles sont ramenées à leur approximation en ligne.
- Dans la **vue du code**, elles apparaissent comme `$...$` / `$$...$$`, avec tous
  les caractères TeX (`\sum`, `\frac`, `_`, `*`) intacts à l'enregistrement.

Exemples :

```
L'énergie est $E = mc^2$.

$$
\sum_{i=1}^n a_i = \frac{n(n+1)}{2}
$$
```

> Dans la source, `$$...$$` peut s'étendre sur plusieurs lignes (style
> Obsidian/Pandoc) ; `$...$` doit ouvrir et fermer sur la même ligne.

## Diagrammes

Un bloc de code avec le langage `mermaid` ou `plantuml` est **prévisualisé comme
une image** juste sous le bloc, sans toucher au code (qui reste modifiable) ni au
Markdown enregistré.

- Il faut que l'outil correspondant soit installé : **`plantuml`** (avec Java)
  pour PlantUML, ou **`mmdc`** (mermaid-cli, avec Node) pour Mermaid.
- Si l'outil manque, un avis avec la commande d'installation de votre système
  d'exploitation apparaît sous le bloc ; le bloc reste du code.
- L'image n'est que de la présentation : elle n'est pas écrite dans le Markdown et
  ne compte pas comme une modification non enregistrée.

Par exemple, un bloc de code étiqueté `mermaid` contenant `flowchart LR  A --> B
--> C` est prévisualisé comme l'organigramme correspondant.

## Correction orthographique

- Souligne en rouge les mots mal orthographiés selon la **langue du document**
  (issue du front matter `lang`, du réglage de langue ou du système). Elle ne
  vérifie ni le code, ni les formules, ni les liens.
- Un **clic droit** sur un mot souligné propose des **suggestions** (un clic le
  remplace), **Ajouter au dictionnaire** (une liste personnelle permanente) et
  **Ignorer** (pour la session).
- Elle s'active/désactive dans **Affichage → Correction orthographique**, et la
  langue se règle dans **Affichage → Langue de correction** (ou se laisse en
  automatique).
- Elle a besoin de dictionnaires Hunspell : sous Linux, ceux du système
  (`hunspell-es`, `hunspell-en-us`…) ; sous Windows/macOS, ils sont fournis avec
  l'application.

## Rechercher et remplacer

- **Rechercher** (Ctrl+F) ouvre une barre en bas avec des champs pour rechercher
  et remplacer, ainsi que des options (casse, mot entier).
- **Suivant** F3 / **Précédent** Maj+F3.
- La barre surligne **toutes** les correspondances du document et affiche un compteur **« N sur M »** (la correspondance actuelle, sur le total). **Tout remplacer** les remplace toutes d'un coup.

## Plan du document

Le panneau latéral gauche affiche le plan des titres (sommaire) : il se met à jour
à la frappe et, au clic sur une entrée, le curseur saute à ce titre. On
l'affiche/masque avec F9. Avec **F6**, vous déplacez le focus clavier
vers le plan (en l'affichant s'il est masqué) ; là, les flèches parcourent les
titres et **Entrée** saute au titre sélectionné en rendant le focus à l'éditeur.
Appuyer de nouveau sur **F6** rend simplement le focus à l'éditeur.

Le **champ de filtre** en haut du panneau n'affiche que les titres correspondant à
votre saisie (et leurs ancêtres) ; les boutons **⊞/⊟** développent ou réduisent
tout. Le pliage que vous appliquez est **conservé** même en continuant à éditer.

Vous pouvez **glisser** une entrée du plan pour **réordonner** cette section —son
titre, son contenu et ses sous-sections— dans le document, sans changer le niveau.
De plus, **Insérer → Table des matières (TOC)** insère dans le document une liste
imbriquée des titres. **Affichage → Aller au titre…** (Ctrl+G) saute à un titre en
tapant une partie de son texte, et **Aller à la ligne…** (Ctrl+L) place le curseur
sur un numéro de ligne (dans la vue source, sur la ligne du Markdown). La
**Palette de commandes** (Ctrl+Maj+P) recherche et exécute n'importe quelle
action des menus en tapant une partie de son nom.

## Statistiques du document

- **Affichage → Statistiques du document…** affiche mots, caractères,
  paragraphes, phrases et temps de lecture estimé (du document ou de la
  sélection).
- **Affichage → Afficher le compteur de mots** active un compteur permanent dans
  la barre d'état.
- **Affichage → Afficher la ligne et la colonne** affiche la position du curseur
  (ligne et colonne) dans la barre d'état.

## Mode sans distraction

**Affichage → Sans distraction** (F11) passe en plein écran avec le menu et les
barres masqués et le texte centré dans une colonne de lecture. Le plan, s'il est
visible, reste accolé au bloc central. ESC ou F11 quittent.

## Mode focus

**Affichage → Mode focus** (F12) vous aide à vous concentrer sur ce que vous écrivez sans
quitter la fenêtre normale. Un seul interrupteur active deux choses à la fois :

- **Machine à écrire** : la ligne du curseur reste centrée verticalement. Au fil de
  la frappe, le texte défile pour que la ligne active demeure à mi-hauteur, au lieu
  de se coller au bord inférieur.
- **Estompage** : tout le document apparaît atténué, sauf le paragraphe où se trouve
  le curseur, qui ressort net.

Cela fonctionne dans l'éditeur visuel et dans la vue du code, et c'est
**indépendant** du mode sans distraction (F11) : vous pouvez utiliser les deux à la
fois ou chacun de son côté.

## Vue du code

**Affichage → Source Markdown** (Ctrl+Maj+M) bascule entre l'éditeur visuel et un
éditeur de texte brut, en plein écran, avec le Markdown brut. Les modifications du
mode source sont reportées dans le document au retour au mode visuel.

**Affichage → Vue partagée** (Ctrl+Maj+D) montre les deux à la fois, côte à côte :
l'éditeur visuel et la source, synchronisés (ce que vous tapez dans l'un apparaît
dans l'autre). Elle est exclusive avec le mode source en plein écran.

Dans la vue source, des **commandes de ligne** au clavier agissent sur la ligne du
curseur : **Alt+↑ / Alt+↓** déplacent la ligne vers le haut/bas, **Ctrl+D** la
duplique, **Ctrl+Maj+K** la supprime et **Ctrl+J** la fusionne avec la suivante.

## Exporter et imprimer

**Fichier → Exporter** propose **PDF**, **HTML**, **ODF (.odt)**, **DOCX (.docx)**,
**LaTeX (.tex)**, **EPUB (.epub)** et **texte brut (.txt)**. En ODF, DOCX, LaTeX et
EPUB, la langue du document est incorporée (issue du front matter `lang`/`language`,
du réglage de l'application ou, en dernier recours, de la langue du système). En PDF,
le titre et l'auteur sont incorporés lorsqu'ils figurent dans le front matter
(`title`, `author`).

Vous pouvez aussi exporter **seulement la sélection en PDF** et utiliser
l'**Aperçu avant impression**.

**Fichier → Imprimer** (Ctrl+P) ouvre la boîte de dialogue du système ;
**Imprimer la sélection** n'imprime que ce qui est sélectionné.

**Affichage → Numéros de page à l'impression** (activé par défaut) ajoute le numéro
de page en pied de page (`N / M`) à l'impression et à l'export PDF.

## Thèmes et apparence

- **Affichage → Thème** propose Clair, Sombre, GitHub Light, GitHub Dark, Monokai, Contraste élevé, Solarized Light et Solarized Dark. **Suivre le système** aligne le thème clair/sombre sur celui
  du système.
- **Affichage → Lumière chaude nocturne** atténue les bleus du fond selon l'heure.
- **Affichage → Interligne** règle la hauteur de ligne de l'éditeur : Simple, 1,5 ligne ou Double.
- **Affichage → Surligner la ligne actuelle** marque la ligne du curseur avec un fond discret.
- **Zoom** : Ctrl+molette, Ctrl++ / Ctrl+- et **Taille normale** (Ctrl+0) mettent
  à l'échelle toute l'interface (pas seulement le texte de l'éditeur).
- **Affichage → Langue** change la langue de l'interface ; elle s'applique
  immédiatement (la fenêtre est recréée).

## Récupération automatique

Pendant que vous éditez, le contenu est enregistré automatiquement toutes les
quelques secondes dans une copie brouillon. Si l'application se ferme anormalement,
elle propose à la réouverture de récupérer ce que vous étiez en train d'écrire.

## Accessibilité

- **Lecteurs d’écran** : l’éditeur, le panneau de plan, les champs de recherche et les autres contrôles ont un nom accessible ; de plus, les messages d’état (enregistré, « introuvable », modifications sur le disque…) sont annoncés à voix haute.
- **Au clavier seul** : chaque action a un raccourci ou une entrée de menu (F10 ou Alt ouvre la barre de menus). Voir le tableau [Raccourcis](#raccourcis).
- **Contraste et taille** : le thème **Contraste élevé** et le **zoom** de toute l’interface aident en cas de basse vision ; la taille de police initiale est celle du système.
- **Focus** : l’élément ciblé est mis en évidence avec la couleur de sélection du thème.

## Raccourcis

| Action                    | Raccourci        |
|---------------------------|------------------|
| Nouveau                   | Ctrl+N           |
| Fermer l'onglet           | Ctrl+W           |
| Rouvrir l'onglet fermé    | Ctrl+Maj+R       |
| Onglet suivant / précédent| Ctrl+Page suiv. / Ctrl+Page préc. (ou Ctrl+Tab / Ctrl+Maj+Tab) |
| Ouvrir                    | Ctrl+O           |
| Enregistrer               | Ctrl+S           |
| Enregistrer sous          | Ctrl+Maj+S       |
| Imprimer                  | Ctrl+P           |
| Annuler / Rétablir        | Ctrl+Z / Ctrl+Y  |
| Gras / Italique           | Ctrl+B / Ctrl+I  |
| Surligner (==marque==)    | Ctrl+Maj+H       |
| Exposant / Indice         | Ctrl+Maj++ / Ctrl+Maj+- |
| Souligné                  | Ctrl+U           |
| Coller comme texte brut   | Ctrl+Maj+V       |
| Coller comme Markdown     | Ctrl+Alt+V       |
| Rechercher                | Ctrl+F           |
| Suivant / Précédent       | F3 / Maj+F3      |
| Titre H1 … H6             | Ctrl+1 … Ctrl+6  |
| Promouvoir / abaisser le titre | Ctrl+Maj+[ / Ctrl+Maj+] |
| Insérer une formule       | Ctrl+Maj+F       |
| Insérer une note          | Ctrl+Maj+N       |
| Aller au titre            | Ctrl+G           |
| Aller à la ligne          | Ctrl+L           |
| Palette de commandes      | Ctrl+Maj+P       |
| Plan / retour à l'éditeur | F6       |
| Vue source Markdown       | Ctrl+Maj+M       |
| Vue partagée              | Ctrl+Maj+D       |
| Déplacer la ligne ↑ / ↓ (source) | Alt+↑ / Alt+↓ |
| Dupliquer / supprimer / fusionner la ligne (source) | Ctrl+D / Ctrl+Maj+K / Ctrl+J |
| Plan                      | F9               |
| Sans distraction          | F11              |
| Mode focus                | F12              |
| Zoom + / − / Normal       | Ctrl++ / Ctrl+− / Ctrl+0 |
| Aide                      | F1               |
