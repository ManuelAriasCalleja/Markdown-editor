#ifndef SPELLCHECKER_H
#define SPELLCHECKER_H

#include <QString>
#include <QStringList>

#include <memory>

// Motor de corrección ortográfica: envoltorio sobre Hunspell. La lógica pura de
// qué es una palabra y qué diccionario elegir vive en `spellscan` (mdspell);
// esta clase solo carga el diccionario y consulta/sugiere.
//
// SIEMPRE se compila: si el soporte de Hunspell no está disponible
// (`HAVE_HUNSPELL` sin definir), se comporta como un *stub* inerte
// (`isAvailable()==false`, `isCorrect()` siempre true, sin sugerencias), para
// que el resto del código lo use sin `#ifdef` repartidos. El detalle de Hunspell
// queda aislado tras un pimpl (el header no incluye nada de Hunspell).
//
// Huella de memoria: carga perezosa de UN solo idioma (el del documento); llamar
// a `setLanguage` con el mismo idioma no recarga. Las listas de palabras
// ignoradas (sesión) y personales (persistibles por el llamador) son pequeñas.
class SpellChecker
{
public:
    SpellChecker();
    ~SpellChecker();
    SpellChecker(const SpellChecker &) = delete;
    SpellChecker &operator=(const SpellChecker &) = delete;

    // Carga (perezosamente) el diccionario para `lang` (código tipo `es`,
    // `es-ES`…). Devuelve true si quedó un diccionario activo. Si no hay
    // diccionario para ese idioma, descarga el actual y devuelve false.
    bool setLanguage(const QString &lang);
    QString language() const;

    // ¿Hay soporte compilado Y un diccionario cargado?
    bool isAvailable() const;

    // true si la palabra es correcta (o si no hay diccionario: no marcamos nada),
    // está en la lista de ignoradas o en la personal.
    bool isCorrect(const QString &word) const;
    // Sugerencias para una palabra mal escrita (vacío si no hay diccionario).
    QStringList suggestions(const QString &word) const;

    // Ignorar la palabra durante esta sesión (no se persiste).
    void ignoreWord(const QString &word);
    // Añadir a la lista personal (se considera correcta a partir de ahora). El
    // llamador persiste la lista vía `personalWords`/`setPersonalWords`.
    void addToPersonal(const QString &word);
    QStringList personalWords() const;
    void setPersonalWords(const QStringList &words);

    // Idiomas (basenames de diccionario) disponibles en las rutas de búsqueda.
    static QStringList availableLanguages();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

#endif // SPELLCHECKER_H
