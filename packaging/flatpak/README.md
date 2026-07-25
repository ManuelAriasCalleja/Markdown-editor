# Flatpak / Flathub (Linux)

[Flathub](https://flathub.org) es hoy la principal tienda de aplicaciones de
Linux. Publicar aquí da mucha visibilidad y **no necesita firma de código**: el
paquete va aislado (sandbox) y se compila desde el código fuente.

> ⚠️ **Esto es un borrador.** El manifiesto
> `io.github.manuelariascalleja.MarkdownEditor.yaml` y el
> `…metainfo.xml` están sin validar en una build real. Pruébalos en local con
> `flatpak-builder` y ajústalos antes de enviarlos a Flathub.

## App-id

`io.github.manuelariascalleja.MarkdownEditor` — el prefijo `io.github.<usuario>`
es el que Flathub acepta para proyectos alojados en GitHub, y la propiedad se
demuestra sola porque el repo está en esa cuenta. El `.desktop`, el icono y el
metainfo deben llamarse igual que el app-id; el manifiesto los renombra en
`post-install` (el `CMakeLists` los instala como «md-editor»).

## Probar en local

```bash
flatpak install flathub org.kde.Platform//6.8 org.kde.Sdk//6.8
flatpak-builder --user --install --force-clean build-flatpak \
    packaging/flatpak/io.github.manuelariascalleja.MarkdownEditor.yaml
flatpak run io.github.manuelariascalleja.MarkdownEditor
```

Valida el metainfo:

```bash
flatpak run org.freedesktop.appstream-glib validate \
    packaging/flatpak/io.github.manuelariascalleja.MarkdownEditor.metainfo.xml
```

## Cosas a verificar / ajustar antes de enviar

1. **Cabeceras privadas de Qt.** La exportación ODF usa `Qt6::GuiPrivate`
   (QZip privado). Confirma que `org.kde.Sdk` trae las cabeceras privadas de Qt;
   si no, habría que vendorizar QZip o desactivar esa ruta en la build de Flatpak.
2. **Directorio de trabajo de `post-install`.** Comprueba que las órdenes
   encuentran el `…metainfo.xml` (es una fuente `type: file`); si no, ajusta la
   ruta.
3. **Captura de pantalla.** El metainfo ya apunta a `docs/screenshot.png` del
   repo (vía `raw.githubusercontent.com`). Flathub valida que la URL resuelva y
   que la imagen tenga una resolución razonable; si cambia de sitio, actualízala.
4. **runtime-version.** Fijada a `6.8`; usa la versión estable de
   `org.kde.Platform` disponible en el momento del envío.
5. **Permisos (`finish-args`).** `--filesystem=home` es amplio; Flathub puede
   pedir algo más acotado (p. ej. portales de archivos). Revísalo.

## Recomendación a futuro

Lo más limpio es **mover el `.metainfo.xml` y un `.desktop` ya nombrados con el
app-id al propio proyecto** e instalarlos desde `CMakeLists.txt`. Así el
manifiesto de Flathub se queda casi vacío (sin renombrados en `post-install`) y
el metainfo viaja con cada release. Hacerlo en una versión futura simplifica el
mantenimiento del paquete.

## Enviar a Flathub

Una vez construya y valide en local, se envía como **Pull Request a
[`flathub/flathub`](https://github.com/flathub/flathub)** siguiendo su guía de
*submission*. Tras la aprobación, cada nueva release se publica actualizando el
`commit`/`tag` del manifiesto en el repositorio que Flathub te crea.

## Mantenimiento

El `tag`/`commit` de este manifiesto y la entrada `<release>` del metainfo los
pone al día solos el job `packaging` de `release.yml` al etiquetar, vía
`scripts/update-packaging.py`; a mano, `python3 scripts/update-packaging.py
<versión>`. La entrada `<release>` que genera el script solo lleva el enlace a
las notas de la release: la descripción, que es lo que muestran las tiendas, se
escribe a mano cuando la versión lo merece.
