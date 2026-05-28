#!/usr/bin/env bash
# Cuenta las líneas de código de este proyecto, excluyendo líneas en blanco y
# comentarios (// ... y /* ... */). Informa de .cpp y .h por separado para la
# aplicación (src/) y las pruebas (tests/), con subtotales por sección y un
# total general al final.
#
# Uso: ./count-loc.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

# Elimina los comentarios C/C++ y las líneas en blanco de la entrada estándar e
# imprime el número de líneas restantes. Maneja comentarios de línea // y de
# bloque /* ... */ (incluidos los multilínea). Las cadenas se tratan de forma
# ingenua: los marcadores de comentario dentro de literales de cadena son raros
# en este código y, como mucho, desviarían la cuenta en una o dos líneas.
count_loc() {
    awk '
        BEGIN { in_block = 0 }
        {
            line = $0
            out = ""
            i = 1
            n = length(line)
            while (i <= n) {
                if (in_block) {
                    p = index(substr(line, i), "*/")
                    if (p == 0) { i = n + 1 }
                    else { i = i + p + 1; in_block = 0 }
                } else {
                    c1 = substr(line, i, 1)
                    c2 = substr(line, i, 2)
                    if (c2 == "//") { i = n + 1 }
                    else if (c2 == "/*") { in_block = 1; i = i + 2 }
                    else { out = out c1; i = i + 1 }
                }
            }
            sub(/[[:space:]]+$/, "", out)
            sub(/^[[:space:]]+/, "", out)
            if (length(out) > 0) print out
        }
    ' | wc -l
}

count_section() {
    local label="$1"; shift
    local dir="$1"; shift
    local cpp_total=0
    local h_total=0
    if [ -d "${dir}" ]; then
        cpp_total=$(find "${dir}" -type f -name '*.cpp' \
                        -exec cat {} + 2>/dev/null | count_loc)
        h_total=$(find "${dir}" -type f -name '*.h' \
                      -exec cat {} + 2>/dev/null | count_loc)
    fi
    local sum=$((cpp_total + h_total))
    printf '%-12s %10s %10s %10s\n' "${label}" "${cpp_total}" "${h_total}" "${sum}"
    # Devuelve los totales para que el llamador acumule el total general.
    echo "${cpp_total} ${h_total} ${sum}" >&3
}

printf '%-12s %10s %10s %10s\n' "" ".cpp" ".h" "subtotal"
printf '%-12s %10s %10s %10s\n' "------------" "----------" "----------" "----------"

# Usa el descriptor de fichero 3 para que la tabla bonita vaya a stdout mientras
# el acumulador captura solo la cola numérica.
exec 3>/tmp/.count-loc-$$
count_section "src/"   "${REPO_ROOT}/src"
count_section "tests/" "${REPO_ROOT}/tests"
exec 3>&-

src_cpp=0; src_h=0; src_sum=0
tst_cpp=0; tst_h=0; tst_sum=0
{ read -r src_cpp src_h src_sum; read -r tst_cpp tst_h tst_sum; } \
    < /tmp/.count-loc-$$
rm -f /tmp/.count-loc-$$

total_cpp=$((src_cpp + tst_cpp))
total_h=$((src_h + tst_h))
total=$((total_cpp + total_h))

printf '%-12s %10s %10s %10s\n' "------------" "----------" "----------" "----------"
printf '%-12s %10s %10s %10s\n' "TOTAL"        "${total_cpp}" "${total_h}" "${total}"
