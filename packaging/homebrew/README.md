# Homebrew cask (macOS)

[Homebrew](https://brew.sh) es la forma habitual de instalar apps en macOS entre
usuarios técnicos. El cask `md-editor.rb` descarga el DMG de la release, lo
verifica con su **SHA-256** e instala `md-editor.app` en `/Applications`.

## La vía realista: un *tap* propio

La app **no está firmada ni notarizada** (Apple cobra 99 $/año por ello). El
repositorio oficial **homebrew/cask** suele **rechazar apps sin firmar**, así que
lo práctico es publicar el cask en un **tap propio**:

1. Crea un repo público llamado **`homebrew-tap`** (el prefijo `homebrew-` es
   obligatorio) en tu cuenta de GitHub.
2. Copia este archivo a `Casks/md-editor.rb` en ese repo.
3. Los usuarios instalan con:
   ```bash
   brew install --cask ManuelAriasCalleja/tap/md-editor
   ```
   (equivale a `brew tap ManuelAriasCalleja/tap` + `brew install --cask md-editor`).

Mientras tanto, también se puede instalar **directamente** desde este archivo:

```bash
brew install --cask https://raw.githubusercontent.com/ManuelAriasCalleja/Markdown-editor/main/packaging/homebrew/md-editor.rb
```

## Gatekeeper (primer arranque)

Al no estar notarizada, macOS bloquea la app la primera vez. El cask muestra un
`caveats` con los pasos (botón derecho → Abrir, o Ajustes → Privacidad y
seguridad → "Abrir de todas formas"). Si algún día la notarizas, se puede quitar
ese aviso.

## Mantenimiento

- `version` + `sha256` apuntan a la release v1.2.0.
- `livecheck` (`:github_latest`) permite que `brew livecheck md-editor` detecte
  nuevas versiones; para actualizar el cask:
  ```bash
  brew bump-cask-pr --version <nueva> ManuelAriasCalleja/tap/md-editor
  # o, a mano: actualizar version y recalcular sha256:
  shasum -a 256 md-editor-<nueva>-macos-universal.dmg
  ```

## Comprobar el cask

```bash
brew audit --cask --new md-editor.rb
brew style md-editor.rb
```

> Nota: `app "md-editor.app"` asume que ese es el nombre del bundle dentro del
> DMG (lo es: el target CMake se llama `md-editor`). El `zap` borra preferencias
> (`md-editor*.plist`) y el estado de ventana (`org.mdeditor.app.savedState`,
> el bundle id del `CMakeLists`). Conviene **verificar la ruta real del `.plist`
> de QSettings en un Mac** y ajustar el patrón si hiciera falta.
