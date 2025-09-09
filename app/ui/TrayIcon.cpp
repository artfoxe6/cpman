#include "TrayIcon.h"

#include <QMenu>
#include <QAction>
#include <QGuiApplication>
#include <QStyleHints>
#include "Theme.h"
#include "../core/Settings.h"

TrayIcon::TrayIcon(QObject* parent) : QObject(parent) {
    m_menu = new QMenu();
    auto actToggle = m_menu->addAction(QStringLiteral("打开/关闭"));
    auto actSettings = m_menu->addAction(QStringLiteral("设置…"));
    m_menu->addSeparator();
    auto actQuit = m_menu->addAction(QStringLiteral("退出"));

    connect(actToggle, &QAction::triggered, this, &TrayIcon::onToggle);
    connect(actSettings, &QAction::triggered, this, &TrayIcon::onSettings);
    connect(actQuit, &QAction::triggered, this, &TrayIcon::onQuit);

    m_tray.setContextMenu(m_menu);
    m_tray.setToolTip(QStringLiteral("剪贴板历史"));
    connect(&m_tray, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);

    updateIcon();
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this]{ updateIcon(); });
#endif
}

void TrayIcon::show() { m_tray.show(); }

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) emit togglePopupRequested();
}

void TrayIcon::onToggle() { emit togglePopupRequested(); }

void TrayIcon::onSettings() { emit settingsRequested(); }

void TrayIcon::onQuit() { emit quitRequested(); }

void TrayIcon::attachSettings(Settings* settings) {
    m_settings = settings;
    connect(m_settings, &Settings::changed, this, [this]{ updateIcon(); });
    updateIcon();
}

void TrayIcon::updateIcon() {
    const auto scheme = Theme::effectiveScheme(m_settings);
    const QString path = Theme::icon("tray", scheme);
    if (!path.isEmpty()) m_tray.setIcon(QIcon(path));
}

// removed: add-to-blacklist action and logic
