#pragma once

#include <Qt>
#include <QString>
#include <QColor>

class Settings;

namespace Theme {

Qt::ColorScheme effectiveScheme(const Settings* settings);
QString icon(const QString& baseName, Qt::ColorScheme scheme);

// Ensure widgets pick up the new system palette/style after a theme change.
void refreshStyleAfterThemeChange();

// App-specific colors
QColor popupWindowColor(Qt::ColorScheme scheme);
QColor listSelectionColor(Qt::ColorScheme scheme);

}
