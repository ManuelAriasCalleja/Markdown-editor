#!/usr/bin/env bash
#
# build.sh — Compila md-editor (Qt6 + CMake).
#
set -euo pipefail

# Trabaja siempre desde el directorio del script, sin importar desde dónde
# se invoque.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

usage() {
    cat <<'EOF'
Uso: ./build.sh [opciones] [-- args-del-programa]

Compila md-editor (Qt6 + CMake).

Opciones:
  -m, --minimize   Minimiza el tamaño del binario tanto como sea posible:
                   build type MinSizeRel (-Os), -ffunction-sections /
                   -fdata-sections, enlazado con --gc-sections y strip (-s).
  -x, --run        Ejecuta el binario tras una compilación correcta.
                   Los argumentos que sigan se pasan al programa.
  -h, -?, --help   Muestra esta ayuda y termina.

Ejemplos:
  ./build.sh                 # compilación normal           -> build/
  ./build.sh -m              # compilación de tamaño mínimo -> build-min/
  ./build.sh -x ejemplo.md   # compila y ejecuta abriendo ejemplo.md
  ./build.sh -mx             # tamaño mínimo y, si va bien, ejecutar
EOF
}

# --- Traduce opciones largas a cortas para que getopts las entienda ---
args=()
for a in "$@"; do
    case "$a" in
        --help)                args+=("-h") ;;
        --minimize|--min-size) args+=("-m") ;;
        --run)                 args+=("-x") ;;
        *)                     args+=("$a") ;;
    esac
done
set -- "${args[@]}"

MINIMIZE=0
RUN=0

# getopts en modo silencioso (':' inicial). bash trata '?' siempre como el
# indicador de error, así que '-?' llega a la rama \? con OPTARG="?": lo
# interpretamos como petición de ayuda. Cualquier otra letra es desconocida.
while getopts ":mxh" opt; do
    case "$opt" in
        m) MINIMIZE=1 ;;
        x) RUN=1 ;;
        h) usage; exit 0 ;;
        \?)
            if [ "${OPTARG:-}" = "?" ] || [ -z "${OPTARG:-}" ]; then
                usage; exit 0          # se invocó -?  -> ayuda
            fi
            echo "build.sh: opción desconocida: -$OPTARG" >&2
            echo "Prueba './build.sh -h' para ver la ayuda." >&2
            exit 1
            ;;
    esac
done
shift $((OPTIND - 1))   # lo que quede son argumentos para el programa

# --- Configuración de CMake según el modo ---
if [ "$MINIMIZE" -eq 1 ]; then
    BUILD_DIR="build-min"
    echo "==> Configurando (tamaño mínimo) en $BUILD_DIR/"
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=MinSizeRel \
        -DCMAKE_CXX_FLAGS="-Os -ffunction-sections -fdata-sections" \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--gc-sections -s"
else
    BUILD_DIR="build"
    echo "==> Configurando en $BUILD_DIR/"
    cmake -S . -B "$BUILD_DIR"
fi

echo "==> Compilando..."
cmake --build "$BUILD_DIR" -j "$(nproc)"

BIN="$BUILD_DIR/md-editor"
echo "==> Compilación correcta: $BIN ($(du -h "$BIN" | cut -f1))"

# --- Ejecutar si se pidió y la compilación fue bien (llegamos hasta aquí) ---
if [ "$RUN" -eq 1 ]; then
    echo "==> Ejecutando: $BIN $*"
    exec "$BIN" "$@"
fi
