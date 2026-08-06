# Diccionarios del corrector ortográfico

Esta carpeta es el punto de **empaquetado** de diccionarios Hunspell para las
compilaciones de **Windows y macOS**, que no traen diccionarios del sistema. En
**Linux** no hace falta: el corrector usa los de `/usr/share/hunspell` (paquetes
`hunspell-es`, `hunspell-en-us`, etc.).

## Cómo se usa

Deja aquí pares `.aff` + `.dic` con el mismo nombre base (uno por idioma):

```
dictionaries/
  es_ES.aff   es_ES.dic
  en_US.aff   en_US.dic
  …
```

Al **instalar/empaquetar** (`cmake --install` / despliegue), CMake los copia
adonde el corrector los busca (ver `searchPaths` en `src/spellchecker.cpp`):

- **Windows**: junto al `.exe` (`bin/dictionaries`).
- **macOS**: dentro del `.app` (`Contents/Resources/dictionaries`).
- **Linux**: `<prefix>/share/hunspell` (por si se quiere una instalación cerrada).

El nombre base es el código de idioma que el corrector empareja con el idioma del
documento (`es_ES`, `en_US`, `de_DE`…); ver `mdspell::pickDictionary`.

## Por qué no vienen incluidos en el repositorio

- **Licencias**: cada diccionario tiene la suya (GPL/LGPL/MPL/BSD según idioma e
  origen). Hay que respetarlas al redistribuir. El proyecto es GPL-3.0, compatible
  con las habituales, pero la elección de qué incluir es del empaquetador.
- **Tamaño**: los 9 idiomas con diccionario suman decenas de MB; no tiene sentido cargar el repo
  con ellos.

Por eso los `.aff/.dic` están en `.gitignore`: cópialos tú al compilar para
Windows/macOS. Fuentes habituales: los diccionarios de **LibreOffice**
(<https://github.com/LibreOffice/dictionaries>) o los paquetes `hunspell-*` de tu
distribución.

## Atajo en Linux

Para copiar aquí los diccionarios del sistema de los 9 idiomas de la interfaz que
tienen diccionario Hunspell (todos menos el chino):

```sh
./scripts/bundle-dictionaries.sh
```
