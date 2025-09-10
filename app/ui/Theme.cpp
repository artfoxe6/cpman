#include "Theme.h"
#include "../core/Settings.h"

#include <QGuiApplication>
#include <QStyleHints>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QPalette>
#include <QColor>

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

void refreshStyleAfterThemeChange() {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    static bool inFlight = false;
    if (inFlight) return;
    inFlight = true;
    // Defer to ensure Qt updates its internal palette first
    QTimer::singleShot(0, []{
        // Reset to platform default palette for current style
        QGuiApplication::setPalette(QPalette());
        inFlight = false;
    });
#endif
}

QColor popupWindowColor(Qt::ColorScheme scheme) {
    // Light: 0xDFDFDF, Dark: 0x070707
    if (scheme == Qt::ColorScheme::Dark) return QColor(0x07, 0x07, 0x07);
    return QColor(0xDF, 0xDF, 0xDF);
}

QColor listSelectionColor(Qt::ColorScheme scheme) {
    // Light: 0x521E72, Dark: 0x4A7F87
    if (scheme == Qt::ColorScheme::Dark) return QColor(0x4A, 0x7F, 0x87);
    return QColor(0x52, 0x1E, 0x72);
}

}
