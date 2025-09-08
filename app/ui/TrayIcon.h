#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class TrayIcon : public QObject {
    Q_OBJECT
public:
    explicit TrayIcon(QObject* parent = nullptr);
    void attachSettings(class Settings* settings);
    void show();

signals:
    void togglePopupRequested();
    void settingsRequested();
    void pauseToggled(bool paused);
    void quitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onToggle();
    void onSettings();
    void onPause();
    void onQuit();

private:
    QSystemTrayIcon m_tray;
    class QMenu* m_menu = nullptr;
    class QAction* m_pauseAction = nullptr;
    bool m_paused = false;
    class Settings* m_settings = nullptr;
    void updateIcon();
public:
    void setPaused(bool paused);
};
