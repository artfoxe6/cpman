#pragma once

#include <QObject>
#include <QKeySequence>

class HotkeyManager : public QObject {
    Q_OBJECT
public:
    explicit HotkeyManager(QObject* parent = nullptr);
    bool registerHotkey(const QKeySequence& seq);
    void unregister();

signals:
    void triggered();

private:
#ifdef HAVE_QHOTKEY
    class QHotkey* m_hotkey = nullptr;
#endif
};
