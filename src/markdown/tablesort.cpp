/// \file
/// \brief Implementación de la ordenación pura de filas de tabla.

#include "tablesort.h"

#include <algorithm>

namespace mdtablesort {
namespace {

double parseNumber(const QString &s, bool *ok)
{
    return s.trimmed().toDouble(ok);  // locale C: decimal con punto
}

}  // namespace

bool looksNumeric(const QStringList &keys)
{
    bool any = false;
    for (const QString &k : keys) {
        const QString t = k.trimmed();
        if (t.isEmpty())
            continue;  // las celdas vacías no descartan el modo numérico
        bool ok = false;
        parseNumber(t, &ok);
        if (!ok)
            return false;
        any = true;
    }
    return any;
}

QList<int> sortedOrder(const QStringList &keys, bool numeric, bool ascending)
{
    QList<int> order;
    order.reserve(keys.size());
    for (int i = 0; i < keys.size(); ++i)
        order.append(i);

    const auto compare = [&](int a, int b) -> int {
        if (numeric) {
            bool oka = false;
            bool okb = false;
            const double da = parseNumber(keys.at(a), &oka);
            const double db = parseNumber(keys.at(b), &okb);
            if (oka && okb)
                return da < db ? -1 : (da > db ? 1 : 0);
            if (oka != okb)
                return oka ? -1 : 1;  // los numéricos, antes que los no numéricos
        }
        return QString::localeAwareCompare(keys.at(a).toLower(), keys.at(b).toLower());
    };

    std::stable_sort(order.begin(), order.end(), [&](int a, int b) {
        const int c = compare(a, b);
        return ascending ? c < 0 : c > 0;  // empates: false → orden original (estable)
    });
    return order;
}

}  // namespace mdtablesort
