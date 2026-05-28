#include "themespec.h"

#include <cmath>

namespace mdtheme {

namespace {

// Construye la tabla de temas una sola vez. Las paletas están elegidas para
// garantizar contraste fondo/texto holgado (verificado en tst_themespec): nada
// de grises medios sobre fondos oscuros para el texto de interfaz.
QList<ThemeSpec> makeThemes()
{
    QList<ThemeSpec> themes;

    // --- Claro (explícito, neutro) ---
    {
        ThemeSpec t;
        t.id = ThemeId::Light;
        t.key = QStringLiteral("light");
        t.displayName = QStringLiteral("Claro");
        t.nameTranslated = true;
        t.isDark = false;
        t.window = QColor(0xf0, 0xf0, 0xf0);
        t.windowText = QColor(0x1a, 0x1a, 0x1a);
        t.base = QColor(0xff, 0xff, 0xff);
        t.altBase = QColor(0xf5, 0xf5, 0xf5);
        t.text = QColor(0x1a, 0x1a, 0x1a);
        t.textSecondary = QColor(0x5a, 0x5a, 0x5a);
        t.button = QColor(0xe8, 0xe8, 0xe8);
        t.buttonText = QColor(0x1a, 0x1a, 0x1a);
        t.brightText = QColor(0xd0, 0x00, 0x00);
        t.link = QColor(0x0a, 0x66, 0xc2);
        t.highlight = QColor(0x0a, 0x66, 0xc2);
        t.highlightedText = QColor(0xff, 0xff, 0xff);
        t.tooltipBase = QColor(0xff, 0xff, 0xff);
        t.tooltipText = QColor(0x1a, 0x1a, 0x1a);
        t.disabledText = QColor(0xa0, 0xa0, 0xa0);
        t.border = QColor(0xc0, 0xc0, 0xc0);
        t.margin = QColor(0xe0, 0xe0, 0xe0);
        t.syntax = {QColor(0x00, 0x00, 0xff), QColor(0xa3, 0x15, 0x15),
                    QColor(0x00, 0x80, 0x00), QColor(0x09, 0x86, 0x58),
                    QColor(0x0b, 0x6e, 0x7a)};
        themes.append(t);
    }

    // --- Oscuro (conserva el tema oscuro histórico) ---
    {
        ThemeSpec t;
        t.id = ThemeId::Dark;
        t.key = QStringLiteral("dark");
        t.displayName = QStringLiteral("Oscuro");
        t.nameTranslated = true;
        t.isDark = true;
        t.window = QColor(0x35, 0x35, 0x35);
        t.windowText = QColor(0xdc, 0xdc, 0xdc);
        t.base = QColor(0x23, 0x23, 0x23);
        t.altBase = QColor(0x35, 0x35, 0x35);
        t.text = QColor(0xdc, 0xdc, 0xdc);
        t.textSecondary = QColor(0xa0, 0xa0, 0xa0);
        t.button = QColor(0x35, 0x35, 0x35);
        t.buttonText = QColor(0xdc, 0xdc, 0xdc);
        t.brightText = QColor(0xff, 0x00, 0x00);
        t.link = QColor(0x5a, 0xa0, 0xff);
        t.highlight = QColor(0x2a, 0x82, 0xda);
        t.highlightedText = QColor(0x00, 0x00, 0x00);
        t.tooltipBase = QColor(0x23, 0x23, 0x23);
        t.tooltipText = QColor(0xdc, 0xdc, 0xdc);
        t.disabledText = QColor(0x7f, 0x7f, 0x7f);
        t.border = QColor(0x55, 0x55, 0x55);
        t.margin = QColor(0x1e, 0x1e, 0x1e);
        t.syntax = {QColor(0x56, 0x9c, 0xd6), QColor(0xce, 0x91, 0x78),
                    QColor(0x6a, 0x99, 0x55), QColor(0xb5, 0xce, 0xa8),
                    QColor(0x7d, 0xd3, 0xfc)};
        themes.append(t);
    }

    // --- GitHub Light ---
    {
        ThemeSpec t;
        t.id = ThemeId::GitHubLight;
        t.key = QStringLiteral("github-light");
        t.displayName = QStringLiteral("GitHub Light");
        t.nameTranslated = false;
        t.isDark = false;
        t.window = QColor(0xf6, 0xf8, 0xfa);
        t.windowText = QColor(0x1f, 0x23, 0x28);
        t.base = QColor(0xff, 0xff, 0xff);
        t.altBase = QColor(0xf6, 0xf8, 0xfa);
        t.text = QColor(0x1f, 0x23, 0x28);
        t.textSecondary = QColor(0x57, 0x60, 0x6a);
        t.button = QColor(0xf6, 0xf8, 0xfa);
        t.buttonText = QColor(0x24, 0x29, 0x2f);
        t.brightText = QColor(0xcf, 0x22, 0x2e);
        t.link = QColor(0x09, 0x69, 0xda);
        t.highlight = QColor(0x09, 0x69, 0xda);
        t.highlightedText = QColor(0xff, 0xff, 0xff);
        t.tooltipBase = QColor(0x24, 0x29, 0x2f);
        t.tooltipText = QColor(0xff, 0xff, 0xff);
        t.disabledText = QColor(0x8c, 0x95, 0x9f);
        t.border = QColor(0xd0, 0xd7, 0xde);
        t.margin = QColor(0xea, 0xee, 0xf2);
        t.syntax = {QColor(0xcf, 0x22, 0x2e), QColor(0x0a, 0x30, 0x69),
                    QColor(0x6e, 0x77, 0x81), QColor(0x05, 0x50, 0xae),
                    QColor(0x82, 0x50, 0xdf)};
        themes.append(t);
    }

    // --- GitHub Dark ---
    {
        ThemeSpec t;
        t.id = ThemeId::GitHubDark;
        t.key = QStringLiteral("github-dark");
        t.displayName = QStringLiteral("GitHub Dark");
        t.nameTranslated = false;
        t.isDark = true;
        t.window = QColor(0x16, 0x1b, 0x22);
        t.windowText = QColor(0xe6, 0xed, 0xf3);
        t.base = QColor(0x0d, 0x11, 0x17);
        t.altBase = QColor(0x16, 0x1b, 0x22);
        t.text = QColor(0xe6, 0xed, 0xf3);
        t.textSecondary = QColor(0x8b, 0x94, 0x9e);
        t.button = QColor(0x21, 0x26, 0x2d);
        t.buttonText = QColor(0xc9, 0xd1, 0xd9);
        t.brightText = QColor(0xff, 0x7b, 0x72);
        t.link = QColor(0x2f, 0x81, 0xf7);
        t.highlight = QColor(0x1f, 0x6f, 0xeb);
        t.highlightedText = QColor(0xff, 0xff, 0xff);
        t.tooltipBase = QColor(0x16, 0x1b, 0x22);
        t.tooltipText = QColor(0xe6, 0xed, 0xf3);
        t.disabledText = QColor(0x6e, 0x76, 0x81);
        t.border = QColor(0x30, 0x36, 0x3d);
        t.margin = QColor(0x01, 0x04, 0x09);
        t.syntax = {QColor(0xff, 0x7b, 0x72), QColor(0xa5, 0xd6, 0xff),
                    QColor(0x8b, 0x94, 0x9e), QColor(0x79, 0xc0, 0xff),
                    QColor(0xd2, 0xa8, 0xff)};
        themes.append(t);
    }

    // --- Monokai ---
    {
        ThemeSpec t;
        t.id = ThemeId::Monokai;
        t.key = QStringLiteral("monokai");
        t.displayName = QStringLiteral("Monokai");
        t.nameTranslated = false;
        t.isDark = true;
        t.window = QColor(0x3e, 0x3d, 0x32);
        t.windowText = QColor(0xf8, 0xf8, 0xf2);
        t.base = QColor(0x27, 0x28, 0x22);
        t.altBase = QColor(0x2d, 0x2e, 0x28);
        t.text = QColor(0xf8, 0xf8, 0xf2);
        t.textSecondary = QColor(0xc7, 0xc7, 0xbf);  // legible (no el #75715e clásico)
        t.button = QColor(0x3e, 0x3d, 0x32);
        t.buttonText = QColor(0xf8, 0xf8, 0xf2);
        t.brightText = QColor(0xf9, 0x26, 0x72);
        t.link = QColor(0x66, 0xd9, 0xef);
        t.highlight = QColor(0x49, 0x48, 0x3e);
        t.highlightedText = QColor(0xf8, 0xf8, 0xf2);
        t.tooltipBase = QColor(0x3e, 0x3d, 0x32);
        t.tooltipText = QColor(0xf8, 0xf8, 0xf2);
        t.disabledText = QColor(0x90, 0x8f, 0x82);
        t.border = QColor(0x49, 0x48, 0x3e);
        t.margin = QColor(0x1d, 0x1e, 0x19);
        t.syntax = {QColor(0xf9, 0x26, 0x72), QColor(0xe6, 0xdb, 0x74),
                    QColor(0x9d, 0x9b, 0x86), QColor(0xae, 0x81, 0xff),
                    QColor(0xfd, 0x97, 0x1f)};
        themes.append(t);
    }

    // --- Alto contraste (oscuro, máximo contraste) ---
    {
        ThemeSpec t;
        t.id = ThemeId::HighContrast;
        t.key = QStringLiteral("high-contrast");
        t.displayName = QStringLiteral("Alto contraste");
        t.nameTranslated = true;
        t.isDark = true;
        t.window = QColor(0x00, 0x00, 0x00);
        t.windowText = QColor(0xff, 0xff, 0xff);
        t.base = QColor(0x00, 0x00, 0x00);
        t.altBase = QColor(0x1a, 0x1a, 0x1a);
        t.text = QColor(0xff, 0xff, 0xff);
        t.textSecondary = QColor(0xff, 0xff, 0xff);  // sin atenuar en alto contraste
        t.button = QColor(0x00, 0x00, 0x00);
        t.buttonText = QColor(0xff, 0xff, 0xff);
        t.brightText = QColor(0xff, 0x00, 0x00);
        t.link = QColor(0xff, 0xff, 0x00);
        t.highlight = QColor(0xff, 0xff, 0x00);
        t.highlightedText = QColor(0x00, 0x00, 0x00);
        t.tooltipBase = QColor(0x00, 0x00, 0x00);
        t.tooltipText = QColor(0xff, 0xff, 0xff);
        t.disabledText = QColor(0xa0, 0xa0, 0xa0);
        t.border = QColor(0xff, 0xff, 0xff);  // bordes blancos explícitos
        t.margin = QColor(0x00, 0x00, 0x00);
        t.syntax = {QColor(0x00, 0xff, 0xff), QColor(0xff, 0xaa, 0x00),
                    QColor(0x00, 0xff, 0x00), QColor(0xff, 0x66, 0xff),
                    QColor(0xff, 0xff, 0x66)};
        themes.append(t);
    }

    return themes;
}

// Canal lineal según WCAG 2.x (sRGB -> luminancia lineal).
double linearize(int component)
{
    const double c = component / 255.0;
    return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

double relativeLuminance(const QColor &c)
{
    return 0.2126 * linearize(c.red())
         + 0.7152 * linearize(c.green())
         + 0.0722 * linearize(c.blue());
}

} // namespace

const QList<ThemeSpec> &allThemes()
{
    static const QList<ThemeSpec> themes = makeThemes();
    return themes;
}

const ThemeSpec &specFor(ThemeId id)
{
    const QList<ThemeSpec> &themes = allThemes();
    for (const ThemeSpec &t : themes) {
        if (t.id == id)
            return t;
    }
    return themes.first();  // Light: la tabla nunca está vacía
}

ThemeId idFromKey(const QString &key, ThemeId fallback)
{
    for (const ThemeSpec &t : allThemes()) {
        if (t.key == key)
            return t.id;
    }
    return fallback;
}

QString keyForId(ThemeId id)
{
    return specFor(id).key;
}

double contrastRatio(const QColor &a, const QColor &b)
{
    const double la = relativeLuminance(a);
    const double lb = relativeLuminance(b);
    const double lighter = std::max(la, lb);
    const double darker = std::min(la, lb);
    return (lighter + 0.05) / (darker + 0.05);
}

} // namespace mdtheme
