#include "HotkeyManager.h"

#ifdef HAVE_QHOTKEY
#include <QHotkey>
#endif

HotkeyManager::HotkeyManager(QObject* parent) : QObject(parent) {}

bool HotkeyManager::registerHotkey(const QKeySequence& seq) {
#ifdef HAVE_QHOTKEY
    if (!m_hotkey) m_hotkey = new QHotkey(this);
    if (m_hotkey->isRegistered()) m_hotkey->setRegistered(false);
    m_hotkey->setShortcut(seq, true);
    QObject::connect(m_hotkey, &QHotkey::activated, this, &HotkeyManager::triggered, Qt::UniqueConnection);
    return m_hotkey->isRegistered();
#else
    Q_UNUSED(seq);
    return false;
#endif
}

void HotkeyManager::unregister() {
#ifdef HAVE_QHOTKEY
    if (m_hotkey) m_hotkey->setRegistered(false);
#endif
}
