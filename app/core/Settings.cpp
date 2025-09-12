#include "Settings.h"

static constexpr const char* KEY_HOTKEY = "hotkey/sequence";
static constexpr const char* KEY_AUTOPASTE = "paste/auto";
static constexpr const char* KEY_PASTE_DELAY = "paste/delayMs";
static constexpr const char* KEY_PRELOAD = "preload/count";
static constexpr const char* KEY_ALLOW_REPEAT = "capture/allowRepeat";
static constexpr const char* KEY_THEME = "theme/mode";
static constexpr const char* KEY_POPUP_SIZE = "window/popupSize";

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_settings("cpman", "cpman")
{
}

QKeySequence Settings::hotkey() const {
    const QString def = QStringLiteral("Ctrl+Shift+M");
    return QKeySequence(m_settings.value(KEY_HOTKEY, def).toString());
}

void Settings::setHotkey(const QKeySequence& seq) {
    m_settings.setValue(KEY_HOTKEY, seq.toString());
    emit changed();
}

bool Settings::autoPaste() const {
    // Default: true, but Wayland default false per spec
    QVariant v = m_settings.value(KEY_AUTOPASTE);
    if (!v.isValid()) {
        const QByteArray sess = qgetenv("XDG_SESSION_TYPE");
        const bool isWayland = QString::fromLatin1(sess).trimmed().compare("wayland", Qt::CaseInsensitive) == 0;
        return !isWayland; // default false on Wayland
    }
    return v.toBool();
}

void Settings::setAutoPaste(bool on) {
    m_settings.setValue(KEY_AUTOPASTE, on);
    emit changed();
}

int Settings::pasteDelayMs() const {
    int def = 1000;
    int v = m_settings.value(KEY_PASTE_DELAY, def).toInt();
    if (v < 0) v = 0;
    if (v > 5000) v = 5000; // clamp to a sane range
    return v;
}

void Settings::setPasteDelayMs(int ms) {
    if (ms < 0) ms = 0;
    if (ms > 5000) ms = 5000;
    m_settings.setValue(KEY_PASTE_DELAY, ms);
    emit changed();
}

int Settings::preloadCount() const {
    return m_settings.value(KEY_PRELOAD, 1000).toInt();
}

void Settings::setPreloadCount(int n) {
    m_settings.setValue(KEY_PRELOAD, n);
    emit changed();
}

bool Settings::allowRepeat() const {
    return m_settings.value(KEY_ALLOW_REPEAT, false).toBool();
}

void Settings::setAllowRepeat(bool on) {
    m_settings.setValue(KEY_ALLOW_REPEAT, on);
    emit changed();
}

QString Settings::themeMode() const {
    // Theme switching disabled: always follow system
    return QStringLiteral("system");
}

void Settings::setThemeMode(const QString& /*mode*/) {
    // No-op: follow system theme only
    emit changed();
}

QSize Settings::popupSize() const {
    // Returns invalid size if never set
    QVariant v = m_settings.value(KEY_POPUP_SIZE);
    if (!v.isValid()) return QSize();
    QSize sz = v.toSize();
    // Clamp to a sane minimum to avoid 0-sized window
    const int minW = 400, minH = 300;
    if (sz.width() < minW || sz.height() < minH) return QSize();
    return sz;
}

void Settings::setPopupSize(const QSize& sz) {
    // Store only if reasonable
    const int minW = 400, minH = 300;
    if (sz.width() < minW || sz.height() < minH) return;
    m_settings.setValue(KEY_POPUP_SIZE, sz);
    emit changed();
}
