#include "shortcodes.h"

#include <QHash>

QString mdshortcode::expand(const QString &name)
{
    static const QHash<QString, QChar> map = {
        // Griego (minúsculas)
        {QStringLiteral("alpha"), QChar(0x03B1)}, {QStringLiteral("beta"), QChar(0x03B2)},
        {QStringLiteral("gamma"), QChar(0x03B3)}, {QStringLiteral("delta"), QChar(0x03B4)},
        {QStringLiteral("epsilon"), QChar(0x03B5)}, {QStringLiteral("zeta"), QChar(0x03B6)},
        {QStringLiteral("eta"), QChar(0x03B7)}, {QStringLiteral("theta"), QChar(0x03B8)},
        {QStringLiteral("iota"), QChar(0x03B9)}, {QStringLiteral("kappa"), QChar(0x03BA)},
        {QStringLiteral("lambda"), QChar(0x03BB)}, {QStringLiteral("mu"), QChar(0x03BC)},
        {QStringLiteral("nu"), QChar(0x03BD)}, {QStringLiteral("xi"), QChar(0x03BE)},
        {QStringLiteral("omicron"), QChar(0x03BF)}, {QStringLiteral("pi"), QChar(0x03C0)},
        {QStringLiteral("rho"), QChar(0x03C1)}, {QStringLiteral("sigma"), QChar(0x03C3)},
        {QStringLiteral("tau"), QChar(0x03C4)}, {QStringLiteral("upsilon"), QChar(0x03C5)},
        {QStringLiteral("phi"), QChar(0x03C6)}, {QStringLiteral("chi"), QChar(0x03C7)},
        {QStringLiteral("psi"), QChar(0x03C8)}, {QStringLiteral("omega"), QChar(0x03C9)},
        // Griego (mayúsculas más distintivas)
        {QStringLiteral("Gamma"), QChar(0x0393)}, {QStringLiteral("Delta"), QChar(0x0394)},
        {QStringLiteral("Theta"), QChar(0x0398)}, {QStringLiteral("Lambda"), QChar(0x039B)},
        {QStringLiteral("Xi"), QChar(0x039E)}, {QStringLiteral("Pi"), QChar(0x03A0)},
        {QStringLiteral("Sigma"), QChar(0x03A3)}, {QStringLiteral("Upsilon"), QChar(0x03A5)},
        {QStringLiteral("Phi"), QChar(0x03A6)}, {QStringLiteral("Psi"), QChar(0x03A8)},
        {QStringLiteral("Omega"), QChar(0x03A9)},
        // Marcas y operadores
        {QStringLiteral("check"), QChar(0x2713)}, {QStringLiteral("tick"), QChar(0x2713)},
        {QStringLiteral("cross"), QChar(0x2717)}, {QStringLiteral("star"), QChar(0x2605)},
        {QStringLiteral("heart"), QChar(0x2665)}, {QStringLiteral("deg"), QChar(0x00B0)},
        {QStringLiteral("pm"), QChar(0x00B1)}, {QStringLiteral("mp"), QChar(0x2213)},
        {QStringLiteral("times"), QChar(0x00D7)}, {QStringLiteral("div"), QChar(0x00F7)},
        {QStringLiteral("ne"), QChar(0x2260)}, {QStringLiteral("le"), QChar(0x2264)},
        {QStringLiteral("ge"), QChar(0x2265)}, {QStringLiteral("approx"), QChar(0x2248)},
        {QStringLiteral("inf"), QChar(0x221E)}, {QStringLiteral("infty"), QChar(0x221E)},
        {QStringLiteral("sum"), QChar(0x2211)}, {QStringLiteral("prod"), QChar(0x220F)},
        {QStringLiteral("int"), QChar(0x222B)}, {QStringLiteral("sqrt"), QChar(0x221A)},
        {QStringLiteral("partial"), QChar(0x2202)}, {QStringLiteral("nabla"), QChar(0x2207)},
        // Moneda y símbolos
        {QStringLiteral("euro"), QChar(0x20AC)}, {QStringLiteral("pound"), QChar(0x00A3)},
        {QStringLiteral("yen"), QChar(0x00A5)}, {QStringLiteral("cent"), QChar(0x00A2)},
        {QStringLiteral("copy"), QChar(0x00A9)}, {QStringLiteral("reg"), QChar(0x00AE)},
        {QStringLiteral("tm"), QChar(0x2122)}, {QStringLiteral("sect"), QChar(0x00A7)},
        {QStringLiteral("para"), QChar(0x00B6)}, {QStringLiteral("dagger"), QChar(0x2020)},
        {QStringLiteral("bullet"), QChar(0x2022)}, {QStringLiteral("middot"), QChar(0x00B7)},
        {QStringLiteral("hellip"), QChar(0x2026)}, {QStringLiteral("mdash"), QChar(0x2014)},
        {QStringLiteral("ndash"), QChar(0x2013)},
        // Flechas
        {QStringLiteral("rightarrow"), QChar(0x2192)}, {QStringLiteral("to"), QChar(0x2192)},
        {QStringLiteral("leftarrow"), QChar(0x2190)}, {QStringLiteral("uparrow"), QChar(0x2191)},
        {QStringLiteral("downarrow"), QChar(0x2193)},
        {QStringLiteral("leftrightarrow"), QChar(0x2194)},
        {QStringLiteral("Rightarrow"), QChar(0x21D2)},
        {QStringLiteral("Leftrightarrow"), QChar(0x21D4)},
    };
    const auto it = map.constFind(name);
    return it != map.cend() ? QString(*it) : QString();
}
