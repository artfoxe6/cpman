#pragma once

#include <QObject>
#include <QClipboard>
#include <QTimer>
#include "Types.h"

class Database;
class InMemoryStore;
class ImageStore;
class Settings;
namespace AppInfo { bool foregroundApp(QString* appName, int* pid); }

class ClipboardWatcher : public QObject {
    Q_OBJECT
public:
    ClipboardWatcher(Database* db, InMemoryStore* mem, ImageStore* img, Settings* settings, QObject* parent = nullptr);
    void start();
    void setPaused(bool on);

signals:
    void itemCaptured(const HistoryItem&);

private slots:
    void onClipboardChanged();

private:
    Database* m_db;
    InMemoryStore* m_mem;
    ImageStore* m_img;
    Settings* m_settings;
    QClipboard* m_clipboard = nullptr;
    QTimer m_timer; // periodic poll fallback
    bool m_paused = false;
    QString m_lastText;
    QString m_lastHash;
    bool isBlacklisted(const QString& appName) const;
};
