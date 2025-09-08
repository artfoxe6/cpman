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
    qint64 createdAt = 0;     // epoch ms
    QString appName;          // optional
    int appPid = 0;           // optional
};

Q_DECLARE_METATYPE(HistoryItem)

