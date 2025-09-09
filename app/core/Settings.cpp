#include "Settings.h"

static constexpr const char* KEY_HOTKEY = "hotkey/sequence";
static constexpr const char* KEY_AUTOPASTE = "paste/auto";
static constexpr const char* KEY_PASTE_DELAY = "paste/delayMs";
static constexpr const char* KEY_PRELOAD = "preload/count";
static constexpr const char* KEY_ALLOW_REPEAT = "capture/allowRepeat";
static constexpr const char* KEY_PAUSED = "capture/paused";
static constexpr const char* KEY_BLACKLIST = "privacy/blacklist";
static constexpr const char* KEY_THEME = "theme/mode";

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

bool Settings::paused() const {
    return m_settings.value(KEY_PAUSED, false).toBool();
}

void Settings::setPaused(bool on) {
    m_settings.setValue(KEY_PAUSED, on);
    emit changed();
}

QStringList Settings::blacklist() const {
    return m_settings.value(KEY_BLACKLIST, QStringList{}).toStringList();
}

void Settings::setBlacklist(const QStringList& list) {
    m_settings.setValue(KEY_BLACKLIST, list);
    emit changed();
}

QString Settings::themeMode() const {
    return m_settings.value(KEY_THEME, QStringLiteral("system")).toString();
}

void Settings::setThemeMode(const QString& mode) {
    m_settings.setValue(KEY_THEME, mode);
    emit changed();
}
