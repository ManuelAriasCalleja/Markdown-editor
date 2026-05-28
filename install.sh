#!/usr/bin/env bash
#
# install.sh — Compila md-editor a su tamaño mínimo posible (de forma
# incremental) e instala el ejecutable, el lanzador (.desktop) y los iconos.
#
# La build de tamaño mínimo usa los mismos ajustes que './build.sh -m'
# (MinSizeRel + -Os + secciones por función/dato + --gc-sections + strip), en el
# directorio build-min/. CMake/Make son incrementales: si no ha habido cambios
# desde la última compilación de este tipo, no se recompila; y si el binario
# resultante es idéntico al instalado, no se vuelve a copiar.
#
# Además del binario en $PREFIX/bin, instala:
#   • $PREFIX/share/applications/md-editor.desktop  (lanzador + asociación .md)
#   • $PREFIX/share/icons/hicolor/<n>x<n>/apps/md-editor.png  (+ scalable .svg)
#
# Destino configurable con PREFIX (por defecto /usr/local):
#   ./install.sh                 # -> /usr/local (sudo si hace falta)
#   PREFIX="$HOME/.local" ./install.sh   # instalación de usuario, sin sudo
#
set -euo pipefail

# Trabaja siempre desde el directorio del script, venga de donde venga.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="build-min"
TARGET="md-editor"
BIN="$BUILD_DIR/$TARGET"

PREFIX="${PREFIX:-/usr/local}"
DEST="$PREFIX/bin/$TARGET"
APPS_DIR="$PREFIX/share/applications"
ICONS_DIR="$PREFIX/share/icons/hicolor"

# 1) Configura la build de tamaño mínimo. Es idempotente: si la caché ya tiene
#    estos ajustes, CMake no hace nada.
echo "==> Configurando (tamaño mínimo) en $BUILD_DIR/"
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DCMAKE_CXX_FLAGS="-Os -ffunction-sections -fdata-sections" \
    -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -s" >/dev/null

# 2) Compila solo el ejecutable, de forma incremental.
echo "==> Compilando $TARGET..."
cmake --build "$BUILD_DIR" --target "$TARGET" -j "$(nproc)"
echo "==> Binario: $BIN ($(du -h "$BIN" | cut -f1))"

# ¿Hace falta sudo? Sí solo si el ancestro existente más cercano de $PREFIX no
# es escribible (así /usr/local pide sudo, pero un PREFIX en $HOME no).
probe="$PREFIX"
while [ ! -e "$probe" ]; do probe="$(dirname "$probe")"; done
SUDO=""
if [ ! -w "$probe" ]; then
    SUDO="sudo"
    echo "==> Se requiere sudo para escribir en $PREFIX."
fi

# 3) Copia el binario solo si difiere del ya instalado (evita un sudo inútil).
if [ -f "$DEST" ] && cmp -s "$BIN" "$DEST"; then
    echo "==> Binario sin cambios: $DEST ya está al día."
else
    $SUDO install -D -m 0755 "$BIN" "$DEST"
    echo "==> Instalado: $DEST"
fi

# 4) Lanzador .desktop (con Exec apuntando al binario instalado, sin depender
#    del PATH) e iconos en el tema hicolor. Idempotente.
echo "==> Instalando lanzador e iconos en $PREFIX/share..."
desktop_tmp="$(mktemp)"
sed "s|^Exec=.*|Exec=$DEST %f|" md-editor.desktop > "$desktop_tmp"
$SUDO install -D -m 0644 "$desktop_tmp" "$APPS_DIR/md-editor.desktop"
rm -f "$desktop_tmp"

for size in 16 24 32 48 64 128 256; do
    $SUDO install -D -m 0644 "src/icons/md-editor-$size.png" \
        "$ICONS_DIR/${size}x${size}/apps/md-editor.png"
done
$SUDO install -D -m 0644 "src/icons/md-editor.svg" \
    "$ICONS_DIR/scalable/apps/md-editor.svg"

# 5) Refresca las cachés del escritorio si las herramientas están disponibles
#    (mejor esfuerzo: que el lanzador y el icono aparezcan sin reiniciar sesión).
if command -v update-desktop-database >/dev/null 2>&1; then
    $SUDO update-desktop-database "$APPS_DIR" 2>/dev/null || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    $SUDO gtk-update-icon-cache -f -t "$ICONS_DIR" 2>/dev/null || true
fi

echo "==> Listo. md-editor instalado en $PREFIX (binario, lanzador e iconos)."
