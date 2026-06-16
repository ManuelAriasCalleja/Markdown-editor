#include "themecontroller.h"

#include "appsettings.h"
#include "codehighlighter.h"
#include "themespec.h"

#include <QApplication>
#include <QColor>
#include <QList>
#include <QPair>
#include <QStyleHints>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QTime>
#include <QTimer>

#include <cmath>

ThemeController::ThemeController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                                 QObject *parent)
    : QObject(parent),
      m_editor(editor),
      m_highlighter(highlighter),
      m_warmLight(AppSettings::warmLight()),
      m_followSystem(AppSettings::followSystemTheme())
{
    qRegisterMetaType<mdtheme::ThemeId>();

    // Refresca el tinte cálido al pasar el tiempo. Un minuto es de sobra fino:
    // las rampas amanecer/anochecer duran una hora, así que el cambio es
    // imperceptible entre ticks y refreshWarmth() evita repintar si no varía.
    m_warmTimer = new QTimer(this);
    m_warmTimer->setInterval(60 * 1000);
    connect(m_warmTimer, &QTimer::timeout, this, &ThemeController::refreshWarmth);
    m_warmTimer->start();

    // Si se sigue el SO, cambiar su esquema claro/oscuro reaplica el tema solo.
    // (La señal trae el esquema nuevo; si el seguimiento está apagado, se ignora.)
    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this,
            [this](Qt::ColorScheme scheme) {
                if (m_followSystem)
                    applyTheme(themeForScheme(scheme));
            });
}

mdtheme::ThemeId ThemeController::currentTheme() const
{
    return m_current;
}

bool ThemeController::isDark() const
{
    return mdtheme::specFor(m_current).isDark;
}

bool ThemeController::isWarmLight() const
{
    return m_warmLight;
}

bool ThemeController::followsSystem() const
{
    return m_followSystem;
}

mdtheme::ThemeId ThemeController::themeForScheme(Qt::ColorScheme scheme)
{
    // El esquema desconocido (algunos entornos Linux) se trata como claro.
    return scheme == Qt::ColorScheme::Dark ? mdtheme::ThemeId::Dark
                                           : mdtheme::ThemeId::Light;
}

mdtheme::ThemeId ThemeController::systemTheme() const
{
    return themeForScheme(qApp->styleHints()->colorScheme());
}

QColor ThemeController::linkColor() const
{
    return mdtheme::specFor(m_current).link;
}

QPalette ThemeController::buildPalette(mdtheme::ThemeId id) const
{
    const mdtheme::ThemeSpec &t = mdtheme::specFor(id);

    QPalette p;
    p.setColor(QPalette::Window, t.window);
    p.setColor(QPalette::WindowText, t.windowText);
    p.setColor(QPalette::Base, t.base);
    p.setColor(QPalette::AlternateBase, t.altBase);
    p.setColor(QPalette::Text, t.text);
    p.setColor(QPalette::PlaceholderText, t.textSecondary);
    p.setColor(QPalette::Button, t.button);
    p.setColor(QPalette::ButtonText, t.buttonText);
    p.setColor(QPalette::BrightText, t.brightText);
    p.setColor(QPalette::Link, t.link);
    p.setColor(QPalette::Highlight, t.highlight);
    p.setColor(QPalette::HighlightedText, t.highlightedText);
    p.setColor(QPalette::ToolTipBase, t.tooltipBase);
    p.setColor(QPalette::ToolTipText, t.tooltipText);

    // Bordes y biseles: que sean visibles (clave en alto contraste).
    p.setColor(QPalette::Mid, t.border);
    p.setColor(QPalette::Dark, t.border);

    // Atenuado de elementos deshabilitados.
    p.setColor(QPalette::Disabled, QPalette::Text, t.disabledText);
    p.setColor(QPalette::Disabled, QPalette::ButtonText, t.disabledText);
    p.setColor(QPalette::Disabled, QPalette::WindowText, t.disabledText);
    return p;
}

