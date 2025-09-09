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
    // Suppress creating a new record for the next clipboard change
    // and instead retime the given item to now.
    void suppressNextWithText(qint64 id, const QString& text);
    void suppressNextWithImage(qint64 id, const QString& hash);

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
    // Internal suppression state when committing a selection
    bool m_suppressPending = false;
    qint64 m_suppressId = 0;
    QString m_suppressText;
    QString m_suppressHash;
    bool isBlacklisted(const QString& appName) const;
};
