#include "InMemoryStore.h"

#include <algorithm>

InMemoryStore::InMemoryStore(QObject* parent) : QObject(parent) {}

void InMemoryStore::setCapacity(int n) { m_capacity = n; }

void InMemoryStore::preload(const QVector<HistoryItem>& items) {
    m_items = items;
    if (m_items.size() > m_capacity) m_items.resize(m_capacity);
    emit itemsChanged();
}

void InMemoryStore::addItem(const HistoryItem& item) {
    if (!m_items.isEmpty()) {
        const auto& last = m_items.front();
        if (last.type == item.type) {
            if (item.type == ItemType::Text && last.text == item.text) return; // dedupe consecutive
            if (item.type == ItemType::Image && last.hash == item.hash) return;
        }
    }
    m_items.prepend(item);
    if (m_items.size() > m_capacity) m_items.resize(m_capacity);
    emit itemsChanged();
}

QVector<HistoryItem> InMemoryStore::filterMemory(const QStringList& tokens, bool onlyFav) const {
    QVector<HistoryItem> out;
    for (const auto& it : m_items) {
        if (onlyFav && !it.favorite) continue;
        bool ok = true;
        const QString hay = (it.type == ItemType::Text ? it.text : it.mime).toLower();
        for (const auto& t : tokens) {
            if (!hay.contains(t.toLower())) { ok = false; break; }
        }
        if (ok) out.push_back(it);
    }
    return out;
}

void InMemoryStore::setFavoriteById(qint64 id, bool on) {
    for (auto& it : m_items) {
        if (it.id == id) {
            if (it.favorite != on) {
                it.favorite = on;
                emit itemUpdated(it);
            }
            break;
        }
    }
}

void InMemoryStore::incrementUsageById(qint64 id) {
    for (auto& it : m_items) {
        if (it.id == id) {
            it.usageCount += 1;
            emit itemUpdated(it);
            break;
        }
    }
}

void InMemoryStore::retimeMoveToFront(qint64 id, qint64 createdAtMs) {
    int idx = -1;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items[i].id == id) { idx = i; break; }
    }
    if (idx < 0) return;
    HistoryItem it = m_items[idx];
    it.createdAt = createdAtMs;
    m_items.removeAt(idx);
    m_items.prepend(it);
    if (m_items.size() > m_capacity) m_items.resize(m_capacity);
    emit itemsChanged();
}

bool InMemoryStore::containsId(qint64 id) const {
    for (const auto& it : m_items) {
        if (it.id == id) return true;
    }
    return false;
}