double ThemeController::warmthForTime(const QTime &t)
{
    const double h = t.hour() + t.minute() / 60.0;

    constexpr double morningStart = 6.0;   // empieza a enfriarse (1 -> 0)
    constexpr double morningEnd   = 7.0;   // ya neutro
    constexpr double eveningStart = 19.0;  // empieza a calentarse (0 -> 1)
    constexpr double eveningEnd   = 23.0;  // máximo

    if (h >= morningEnd && h <= eveningStart)
        return 0.0;                                  // día: neutro
    if (h >= eveningEnd || h < morningStart)
        return 1.0;                                  // noche cerrada: máximo
    if (h > eveningStart)
        return (h - eveningStart) / (eveningEnd - eveningStart);  // rampa tarde
    return 1.0 - (h - morningStart) / (morningEnd - morningStart); // rampa mañana
}

double ThemeController::currentWarmth() const
{
    return m_warmLight ? warmthForTime(QTime::currentTime()) : 0.0;
}

void ThemeController::applyWarmth(QPalette &palette, double w) const
{
    if (w <= 0.0)
        return;

    // Filtro cálido multiplicativo sobre el fondo del editor: recorta el azul
    // (hasta ~16%) y un poco el verde (~5%), dejando el rojo intacto. El efecto
    // lleva un blanco puro a un crema suave y un gris oscuro a un gris cálido.
    auto warm = [w](QColor c) {
        c.setBlue(std::lround(c.blue() * (1.0 - 0.16 * w)));
        c.setGreen(std::lround(c.green() * (1.0 - 0.05 * w)));
        return c;
    };
    palette.setColor(QPalette::Base, warm(palette.color(QPalette::Base)));
    palette.setColor(QPalette::AlternateBase,
                     warm(palette.color(QPalette::AlternateBase)));
}

void ThemeController::applyPaletteWithWarmth(mdtheme::ThemeId id)
{
    const double w = currentWarmth();
    m_lastWarmth = w;
    QPalette p = buildPalette(id);
    applyWarmth(p, w);
    qApp->setPalette(p);
}

void ThemeController::applyTheme(mdtheme::ThemeId id)
{
    m_current = id;

    applyPaletteWithWarmth(id);

    AppSettings::setThemeKey(mdtheme::keyForId(id));

    recolorLinks();

    // Reajusta los colores del resaltado de sintaxis al tema.
    if (m_highlighter)
        m_highlighter->setSyntaxColors(mdtheme::specFor(id).syntax);

    emit themeChanged(id);
}

void ThemeController::setWarmLight(bool on)
{
    if (m_warmLight == on)
        return;
    m_warmLight = on;
    AppSettings::setWarmLight(on);
    applyTheme(m_current);  // reaplica el tema con o sin tinte
}

void ThemeController::setFollowSystem(bool on)
{
    if (m_followSystem == on)
        return;
    m_followSystem = on;
    AppSettings::setFollowSystemTheme(on);
    if (on)
        applyTheme(systemTheme());  // al apagarlo se conserva el tema vigente
}

void ThemeController::refreshWarmth()
{
    if (std::abs(currentWarmth() - m_lastWarmth) < 0.02)
        return;  // sin cambio apreciable: no repintamos
    // Solo cambia el fondo; los enlaces y el resaltado no se ven afectados.
    applyPaletteWithWarmth(m_current);
}

void ThemeController::recolorLinks()
{
    const QColor color = linkColor();
    QTextDocument *doc = m_editor->document();

    // Primero recogemos los rangos de los enlaces (modificar mientras se itera
    // invalidaría los iteradores de fragmentos).
    QList<QPair<int, int>> ranges;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.isValid() && frag.charFormat().isAnchor())
                ranges.append({frag.position(), frag.length()});
        }
    }
    if (ranges.isEmpty())
        return;

    const bool wasModified = doc->isModified();
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    for (const auto &r : ranges) {
        QTextCursor fc(doc);
        fc.setPosition(r.first);
        fc.setPosition(r.first + r.second, QTextCursor::KeepAnchor);
        QTextCharFormat fmt;
        fmt.setForeground(color);  // solo cambia el color; conserva href
        fc.mergeCharFormat(fmt);
    }
    cursor.endEditBlock();
    doc->setModified(wasModified);  // recolorear no cuenta como editar
}
