# Fonctionnalités

Récapitulatif de tout ce que propose md-editor. Pour la référence complète et
technique, consultez `especificacion.md` dans le dépôt.

## Édition WYSIWYG et round-trip

Vous éditez sur le texte rendu et, à l'enregistrement, le document est sérialisé en
Markdown propre en UTF-8. Ce que vous ouvrez est ce que vous enregistrez : les
tableaux avec alignement, les listes imbriquées, les listes de tâches, les citations,
les blocs de code et les formules sont conservés fidèlement.

## Modes d'affichage

- WYSIWYG, Source Markdown (Ctrl+Shift+M) et Vue divisée (Ctrl+Shift+D).
- En vue divisée, rendu et code se synchronisent : seul le panneau que vous n'êtes
  pas en train d'éditer se met à jour, sans sauts de curseur.

## Mode sans distraction

F11 passe en plein écran avec le texte centré dans une colonne de lecture et sans
barres. ESC ou F11 en sortent.

## Thèmes et lumière chaude nocturne

- **Six thèmes** : Clair, Sombre, GitHub Light, GitHub Dark, Monokai et Contraste
  élevé.
- **Lumière chaude nocturne** (activée par défaut) : atténue le bleu du fond de façon
  automatique et progressive selon l'heure, pour réduire la fatigue visuelle la nuit.
  Neutre le jour (07–19 h), elle se réchauffe l'après-midi (19–23 h), atteint son
  maximum la nuit (23–06 h) et se refroidit à l'aube (06–07 h). Elle se réévalue
  toute seule chaque minute et n'affecte que le fond (ni les liens ni la
  coloration).

## Plan du document

Panneau latéral (F9) avec l'index des titres ; un clic saute à la section.

## Formules TeX

Formules en ligne (`$...$`) et en bloc (`$$...$$`) avec syntaxe LaTeX, sans
dépendances externes :

- Insertion avec aperçu en direct (Ctrl+Shift+F) et édition par double-clic.
- Vrais exposants et indices, lettres grecques, opérateurs, `\frac`, `\sqrt`,
  `\mathbb`…
- Elles sont atomiques dans l'éditeur et survivent au round-trip et à l'export.
- Limitations : `$...$` doit s'ouvrir et se fermer sur la même ligne ; il n'y a pas
  de *layout* 2D (les grandes fractions comme `(a)/(b)`).

## Coloration syntaxique

Les blocs de code sont colorés selon leur langage (familles C/C++/Java…,
JS/TS/JSON, Python, shell/YAML/TOML… et un mode générique).

## Images

Coller ou déposer une image l'enregistre en PNG à côté du document et l'insère sous
la forme `![](ruta)` — sans l'incorporer —, de sorte que le Markdown reste portable.

## Export et impression

PDF, HTML, ODF (.odt) et LaTeX (.tex), plus l'impression (Ctrl+P). ODF et LaTeX
incorporent la langue du document (du front matter, du réglage de l'application ou du
système).

## Zoom de toute l'interface

Ctrl++, Ctrl+- et Ctrl+0 (ou Ctrl + molette) mettent à l'échelle toute l'interface,
pas seulement le texte de l'éditeur. Le niveau est mémorisé.

## Rechercher et remplacer

Ctrl+F / Ctrl+H, avec précédent/suivant, remplacer tout et sensibilité à la casse.

## Fichiers et sécurité de vos données

- **Fichiers récents**, ouverture par glisser-déposer et confirmation des
  modifications non enregistrées.
- **Front matter** YAML/TOML conservé verbatim.
- **Surveillance du fichier sur le disque** : détecte les changements externes et
  propose de recharger.
- **Enregistrement automatique et récupération** après une fermeture anormale.

## Internationalisation

Interface en 9 langues : espagnol, anglais, allemand, français, italien, portugais,
polonais, néerlandais et roumain (Affichage → Langue ; appliquée au redémarrage).
