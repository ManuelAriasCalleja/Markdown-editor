# Fonctionnalités

Récapitulatif de tout ce que propose md-editor. Pour la référence complète et
technique, consultez `docs/REQUISITOS.md` dans le dépôt.

## Édition WYSIWYG et round-trip

Vous éditez sur le texte rendu et, à l'enregistrement, le document est sérialisé en
Markdown propre en UTF-8. Ce que vous ouvrez est ce que vous enregistrez : les
tableaux avec alignement, les listes imbriquées, les listes de tâches, les citations,
les blocs de code, les notes de bas de page, les admonitions et les formules sont
conservés fidèlement.

## Édition par onglets

Ouvrez plusieurs documents à la fois, chacun dans son onglet, et passez de l'un à
l'autre. Fermer un onglet avec Ctrl+W. La session rouvre les onglets au redémarrage.

## Modes d'affichage

- WYSIWYG, Source Markdown (Ctrl+Shift+M) et Vue divisée (Ctrl+Shift+D).
- En vue divisée, rendu et code se synchronisent : seul le panneau que vous n'êtes
  pas en train d'éditer se met à jour, sans sauts de curseur.

## Mode sans distraction

F11 passe en plein écran avec le texte centré dans une colonne de lecture et sans
barres. ESC ou F11 en sortent.

## Thèmes et lumière chaude nocturne

- **Huit thèmes** : Clair, Sombre, GitHub Light, GitHub Dark, Monokai, Contraste
  élevé, Solarized Light et Solarized Dark.
- **Lumière chaude nocturne** (activée par défaut) : atténue le bleu du fond de façon
  automatique et progressive selon l'heure, pour réduire la fatigue visuelle la nuit.
  Neutre le jour (07–19 h), elle se réchauffe l'après-midi (19–23 h), atteint son
  maximum la nuit (23–06 h) et se refroidit à l'aube (06–07 h). Elle se réévalue
  toute seule chaque minute et n'affecte que le fond (ni les liens ni la
  coloration).

## Plan du document

Panneau latéral (F9) avec l'index des titres ; un clic saute à la section. « Aller au
titre » (Ctrl+G) ouvre un sélecteur rapide de titres.

## Formules TeX

Formules en ligne (`$...$`) et en bloc (`$$...$$`) avec syntaxe LaTeX, sans
dépendances externes :

- Insertion avec aperçu en direct (Ctrl+Shift+F) et édition par double-clic.
- **Vraie mise en page 2D** : fractions empilées (`\frac`), racines avec vinculum
  (`\sqrt`), coefficients binomiaux (`\binom`), matrices et environnements (`matrix`,
  `pmatrix`, `cases`…), grands opérateurs avec limites au-dessus et en dessous
  (`\sum`, `\int`, `\prod`…), accents (`\hat`, `\vec`…), vrais exposants et indices,
  lettres grecques et `\mathbb`.
- Elles sont atomiques dans l'éditeur, s'adaptent au zoom et survivent au round-trip
  et à l'export. Les blocs `$$...$$` peuvent occuper plusieurs lignes.
- Limitations : `$...$` doit s'ouvrir et se fermer sur la même ligne ; les formules 2D
  en ligne apparaissent un peu hautes (celles en bloc s'affichent bien).

## Correction orthographique (en option)

Souligne les mots mal orthographiés selon la langue du document (Affichage →
Correction orthographique). La langue se choisit toute seule (front matter, réglage
ou système) ou à la main (Affichage → Langue de correction). Le clic droit propose des
suggestions et l'ajout au dictionnaire personnel. Nécessite Hunspell ; sans lui, le
reste fonctionne normalement.

## Diagrammes (en option)

Les blocs ```` ```mermaid ```` et ```` ```plantuml ```` sont rendus en image sous le
bloc, en exécutant l'outil externe (`mmdc` / `plantuml`) s'il est installé. S'il
manque, la commande d'installation pour votre système s'affiche. L'image n'est pas
enregistrée dans le Markdown.

## Coloration syntaxique

Les blocs de code sont colorés selon leur langage (familles C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… et un mode générique).

## Images

Coller ou déposer une image l'enregistre en PNG à côté du document et l'insère sous
la forme `![](ruta)` — sans l'incorporer —, de sorte que le Markdown reste portable.

## Insérer et transformer

- Insérer : lien, image, tableau, ligne, table des matières (TOC), formule, note de
  bas de page, admonition (note/avertissement…), symboles spéciaux et date/heure.
- Coller comme Markdown (Ctrl+Alt+V) convertit le HTML du presse-papiers en Markdown.
- Transformer le texte : MAJUSCULES/minuscules, capitaliser, trier les lignes et
  typographie intelligente (—, –, …, guillemets typographiques).
- Statistiques du document : mots, caractères, paragraphes, phrases et temps de
  lecture.

## Export et impression

PDF, HTML, ODF (.odt), DOCX (.docx), LaTeX (.tex) et EPUB (.epub), plus l'aperçu
avant impression et l'impression (Ctrl+P). ODF, DOCX et LaTeX incorporent la langue
du document (du front matter, du réglage de l'application ou du système).

## Zoom de toute l'interface

Ctrl++, Ctrl+- et Ctrl+0 (ou Ctrl + molette) mettent à l'échelle toute l'interface,
pas seulement le texte de l'éditeur. Le niveau est mémorisé.

## Rechercher et remplacer

Ctrl+F / Ctrl+H, avec précédent/suivant, remplacer tout et sensibilité à la casse.

## Fichiers et sécurité de vos données

- **Fichiers récents**, ouverture par glisser-déposer et confirmation des
  modifications non enregistrées.
- **Modèles de document** (Fichier → Nouveau depuis un modèle).
- **Front matter** YAML/TOML conservé verbatim.
- **Surveillance du fichier sur le disque** : détecte les changements externes et
  propose de recharger.
- **Enregistrement automatique et récupération** après une fermeture anormale.

## Internationalisation

Interface en 10 langues : espagnol, anglais, allemand, français, italien, portugais,
polonais, néerlandais, roumain et chinois simplifié (Affichage → Langue ; appliquée
immédiatement — la fenêtre est reconstruite).
