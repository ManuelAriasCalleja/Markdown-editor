#ifndef THEMESPEC_H
#define THEMESPEC_H

/// \file
/// \brief Catálogo declarativo de temas: colores de paleta, resaltado y contraste WCAG.

#include <QColor>
#include <QList>
#include <QMetaType>
#include <QString>

/// Catálogo declarativo de temas. Cada ThemeSpec describe, en datos puros (sin
/// GUI), todos los colores de un tema: los roles de QPalette que ThemeController
/// vuelca a la aplicación, el color de las franjas del modo sin distracciones y
/// los colores del resaltado de sintaxis. Añadir un tema = añadir un ThemeSpec a
/// allThemes(); nada más toca colores.
///
/// La legibilidad es el requisito rector: contrastRatio() (WCAG 2.x) permite
/// verificar en pruebas que cada tema mantiene contraste suficiente entre fondo y
/// texto, evitando combinaciones de bajo contraste (grises medios sobre oscuro).
namespace mdtheme {

/// \brief Identificador de cada uno de los temas disponibles.
enum class ThemeId {
    Light,
    Dark,
    GitHubLight,
    GitHubDark,
    Monokai,
    HighContrast,
};

/// Colores del resaltado de código y de las fórmulas matemáticas (los consume
/// CodeBlockHighlighter: el resaltador de bloques de código también pinta las
/// fórmulas porque QTextDocument solo admite un QSyntaxHighlighter por
/// documento).
struct SyntaxColors {
    QColor keyword;
    QColor string;
    QColor comment;
    QColor number;
    QColor math;     // foreground de los fragmentos con IsMathProperty
};

/// \brief Descripción completa de un tema (paleta, resaltado y metadatos).
struct ThemeSpec {
    ThemeId id;
    QString key;          // identificador estable para QSettings ("light", "dark"…)
    QString displayName;  // texto del menú en español (idioma de origen)
    bool nameTranslated;  // true: el nombre pasa por tr(); false: nombre propio

    bool isDark;          // gobierna la luz cálida nocturna y el valor por defecto

    // Roles de paleta.
    QColor window;           // QPalette::Window  (fondo general / cromo)
    QColor windowText;       // QPalette::WindowText (texto principal del cromo)
    QColor base;             // QPalette::Base (página del editor, inputs)
    QColor altBase;          // QPalette::AlternateBase (filas alternas)
    QColor text;             // QPalette::Text (texto principal del documento)
    QColor textSecondary;    // texto atenuado legible (no es gris medio aleatorio)
    QColor button;           // QPalette::Button
    QColor buttonText;       // QPalette::ButtonText (también tiñe los iconos)
    QColor brightText;       // QPalette::BrightText
    QColor link;             // QPalette::Link (y color de los enlaces del doc)
    QColor highlight;        // QPalette::Highlight (selección)
    QColor highlightedText;  // QPalette::HighlightedText
    QColor tooltipBase;      // QPalette::ToolTipBase
    QColor tooltipText;      // QPalette::ToolTipText
    QColor disabledText;     // QPalette::Disabled (Text/ButtonText)
    QColor border;           // roles Mid/Dark/Light (bordes; clave en alto contraste)

    SyntaxColors syntax;
};

/// \brief Todos los temas, en el orden en que aparecen en el menú.
const QList<ThemeSpec> &allThemes();

/// \brief Spec de un id (devuelve Light si no se encuentra, nunca falla).
const ThemeSpec &specFor(ThemeId id);

/// \brief Convierte una clave persistente a su id (devuelve `fallback` si no existe).
ThemeId idFromKey(const QString &key, ThemeId fallback);
/// \brief Clave persistente del id dado.
QString keyForId(ThemeId id);

/// \brief Cociente de contraste WCAG entre dos colores, en [1, 21]. Simétrico.
double contrastRatio(const QColor &a, const QColor &b);

} // namespace mdtheme

/// Permite transportar ThemeId por señales (QSignalSpy, conexiones en cola).
Q_DECLARE_METATYPE(mdtheme::ThemeId)

#endif // THEMESPEC_H
