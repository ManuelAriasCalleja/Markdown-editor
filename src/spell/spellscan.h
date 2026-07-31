#ifndef SPELLSCAN_H
#define SPELLSCAN_H

/// \file
/// \brief Lógica pura del corrector: tokenización de palabras y selección de diccionario.

#include <QList>
#include <QPair>
#include <QString>
#include <QStringList>

/// Lógica PURA del corrector ortográfico (sin GUI ni motor de diccionario): qué
/// trozos de un texto son «palabras» candidatas a corregir, y qué fichero de
/// diccionario elegir para un idioma. El motor real (Hunspell) y la integración
/// en el resaltado viven aparte; aquí solo hay manipulación de cadenas, para
/// poder probarlo aislado (`tst_spellscan`).
namespace mdspell {

/// Una palabra encontrada en el texto: posición y longitud (en code units de
/// QString), para que el llamador pueda subrayarla o consultarla al diccionario.
struct Word {
    int start;
    int length;
};

/// Extrae las palabras candidatas a corregir de `text`: runs maximales de letras
/// Unicode con apóstrofos internos (`don't`, `l'eau`; valen el apóstrofo recto y el
/// tipográfico). Reglas:
///   - se descartan los tokens que contienen dígitos (`h2o`, `v1`, `utf8`): no
///     son palabras de un idioma natural;
///   - el guion separa (`bien-venido` → `bien`, `venido`), como hacen los
///     correctores al verificar cada parte;
///   - los apóstrofos al principio/fin se recortan (`'tis` → `tis`, `dogs'` →
///     `dogs`).
/// NO sabe de URLs, código ni fórmulas: esas regiones las filtra el llamador
/// (p. ej. el resaltado, que ya distingue fragmentos de math y bloques de
/// código, y `mdurl` para los enlaces).
QList<Word> tokenize(const QString &text);

/// Elige el mejor diccionario disponible para `lang` (código tipo `es`, `es-ES`,
/// `pt_BR`; se aceptan `-`/`_` y cualquier caja). `available` son los basenames
/// presentes (p. ej. `{"es", "es_ES", "en_US"}`, como en /usr/share/hunspell).
/// Prioridad: variante regional exacta › idioma base › primera variante regional
/// del idioma › vacío si no hay ninguno. Devuelve el basename tal cual aparece en
/// `available` (para construir las rutas `.aff`/`.dic`).
/// \param lang código de idioma solicitado (`es`, `es-ES`, `pt_BR`…).
/// \param available basenames de diccionario disponibles.
/// \return el basename elegido tal cual aparece en `available`, o vacío si no hay ninguno.
QString pickDictionary(const QString &lang, const QStringList &available);

/// \brief Nombre del paquete de diccionario de una distribución para `lang`
/// (`es` → `hunspell-es`, `en_US` → `hunspell-en-us`). Es la convención que
/// siguen Debian/Ubuntu, Fedora y Arch.
QString dictionaryPackage(const QString &lang);

/// \brief Orden con la que el usuario instala a mano el diccionario de `lang` en
/// su sistema, según `productType` (`QSysInfo::productType()`: "ubuntu", "debian",
/// "fedora", "arch"…). Fuera de Linux no hay repositorio de diccionarios que
/// valga, así que devuelve vacío y el llamante ofrece la vía manual.
/// \param lang código o basename del diccionario (`es`, `es_ES`…).
/// \param productType identificador del sistema; vacío = familia Debian (lo más común).
QString dictionaryInstallCommand(const QString &lang, const QString &productType);

/// \brief Par de URLs (.aff y .dic) del diccionario de `lang` en el repositorio de
/// diccionarios de LibreOffice, para poder descargarlo desde el programa.
///
/// Solo los nueve idiomas de la interfaz: las rutas de ese repositorio no siguen
/// un patrón (el alemán es `de/de_DE_frami`, el francés `fr_FR/dictionaries/fr`),
/// así que se listan a mano y para el resto se devuelve vacío (el llamante ofrece
/// entonces la instalación manual). Es la MISMA tabla que usa
/// `scripts/fetch-dictionaries.sh` al empaquetar: si cambia una ruta upstream,
/// hay que tocar los dos sitios (el script lo detecta al publicar, con `--fail`).
/// \return {url del .aff, url del .dic}, o dos cadenas vacías si no se conoce.
QPair<QString, QString> dictionaryUrls(const QString &lang);

} // namespace mdspell

#endif // SPELLSCAN_H
