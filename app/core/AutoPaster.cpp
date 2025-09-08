#include "AutoPaster.h"

#include <QTimer>

#if defined(CP_PLATFORM_WINDOWS)
void platform_paste_windows();
#elif defined(CP_PLATFORM_MAC)
void platform_paste_mac();
#elif defined(CP_PLATFORM_LINUX)
void platform_paste_x11_or_wayland();
#endif

AutoPaster::AutoPaster(QObject* parent) : QObject(parent) {}

void AutoPaster::schedulePaste(int delayMs) {
    QTimer::singleShot(delayMs, this, &AutoPaster::doPaste);
}

void AutoPaster::doPaste() {
#if defined(CP_PLATFORM_WINDOWS)
    platform_paste_windows();
#elif defined(CP_PLATFORM_MAC)
    platform_paste_mac();
#elif defined(CP_PLATFORM_LINUX)
    platform_paste_x11_or_wayland();
#endif
}

