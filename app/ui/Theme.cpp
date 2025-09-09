#include "Theme.h"
#include "../core/Settings.h"

#include <QGuiApplication>
#include <QStyleHints>

namespace Theme {

Qt::ColorScheme effectiveScheme(const Settings* /*settings*/) {
    // Always follow system color scheme
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    return QGuiApplication::styleHints()->colorScheme();
#else
    return Qt::ColorScheme::Light;
#endif
}

QString icon(const QString& baseName, Qt::ColorScheme scheme) {
    const bool dark = (scheme == Qt::ColorScheme::Dark);
    if (baseName == QLatin1String("tray"))
        return dark ? QStringLiteral(":/icons/dark/tray_dark.svg") : QStringLiteral(":/icons/light/tray_light.svg");
    if (baseName == QLatin1String("heart_empty"))
        return dark ? QStringLiteral(":/icons/dark/heart_empty_dark.svg") : QStringLiteral(":/icons/light/heart_empty_light.svg");
    if (baseName == QLatin1String("heart_filled"))
        return dark ? QStringLiteral(":/icons/dark/heart_filled_dark.svg") : QStringLiteral(":/icons/light/heart_filled_light.svg");
    if (baseName == QLatin1String("search"))
        return dark ? QStringLiteral(":/icons/dark/search_dark.svg") : QStringLiteral(":/icons/light/search_light.svg");
    if (baseName == QLatin1String("db"))
        return dark ? QStringLiteral(":/icons/dark/db_dark.svg") : QStringLiteral(":/icons/light/db_light.svg");
    return QString();
}

}
