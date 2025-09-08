#pragma once

#include <QString>

namespace AppInfo {

// Returns true if foreground app info was obtained.
bool foregroundApp(QString* appName, int* pid);

}

