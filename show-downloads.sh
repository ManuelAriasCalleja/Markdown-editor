#!/usr/bin/env bash

# Histórico de releases BORRADAS (v1.0.0–v2.3.0), snapshot del 2026-07-01 tomado
# justo antes de borrarlas. GitHub no conserva las descargas de releases eliminadas
# (el download_count se va con la release y no hay API para recuperarlo), así que se
# hornea aquí como TOTAL. No hay desglose por SO de esas releases: la captura era por
# release. Detalle y tabla por versión en docs/DESCARGAS-HISTORICO.md.
HISTORICAL_DELETED=38

API_URL="https://api.github.com/repos/ManuelAriasCalleja/Markdown-editor/releases"

json=$(curl -s "$API_URL")

# Descargas por sistema operativo de las releases VIVAS (las que quedan en GitHub).
echo "$json" | jq -r '
def detect_os:
  if .name | test("windows|win|\\.exe$|\\.msi$"; "i") then "Windows"
  elif .name | test("linux|\\.AppImage$|\\.deb$|\\.rpm$|\\.tar\\.gz$|\\.tar\\.xz$"; "i") then "Linux"
  elif .name | test("macos|mac|darwin|\\.dmg$|\\.pkg$"; "i") then "macOS"
  else "Otros"
  end;

[
  .[].assets[]
  | {
      os: detect_os,
      downloads: .download_count
    }
]
| group_by(.os)
| map({
    os: .[0].os,
    total: (map(.downloads) | add)
  })
| sort_by(.os)
| .[]
| "\(.os): \(.total)"
'

# Total de las releases vivas + histórico de las borradas = acumulado real.
live_total=$(echo "$json" | jq '[.[].assets[].download_count] | add // 0')
echo "---"
echo "Vivas (total): $live_total"
echo "Histórico borrado (sin desglose por SO): $HISTORICAL_DELETED"
echo "Total acumulado: $((live_total + HISTORICAL_DELETED))"
