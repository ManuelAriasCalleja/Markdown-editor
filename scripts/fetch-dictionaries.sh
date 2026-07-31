#!/usr/bin/env bash
# Descarga los diccionarios de Hunspell de los nueve idiomas de la interfaz desde
# el repositorio de diccionarios de LibreOffice y los deja, con sus licencias, en
# el directorio que se le pase.
#
# Por qué existe: en Windows y macOS no hay diccionarios del sistema, así que si
# no viajan dentro del paquete el corrector no funciona por mucho que el motor
# esté compilado. En Linux hay diccionarios del sistema, pero el AppImage se
# ejecuta en cualquier distribución y no puede darlos por hechos, así que también
# los lleva (los del sistema siguen valiendo: se buscan después, ver
# SpellChecker::searchPaths).
#
# Los .aff/.dic NO están en el repositorio de código (ver .gitignore): son de
# terceros, cambian por su cuenta y pesan. Se traen al empaquetar.
#
# Uso:  scripts/fetch-dictionaries.sh <directorio-destino>
# En CI, exporta GITHUB_TOKEN para no chocar con el límite de peticiones anónimas.
set -euo pipefail

DEST="${1:?Uso: $0 <directorio-destino>}"
BASE="https://raw.githubusercontent.com/LibreOffice/dictionaries/master"
API="https://api.github.com/repos/LibreOffice/dictionaries/contents"

# destino=carpeta-en-el-repo/nombre-de-los-ficheros
#
# El nombre de destino se normaliza a <idioma>_<REGIÓN>, que es lo que espera la
# selección de diccionario (mdspell::pickDictionary): el alemán viaja upstream
# como `de_DE_frami` y el francés como `fr`, y con esos nombres no se encontrarían
# al pedir «de_DE» o «fr_FR».
DICTS=(
  "es_ES=es/es_ES"
  "en_US=en/en_US"
  "de_DE=de/de_DE_frami"
  "fr_FR=fr_FR/dictionaries/fr"
  "it_IT=it_IT/it_IT"
  "pt_BR=pt_BR/pt_BR"
  "pl_PL=pl_PL/pl_PL"
  "nl_NL=nl_NL/nl_NL"
  "ro_RO=ro/ro_RO"
)

mkdir -p "$DEST/licenses"
THIRDPARTY="$DEST/THIRDPARTY-DICTIONARIES.txt"
{
  echo "Diccionarios de corrección ortográfica"
  echo "======================================"
  echo
  echo "Estos diccionarios NO son parte de md-editor: se distribuyen con él por"
  echo "comodidad. Cada uno conserva la licencia de sus autores; los textos que"
  echo "acompañan a cada idioma están en la carpeta licenses/."
  echo
  echo "Origen: https://github.com/LibreOffice/dictionaries"
  echo
} > "$THIRDPARTY"

for entry in "${DICTS[@]}"; do
  name="${entry%%=*}"      # p. ej. de_DE
  path="${entry#*=}"       # p. ej. de/de_DE_frami
  dir="${path%/*}"         # carpeta en el repo de LibreOffice

  echo "· $name  ←  $path"
  # --fail: un 404 (ruta que cambió upstream) tiene que romper el empaquetado,
  # no colarse como un idioma que falta sin avisar.
  curl -sSL --fail --max-time 120 "$BASE/$path.aff" -o "$DEST/$name.aff"
  curl -sSL --fail --max-time 120 "$BASE/$path.dic" -o "$DEST/$name.dic"

  # Licencias y READMEs de esa carpeta (los nombres varían por idioma, así que se
  # listan con la API en vez de adivinarlos). Si la API no responde, se avisa y se
  # sigue: sin diccionario no hay paquete, sin el README sí, pero queda anotado.
  mkdir -p "$DEST/licenses/$name"
  auth=()
  [ -n "${GITHUB_TOKEN:-}" ] && auth=(-H "Authorization: Bearer $GITHUB_TOKEN")
  if listing=$(curl -sSL --fail --max-time 60 "${auth[@]}" "$API/$dir"); then
    # Los nombres se sacan con grep/sed a propósito, sin python ni jq: este script
    # corre también en el runner de Windows (Git Bash), donde `python3` no está en
    # el PATH y `jq` tampoco es seguro.
    echo "$listing" \
      | grep -o '"name": *"[^"]*"' \
      | sed 's/.*: *"\(.*\)"/\1/' \
      | grep -iE '^(README|COPYING|LICEN)' \
      | while read -r lic; do
          curl -sSL --fail --max-time 60 "$BASE/$dir/$lic" -o "$DEST/licenses/$name/$lic" || true
        done
    echo "$name: $BASE/$path.{aff,dic} — licencias en licenses/$name/" >> "$THIRDPARTY"
  else
    echo "  aviso: no se pudo listar $dir (licencias sin traer)" >&2
    echo "$name: $BASE/$path.{aff,dic} — licencia en https://github.com/LibreOffice/dictionaries/tree/master/$dir" >> "$THIRDPARTY"
  fi
done

echo
echo "Diccionarios en $DEST:"
ls -1 "$DEST"/*.dic | sed 's/^/  /'
