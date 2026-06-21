#!/usr/bin/env bash
# Copia los diccionarios Hunspell del sistema (Linux) a dictionaries/, para
# empaquetarlos en compilaciones de Windows/macOS. Elige, por cada idioma de la
# interfaz, la primera variante que encuentre. Ver dictionaries/README.md.
set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
dest="$here/dictionaries"
src="/usr/share/hunspell"

# Idiomas de la interfaz → variantes a probar (la primera que exista gana).
langs=(
  "es es_ES es_AR"
  "en en_US en_GB"
  "de de_DE"
  "fr fr_FR"
  "it it_IT"
  "pt pt_PT pt_BR"
  "pl pl_PL"
  "nl nl_NL"
  "ro ro_RO"
)

[ -d "$src" ] || { echo "No existe $src (¿instalaste hunspell-*?)" >&2; exit 1; }
mkdir -p "$dest"

copied=0
for row in "${langs[@]}"; do
  picked=""
  for code in $row; do
    if [ -f "$src/$code.dic" ] && [ -f "$src/$code.aff" ]; then
      picked="$code"; break
    fi
  done
  if [ -n "$picked" ]; then
    cp -v "$src/$picked.aff" "$src/$picked.dic" "$dest/"
    copied=$((copied + 1))
  else
    echo "  (sin diccionario para: ${row%% *})" >&2
  fi
done

echo "Copiados $copied diccionario(s) en $dest"
