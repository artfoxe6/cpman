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
    void quitRequested();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onToggle();
    void onSettings();
    void onQuit();

private:
    QSystemTrayIcon m_tray;
    class QMenu* m_menu = nullptr;
    class Settings* m_settings = nullptr;
    void updateIcon();
};
