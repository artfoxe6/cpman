#pragma once

#include <QObject>
#include <QVector>
#include <QImage>
#include <QHash>
#include "Types.h"

class InMemoryStore : public QObject {
    Q_OBJECT
public:
    explicit InMemoryStore(QObject* parent = nullptr);
    void setCapacity(int n);
    int capacity() const { return m_capacity; }

    void preload(const QVector<HistoryItem>& items);
    void addItem(const HistoryItem& item);
    QVector<HistoryItem> items() const { return m_items; }

    QVector<HistoryItem> filterMemory(const QStringList& tokens, bool onlyFav) const;

    void setFavoriteById(qint64 id, bool on);
    void incrementUsageById(qint64 id);
    void retimeMoveToFront(qint64 id, qint64 createdAtMs);

signals:
    void itemsChanged();

private:
    int m_capacity = 1000;
    QVector<HistoryItem> m_items; // newest first
};
