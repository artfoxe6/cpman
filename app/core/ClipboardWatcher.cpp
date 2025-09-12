#include "ClipboardWatcher.h"

#include <QGuiApplication>
#include <QMimeData>
#include <QDateTime>
#include <QCryptographicHash>
#include <QBuffer>
#include "Database.h"
#include "InMemoryStore.h"
#include "ImageStore.h"
#include "Settings.h"
#include "AppInfo.h"

ClipboardWatcher::ClipboardWatcher(Database* db, InMemoryStore* mem, ImageStore* img, Settings* settings, QObject* parent)
    : QObject(parent), m_db(db), m_mem(mem), m_img(img), m_settings(settings) {
    m_clipboard = QGuiApplication::clipboard();
}

void ClipboardWatcher::suppressNextWithText(qint64 id, const QString& text) {
    m_suppressPending = true;
    m_suppressId = id;
    m_suppressText = text;
    m_suppressHash.clear();
}

void ClipboardWatcher::suppressNextWithImage(qint64 id, const QString& hash) {
    m_suppressPending = true;
    m_suppressId = id;
    m_suppressHash = hash;
    m_suppressText.clear();
}

void ClipboardWatcher::start() {
    // Some platforms emit either dataChanged() or changed(mode). Listen to both
    connect(m_clipboard, &QClipboard::dataChanged, this, &ClipboardWatcher::onClipboardChanged);
    connect(m_clipboard, &QClipboard::changed, this, [this](QClipboard::Mode mode){
        if (mode == QClipboard::Clipboard) onClipboardChanged();
    });
    // Fallback polling for platforms/environments where signals are unreliable
    m_timer.setInterval(800);
    connect(&m_timer, &QTimer::timeout, this, &ClipboardWatcher::onClipboardChanged);
    m_timer.start();
}

void ClipboardWatcher::setPaused(bool on) { m_paused = on; }

static QString imageHash(const QImage& img) {
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    return QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
}

void ClipboardWatcher::onClipboardChanged() {
    if (m_paused) return;
    if (m_suppressPending) {
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        m_db->retimeItem(m_suppressId, now);
        m_mem->retimeMoveToFront(m_suppressId, now);
        if (!m_suppressText.isEmpty()) m_lastText = m_suppressText;
        if (!m_suppressHash.isEmpty()) m_lastHash = m_suppressHash;
        m_suppressPending = false;
        return;
    }
    const QMimeData* md = m_clipboard->mimeData();
    if (!md) return;

    HistoryItem item;
    item.createdAt = QDateTime::currentMSecsSinceEpoch();

    // Try to capture foreground app info
    QString appName; int pid = 0;
    if (AppInfo::foregroundApp(&appName, &pid)) {
        item.appName = appName;
        item.appPid = pid;
    }

    if (md->hasText()) {
        const QString raw = md->text();
        const QString text = raw.trimmed(); // trim leading/trailing whitespace only
        if (text.toUtf8().size() > 100*1024) return; // limit 100KB
        if (text.isEmpty()) return;
        // Full DB dedupe: if exists, retime instead of inserting
        HistoryItem existing;
        if (m_db->findByExactText(text, &existing)) {
            const qint64 now = item.createdAt;
            m_db->retimeItem(existing.id, now);
            if (m_mem->containsId(existing.id)) m_mem->retimeMoveToFront(existing.id, now);
            else { existing.createdAt = now; m_mem->addItem(existing); }
            m_lastText = text;
            return;
        }
        m_lastText = text;
        item.type = ItemType::Text;
        item.text = text;
        item.mime = QStringLiteral("text/plain");
    } else if (md->hasImage()) {
        const QImage img = qvariant_cast<QImage>(md->imageData());
        if (img.isNull()) return;
        const QString hash = imageHash(img);
        // Full DB dedupe: if exists by hash, retime
        HistoryItem existing;
        if (m_db->findByImageHash(hash, &existing)) {
            const qint64 now = item.createdAt;
            m_db->retimeItem(existing.id, now);
            if (m_mem->containsId(existing.id)) m_mem->retimeMoveToFront(existing.id, now);
            else { existing.createdAt = now; m_mem->addItem(existing); }
            m_lastHash = hash;
            return;
        }
        QString thumb;
        int w=0,h=0;
        const QString path = m_img->saveImage(img, &thumb, &w, &h);
        m_lastHash = hash;
        item.type = ItemType::Image;
        item.mediaPath = path;
        item.mime = QStringLiteral("image/png");
        item.width = w;
        item.height = h;
        item.hash = hash;
    } else {
        // ignore other formats
        return;
    }

    qint64 id = 0;
    if (m_db->insertItem(item, &id)) {
        item.id = id;
        m_mem->addItem(item);
        emit itemCaptured(item);
    }
}
