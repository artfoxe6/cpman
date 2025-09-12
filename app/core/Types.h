#pragma once

#include <QString>
#include <QImage>
#include <QMetaType>

enum class ItemType {
    Text,
    Image
};

struct HistoryItem {
    qint64 id = 0;
    ItemType type = ItemType::Text;
    QString text;             // for Text
    QString mediaPath;        // for Image
    QString mime;
    int width = 0;
    int height = 0;
    QString hash;             // sha256
    bool favorite = false;
    int usageCount = 0;       // times used (committed)
    qint64 createdAt = 0;     // epoch ms
    QString appName;          // optional
    int appPid = 0;           // optional
};

inline bool operator==(const HistoryItem& a, const HistoryItem& b) {
    return a.id == b.id && a.type == b.type && a.text == b.text &&
           a.mediaPath == b.mediaPath && a.mime == b.mime &&
           a.width == b.width && a.height == b.height && a.hash == b.hash &&
           a.favorite == b.favorite && a.usageCount == b.usageCount &&
           a.createdAt == b.createdAt && a.appName == b.appName &&
           a.appPid == b.appPid;
}

Q_DECLARE_METATYPE(HistoryItem)
