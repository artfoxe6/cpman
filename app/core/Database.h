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
    bool vacuum();
    bool hasFts() const { return m_hasFts; }
    bool deleteOlderThan(qint64 cutoffMs, QStringList* outMediaPaths = nullptr);

signals:
    void error(const QString& message);

private:
    bool migrate();
    bool ensureFts();
    static HistoryItem fromQuery(const QSqlQuery& q);

    QSqlDatabase m_db;
    QString m_dbPath;
    bool m_hasFts = false;
};
