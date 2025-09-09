#pragma once

#include <QObject>
#include <QSettings>
#include <QKeySequence>

class Settings : public QObject {
    Q_OBJECT
public:
    explicit Settings(QObject* parent = nullptr);

    QKeySequence hotkey() const;
    void setHotkey(const QKeySequence& seq);

    bool autoPaste() const;
    void setAutoPaste(bool on);

    int pasteDelayMs() const;
    void setPasteDelayMs(int ms);

    int preloadCount() const;
    void setPreloadCount(int n);

    bool allowRepeat() const;
    void setAllowRepeat(bool on);

    bool paused() const;
    void setPaused(bool on);

    QStringList blacklist() const;
    void setBlacklist(const QStringList& list);

    QString themeMode() const; // system|light|dark
    void setThemeMode(const QString& mode);

signals:
    void changed();

private:
    QSettings m_settings;
};
