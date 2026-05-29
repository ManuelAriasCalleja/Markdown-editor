#!/usr/bin/env bash

API_URL="https://api.github.com/repos/ManuelAriasCalleja/Markdown-editor/releases"

curl -s "$API_URL" | jq -r '
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
