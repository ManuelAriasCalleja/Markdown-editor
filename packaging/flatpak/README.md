# Flatpak / Flathub (Linux)

[Flathub](https://flathub.org) es hoy la principal tienda de aplicaciones de
Linux. Publicar aquí da mucha visibilidad y **no necesita firma de código**: el
paquete va aislado (sandbox) y se compila desde el código fuente.

> ✅ **Validado en local** (25-07-2026): el manifiesto compila con
> `org.flatpak.Builder` sobre `org.kde.Platform//6.11`, exporta el `.desktop`,
> los iconos y el metainfo, y pasa `flatpak-builder-lint` salvo el punto de
> permisos que se discute abajo. Lo que queda es el envío a Flathub.

## App-id

`io.github.manuelariascalleja.Markdown-editor` — el prefijo `io.github.<usuario>`
es el que Flathub acepta para proyectos alojados en GitHub, y la propiedad se
demuestra sola porque el repo está en esa cuenta. El `.desktop`, el icono y el
metainfo deben llamarse igual que el app-id; el manifiesto los renombra en
`post-install` (el `CMakeLists` los instala como «md-editor»).

## Probar en local

`flatpak-builder` ya no hace falta instalarlo del sistema: va como aplicación de
Flathub y trae dentro el **mismo linter que usa el CI de Flathub**, que es el
filtro que de verdad decide si el envío pasa.

```bash
flatpak install flathub org.flatpak.Builder org.kde.Platform//6.11 org.kde.Sdk//6.11

cd packaging/flatpak
flatpak run org.flatpak.Builder --user --install --force-clean build-flatpak \
    io.github.manuelariascalleja.Markdown-editor.yaml
flatpak run io.github.manuelariascalleja.Markdown-editor

# El linter de Flathub, sobre el manifiesto:
flatpak run --command=flatpak-builder-lint org.flatpak.Builder \
    manifest io.github.manuelariascalleja.Markdown-editor.yaml
```

Si el directorio de compilación cae en otro sistema de ficheros que el
`.flatpak-builder` de estado, `flatpak-builder` aborta; en ese caso pásale
`--state-dir` al lado del de salida.

Valida el metainfo por separado:

```bash
appstreamcli validate --pedantic \
    packaging/flatpak/io.github.manuelariascalleja.Markdown-editor.metainfo.xml
```

## Lo que se comprobó en esa validación

Resuelto, no hay que volver sobre ello:

- **Cabeceras privadas de Qt.** `org.kde.Sdk` las trae
  (`/usr/include/QtCore/6.8.3/QtCore/private/qzipwriter_p.h` y compañía), así que
  la exportación ODF compila sin vendorizar nada.
- **Corrector ortográfico.** El runtime trae `libhunspell` **y los diccionarios**,
  enlazados desde `/usr/share/hunspell` a la extensión de idiomas
  (`org.kde.Platform.Locale`). Por eso el manifiesto pasa `-DSPELL_CHECK_STATIC=OFF`:
  el estático por defecto es para el `.zip`/`.dmg`, donde no hay Hunspell en destino.
  Ojo: solo resuelven los diccionarios de los idiomas que el usuario tenga
  instalados (`flatpak config --set extra-languages …` añade más); para los demás,
  el corrector degrada con su aviso, como ya hace en cualquier otra plataforma.
- **`post-install`.** Encuentra el metainfo y renombra `.desktop` e iconos
  correctamente; la exportación los saca ya con el nombre del app-id.
- **App-id.** Era `…MarkdownEditor` y el linter lo rechazaba: deriva de él la URL
  `github.com/manuelariascalleja/markdowneditor`, que da 404. Con
  `…Markdown-editor` la URL resuelve (GitHub no distingue mayúsculas) y pasa.
- **`runtime-version`.** La `6.8` está EOL y el linter la rechaza; el manifiesto
  va ya a `6.11`. Conviene repasarlo en cada envío.
- **Captura de pantalla.** `docs/screenshot.png` existe y la URL de
  `raw.githubusercontent.com` resuelve.

## Lo único que queda abierto: los permisos

`flatpak-builder-lint` marca `finish-args-home-filesystem-access` como error:
Flathub desaconseja `--filesystem=home` y prefiere que se use el portal de
archivos. El problema es que el portal da acceso **al fichero abierto, no a su
carpeta**, y este editor necesita la carpeta:

- resolver las imágenes con ruta relativa de un documento (`![](img/foo.png)`),
- escribir junto al `.md` la imagen que se pega desde el portapapeles,
- vigilar el fichero en disco y recargarlo si cambia fuera del editor.

Con permisos de solo-portal, un documento con imágenes se vería roto. Las
opciones son pedir la **excepción** al enviar (es lo habitual en editores, y el
argumento de las rutas relativas es justo el que Flathub acepta) o acotar a
`--filesystem=xdg-documents`, que no da error pero deja fuera los `.md` que estén
en cualquier otro sitio. Es una decisión de producto, no técnica.

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
