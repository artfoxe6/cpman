#pragma once

#include <Qt>
#include <QString>

class Settings;

namespace Theme {

Qt::ColorScheme effectiveScheme(const Settings* settings);
QString icon(const QString& baseName, Qt::ColorScheme scheme);

}

