#pragma once

#include <QObject>
#include <QSettings>
#include <QKeySequence>
#include <QSize>

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

    QStringList blacklist() const;
    void setBlacklist(const QStringList& list);

    QString themeMode() const; // system|light|dark
    void setThemeMode(const QString& mode);

    // Popup default size (when first shown)
    QSize popupSize() const;
    void setPopupSize(const QSize& sz);

signals:
    void changed();

private:
    QSettings m_settings;
};
