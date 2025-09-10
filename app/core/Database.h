#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>
#include "Types.h"

class Database : public QObject {
    Q_OBJECT
public:
    explicit Database(QObject* parent = nullptr);
    bool open();
    QString dbPath() const { return m_dbPath; }

    bool insertItem(const HistoryItem& item, qint64* outId = nullptr);
    QVector<HistoryItem> fetchRecent(int limit);
    QVector<HistoryItem> searchMemoryLike(const QStringList& tokens, bool onlyFav, int limit);
    QVector<HistoryItem> searchFts(const QStringList& tokens, bool onlyFav, int limit);
    bool toggleFavorite(qint64 id, bool on);
    bool incrementUsage(qint64 id);
    bool retimeItem(qint64 id, qint64 createdAtMs);
    bool vacuum();
    bool hasFts() const { return m_hasFts; }
    // Delete items older than cutoff. If usageSkipGreaterThan >= 0, keep rows with usage_count > that value.
    // Optionally returns number of rows deleted via outDeletedCount.
    bool deleteOlderThan(qint64 cutoffMs, QStringList* outMediaPaths = nullptr, int usageSkipGreaterThan = -1, int* outDeletedCount = nullptr);

    bool hasUsageCount() const { return m_hasUsageCount; }

    // Full-database deduplication helpers
    bool findByExactText(const QString& text, HistoryItem* outItem);
    bool findByImageHash(const QString& hash, HistoryItem* outItem);

signals:
    void error(const QString& message);

private:
    bool migrate();
    bool ensureFts();
    static HistoryItem fromQuery(const QSqlQuery& q);

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_hasFts = false;
    bool m_hasUsageCount = false;
};
