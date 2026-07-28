#!/usr/bin/env python3
"""Pone los manifiestos de `packaging/` al día con una versión ya publicada.

Los manifiestos fijan versión y hash de los artefactos de la Release, así que
quedarse atrás no rompe nada visible: simplemente se sigue instalando una
versión vieja. Eso ya pasó (estuvieron seis versiones clavados en la 1.2.0),
de ahí que esto sea un script y lo llame el propio `release.yml` al etiquetar.

    python3 scripts/update-packaging.py 2.8.0
    python3 scripts/update-packaging.py 2.8.0 --assets-dir /ruta/con/los/binarios

Sin `--assets-dir` descarga los artefactos de la Release con `gh` (en CI hace
falta GH_TOKEN). Cada sustitución lleva su número de apariciones esperado: si
un manifiesto cambia de forma y deja de encajar, aborta en vez de dejarlo a
medias sin avisar.
"""
from __future__ import annotations

import argparse
import hashlib
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

REPO = "ManuelAriasCalleja/Markdown-editor"
ROOT = pathlib.Path(__file__).resolve().parent.parent
PKG = ROOT / "packaging"
METAINFO = PKG / "flatpak" / "io.github.manuelariascalleja.Markdown-editor.metainfo.xml"
FLATPAK = PKG / "flatpak" / "io.github.manuelariascalleja.Markdown-editor.yaml"
SCOOP = PKG / "scoop" / "md-editor.json"
HOMEBREW = PKG / "homebrew" / "md-editor.rb"
# El README no es un manifiesto, pero es la primera página que ve cualquiera y
# tiene la versión escrita en dos sitios (la insignia y los nombres de fichero de
# la tabla de descargas). Se quedaba atrás exactamente igual que los manifiestos
# —y más a la vista—, así que va en el mismo automatismo.
README = ROOT / "README.md"


def run(*cmd: str) -> str:
    try:
        return subprocess.run(cmd, cwd=ROOT, check=True, capture_output=True,
                              text=True).stdout.strip()
    except subprocess.CalledProcessError as exc:
        sys.exit(f"falló «{' '.join(cmd)}»:\n{exc.stderr.strip()}")


def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def edit(path: pathlib.Path, rules: list[tuple[str, str, int]]) -> bool:
    """Aplica (patrón, reemplazo, n_esperado) sobre `path`. Devuelve si cambió."""
    text = original = path.read_text(encoding="utf-8")
    for pattern, repl, expected in rules:
        text, n = re.subn(pattern, repl, text)
        if n != expected:
            sys.exit(f"{path.name}: el patrón {pattern!r} casó {n} veces, "
                     f"se esperaban {expected}. Manifiesto sin tocar.")
    if text != original:
        path.write_text(text, encoding="utf-8")
    return text != original


def assets(version: str, tag: str, given: pathlib.Path | None) -> dict[str, str]:
    """SHA-256 del ZIP de Windows y del DMG de macOS."""
    names = {"zip": f"md-editor-{version}-windows-x64.zip",
             "dmg": f"md-editor-{version}-macos-universal.dmg"}
    if given:
        found = {k: given / n for k, n in names.items()}
    else:
        tmp = pathlib.Path(tempfile.mkdtemp(prefix="md-editor-assets-"))
        run("gh", "release", "download", tag, "-R", REPO, "-D", str(tmp),
            "-p", "*windows-x64.zip", "-p", "*macos-universal.dmg")
        found = {k: tmp / n for k, n in names.items()}
    for kind, path in found.items():
        if not path.is_file():
            sys.exit(f"falta el artefacto {kind}: {path}")
    digests = {k: sha256(p) for k, p in found.items()}
    if not given:
        shutil.rmtree(found["zip"].parent, ignore_errors=True)
    return digests


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("version", help="versión publicada, sin la «v» (p. ej. 2.8.0)")
    ap.add_argument("--assets-dir", type=pathlib.Path, default=None,
                    help="carpeta con los binarios ya descargados (evita usar gh)")
    args = ap.parse_args()

    version = args.version.lstrip("v")
    if not re.fullmatch(r"\d+\.\d+\.\d+", version):
        sys.exit(f"versión con formato inesperado: {version!r}")
    tag = f"v{version}"

    commit = run("git", "rev-parse", f"{tag}^{{commit}}")
    date = run("git", "log", "-1", "--format=%cs", tag)
    digests = assets(version, tag, args.assets_dir)

    changed = []

    # Scoop: versión, URL y hash del ZIP.
    if edit(SCOOP, [
        (r'"version": "[^"]+"', f'"version": "{version}"', 1),
        (r'releases/download/v[\d.]+/md-editor-[\d.]+-windows-x64\.zip"',
         f'releases/download/v{version}/md-editor-{version}-windows-x64.zip"', 1),
        (r'"hash": "[0-9a-f]{64}"', f'"hash": "{digests["zip"]}"', 1),
    ]):
        changed.append(SCOOP)

    # Homebrew: versión y sha256 del DMG (la URL se compone con #{version}).
    if edit(HOMEBREW, [
        (r'version "[^"]+"', f'version "{version}"', 1),
        (r'sha256 "[0-9a-f]{64}"', f'sha256 "{digests["dmg"]}"', 1),
    ]):
        changed.append(HOMEBREW)

    # README: la insignia de versión y los tres nombres de fichero de la tabla de
    # descargas. Los enlaces de esa tabla apuntan a `releases/latest`, así que no
    # se rompen al quedarse atrás; lo que envejece es el texto, que anuncia una
    # versión que ya no es la última. Un nombre por sistema, de ahí el 1 en cada
    # regla: si algún día se añade otro artefacto, esto aborta y hay que venir.
    if edit(README, [
        (r'badge/version-[\d.]+-blue', f'badge/version-{version}-blue', 1),
        (r'md-editor-[\d.]+-x86_64\.AppImage', f'md-editor-{version}-x86_64.AppImage', 1),
        (r'md-editor-[\d.]+-windows-x64\.zip', f'md-editor-{version}-windows-x64.zip', 1),
        (r'md-editor-[\d.]+-macos-universal\.dmg',
         f'md-editor-{version}-macos-universal.dmg', 1),
    ]):
        changed.append(README)

    # Flatpak: compila desde el tag, así que van tag y commit fijado.
    if edit(FLATPAK, [
        (r'tag: v[\d.]+', f'tag: {tag}', 1),
        (r'commit: [0-9a-f]{40}', f'commit: {commit}', 1),
    ]):
        changed.append(FLATPAK)

    # AppStream: una entrada <release> por versión (es lo que muestran las
    # tiendas). La descripción se escribe a mano cuando merece la pena; aquí
    # solo se garantiza que la versión figure, con enlace a sus notas.
    meta = METAINFO.read_text(encoding="utf-8")
    if f'<release version="{version}"' in meta:
        print(f"metainfo: la {version} ya figura, se deja como está")
    else:
        entry = (f'    <release version="{version}" date="{date}">\n'
                 f'      <url type="details">https://github.com/{REPO}/releases/tag/{tag}</url>\n'
                 f'    </release>\n')
        if edit(METAINFO, [(r'  <releases>\n', f'  <releases>\n{entry}', 1)]):
            changed.append(METAINFO)

    if not changed:
        print(f"los manifiestos ya estaban en la {version}")
        return 0
    print(f"actualizados a la {version} (commit {commit[:9]}, {date}):")
    for path in changed:
        print(f"  {path.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
