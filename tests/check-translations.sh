#!/usr/bin/env bash
#
# Guard de traducciones (lo ejecuta CTest como prueba 'tst_translations').
# Comprueba dos cosas:
#   1) Completitud: ningún .ts tiene cadenas sin traducir (type="unfinished").
#   2) Sincronía con el código: lupdate en seco (sobre copias temporales) no
#      detecta cadenas nuevas ni obsoletas para los .ts completos.
#
# md-editor_es.ts es parcial a propósito (solo los plurales, ya que el español
# es el idioma de origen), así que se excluye de la comprobación de sincronía.
#
# Uso: check-translations.sh <ruta-lupdate> <dir-fuentes> <dir-traducciones>
set -eu

LUPDATE="$1"   # lupdate de Qt6 (la versión del PATH suele ser un envoltorio Qt5)
SRC="$2"       # directorio con el código fuente a escanear
TSDIR="$3"     # directorio con los .ts

fail=0

# 1) Completitud: nada sin traducir.
for f in "$TSDIR"/*.ts; do
    if grep -q 'type="unfinished"' "$f"; then
        echo "FALLO: $(basename "$f") tiene cadenas sin traducir (type=\"unfinished\")."
        fail=1
    fi
done

# 2) Sincronía: lupdate no debería añadir ni quitar nada en los .ts completos.
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
for f in "$TSDIR"/*.ts; do
    base="$(basename "$f")"
    [ "$base" = "md-editor_es.ts" ] && continue   # parcial por diseño
    cp "$f" "$tmp/$base"
    out="$("$LUPDATE" -no-obsolete -locations none "$SRC" -ts "$tmp/$base" 2>&1 || true)"
    if echo "$out" | grep -qE '\([1-9][0-9]* new'; then
        echo "FALLO: a $base le faltan cadenas del código." \
             "Ejecuta 'cmake --build build --target update_translations' y traduce las nuevas."
        fail=1
    fi
    if echo "$out" | grep -qE 'Removed [1-9]'; then
        echo "FALLO: $base tiene cadenas obsoletas. Ejecuta 'update_translations'."
        fail=1
    fi
done

if [ "$fail" -eq 0 ]; then
    echo "Traducciones OK: completas y sincronizadas con el código."
fi
exit "$fail"
