# Scoop (Windows)

[Scoop](https://scoop.sh) instala apps portables sin pedir permisos de
administrador ni mostrar avisos de SmartScreen molestos: descarga el ZIP, lo
verifica con su **SHA-256** y crea el acceso directo. No requiere firma de código.

## Instalación directa desde este manifiesto

```powershell
scoop install https://raw.githubusercontent.com/ManuelAriasCalleja/Markdown-editor/main/packaging/scoop/md-editor.json
```

## Mantenimiento

- `version`, `url` y `hash` apuntan a la release v1.2.0.
- `checkver: github` y `autoupdate` permiten actualizar el manifiesto solo:
  ```powershell
  # con el repo de scoop (https://github.com/ScoopInstaller/Scoop) clonado:
  .\bin\checkver.ps1 -App md-editor -Dir <ruta>\packaging\scoop -Update
  ```
  Esto recalcula `version` y `hash` a partir de la última release.

## (Opcional) Publicar en un *bucket* propio

Para que la gente instale con `scoop bucket add ... && scoop install md-editor`,
crea un repositorio `scoop-bucket` y coloca ahí `md-editor.json` (Scoop espera los
manifiestos en la raíz o en `bucket/`). Mantenlo al día con `checkver`/`autoupdate`.
