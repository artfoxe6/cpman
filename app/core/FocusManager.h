#pragma once

#include <QObject>
#include <QtGlobal>

class FocusManager : public QObject {
    Q_OBJECT
public:
    explicit FocusManager(QObject* parent = nullptr);

    // Capture the current foreground app/window so we can return to it.
    void rememberForeground();
    // Try to restore focus to the previously remembered app/window.
    bool restoreForeground();

private:
#if defined(CP_PLATFORM_WINDOWS)
    void* m_hwnd = nullptr; // HWND
#elif defined(CP_PLATFORM_MAC)
    qint64 m_pid = 0;       // pid_t
#elif defined(CP_PLATFORM_LINUX)
    unsigned long m_win = 0; // X11 Window id
#else
    int m_dummy = 0;
#endif
};
